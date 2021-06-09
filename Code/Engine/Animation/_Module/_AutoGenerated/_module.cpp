//-------------------------------------------------------------------------
// This is an auto-generated file - DO NOT edit
//-------------------------------------------------------------------------

#include "D:\Kruger\Code\Engine\Animation\_Module\Module.h"
#include "System/Resource/ResourceSystem.h"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\1269671447.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\1407246524.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\1897638207.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\2320873778.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\2353136754.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\2450972253.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\2466578237.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\2642939929.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\3010573638.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\3088240376.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\3233922586.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\3244683732.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\3270222065.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\3513580283.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\3614657310.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\3747953061.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\3752789687.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\3861324641.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\3975240535.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\436129338.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\485968716.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\53069741.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\732180760.cpp"
#include "D:\Kruger\Code\Engine\Animation\_Module\_AutoGenerated\879625011.cpp"

//-------------------------------------------------------------------------

void KRG::Animation::EngineModule::RegisterTypes( TypeSystem::TypeRegistry& typeRegistry )
{
    TypeSystem::EnumHelpers::EnumHelper_KRG_Animation_AnimationPlayerComponent_PlayMode::RegisterEnum( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::AnimationComponent>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::AnimationPlayerComponent>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Event>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Events::IDEvent>::RegisterType( typeRegistry );
    TypeSystem::EnumHelpers::EnumHelper_KRG_Animation_Events_FootstepEvent_Phase::RegisterEnum( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Events::FootstepEvent>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::GraphNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::PassthroughNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::RootMotionOverrideNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ZeroPoseNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ReferencePoseNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::AnimationPoseNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::AnimationSystem>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::SelectorNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::AnimationSelectorNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ConstBoolNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ConstIDNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ConstIntNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ConstFloatNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ConstVectorNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ConstTargetNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::SpeedScaleNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VelocityBasedSpeedScaleNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::EnumHelpers::EnumHelper_KRG_Animation_Event_EventType::RegisterEnum( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::AnimationGraphComponent>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::AndNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::OrNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::NotNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::FloatComparisonNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::RangeComparisonNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::IDComparisonNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::MultipleIDComparisonNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::IsTargetSetNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::AnimationClipNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::AnimatedMeshComponent>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ControlParameterBoolNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ControlParameterIDNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ControlParameterIntNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ControlParameterFloatNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ControlParameterVectorNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ControlParameterTargetNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ControlParameterBoneMaskNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VirtualParameterBoolNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VirtualParameterIDNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VirtualParameterIntNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VirtualParameterFloatNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VirtualParameterVectorNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VirtualParameterTargetNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VirtualParameterBoneMaskNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ParameterizedBlendNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::RangedBlendNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VelocityBlendNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::FloatRemapNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::FloatClampNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::FloatAbsNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::IDToFloatNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VectorInfoNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::FloatBlendNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::FloatMathNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::TargetInfoNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::FloatSwitchNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::TargetOffsetNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VectorNegateNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::BoneMaskNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::BoneMaskBlendNode::Settings>::RegisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::BoneMaskSelectorNode::Settings>::RegisterType( typeRegistry );
}

void KRG::Animation::EngineModule::UnregisterTypes( TypeSystem::TypeRegistry& typeRegistry )
{
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::BoneMaskSelectorNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::BoneMaskBlendNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::BoneMaskNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VectorNegateNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::TargetOffsetNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::FloatSwitchNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::TargetInfoNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::FloatMathNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::FloatBlendNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VectorInfoNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::IDToFloatNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::FloatAbsNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::FloatClampNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::FloatRemapNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VelocityBlendNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::RangedBlendNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ParameterizedBlendNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VirtualParameterBoneMaskNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VirtualParameterTargetNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VirtualParameterVectorNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VirtualParameterFloatNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VirtualParameterIntNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VirtualParameterIDNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VirtualParameterBoolNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ControlParameterBoneMaskNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ControlParameterTargetNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ControlParameterVectorNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ControlParameterFloatNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ControlParameterIntNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ControlParameterIDNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ControlParameterBoolNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::AnimatedMeshComponent>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::AnimationClipNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::IsTargetSetNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::MultipleIDComparisonNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::IDComparisonNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::RangeComparisonNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::FloatComparisonNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::NotNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::OrNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::AndNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::AnimationGraphComponent>::UnregisterType( typeRegistry );
    TypeSystem::EnumHelpers::EnumHelper_KRG_Animation_Event_EventType::UnregisterEnum( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::VelocityBasedSpeedScaleNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::SpeedScaleNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ConstTargetNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ConstVectorNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ConstFloatNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ConstIntNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ConstIDNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ConstBoolNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::AnimationSelectorNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::SelectorNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::AnimationSystem>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::AnimationPoseNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ReferencePoseNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::ZeroPoseNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::RootMotionOverrideNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::PassthroughNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Graph::GraphNode::Settings>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Events::FootstepEvent>::UnregisterType( typeRegistry );
    TypeSystem::EnumHelpers::EnumHelper_KRG_Animation_Events_FootstepEvent_Phase::UnregisterEnum( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Events::IDEvent>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::Event>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::AnimationPlayerComponent>::UnregisterType( typeRegistry );
    TypeSystem::TypeHelpers::TTypeHelper<KRG::Animation::AnimationComponent>::UnregisterType( typeRegistry );
    TypeSystem::EnumHelpers::EnumHelper_KRG_Animation_AnimationPlayerComponent_PlayMode::UnregisterEnum( typeRegistry );
}

