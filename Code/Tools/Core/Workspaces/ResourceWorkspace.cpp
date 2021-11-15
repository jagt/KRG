#include "ResourceWorkspace.h"
#include "Tools/Core/TypeSystem/Serialization/TypeReader.h"
#include "Tools/Core/Resource/Compilers/ResourceDescriptor.h"

//-------------------------------------------------------------------------

namespace KRG
{
    KRG_DEFINE_GLOBAL_REGISTRY( ResourceWorkspaceFactory );

    //-------------------------------------------------------------------------

    class DescriptorUndoableAction : public IUndoableAction
    {
    public:

        DescriptorUndoableAction( TypeSystem::TypeRegistry const& typeRegistry, IRegisteredType* pTypeInstance )
            : m_typeRegistry( typeRegistry ) 
            , m_pEditedType( pTypeInstance )
        {
            KRG_ASSERT( m_pEditedType != nullptr );
        }

        void SerializeBeforeState()
        {
            TypeSystem::Serialization::WriteNativeTypeToString( m_typeRegistry, m_pEditedType, m_valueBefore );
        }

        void SerializeAfterState()
        {
            TypeSystem::Serialization::WriteNativeTypeToString( m_typeRegistry, m_pEditedType, m_valueAfter );
        }

        virtual void Undo()
        {
            TypeSystem::Serialization::ReadNativeTypeFromString( m_typeRegistry, m_valueBefore, m_pEditedType );
        }

        virtual void Redo()
        {
            TypeSystem::Serialization::ReadNativeTypeFromString( m_typeRegistry, m_valueAfter, m_pEditedType );
        }

    private:

        TypeSystem::TypeRegistry const&     m_typeRegistry;
        IRegisteredType*                    m_pEditedType = nullptr;
        String                              m_valueBefore;
        String                              m_valueAfter;
    };

    //-------------------------------------------------------------------------

    GenericResourceWorkspace::GenericResourceWorkspace( EditorContext const& context, EntityWorld* pWorld, ResourceID const& resourceID )
        : EditorWorkspace( context, pWorld, resourceID.GetResourcePath().ToFileSystemPath( context.m_sourceResourceDirectory ).GetFileNameWithoutExtension() )
        , m_descriptorID( resourceID )
        , m_descriptorPath( resourceID.GetResourcePath().ToFileSystemPath( context.m_sourceResourceDirectory ) )
        , m_propertyGrid( *context.m_pTypeRegistry, context.m_sourceResourceDirectory )
    {
        KRG_ASSERT( resourceID.IsValid() );

        m_preEditEventBindingID = m_propertyGrid.OnPreEdit().Bind( [this] ( PropertyEditInfo const& info ) { PreEdit( info ); } );
        m_postEditEventBindingID = m_propertyGrid.OnPostEdit().Bind( [this] ( PropertyEditInfo const& info ) { PostEdit( info ); } );
    }

    GenericResourceWorkspace::~GenericResourceWorkspace()
    {
        KRG_ASSERT( m_pDescriptor == nullptr );
        KRG_ASSERT( m_pActiveUndoableAction == nullptr );

        m_propertyGrid.OnPreEdit().Unbind( m_preEditEventBindingID );
        m_propertyGrid.OnPostEdit().Unbind( m_postEditEventBindingID );
    }

    void GenericResourceWorkspace::Initialize( UpdateContext const& context )
    {
        EditorWorkspace::Initialize( context );

        m_descriptorWindowName.sprintf( "Descriptor##%u", GetID() );

        TypeSystem::Serialization::TypeReader typeReader( *m_editorContext.m_pTypeRegistry );
        if ( typeReader.ReadFromFile( m_descriptorPath ) )
        {
            TypeSystem::TypeDescriptor typeDesc;
            if ( typeReader.ReadType( typeDesc ) )
            {
                m_pDescriptor = typeDesc.CreateTypeInstance<Resource::ResourceDescriptor>( *m_editorContext.m_pTypeRegistry );
                m_propertyGrid.SetTypeToEdit( m_pDescriptor );
            }
        }
    }

    void GenericResourceWorkspace::Shutdown( UpdateContext const& context )
    {
        KRG::Delete( m_pDescriptor );
        EditorWorkspace::Shutdown( context );
    }

    void GenericResourceWorkspace::InitializeDockingLayout( ImGuiID dockspaceID ) const
    {
        ImGui::DockBuilderDockWindow( m_descriptorWindowName.c_str(), dockspaceID );
    }

    void GenericResourceWorkspace::UpdateAndDrawWindows( UpdateContext const& context, ImGuiWindowClass* pWindowClass )
    {
        if ( ImGui::Begin( m_descriptorWindowName.c_str() ) )
        {
            if ( auto pDockNode = ImGui::GetWindowDockNode() )
            {
                pDockNode->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar;
            }

            //-------------------------------------------------------------------------

            if ( m_pDescriptor == nullptr )
            {
                ImGui::Text( "Failed to load descriptor!" );
            }
            else
            {
                {
                    ImGuiX::ScopedFont sf( ImGuiX::Font::Medium );
                    ImGui::Text( "Descriptor: %s", m_descriptorID.c_str() );
                }

                ImGui::BeginDisabled( !m_propertyGrid.IsDirty() );
                if ( ImGuiX::ButtonColored( Colors::LimeGreen.ToFloat4(), KRG_ICON_FLOPPY_O " Save", ImVec2( -1, 0 ) ) )
                {
                    Save();
                    m_propertyGrid.ClearDirty();
                }
                ImGui::EndDisabled();

                m_propertyGrid.DrawGrid();
            }
        }
        ImGui::End();
    }

    bool GenericResourceWorkspace::Save()
    {
        KRG_ASSERT( m_descriptorID.IsValid() && m_descriptorPath.IsFile() );
        KRG_ASSERT( m_pDescriptor != nullptr );
        
        if ( WriteResourceDescriptorToFile( *m_editorContext.m_pTypeRegistry, m_descriptorPath, m_pDescriptor ) )
        {
            m_propertyGrid.ClearDirty();
            return true;
        }

        return false;
    }

    void GenericResourceWorkspace::PreEdit( PropertyEditInfo const& info )
    {
        KRG_ASSERT( m_pActiveUndoableAction == nullptr );
        auto pUndoableAction = KRG::New<DescriptorUndoableAction>( *m_editorContext.m_pTypeRegistry, m_pDescriptor );
        pUndoableAction->SerializeBeforeState();
        m_pActiveUndoableAction = pUndoableAction;
    }

    void GenericResourceWorkspace::PostEdit( PropertyEditInfo const& info )
    {
        KRG_ASSERT( m_pActiveUndoableAction != nullptr );
        auto pUndoableAction = static_cast<DescriptorUndoableAction*>( m_pActiveUndoableAction );
        pUndoableAction->SerializeAfterState();
        m_undoStack.RegisterAction( pUndoableAction );
        m_pActiveUndoableAction = nullptr;
    }

    //-------------------------------------------------------------------------

    bool ResourceWorkspaceFactory::HasCustomWorkspace( ResourceTypeID const& resourceTypeID )
    {
        auto pCurrentFactory = s_pHead;
        while ( pCurrentFactory != nullptr )
        {
            if ( resourceTypeID == pCurrentFactory->GetSupportedResourceTypeID() )
            {
                return true;
            }

            pCurrentFactory = pCurrentFactory->GetNextItem();
        }

        return false;
    }

    EditorWorkspace* ResourceWorkspaceFactory::TryCreateWorkspace( EditorContext const& context, EntityWorld* pWorld, ResourceID const& resourceID )
    {
        KRG_ASSERT( resourceID.IsValid() );
        auto resourceTypeID = resourceID.GetResourceTypeID();
        KRG_ASSERT( resourceTypeID.IsValid() );

        auto pCurrentFactory = s_pHead;
        while ( pCurrentFactory != nullptr )
        {
            if ( resourceTypeID == pCurrentFactory->GetSupportedResourceTypeID() )
            {
                return pCurrentFactory->CreateWorkspace( context, pWorld, resourceID );
            }

            pCurrentFactory = pCurrentFactory->GetNextItem();
        }

        // Create generic descriptor workspace
        //-------------------------------------------------------------------------

        return KRG::New<GenericResourceWorkspace>( context, pWorld, resourceID );
    }
}