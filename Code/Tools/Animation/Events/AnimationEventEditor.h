#pragma once

#include "AnimationEventData.h"
#include "Tools/Core/TimelineEditor/TimelineEditor.h"
#include "Engine/Animation/AnimationEvent.h"
#include "Engine/Animation/AnimationClip.h"
#include "System/TypeSystem/TypeRegistry.h"

//-------------------------------------------------------------------------

namespace KRG { class UpdateContext; }
namespace KRG::Animation { class AnimationPlayerComponent; }

//-------------------------------------------------------------------------

namespace KRG::Animation
{
    class KRG_TOOLS_ANIMATION_API EventEditor final : public TimelineEditor
    {
    public:

        EventEditor( TypeSystem::TypeRegistry const& typeRegistry, FileSystem::Path const& sourceDataDirectory, AnimationClip const* pAnimation );

        inline ResourceID const& GetAnimationID() const { return m_animationID; }

        void Update( UpdateContext const& context, AnimationPlayerComponent* pPreviewAnimationComponent );

    private:

        virtual bool DrawAddTracksMenu() override;
        virtual void RequestSave() override;

        bool LoadFromFile();

    private:

        TypeSystem::TypeRegistry const&             m_typeRegistry;
        FileSystem::Path const                      m_sourceDataDirectory;
        TVector<TypeSystem::TypeInfo const*>        m_eventTypes;
        ResourceID                                  m_animationID; // This is tracked independently since the animation ptr can be invalidated externally
        AnimationClip const*                        m_pAnimation = nullptr;
        FileSystem::Path                            m_eventDataFilePath;
    };
}