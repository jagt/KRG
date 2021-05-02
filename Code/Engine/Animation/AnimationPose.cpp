#include "AnimationPose.h"
#include "System/Core/Debug/DebugDrawing.h"

//-------------------------------------------------------------------------

namespace KRG::Animation
{
    Pose::Pose( Skeleton const* pSkeleton, InitialState initialState )
        : m_pSkeleton( pSkeleton )
        , m_localTransforms( pSkeleton->GetNumBones() )
    {
        KRG_ASSERT( pSkeleton != nullptr );
        Reset( initialState );
    }

    Pose::Pose( Pose&& rhs )
    {
        KRG_ASSERT( rhs.m_pSkeleton != nullptr );
        operator=( eastl::move( rhs ) );
    }

    Pose& Pose::operator=( Pose&& rhs )
    {
        m_pSkeleton = rhs.m_pSkeleton;
        m_localTransforms.swap( rhs.m_localTransforms );
        m_globalTransforms.swap( rhs.m_globalTransforms );
        m_state = rhs.m_state;

        return *this;
    }

    void Pose::CopyFrom( Pose const& rhs )
    {
        m_pSkeleton = rhs.m_pSkeleton;
        m_localTransforms = rhs.m_localTransforms;
        m_globalTransforms = rhs.m_globalTransforms;
        m_state = rhs.m_state;
    }

    //-------------------------------------------------------------------------

    void Pose::Reset( InitialState initialState, bool calcGlobalPose )
    {
        switch ( initialState )
        {
            case InitialState::ReferencePose:
            {
                SetToReferencePose( calcGlobalPose );
            }
            break;

            case InitialState::ZeroPose:
            {
                SetToZeroPose( calcGlobalPose );
            }
            break;

            default:
            {
                // Leave memory intact, just change state
                m_state = State::Unset;
            }
            break;
        }
    }

    void Pose::SetToReferencePose( bool setGlobalPose )
    {
        m_localTransforms = m_pSkeleton->GetLocalReferencePose();

        if ( setGlobalPose )
        {
            m_localTransforms = m_pSkeleton->GetGlobalReferencePose();
        }
        else
        {
            m_globalTransforms.clear();
        }

        m_state = State::ReferencePose;
    }

    void Pose::SetToZeroPose( bool setGlobalPose )
    {
        auto const numBones = m_pSkeleton->GetNumBones();
        m_localTransforms.resize( numBones, Transform::Identity );

        if ( setGlobalPose )
        {
            m_globalTransforms = m_localTransforms;
        }
        else
        {
            m_globalTransforms.clear();
        }

        m_state = State::ZeroPose;
    }

    //-------------------------------------------------------------------------

    void Pose::CalculateGlobalTransforms()
    {
        int32 const numBones = m_pSkeleton->GetNumBones();
        m_globalTransforms.resize( numBones );

        m_globalTransforms[0] = m_localTransforms[0];
        for ( auto boneIdx = 1; boneIdx < numBones; boneIdx++ )
        {
            int32 const parentIdx = m_pSkeleton->GetParentIndex( boneIdx );
            m_globalTransforms[boneIdx] = m_localTransforms[boneIdx] * m_globalTransforms[parentIdx];
        }
    }

    Transform Pose::GetGlobalTransform( int32 boneIdx ) const
    {
        KRG_ASSERT( boneIdx < m_pSkeleton->GetNumBones() );

        Transform boneGlobalTransform;
        if ( !m_globalTransforms.empty() )
        {
            boneGlobalTransform = m_globalTransforms[boneIdx];
        }
        else
        {
            auto boneParents = KRG_STACK_ARRAY_ALLOC( int32, m_pSkeleton->GetNumBones() );
            int32 nextEntry = 0;

            // Get parent list
            int32 parentIdx = m_pSkeleton->GetParentIndex( boneIdx );
            while ( parentIdx != InvalidIndex )
            {
                boneParents[nextEntry++] = parentIdx;
                parentIdx = m_pSkeleton->GetParentIndex( parentIdx );
            }

            // If we have parents
            boneGlobalTransform = m_localTransforms[boneIdx];
            if ( nextEntry > 0 )
            {
                // Calculate global transform of parent
                int32 arrayIdx = nextEntry - 1;
                parentIdx = boneParents[arrayIdx--];
                auto parentGlobalTransform = m_localTransforms[parentIdx];
                for ( arrayIdx; arrayIdx >= 0; arrayIdx-- )
                {
                    int32 const nextIdx = boneParents[arrayIdx];
                    auto const nextTransform = m_localTransforms[nextIdx];
                    parentGlobalTransform = nextTransform * parentGlobalTransform;
                }

                // Calculate global transform of bone
                boneGlobalTransform = boneGlobalTransform * parentGlobalTransform;
            }
        }

        return boneGlobalTransform;
    }

    //-------------------------------------------------------------------------

    #if KRG_DEVELOPMENT_TOOLS
    void Pose::DrawDebug( Debug::DrawingContext& ctx, Transform const& worldTransform ) const
    {
        auto const& parentIndices = m_pSkeleton->GetParentIndices();

        //-------------------------------------------------------------------------

        auto const numBones = m_localTransforms.size();
        if ( numBones > 0 )
        {
            Transform globalTransforms[256];
            globalTransforms[0] = m_localTransforms[0] * worldTransform;
            for ( auto i = 1; i < numBones; i++ )
            {
                auto const& parentIdx = parentIndices[i];
                auto const& parentTransform = globalTransforms[parentIdx];
                globalTransforms[i] = m_localTransforms[i] * parentTransform;
            }

            for ( auto boneIdx = 1; boneIdx < numBones; boneIdx++ )
            {
                auto const& parentIdx = parentIndices[boneIdx];
                auto const& parentTransform = globalTransforms[parentIdx];
                auto const& boneTransform = globalTransforms[boneIdx];

                ctx.DrawLine( boneTransform.GetTranslation().ToFloat3(), parentTransform.GetTranslation().ToFloat3(), Colors::HotPink, 2.0f );
                ctx.DrawAxis( boneTransform, 0.03f, 3.0f );
            }
        }
    }
    #endif
}