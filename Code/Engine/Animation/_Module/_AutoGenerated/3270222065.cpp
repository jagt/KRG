//-------------------------------------------------------------------------
// This is an auto-generated file - DO NOT edit
//-------------------------------------------------------------------------

#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\3270222065.h"

//-------------------------------------------------------------------------
// TypeHelper: KRG::Animation::AnimatedMeshComponent
//-------------------------------------------------------------------------

namespace KRG
{
    TypeSystem::TypeInfo const* KRG::Animation::AnimatedMeshComponent::s_pTypeInfo = nullptr;
    namespace TypeSystem
    {
        namespace TypeHelpers
        {
            void const* TTypeHelper<KRG::Animation::AnimatedMeshComponent>::s_pDefaultTypeInstancePtr = nullptr;

            TTypeHelper<KRG::Animation::AnimatedMeshComponent> TTypeHelper<KRG::Animation::AnimatedMeshComponent>::StaticTypeHelper;
        }
    }

    TypeSystem::TypeInfo const* KRG::Animation::AnimatedMeshComponent::GetTypeInfo() const
    {
        return KRG::Animation::AnimatedMeshComponent::s_pTypeInfo;
    }

    void KRG::Animation::AnimatedMeshComponent::Load( EntityModel::LoadingContext const& context, UUID requesterID )
    {
        KRG::Animation::AnimatedMeshComponent::s_pTypeInfo->m_pTypeHelper->LoadResources( context.m_pResourceSystem, requesterID, this );
        m_status = Status::Loading;
    }

    void KRG::Animation::AnimatedMeshComponent::Unload( EntityModel::LoadingContext const& context, UUID requesterID )
    {
        KRG::Animation::AnimatedMeshComponent::s_pTypeInfo->m_pTypeHelper->UnloadResources( context.m_pResourceSystem, requesterID, this );
        m_status = Status::Unloaded;
    }

    void KRG::Animation::AnimatedMeshComponent::UpdateLoading()
    {
        if( m_status == Status::Loading )
        {
            auto const resourceLoadingStatus = KRG::Animation::AnimatedMeshComponent::s_pTypeInfo->m_pTypeHelper->GetResourceLoadingStatus( this );
            if ( resourceLoadingStatus == LoadingStatus::Loading )
            {
                return; // Something is still loading so early-out
            }

            if ( resourceLoadingStatus == LoadingStatus::Failed )
            {
                m_status = EntityComponent::Status::LoadingFailed;
            }
            else
            {
                m_status = EntityComponent::Status::Loaded;
            }
        }
    }
}
