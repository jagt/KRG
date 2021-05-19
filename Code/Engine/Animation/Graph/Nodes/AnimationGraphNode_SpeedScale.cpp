#include "AnimationGraphNode_SpeedScale.h"
#include "System/Core/Logging/Log.h"
#include "Engine/Animation/AnimationClip.h"

//-------------------------------------------------------------------------

namespace KRG::Animation::Graph
{
    void SpeedScaleNode::OnConstruct( GraphNode::Settings const* pSettings, TVector<GraphNode*> const& nodePtrs, AnimationGraphDataSet const& dataSet )
    {
        PassthroughNode::OnConstruct( pSettings, nodePtrs, dataSet );
        SetNodePtrFromIndex( nodePtrs, GetSettings<SpeedScaleNode>()->m_scaleValueNodeIdx, m_pScaleValueNode );
    }

    void SpeedScaleNode::InitializeInternal( GraphContext& context, SyncTrackTime const& initialTime )
    {
        KRG_ASSERT( context.IsValid() );
        KRG_ASSERT( m_pScaleValueNode != nullptr );
        PassthroughNode::InitializeInternal( context, initialTime );
        m_pScaleValueNode->Initialize( context );
        m_blendWeight = 1.0f;
    }

    void SpeedScaleNode::ShutdownInternal( GraphContext& context )
    {
        KRG_ASSERT( context.IsValid() );
        KRG_ASSERT( m_pScaleValueNode != nullptr );
        m_pScaleValueNode->Shutdown( context );
        PassthroughNode::ShutdownInternal( context );
    }

    UpdateResult SpeedScaleNode::Update( GraphContext& context )
    {
        // Record old delta time
        auto const deltaTime = context.m_deltaTime;

        // Adjust the delta time for the child node
        //-------------------------------------------------------------------------

        if ( IsChildValid() )
        {
            auto speedScale = 1.0f;
            if ( m_pScaleValueNode != nullptr )
            {
                speedScale = GetSpeedScale( context );

                if ( m_blendWeight < 1.0f )
                {
                    auto pSettings = GetSettings<SpeedScaleNode>();
                    KRG_ASSERT( pSettings->m_blendTime > 0.0f );
                    float const blendWeightDelta = context.m_deltaTime / pSettings->m_blendTime;
                    m_blendWeight = Math::Clamp( m_blendWeight + blendWeightDelta, 0.0f, 1.0f );
                    speedScale = Math::Lerp( 1.0f, speedScale, m_blendWeight );
                }

                context.m_deltaTime *= speedScale;
                m_duration = m_pChildNode->GetDuration() / speedScale;
            }
        }

        // Update the child node
        //-------------------------------------------------------------------------

        UpdateResult result = PassthroughNode::Update( context );

        // Reset the delta time
        //-------------------------------------------------------------------------

        context.m_deltaTime = deltaTime;
        return result;
    }

    UpdateResult SpeedScaleNode::Update( GraphContext& context, SyncTrackTimeRange const& updateRange )
    {
        m_blendWeight = ( Math::IsNearZero( GetSettings<SpeedScaleNode>()->m_blendTime ) ) ? 1.0f : 0.0f;
        return PassthroughNode::Update( context, updateRange );
    }

    //-------------------------------------------------------------------------

    void VelocityBasedSpeedScaleNode::OnConstruct( GraphNode::Settings const* pSettings, TVector<GraphNode*> const& nodePtrs, AnimationGraphDataSet const& dataSet )
    {
        PassthroughNode::OnConstruct( pSettings, nodePtrs, dataSet );
        SetNodePtrFromIndex( nodePtrs, GetSettings<VelocityBasedSpeedScaleNode>()->m_desiredVelocityValueNodeIdx, m_pDesiredVelocityValueNode );
    }

    void VelocityBasedSpeedScaleNode::InitializeInternal( GraphContext& context, SyncTrackTime const& initialTime )
    {
        KRG_ASSERT( context.IsValid() );
        KRG_ASSERT( m_pDesiredVelocityValueNode != nullptr );
        PassthroughNode::InitializeInternal( context, initialTime );
        m_pDesiredVelocityValueNode->Initialize( context );
        m_blendWeight = 1.0f;
    }

    void VelocityBasedSpeedScaleNode::ShutdownInternal( GraphContext& context )
    {
        KRG_ASSERT( context.IsValid() );
        KRG_ASSERT( m_pDesiredVelocityValueNode != nullptr );
        m_pDesiredVelocityValueNode->Shutdown( context );
        PassthroughNode::ShutdownInternal( context );
    }

    UpdateResult VelocityBasedSpeedScaleNode::Update( GraphContext& context )
    {
        // Record old delta time
        auto const deltaTime = context.m_deltaTime;

        // Adjust the delta time for the child node
        //-------------------------------------------------------------------------

        if ( IsChildValid() )
        {
            auto speedMultiplier = 1.0f;
            if ( m_pDesiredVelocityValueNode != nullptr )
            {
                float const desiredVelocity = m_pDesiredVelocityValueNode->GetValue<float>( context );
                if ( desiredVelocity > 0.0f )
                {
                    auto const pReferenceNode = static_cast<AnimationClipReferenceNode*>( m_pChildNode );
                    KRG_ASSERT( pReferenceNode->IsValid() );

                    float const averageVelocity = pReferenceNode->GetAnimation()->GetAverageLinearVelocity();
                    if ( !Math::IsNearZero( averageVelocity ) )
                    {
                        speedMultiplier = desiredVelocity / averageVelocity;
                    }
                }
                else
                {
                    KRG_LOG_WARNING( "Animation", "Requesting a negative velocity is not supported!" );
                }

                //-------------------------------------------------------------------------

                if ( m_blendWeight < 1.0f )
                {
                    auto pSettings = GetSettings<VelocityBasedSpeedScaleNode>();
                    KRG_ASSERT( pSettings->m_blendTime > 0.0f );
                    float const BlendWeightDelta = context.m_deltaTime / pSettings->m_blendTime;
                    m_blendWeight = Math::Clamp( m_blendWeight + BlendWeightDelta, 0.0f, 1.0f );
                    speedMultiplier = Math::Lerp( 1.0f, speedMultiplier, m_blendWeight );
                }

                context.m_deltaTime *= speedMultiplier;
                m_duration = m_pChildNode->GetDuration() / speedMultiplier;
            }
        }

        // Update the child node
        //-------------------------------------------------------------------------

        UpdateResult result = PassthroughNode::Update( context );

        // Reset the time delta
        //-------------------------------------------------------------------------

        context.m_deltaTime = deltaTime;
        return result;
    }

    UpdateResult VelocityBasedSpeedScaleNode::Update( GraphContext& context, SyncTrackTimeRange const& updateRange )
    {
        m_blendWeight = ( Math::IsNearZero( GetSettings<VelocityBasedSpeedScaleNode>()->m_blendTime ) ) ? 1.0f : 0.0f;
        return PassthroughNode::Update( context, updateRange );
    }
}