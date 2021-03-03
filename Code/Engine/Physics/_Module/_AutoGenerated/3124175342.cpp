//-------------------------------------------------------------------------
// This is an auto-generated file - DO NOT edit
//-------------------------------------------------------------------------

#include "D:\Kruger\Code\Engine\Physics\_Module\_AutoGenerated\3124175342.h"

//-------------------------------------------------------------------------
// Enum Helper: KRG::Physics::PhysicMaterialCombineMode
//-------------------------------------------------------------------------

namespace KRG
{
    namespace TypeSystem
    {
        namespace EnumHelpers
        {
            void EnumHelper_KRG_Physics_PhysicMaterialCombineMode::RegisterEnum( TypeSystem::TypeRegistry& typeRegistry )
            {
                TypeSystem::TypeInfo typeInfo;
                typeInfo.m_ID = TypeSystem::TypeID( "KRG::Physics::PhysicMaterialCombineMode" );
                typeInfo.m_size = sizeof( KRG::Physics::PhysicMaterialCombineMode );
                typeInfo.m_alignment = alignof( KRG::Physics::PhysicMaterialCombineMode );
                typeRegistry.RegisterType( typeInfo );

                TypeSystem::EnumInfo enumInfo;
                enumInfo.m_ID = TypeSystem::TypeID( "KRG::Physics::PhysicMaterialCombineMode" );
                enumInfo.m_underlyingType = TypeSystem::CoreTypes::S32;
                enumInfo.m_constants.insert( TPair<StringID, S64>( StringID( "Max" ), 3 ) );
                enumInfo.m_constants.insert( TPair<StringID, S64>( StringID( "Average" ), 0 ) );
                enumInfo.m_constants.insert( TPair<StringID, S64>( StringID( "Multiply" ), 2 ) );
                enumInfo.m_constants.insert( TPair<StringID, S64>( StringID( "Min" ), 1 ) );

                typeRegistry.RegisterEnum( enumInfo );
            }

            void EnumHelper_KRG_Physics_PhysicMaterialCombineMode::UnregisterEnum( TypeSystem::TypeRegistry& typeRegistry )
            {
                auto const ID = TypeSystem::TypeID( "KRG::Physics::PhysicMaterialCombineMode" );
                typeRegistry.UnregisterType( ID );
                typeRegistry.UnregisterEnum( ID );
            }
        }
    }
}

//-------------------------------------------------------------------------
// TypeHelper: KRG::Physics::PhysicsMaterialSettings
//-------------------------------------------------------------------------

namespace KRG
{
    TypeSystem::TypeInfo const* KRG::Physics::PhysicsMaterialSettings::StaticTypeInfo = nullptr;
    namespace TypeSystem
    {
        namespace TypeHelpers
        {
            void const* TTypeHelper<KRG::Physics::PhysicsMaterialSettings>::DefaultTypeInstancePtr = nullptr;

            TTypeHelper<KRG::Physics::PhysicsMaterialSettings> TTypeHelper<KRG::Physics::PhysicsMaterialSettings>::StaticTypeHelper;
        }
    }
}
