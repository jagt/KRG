//-------------------------------------------------------------------------
// This is an auto-generated file - DO NOT edit
//-------------------------------------------------------------------------

#include "System/Entity/EntityTypeHelpers.h"
#include "System/TypeSystem/TypeHelpers.h"
#include "System/TypeSystem/TypeRegistry.h"

#include "D:\Kruger\Code\Tools\Render\_Module\Module.h"
#include "D:\Kruger\Code\Engine\Core\_Module\Module.h"
#include "D:\Kruger\Code\Engine\Render\_Module\Module.h"
#include "D:\Kruger\Code\Engine\Animation\_Module\Module.h"
#include "D:\Kruger\Code\Engine\Camera\_Module\Module.h"
#include "D:\Kruger\Code\Engine\Navmesh\_Module\Module.h"
#include "D:\Kruger\Code\Engine\Physics\_Module\Module.h"
#include "D:\Kruger\Code\Game\Core\_Module\Module.h"
#include "D:\Kruger\Code\Tools\Core\_Module\Module.h"
#include "D:\Kruger\Code\Tools\Animation\_Module\Module.h"
#include "D:\Kruger\Code\Tools\Physics\_Module\Module.h"
#include "D:\Kruger\Code\Tools\Entity\_Module\Module.h"

namespace KRG
{
    namespace AutoGenerated::Tools
    {
        inline void RegisterTypes( TypeSystem::TypeRegistry& typeRegistry )
        {
            TypeSystem::RegisterCoreTypeSystemTypes( typeRegistry );
            TypeSystem::RegisterCoreEntityTypes( typeRegistry );

            KRG::EngineCore::EngineModule::RegisterTypes( typeRegistry );
            KRG::Render::EngineModule::RegisterTypes( typeRegistry );
            KRG::Animation::EngineModule::RegisterTypes( typeRegistry );
            KRG::Camera::EngineModule::RegisterTypes( typeRegistry );
            KRG::Navmesh::EngineModule::RegisterTypes( typeRegistry );
            KRG::Physics::EngineModule::RegisterTypes( typeRegistry );
            KRG::Game::GameModule::RegisterTypes( typeRegistry );
            KRG::ToolsModule::RegisterTypes( typeRegistry );
            KRG::Animation::ToolsModule::RegisterTypes( typeRegistry );
            KRG::Physics::ToolsModule::RegisterTypes( typeRegistry );
            KRG::EntityModel::ToolsModule::RegisterTypes( typeRegistry );
            KRG::Render::ToolsModule::RegisterTypes( typeRegistry );

            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::EntityModel::EntityMapDescriptor"), ResourceTypeID( "MAP" ) );
            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::Render::Shader"), ResourceTypeID( "SHDR" ) );
            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::Render::PixelShader"), ResourceTypeID( "PSDR" ) );
            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::Render::GeometryShader"), ResourceTypeID( "GSDR" ) );
            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::Render::VertexShader"), ResourceTypeID( "VSDR" ) );
            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::Render::Texture"), ResourceTypeID( "TXTR" ) );
            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::Render::Material"), ResourceTypeID( "MTRL" ) );
            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::Render::StaticMesh"), ResourceTypeID( "MSH" ) );
            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::Render::SkeletalMesh"), ResourceTypeID( "SMSH" ) );
            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::Animation::Skeleton"), ResourceTypeID( "SKEL" ) );
            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::Animation::AnimationClip"), ResourceTypeID( "ANIM" ) );
            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::Animation::AnimationGraphDataSet"), ResourceTypeID( "AGDS" ) );
            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::Animation::AnimationGraphDefinition"), ResourceTypeID( "AG" ) );
            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::Animation::AnimationGraphVariation"), ResourceTypeID( "AGV" ) );
            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::Navmesh::NavmeshData"), ResourceTypeID( "NAV" ) );
            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::Physics::PhysicsMesh"), ResourceTypeID( "PMSH" ) );
            typeRegistry.RegisterResourceTypeID( TypeSystem::TypeID( "KRG::Physics::PhysicsMaterialDatabase"), ResourceTypeID( "PMDB" ) );
        }

        inline void UnregisterTypes( TypeSystem::TypeRegistry& typeRegistry )
        {
            KRG::Render::ToolsModule::UnregisterTypes( typeRegistry );
            KRG::EntityModel::ToolsModule::UnregisterTypes( typeRegistry );
            KRG::Physics::ToolsModule::UnregisterTypes( typeRegistry );
            KRG::Animation::ToolsModule::UnregisterTypes( typeRegistry );
            KRG::ToolsModule::UnregisterTypes( typeRegistry );
            KRG::Game::GameModule::UnregisterTypes( typeRegistry );
            KRG::Physics::EngineModule::UnregisterTypes( typeRegistry );
            KRG::Navmesh::EngineModule::UnregisterTypes( typeRegistry );
            KRG::Camera::EngineModule::UnregisterTypes( typeRegistry );
            KRG::Animation::EngineModule::UnregisterTypes( typeRegistry );
            KRG::Render::EngineModule::UnregisterTypes( typeRegistry );
            KRG::EngineCore::EngineModule::UnregisterTypes( typeRegistry );

            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::Physics::PhysicsMaterialDatabase" ) );
            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::Physics::PhysicsMesh" ) );
            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::Navmesh::NavmeshData" ) );
            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::Animation::AnimationGraphVariation" ) );
            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::Animation::AnimationGraphDefinition" ) );
            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::Animation::AnimationGraphDataSet" ) );
            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::Animation::AnimationClip" ) );
            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::Animation::Skeleton" ) );
            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::Render::SkeletalMesh" ) );
            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::Render::StaticMesh" ) );
            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::Render::Material" ) );
            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::Render::Texture" ) );
            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::Render::VertexShader" ) );
            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::Render::GeometryShader" ) );
            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::Render::PixelShader" ) );
            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::Render::Shader" ) );
            typeRegistry.UnregisterResourceTypeID( TypeSystem::TypeID( "KRG::EntityModel::EntityMapDescriptor" ) );

            TypeSystem::UnregisterCoreEntityTypes( typeRegistry );
            TypeSystem::UnregisterCoreTypeSystemTypes( typeRegistry );
        }
    }
}
