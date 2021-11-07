#pragma once

#include "Applications/Editor/Editor.h"
#include "Engine.h"
#include "Win32/Application_Win32.h"

#include "_AutoGenerated/ToolsTypeRegistration.h"

//-------------------------------------------------------------------------

namespace KRG
{
    class EditorEngine final : public Engine
    {
        friend class EditorApplication;

    public:

        using Engine::Engine;

        virtual void RegisterTypes() override
        {
            AutoGenerated::Tools::RegisterTypes( *m_pTypeRegistry );
        }

        virtual void UnregisterTypes() override
        {
            AutoGenerated::Tools::UnregisterTypes( *m_pTypeRegistry );
        }

        #if KRG_DEVELOPMENT_TOOLS
        virtual void CreateDevelopmentToolsUI() { m_pDevToolsUI = KRG::New<Editor>(); }
        #endif
    };

    //-------------------------------------------------------------------------

    class EditorApplication final : public Win32Application
    {

    public:

        EditorApplication( HINSTANCE hInstance );

    private:

        virtual bool ReadSettings( int32 argc, char** argv ) override;
        virtual bool Initialize() override;
        virtual bool Shutdown() override;

        virtual LRESULT WndProcess( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

        virtual bool ApplicationLoop() override;

    private:

        EditorEngine                    m_editorEngine;
    };
}