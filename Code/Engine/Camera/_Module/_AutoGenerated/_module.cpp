//-------------------------------------------------------------------------
// This is an auto-generated file - DO NOT edit
//-------------------------------------------------------------------------

#include "D:\Kruger\Code\Engine\Camera\_Module\Module.h"
#include "System/Resource/ResourceSystem.h"
#include "D:\Kruger\Code\Engine\Camera\_Module\_AutoGenerated\2012943629.cpp"

//-------------------------------------------------------------------------

void KRG::Camera::EngineModule::RegisterTypes( TypeSystem::TypeRegistry& typeRegistry )
{
    TypeSystem::EnumHelpers::EnumHelper_KRG_Camera_CameraComponent_ProjectionType::RegisterEnum( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Camera::CameraComponent>::RegisterType( typeRegistry );
}

void KRG::Camera::EngineModule::UnregisterTypes( TypeSystem::TypeRegistry& typeRegistry )
{
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Camera::CameraComponent>::UnregisterType( typeRegistry );
    TypeSystem::EnumHelpers::EnumHelper_KRG_Camera_CameraComponent_ProjectionType::UnregisterEnum( typeRegistry );
}

