#ifdef _WIN32
#include "Resource.h"
#include "EngineApplication_win32.h"
#include "Applications/EngineShared/Engine_Win32.h"
#include "iniparser/krg_ini.h"
#include "Applications/Shared/cmdParser/krg_cmdparser.h"
#include "Applications/Shared/Win32/SharedHelpers_Win32.h"
#include "System/Input/InputSystem.h"
#include "System/Core/FileSystem/FileSystem.h"
#include "System/Core/Platform/PlatformHelpers_Win32.h"
#include "System/Core/Time/Timers.h"
#include <tchar.h>
#include <windows.h>

#if KRG_ENABLE_LPP
#include "LPP_API.h"
#endif

//-------------------------------------------------------------------------

namespace KRG
{
    EngineApplication::EngineApplication( HINSTANCE hInstance )
        : Win32Application( hInstance, "Kruger Engine", IDI_ENGINE_ICON )
        , m_engine( TFunction<bool( KRG::String const& error )>( [this] ( String const& error )-> bool  { return FatalError( error ); } ) )
    {}

    bool EngineApplication::ReadSettings( int32 argc, char** argv )
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
                m_engine.m_startupMap = ResourcePath( map.c_str() );
            }
        }

        // Read configuration settings from ini
        //-------------------------------------------------------------------------

        {
            FileSystem::Path const iniPath = FileSystem::GetCurrentProcessPath().Append( "KRG.ini" );
            if ( !m_engine.m_settingsRegistry.LoadFromFile( iniPath ) )
            {
                return FatalError( "Failed to read required settings from INI file" );
            }
        }

        return true;
    }

    bool EngineApplication::Initialize()
    {
        #if KRG_DEVELOPMENT_TOOLS
        if ( !EnsureResourceServerIsRunning( m_engine.m_resourceSettings.m_resourceServerExecutablePath ) )
        {
            return FatalError( "Couldn't start resource server!" );
        }
        #endif

        Int2 const windowDimensions( ( m_windowRect.right - m_windowRect.left ), ( m_windowRect.bottom - m_windowRect.top ) );
        if ( !m_engine.Initialize( m_applicationNameNoWhitespace, windowDimensions ) )
        {
            return FatalError( "Failed to initialize engine" );
        }

        return true;
    }

    bool EngineApplication::Shutdown()
    {
        return m_engine.Shutdown();
    }

    bool EngineApplication::ApplicationLoop()
    {
        // Uncomment for live editing of ImguiTheme
        //ImGuiX::Theme::ApplyTheme();
        return m_engine.Update();
    }

    //-------------------------------------------------------------------------

    LRESULT EngineApplication::WndProcess( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
    {
        return DefaultEngineWindowProcessor( &m_engine, hWnd, message, wParam, lParam );
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
    KRG::EngineApplication engineApplication( hInstance );
    return engineApplication.Run( __argc, __argv );
}

#endif