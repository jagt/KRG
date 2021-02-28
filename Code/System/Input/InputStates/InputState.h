#pragma once

#include "System\Core\Time\Timers.h"
#include "System\Core\Types\Containers.h"

//-------------------------------------------------------------------------

namespace KRG
{
    namespace Input
    {
        class Button
        {
            template<U32> friend class ButtonStates;

            enum class State
            {
                None = 0,
                Pressed,
                Held,
                Released,
            };

        public:

            inline bool WasPressed() const { return m_state == State::Pressed; }
            inline bool WasReleased() const { return m_state == State::Released; }
            inline bool IsHeldDown() const { return m_state == State::Held; }

            inline Milliseconds GetTimeHeld() const 
            {
                KRG_ASSERT( IsHeldDown() || WasPressed() );
                return m_holdTimer.GetElapsedTimeMilliseconds(); 
            }

        private:

            inline void UpdateState( State inState )
            {
                m_state = inState;

                // Track state changes
                if ( inState == State::Pressed )
                {
                    m_stateChangedThisFrame = true;
                    m_holdTimer.Start();
                }
                else if ( inState == State::Released )
                {
                    m_stateChangedThisFrame = true;
                }
            }

        private:

            EngineTimer      m_holdTimer;
            State            m_state = State::None;
            bool             m_stateChangedThisFrame = false;
            bool             m_needsUpdate = false;
        };

        //-------------------------------------------------------------------------

        template<U32 NumButtons>
        class ButtonStates
        {
            friend class InputDevice;

        public:

            inline ButtonStates()
            {
                m_buttons.resize( NumButtons );
            }

            inline bool WasPressed( U32 buttonIdx ) const
            {
                KRG_ASSERT( buttonIdx < NumButtons );
                return ( m_buttons[buttonIdx].WasPressed() );
            }

            inline bool WasReleased( U32 buttonIdx ) const
            {
                KRG_ASSERT( buttonIdx < NumButtons );
                return ( m_buttons[buttonIdx].WasReleased() );
            }

            inline bool IsHeldDown( U32 buttonIdx ) const
            {
                return WasPressed( buttonIdx ) || m_buttons[buttonIdx].IsHeldDown();
            }

            inline Milliseconds GetHeldDuration( U32 buttonIdx ) const
            {
                KRG_ASSERT( buttonIdx < NumButtons );
                return WasPressed( buttonIdx ) ? m_buttons[buttonIdx].GetTimeHeld() : 0.0f;
            }

        protected:

            inline void ClearButtonState()
            {
                KRG::Memory::MemsetZero( m_buttons.data(), m_buttons.size() * sizeof( Button ) );
            }

            inline void Update()
            {
                for ( auto i = 0; i < NumButtons; i++ )
                {
                    // If the state changed this frame, register the button for an update on the next frame
                    if ( m_buttons[i].m_stateChangedThisFrame )
                    {
                        m_buttons[i].m_stateChangedThisFrame = false;
                        m_buttons[i].m_needsUpdate = true;
                    }
                    else if ( m_buttons[i].m_needsUpdate )
                    {
                        // Transform the pressed and released events into their subsequent states
                        if ( m_buttons[i].m_state == Button::State::Pressed )
                        {
                            m_buttons[i].UpdateState( Button::State::Held );
                        }
                        else if( m_buttons[i].m_state == Button::State::Released )
                        {
                            m_buttons[i].UpdateState( Button::State::None );
                        }

                        m_buttons[i].m_needsUpdate = false;
                    }
                }
            }

            // Called when we detect a pressed event for a button
            inline void Press( U32 buttonIdx )
            {
                m_buttons[buttonIdx].UpdateState( Button::State::Pressed );
            }

            // Called when we detect a released event for a button
            inline void Release( U32 buttonIdx )
            {
                m_buttons[buttonIdx].UpdateState( Button::State::Released );
            }

        private:

            TInlineVector<Button, NumButtons>   m_buttons;
        };
    }
}