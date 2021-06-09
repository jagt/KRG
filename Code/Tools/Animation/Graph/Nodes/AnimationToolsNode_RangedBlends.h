#pragma once
#include "../AnimationGraphTools_Node.h"
#include "Engine/Animation/Graph/Nodes/AnimationGraphNode_RangedBlends.h"

//-------------------------------------------------------------------------

namespace KRG::Animation::Graph
{
    class ParameterizedBlendToolsNode : public ToolsNode
    {
        KRG_REGISTER_TYPE( ParameterizedBlendToolsNode );

    public:

        virtual char const* GetCategory() const override { return "Blends"; }
        virtual TBitFlags<GraphType> GetAllowedParentGraphTypes() const override final { return TBitFlags<GraphType>( GraphType::BlendTree ); }
        virtual bool SupportsDynamicInputPins() const override { return true; }
        virtual char const* GetDynamicInputPinName() const override { return "Option"; }
        virtual uint32 GetDynamicInputPinValueType() const override { return (uint32) NodeValueType::Pose; }

    protected:

        ParameterizedBlendToolsNode();

        bool CompileParameterAndSourceNodes( ToolsGraphCompilationContext& context, ParameterizedBlendNode::Settings* pSettings ) const;

    protected:

        EXPOSE bool                 m_isSynchronized = false;
    };

    //-------------------------------------------------------------------------

    class RangedBlendToolsNode final : public ParameterizedBlendToolsNode
    {
        KRG_REGISTER_TYPE( RangedBlendToolsNode );

        virtual char const* GetTypeName() const override { return "Ranged Blend"; }
        virtual NodeIndex Compile( ToolsGraphCompilationContext& context ) const override;

    private:

        REGISTER TVector<float>     m_parameterValues;
    };

    //-------------------------------------------------------------------------

    class VelocityBlendToolsNode final : public ParameterizedBlendToolsNode
    {
        KRG_REGISTER_TYPE( VelocityBlendToolsNode );

        virtual char const* GetTypeName() const override { return "Velocity Blend"; }
        virtual NodeIndex Compile( ToolsGraphCompilationContext& context ) const override;
    };
}