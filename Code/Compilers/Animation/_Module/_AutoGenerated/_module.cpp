//-------------------------------------------------------------------------
// This is an auto-generated file - DO NOT edit
//-------------------------------------------------------------------------

#include "D:\Kruger\Code\Compilers\Animation\_Module\Module.h"
#include "System/Resource/ResourceSystem.h"
#include "D:\Kruger\Code\Compilers\Animation\_Module\_AutoGenerated\2383976657.cpp"
#include "D:\Kruger\Code\Compilers\Animation\_Module\_AutoGenerated\3352258366.cpp"

//-------------------------------------------------------------------------

void KRG::Animation::ResourceCompilerModule::RegisterTypes( TypeSystem::TypeRegistry& typeRegistry )
{
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::SkeletonResourceDescriptor>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::AnimationResourceDescriptor>::RegisterType( typeRegistry );
}

void KRG::Animation::ResourceCompilerModule::UnregisterTypes( TypeSystem::TypeRegistry& typeRegistry )
{
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::AnimationResourceDescriptor>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::SkeletonResourceDescriptor>::UnregisterType( typeRegistry );
}

