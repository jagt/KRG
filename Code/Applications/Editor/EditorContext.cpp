#include "EditorContext.h"
#include "Applications/EngineShared/RenderingSystem.h"
#include "Tools/Entity/Workspaces/Workspace_MapEditor.h"
#include "Tools/Entity/Workspaces/Workspace_GamePreviewer.h"
#include "Tools/Entity/Workspaces/Workspace_EntityCollectionEditor.h"
#include "Tools/Core/Workspaces/ResourceWorkspace.h"
#include "Tools/Core/ThirdParty/pfd/portable-file-dialogs.h"
#include "Engine/Core/Entity/EntityWorld.h"
#include "Engine/Core/Entity/EntityWorldManager.h"
#include "System/Resource/ResourceSettings.h"
#include "System/Resource/ResourceSystem.h"
#include "System/Core/Settings/SettingsRegistry.h"
#include "Engine/Core/Update/UpdateContext.h"

//-------------------------------------------------------------------------

namespace KRG
{
    EditorContext::~EditorContext()
    {
        KRG_ASSERT( m_workspaces.empty() );
    }

    void EditorContext::Initialize( UpdateContext const& context )
    {
        auto pSettingsRegistry = context.GetSystem<SettingsRegistry>();
        KRG_ASSERT( pSettingsRegistry != nullptr );
        
        auto pResourceSettings = pSettingsRegistry->GetSettings<Resource::Settings>();
        KRG_ASSERT( pResourceSettings != nullptr );

        //-------------------------------------------------------------------------

        m_workspaceInitContext.m_pTypeRegistry = context.GetSystem<TypeSystem::TypeRegistry>();
        m_workspaceInitContext.m_pResourceSystem = context.GetSystem<Resource::ResourceSystem>();
        m_pWorldManager = context.GetSystem<EntityWorldManager>();
        m_pRenderingSystem = context.GetSystem<Render::RenderingSystem>();

        m_resourceDB.Initialize( m_workspaceInitContext.m_pTypeRegistry, pResourceSettings->m_rawResourcePath, pResourceSettings->m_compiledResourcePath );
        m_workspaceInitContext.m_pResourceDatabase = &m_resourceDB;

        // Create map editor workspace
        //-------------------------------------------------------------------------

        // Destroy the default created game world
        m_pWorldManager->DestroyWorld( m_pWorldManager->GetWorlds()[0] );

        // Create the map editor world
        auto pMapEditorWorld = m_pWorldManager->CreateWorld( EntityWorldType::Editor );
        m_pRenderingSystem->CreateCustomRenderTargetForViewport( pMapEditorWorld->GetViewport(), true );
        m_pMapEditor = KRG::New<EntityModel::EntityMapEditor>( m_workspaceInitContext, pMapEditorWorld );
        m_pMapEditor->Initialize( context );
        m_workspaces.emplace_back( m_pMapEditor );
    }

    void EditorContext::Shutdown( UpdateContext const& context )
    {
        while( !m_workspaces.empty() )
        {
            DestroyWorkspaceInternal( context, m_workspaces[0] );
        }

        m_workspaces.clear();

        //-------------------------------------------------------------------------

        m_pWorldManager = nullptr;
        m_pRenderingSystem = nullptr;
        m_workspaceInitContext.m_pResourceSystem = nullptr;
        m_workspaceInitContext.m_pTypeRegistry = nullptr;
        m_workspaceInitContext.m_pResourceDatabase = nullptr;

        m_resourceDB.Shutdown();
    }

    void EditorContext::Update( UpdateContext const& context )
    {
        // Update the resource database
        m_resourceDB.Update();

        // Destroy all required workspaces
        // We needed to defer this to the start of the update since we may have references resources that we might unload (i.e. textures)
        for ( auto pWorkspaceToDestroy : m_workspaceDestructionRequests )
        {
            DestroyWorkspace( context, pWorkspaceToDestroy );
        }
        m_workspaceDestructionRequests.clear();

        // Create all workspaces
        for ( auto const& resourceID : m_workspaceCreationRequests )
        {
            TryCreateWorkspace( context, resourceID );
        }
        m_workspaceCreationRequests.clear();
    }

    //-------------------------------------------------------------------------

    bool EditorContext::TryCreateWorkspace( UpdateContext const& context, ResourceID const& resourceID )
    {
        ResourceTypeID const resourceTypeID = resourceID.GetResourceTypeID();

        // Handle maps/ECs explicitly
        //-------------------------------------------------------------------------

        if ( resourceTypeID == EntityModel::EntityMapDescriptor::GetStaticResourceTypeID() )
        {
            m_pMapEditor->LoadMap( resourceID );
            ImGuiX::MakeTabVisible( m_pMapEditor->GetWorkspaceWindowID() );
            return true;
        }
        else if ( resourceTypeID == EntityModel::EntityCollectionDescriptor::GetStaticResourceTypeID() )
        {
            // Create preview world
            auto pPreviewWorld = m_pWorldManager->CreateWorld( EntityWorldType::Editor );
            pPreviewWorld->LoadMap( ResourcePath( "data://Editor/EditorMap.map" ) );
            m_pRenderingSystem->CreateCustomRenderTargetForViewport( pPreviewWorld->GetViewport() );

            // Create EC workspace
            auto pWorkspace = KRG::New<EntityModel::EntityCollectionEditor>( m_workspaceInitContext, pPreviewWorld, resourceID );
            pWorkspace->Initialize( context );
            m_workspaces.emplace_back( pWorkspace );

            return true;
        }

        // Other resource types
        //-------------------------------------------------------------------------

        auto pExistingWorkspace = FindResourceWorkspace( resourceID );
        if ( pExistingWorkspace == nullptr )
        {
            // Create preview world
            auto pPreviewWorld = m_pWorldManager->CreateWorld( EntityWorldType::Editor );
            pPreviewWorld->LoadMap( ResourcePath( "data://Editor/EditorMap.map" ) );
            m_pRenderingSystem->CreateCustomRenderTargetForViewport( pPreviewWorld->GetViewport() );

            // Create workspace
            auto pCreatedWorkspace = ResourceWorkspaceFactory::TryCreateWorkspace( m_workspaceInitContext, pPreviewWorld, resourceID );
            KRG_ASSERT( pCreatedWorkspace != nullptr );
            pCreatedWorkspace->Initialize( context );
            m_workspaces.emplace_back( pCreatedWorkspace );
        }
        else
        {
            ImGuiX::MakeTabVisible( pExistingWorkspace->GetWorkspaceWindowID() );
        }

        return true;
    }

    void EditorContext::DestroyWorkspace( UpdateContext const& context, EditorWorkspace* pWorkspace )
    {
        KRG_ASSERT( m_pMapEditor != pWorkspace );
        DestroyWorkspaceInternal( context, pWorkspace );
    }

    void EditorContext::QueueDestroyWorkspace( EditorWorkspace* pWorkspace )
    {
        KRG_ASSERT( m_pMapEditor != pWorkspace );
        m_workspaceDestructionRequests.emplace_back( pWorkspace );
    }

    void EditorContext::DestroyWorkspaceInternal( UpdateContext const& context, EditorWorkspace* pWorkspace )
    {
        KRG_ASSERT( pWorkspace != nullptr );

        auto foundWorkspaceIter = eastl::find( m_workspaces.begin(), m_workspaces.end(), pWorkspace );
        KRG_ASSERT( foundWorkspaceIter != m_workspaces.end() );

        if ( pWorkspace->IsDirty() )
        {
            auto messageDialog = pfd::message( "Unsaved Changes", "You have unsaved changes!\nDo you wish to save these changes before closing?", pfd::choice::yes_no_cancel );
            switch ( messageDialog.result() )
            {
                case pfd::button::yes:
                {
                    if ( !pWorkspace->Save() )
                    {
                        return;
                    }
                }
                break;

                case pfd::button::cancel:
                {
                    return;
                }
                break;
            }
        }

        //-------------------------------------------------------------------------

        bool const isGamePreviewerWorkspace = m_pGamePreviewer == pWorkspace;

        // Destroy the custom viewport render target
        auto pPreviewWorld = pWorkspace->GetWorld();
        m_pRenderingSystem->DestroyCustomRenderTargetForViewport( pPreviewWorld->GetViewport() );

        // Destroy workspace
        pWorkspace->Shutdown( context );
        KRG::Delete( pWorkspace );
        m_workspaces.erase( foundWorkspaceIter );

        // Clear the game previewer workspace ptr if we just destroyed it
        if ( isGamePreviewerWorkspace )
        {
            m_pMapEditor->OnGamePreviewEnded();
            m_pGamePreviewer = nullptr;
        }

        // Destroy preview world
        m_pWorldManager->DestroyWorld( pPreviewWorld );
    }

    bool EditorContext::HasDescriptorForResourceType( ResourceTypeID resourceTypeID ) const
    {
        if ( resourceTypeID == EntityModel::EntityMapDescriptor::GetStaticResourceTypeID() )
        {
            return false;
        }

        return m_workspaceInitContext.m_pTypeRegistry->IsRegisteredResourceType( resourceTypeID );
    }

    void EditorContext::LoadMap( ResourceID const& mapResourceID ) const
    {
        KRG_ASSERT( m_pMapEditor != nullptr );
        KRG_ASSERT( mapResourceID.GetResourceTypeID() == EntityModel::EntityMapDescriptor::GetStaticResourceTypeID() );
        m_pMapEditor->LoadMap( mapResourceID );
    }

    bool EditorContext::IsMapEditorWorkspace( EditorWorkspace const* pWorkspace ) const
    {
        return m_pMapEditor == pWorkspace;
    }

    char const* EditorContext::GetMapEditorWindowName() const
    {
        return m_pMapEditor->GetWorkspaceWindowID();
    }

    bool EditorContext::IsGamePreviewWorkspace( EditorWorkspace const* pWorkspace ) const
    {
        return m_pGamePreviewer == pWorkspace;
    }

    void* EditorContext::GetViewportTextureForWorkspace( EditorWorkspace* pWorkspace ) const
    {
        KRG_ASSERT( pWorkspace != nullptr );
        auto pWorld = pWorkspace->GetWorld();
        return (void*) &m_pRenderingSystem->GetRenderTargetTextureForViewport( pWorld->GetViewport() );
    }

    Render::PickingID EditorContext::GetViewportPickingID( EditorWorkspace* pWorkspace, Int2 const& pixelCoords ) const
    {
        KRG_ASSERT( pWorkspace != nullptr );
        auto pWorld = pWorkspace->GetWorld();
        return m_pRenderingSystem->GetViewportPickingID( pWorld->GetViewport(), pixelCoords );
    }

    EditorWorkspace* EditorContext::FindResourceWorkspace( ResourceID const& resourceID ) const
    {
        KRG_ASSERT( resourceID.IsValid() );
        uint32 const resourcePathID = resourceID.GetResourcePath().GetID();

        auto foundWorkspaceIter = eastl::find( m_workspaces.begin(), m_workspaces.end(), resourcePathID, [] ( EditorWorkspace* const& pExistingWorkspace, uint32 ID ) { return pExistingWorkspace->GetID() == ID; } );
        if ( foundWorkspaceIter != m_workspaces.end() )
        {
            return *foundWorkspaceIter;
        }

        return nullptr;
    }

    EditorWorkspace* EditorContext::FindResourceWorkspace( uint32 workspaceID ) const
    {
        auto foundWorkspaceIter = eastl::find( m_workspaces.begin(), m_workspaces.end(), workspaceID, [] ( EditorWorkspace* const& pExistingWorkspace, uint32 ID ) { return pExistingWorkspace->GetID() == ID; } );
        if ( foundWorkspaceIter != m_workspaces.end() )
        {
            return *foundWorkspaceIter;
        }

        return nullptr;
    }

    //-------------------------------------------------------------------------

    bool EditorContext::IsGamePreviewAllowed() const
    {
        return m_pMapEditor->HasLoadedMap();
    }

    void EditorContext::StartGamePreview( UpdateContext const& context )
    {
        KRG_ASSERT( !IsGamePreviewRunning() );

        auto pPreviewWorld = m_pWorldManager->CreateWorld( EntityWorldType::Game );
        m_pRenderingSystem->CreateCustomRenderTargetForViewport( pPreviewWorld->GetViewport() );
        m_pGamePreviewer = KRG::New<GamePreviewer>( m_workspaceInitContext, pPreviewWorld );
        m_pGamePreviewer->Initialize( context );
        m_pGamePreviewer->LoadMapToPreview( m_pMapEditor->GetLoadedMap() );
        m_workspaces.emplace_back( m_pGamePreviewer );

        m_pMapEditor->OnGamePreviewStarted();
    }

    void EditorContext::StopGamePreview( UpdateContext const& context )
    {
        KRG_ASSERT( IsGamePreviewRunning() );
        DestroyWorkspace( context, m_pGamePreviewer );
        m_pGamePreviewer = nullptr;
    }
}