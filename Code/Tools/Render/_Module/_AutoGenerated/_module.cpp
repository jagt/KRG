//-------------------------------------------------------------------------
// This is an auto-generated file - DO NOT edit
//-------------------------------------------------------------------------

#include "D:\Kruger\Code\Tools\Render\_Module\Module.h"
#include "System/Resource/ResourceSystem.h"
#include "D:\Kruger\Code\Tools\Render\_Module\_AutoGenerated\3051453607.cpp"
#include "D:\Kruger\Code\Tools\Render\_Module\_AutoGenerated\3476150272.cpp"
#include "D:\Kruger\Code\Tools\Render\_Module\_AutoGenerated\3899843851.cpp"
#include "D:\Kruger\Code\Tools\Render\_Module\_AutoGenerated\3989763495.cpp"
#include "D:\Kruger\Code\Tools\Render\_Module\_AutoGenerated\632646782.cpp"
#include "D:\Kruger\Code\Tools\Render\_Module\_AutoGenerated\985831229.cpp"

//-------------------------------------------------------------------------

void KRG::Render::ToolsModule::RegisterTypes( TypeSystem::TypeRegistry& typeRegistry )
{
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Render::MeshResourceDescriptor>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Render::StaticMeshResourceDescriptor>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Render::SkeletalMeshResourceDescriptor>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Render::MaterialResourceDescriptor>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Render::TextureResourceDescriptor>::RegisterType( typeRegistry );
    TypeSystem::EnumHelpers::EnumHelper_KRG_Render_ShaderType::RegisterEnum( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Render::ShaderResourceDescriptor>::RegisterType( typeRegistry );
}

void KRG::Render::ToolsModule::UnregisterTypes( TypeSystem::TypeRegistry& typeRegistry )
{
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Render::ShaderResourceDescriptor>::UnregisterType( typeRegistry );
    TypeSystem::EnumHelpers::EnumHelper_KRG_Render_ShaderType::UnregisterEnum( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Render::TextureResourceDescriptor>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Render::MaterialResourceDescriptor>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Render::SkeletalMeshResourceDescriptor>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Render::StaticMeshResourceDescriptor>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Render::MeshResourceDescriptor>::UnregisterType( typeRegistry );
}

