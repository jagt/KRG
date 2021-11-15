#include "Engine.h"
#include "Engine/Core/Modules/EngineModuleContext.h"
#include "System/Network/NetworkSystem.h"
#include "System/Core/Profiling/Profiling.h"
#include "System/Core/FileSystem/FileSystem.h"
#include "System/Core/Time/Timers.h"

#include "_AutoGenerated/EngineTypeRegistration.h"

//-------------------------------------------------------------------------

namespace KRG
{
    Engine::Engine( TFunction<bool( KRG::String const& error )>&& errorHandler )
        : m_fatalErrorHandler( errorHandler )
        , m_module_engine_core( m_settingsRegistry )
    {
        m_settingsRegistry.RegisterSettings( &m_resourceSettings );
        m_settingsRegistry.RegisterSettings( &m_renderSettings );
        m_settingsRegistry.RegisterSettings( &m_physicsSettings );
    }

    Engine::~Engine()
    {
        m_settingsRegistry.UnregisterSettings( &m_resourceSettings );
        m_settingsRegistry.UnregisterSettings( &m_renderSettings );
        m_settingsRegistry.UnregisterSettings( &m_physicsSettings );
    }

    //-------------------------------------------------------------------------

    bool Engine::InitializeModules()
    {
        // !!! Order is important !!!
        //-------------------------------------------------------------------------

        // CORE MODULE IS SPECIAL

        if ( !m_module_engine_render.Initialize( m_moduleContext ) )
        {
            return m_fatalErrorHandler( "Failed to initialize module" );
        }

        if ( !m_module_engine_physics.Initialize( m_moduleContext ) )
        {
            return m_fatalErrorHandler( "Failed to initialize module" );
        }

        if ( !m_module_engine_animation.Initialize( m_moduleContext ) )
        {
            return m_fatalErrorHandler( "Failed to initialize module" );
        }

        if ( !m_module_engine_navmesh.Initialize( m_moduleContext ) )
        {
            return m_fatalErrorHandler( "Failed to initialize module" );
        }

        //-------------------------------------------------------------------------

        if ( !m_module_game_core.Initialize( m_moduleContext ) )
        {
            return m_fatalErrorHandler( "Failed to initialize module" );
        }

        // Fill module array
        //-------------------------------------------------------------------------

        m_modules.emplace_back( &m_module_engine_core );
        m_modules.emplace_back( &m_module_engine_physics );
        m_modules.emplace_back( &m_module_engine_render );
        m_modules.emplace_back( &m_module_engine_animation );
        m_modules.emplace_back( &m_module_engine_navmesh );
        m_modules.emplace_back( &m_module_game_core );

        return true;
    }

    void Engine::LoadModuleResources( Resource::ResourceSystem& resourceSystem )
    {
        for ( auto pModule : m_modules )
        {
            pModule->LoadModuleResources( resourceSystem );
        }
    }

    bool Engine::OnModuleResourceLoadingComplete()
    {
        for ( auto pModule : m_modules )
        {
            if ( !pModule->OnEngineResourceLoadingComplete() )
            {
                return false;
            }
        }

        return true;
    }

    void Engine::UnloadModuleResources( Resource::ResourceSystem& resourceSystem )
    {
        for ( auto pModule : m_modules )
        {
            pModule->UnloadModuleResources( resourceSystem );
        }
    }

    void Engine::ShutdownModules()
    {
        // !!! Order is important !!!
        //-------------------------------------------------------------------------

        m_module_game_core.Shutdown( m_moduleContext );

        //-------------------------------------------------------------------------

        m_module_engine_navmesh.Shutdown( m_moduleContext );
        m_module_engine_animation.Shutdown( m_moduleContext );
        m_module_engine_physics.Shutdown( m_moduleContext );
        m_module_engine_render.Shutdown( m_moduleContext );

        // Core module is special

        //-------------------------------------------------------------------------

        m_modules.clear();
    }

    //-------------------------------------------------------------------------

    void Engine::RegisterTypes()
    {
        AutoGenerated::Engine::RegisterTypes( *m_pTypeRegistry );
    }

    void Engine::UnregisterTypes()
    {
        AutoGenerated::Engine::UnregisterTypes( *m_pTypeRegistry );
    }

    //-------------------------------------------------------------------------

    bool Engine::Initialize( String const& applicationName, Int2 const& windowDimensions )
    {
        KRG_LOG_MESSAGE( "System", "Engine Application Startup" );

        m_moduleContext.m_applicationName = applicationName;

        //-------------------------------------------------------------------------
        // Initialize Core
        //-------------------------------------------------------------------------

        if ( !m_module_engine_core.Initialize( m_moduleContext ) )
        {
            return m_fatalErrorHandler( "Failed to initialize engine core" );
        }

        m_pTaskSystem = m_module_engine_core.GetTaskSystem();
        m_pTypeRegistry = m_module_engine_core.GetTypeRegistry();
        m_pSystemRegistry = m_module_engine_core.GetSystemRegistry();
        m_pInputSystem = m_module_engine_core.GetInputSystem();
        m_pResourceSystem = m_module_engine_core.GetResourceSystem();
        m_pRenderDevice = m_module_engine_core.GetRenderDevice();
        m_pEntityWorldManager = m_module_engine_core.GetEntityWorldManager();

        #if KRG_DEVELOPMENT_TOOLS
        m_pImguiSystem = m_module_engine_core.GetImguiSystem();
        #endif

        //-------------------------------------------------------------------------
        // Register Types
        //-------------------------------------------------------------------------

        RegisterTypes();

        //-------------------------------------------------------------------------
        // Modules
        //-------------------------------------------------------------------------

        m_moduleInitStageReached = true;

        m_moduleContext.m_pSettingsRegistry = &m_settingsRegistry;
        m_moduleContext.m_pTaskSystem = m_pTaskSystem;
        m_moduleContext.m_pTypeRegistry = m_pTypeRegistry;
        m_moduleContext.m_pResourceSystem = m_pResourceSystem;
        m_moduleContext.m_pSystemRegistry = m_pSystemRegistry;
        m_moduleContext.m_pRenderDevice = m_pRenderDevice;
        m_moduleContext.m_pEntityWorldManager = m_pEntityWorldManager;

        if ( !InitializeModules() )
        {
            return m_fatalErrorHandler( "Engine Application Startup failed - See EngineApplication log for details" );
        }

        m_pPhysicsSystem = m_pSystemRegistry->GetSystem<Physics::PhysicsSystem>();
        m_pNavmeshSystem = m_pSystemRegistry->GetSystem<Navmesh::NavmeshSystem>();

        m_updateContext.m_pSystemRegistry = m_pSystemRegistry;

        //-------------------------------------------------------------------------
        // Load engine resources
        //-------------------------------------------------------------------------

        m_moduleResourcesInitStageReached = true;

        LoadModuleResources( *m_pResourceSystem );

        while ( m_pResourceSystem->IsBusy() )
        {
            Network::NetworkSystem::Update();
            m_pResourceSystem->Update();
        }

        if ( !OnModuleResourceLoadingComplete() )
        {
            return m_fatalErrorHandler( "Failed to load required engine resources - See EngineApplication log for details" );
        }

        //-------------------------------------------------------------------------
        // Final initialization
        //-------------------------------------------------------------------------

        m_finalInitStageReached = true;

        // Initialize entity world manager and load startup map
        m_pEntityWorldManager->Initialize( *m_pSystemRegistry );
        if ( m_startupMap.IsValid() )
        {
            auto const sceneResourceID = KRG::ResourceID( m_startupMap );
            m_pEntityWorldManager->GetPrimaryWorld()->LoadMap( sceneResourceID );
        }

        // Initialize rendering system
        m_renderingSystem.Initialize( m_pRenderDevice, Float2( windowDimensions ), m_module_engine_render.GetRendererRegistry(), m_pEntityWorldManager );
        m_pSystemRegistry->RegisterSystem( &m_renderingSystem );

        // Create development tools
        #if KRG_DEVELOPMENT_TOOLS
        CreateDevelopmentToolsUI();
        KRG_ASSERT( m_pDevToolsUI != nullptr );
        m_pDevToolsUI->Initialize( m_updateContext );
        #endif

        m_initialized = true;
        return true;
    }

    bool Engine::Shutdown()
    {
        KRG_LOG_MESSAGE( "System", "Engine Application Shutdown Started" );

        if ( m_pTaskSystem != nullptr )
        {
            m_pTaskSystem->WaitForAll();
        }

        //-------------------------------------------------------------------------
        // Shutdown World and runtime state
        //-------------------------------------------------------------------------

        if ( m_finalInitStageReached )
        {
            // Destroy development tools
            #if KRG_DEVELOPMENT_TOOLS
            KRG_ASSERT( m_pDevToolsUI != nullptr );
            m_pDevToolsUI->Shutdown( m_updateContext );
            DestroyDevelopmentToolsUI();
            KRG_ASSERT( m_pDevToolsUI == nullptr );
            #endif

            // Shutdown rendering system
            m_pSystemRegistry->UnregisterSystem( &m_renderingSystem );
            m_renderingSystem.Shutdown();

            // Wait for resource/object systems to complete all resource unloading
            m_pEntityWorldManager->Shutdown();

            while ( m_pEntityWorldManager->IsBusyLoading() || m_pResourceSystem->IsBusy() )
            {
                m_pResourceSystem->Update();
            }

            m_finalInitStageReached = false;
        }

        //-------------------------------------------------------------------------
        // Unload engine resources
        //-------------------------------------------------------------------------

        if( m_moduleResourcesInitStageReached )
        {
            if ( m_pResourceSystem != nullptr )
            {
                UnloadModuleResources( *m_pResourceSystem );

                while ( m_pResourceSystem->IsBusy() )
                {
                    m_pResourceSystem->Update();
                }
            }

            m_moduleResourcesInitStageReached = false;
        }

        //-------------------------------------------------------------------------
        // Shutdown modules
        //-------------------------------------------------------------------------

        if ( m_moduleInitStageReached )
        {
            m_updateContext.m_pSystemRegistry = nullptr;

            ShutdownModules();

            m_moduleContext.m_pSettingsRegistry = nullptr;
            m_moduleContext.m_pTaskSystem = nullptr;
            m_moduleContext.m_pTypeRegistry = nullptr;
            m_moduleContext.m_pResourceSystem = nullptr;
            m_moduleContext.m_pSystemRegistry = nullptr;
            m_moduleContext.m_pRenderDevice = nullptr;
            m_moduleContext.m_pEntityWorldManager = nullptr;
        }

        //-------------------------------------------------------------------------
        // Unregister types
        //-------------------------------------------------------------------------

        if ( m_moduleInitStageReached )
        {
            UnregisterTypes();
            m_moduleInitStageReached = false;
        }

        //-------------------------------------------------------------------------
        // Shutdown Core
        //-------------------------------------------------------------------------

        m_module_engine_core.Shutdown( m_moduleContext );

        m_pTaskSystem = nullptr;
        m_pTypeRegistry = nullptr;
        m_pSystemRegistry = nullptr;
        m_pInputSystem = nullptr;
        m_pResourceSystem = nullptr;
        m_pRenderDevice = nullptr;
        m_pEntityWorldManager = nullptr;

        #if KRG_DEVELOPMENT_TOOLS
        m_pImguiSystem = nullptr;
        #endif

        //-------------------------------------------------------------------------

        m_initialized = false;
        return true;
    }

    //-------------------------------------------------------------------------

    bool Engine::Update()
    {
        KRG_ASSERT( m_initialized );

        // Check for fatal errors
        //-------------------------------------------------------------------------

        if ( Log::HasFatalErrorOccurred() )
        {
            return m_fatalErrorHandler( Log::GetFatalError().m_message );
        }

        // Perform Frame Update
        //-------------------------------------------------------------------------

        Profiling::StartFrame();

        Milliseconds deltaTime = 0;
        {
            ScopedSystemTimer frameTimer( deltaTime );

            // Frame Start
            //-------------------------------------------------------------------------
            {
                KRG_PROFILE_SCOPE_SCENE( "Frame Start" );
                m_updateContext.m_stage = UpdateStage::FrameStart;

                //-------------------------------------------------------------------------

                {
                    KRG_PROFILE_SCOPE_NETWORK( "Networking" );
                    Network::NetworkSystem::Update();
                }

                //-------------------------------------------------------------------------

                m_pEntityWorldManager->StartFrame();

                #if KRG_DEVELOPMENT_TOOLS
                m_pImguiSystem->StartFrame( m_updateContext.GetDeltaTime() );
                m_pDevToolsUI->FrameStartUpdate( m_updateContext );
                #endif

                //-------------------------------------------------------------------------

                {
                    KRG_PROFILE_SCOPE_RESOURCE( "Loading/Streaming" );
                    m_pResourceSystem->Update();

                    // Handle hot-reloading of entities
                    #if KRG_DEVELOPMENT_TOOLS
                    if ( m_pResourceSystem->RequiresHotReloading() )
                    {
                        m_pEntityWorldManager->BeginHotReload( m_pResourceSystem->GetUsersToBeReloaded() );
                        m_pDevToolsUI->BeginHotReload( m_pResourceSystem->GetResourcesToBeReloaded() );
                        m_pResourceSystem->ClearHotReloadRequests();
                        m_pResourceSystem->Update( true );
                        m_pEntityWorldManager->EndHotReload();
                        m_pDevToolsUI->EndHotReload();
                    }
                    #endif

                    m_pEntityWorldManager->UpdateLoading();
                }

                //-------------------------------------------------------------------------

                m_pInputSystem->Update();
                m_pEntityWorldManager->UpdateWorlds( m_updateContext );
            }

            // Pre-Physics
            //-------------------------------------------------------------------------
            {
                KRG_PROFILE_SCOPE_SCENE( "Pre-Physics Update" );
                m_updateContext.m_stage = UpdateStage::PrePhysics;

                //-------------------------------------------------------------------------

                m_pEntityWorldManager->UpdateWorlds( m_updateContext );
            }

            // Physics
            //-------------------------------------------------------------------------
            {
                KRG_PROFILE_SCOPE_SCENE( "Physics Update" );
                m_updateContext.m_stage = UpdateStage::Physics;

                //-------------------------------------------------------------------------

                m_pPhysicsSystem->Update( m_updateContext );
                m_pEntityWorldManager->UpdateWorlds( m_updateContext );
            }

            // Post-Physics
            //-------------------------------------------------------------------------
            {
                KRG_PROFILE_SCOPE_SCENE( "Post-Physics Update" );
                m_updateContext.m_stage = UpdateStage::PostPhysics;

                //-------------------------------------------------------------------------
                
                m_pNavmeshSystem->Update( m_updateContext );
                m_pEntityWorldManager->UpdateWorlds( m_updateContext );
            }

            // Frame End
            //-------------------------------------------------------------------------
            {
                KRG_PROFILE_SCOPE_SCENE( "Frame End" );
                m_updateContext.m_stage = UpdateStage::FrameEnd;

                //-------------------------------------------------------------------------

                m_pEntityWorldManager->UpdateWorlds( m_updateContext );

                #if KRG_DEVELOPMENT_TOOLS
                m_pDevToolsUI->FrameEndUpdate( m_updateContext );
                m_pImguiSystem->EndFrame();
                #endif

                m_pEntityWorldManager->EndFrame();

                m_renderingSystem.Update( m_updateContext );
                m_pInputSystem->ClearFrameState();
            }
        }

        // Update Time
        //-------------------------------------------------------------------------

        // Ensure we dont get crazy time delta's when we hit breakpoints
        #if KRG_DEVELOPMENT_TOOLS
        if ( deltaTime.ToSeconds() > 1.0f )
        {
            deltaTime = m_updateContext.GetDeltaTime(); // Keep last frame delta
        }
        #endif

        m_updateContext.UpdateDeltaTime( deltaTime );
        EngineClock::Update( deltaTime );
        Profiling::EndFrame();

        // Should we exit?
        //-------------------------------------------------------------------------

        return true;
    }
}