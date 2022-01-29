#ifdef _WIN32
#pragma once

#include <windows.h>
#include "Applications/Shared/Win32/Application_Win32.h"
#include "Applications/EngineShared/Engine.h"
#include "Engine/Core/ToolsUI/EngineToolsUI.h"

//-------------------------------------------------------------------------

namespace KRG
{
    class StandaloneEngine final : public Engine
    {
        friend class ResourceEditorApplication;

    public:

        using Engine::Engine;

        #if KRG_DEVELOPMENT_TOOLS
        virtual void CreateToolsUI() { m_pToolsUI = KRG::New<EngineToolsUI>(); }
        #endif
    };

    //-------------------------------------------------------------------------

    class EngineApplication : public Win32Application
    {

    public:

        EngineApplication( HINSTANCE hInstance );

    private:

        virtual bool ReadSettings( int32 argc, char** argv ) override;
        virtual bool Initialize();
        virtual bool Shutdown();

        virtual LRESULT WndProcess( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

        virtual bool ApplicationLoop() override;

    private:

        StandaloneEngine            m_engine;
    };
}

#endif