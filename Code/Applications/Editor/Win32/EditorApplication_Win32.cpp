#include "EditorApplication_Win32.h"
#include "Applications/EngineShared/Engine_Win32.h"
#include "Resource.h"
#include "iniparser/krg_ini.h"
#include "Applications/Shared/cmdParser/krg_cmdparser.h"
#include "Applications/Shared/Win32/SharedHelpers_Win32.h"
#include "System/Input/InputSystem.h"
#include "System/Core/FileSystem/FileSystem.h"
#include "System/Core/Time/Timers.h"
#include <tchar.h>
#include <windows.h>

#if KRG_ENABLE_LPP
#include "LPP_API.h"
#endif

//-------------------------------------------------------------------------

namespace KRG
{
    EditorEngine::EditorEngine( TFunction<bool( KRG::String const& error )>&& errorHandler )
        : Engine( eastl::forward<TFunction<bool( KRG::String const& error )>&&>( errorHandler ) )
    {
        //m_module_engine_core.EnableImguiViewports();
    }

    void EditorEngine::CreateToolsUI()
    {
        auto pEditorUI = KRG::New<EditorUI>();
        if ( m_editorStartupMap.IsValid() )
        {
            pEditorUI->SetStartupMap( m_editorStartupMap );
        }
        m_pToolsUI = pEditorUI;
    }

    //-------------------------------------------------------------------------

    EditorApplication::EditorApplication( HINSTANCE hInstance )
        : Win32Application( hInstance, "Kruger Editor", IDI_EDITOR_ICON )
        , m_editorEngine( TFunction<bool( String const& error )>( [this] ( String const& error )-> bool  { return FatalError( error ); } ) )
    {}

    bool EditorApplication::ReadSettings( int32 argc, char** argv )
    {
        // Get command line settings
        //-------------------------------------------------------------------------

        {
            cli::Parser cmdParser( argc, argv );
            cmdParser.set_optional<std::string>( "map", "map", "", "The startup map." );

            if ( !cmdParser.run() )
            {
                return FatalError( "Invalid command line arguments!" );
            }

            std::string const map = cmdParser.get<std::string>( "map" );
            if ( !map.empty() )
            {
                m_editorEngine.m_editorStartupMap = ResourcePath( map.c_str() );
            }
        }

        // Read configuration settings from ini
        //-------------------------------------------------------------------------

        FileSystem::Path const iniPath = FileSystem::GetCurrentProcessPath().Append( "KRG.ini" );
        if ( !m_editorEngine.m_settingsRegistry.LoadFromFile( iniPath ) )
        {
            return FatalError( "Failed to read required settings from INI file" );
        }

        return true;
    }

    bool EditorApplication::Initialize()
    {
        if ( !EnsureResourceServerIsRunning( m_editorEngine.m_resourceSettings.m_resourceServerExecutablePath ) )
        {
            return FatalError( "Couldn't start resource server!" );
        }

        // Initialize editor
        //-------------------------------------------------------------------------

        Int2 const windowDimensions( ( m_windowRect.right - m_windowRect.left ), ( m_windowRect.bottom - m_windowRect.top ) );
        if ( !m_editorEngine.Initialize( m_applicationNameNoWhitespace, windowDimensions ) )
        {
            return FatalError( "Failed to initialize engine" );
        }

        return true;
    }

    bool EditorApplication::Shutdown()
    {
        return m_editorEngine.Shutdown();
    }

    bool EditorApplication::ApplicationLoop()
    {
        // Uncomment for live editing of ImguiTheme
        ImGuiX::Style::Apply();
        return m_editorEngine.Update();
    }

    //-------------------------------------------------------------------------

    LRESULT EditorApplication::WndProcess( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
    {
        return DefaultEngineWindowProcessor( &m_editorEngine, hWnd, message, wParam, lParam );
    }
}

//-------------------------------------------------------------------------

int APIENTRY _tWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
    //-------------------------------------------------------------------------
    // Live++ Support
    //-------------------------------------------------------------------------

    #if KRG_ENABLE_LPP
    HMODULE livePP = lpp::lppLoadAndRegister( L"../../External/LivePP", "Quickstart" );
    lpp::lppEnableAllCallingModulesSync( livePP );
    #endif

    //-------------------------------------------------------------------------

    KRG::ApplicationGlobalState globalState;
    KRG::EditorApplication editorApplication( hInstance );
    return editorApplication.Run( __argc, __argv );
}