#include "AnimationClipCompiler.h"
#include "AnimationSkeletonCompiler.h"
#include "Tools/Animation/Events/AnimationEventData.h"
#include "Tools/Core/Resource/RawAssets/RawAssetReader.h"
#include "Tools/Core/Resource/RawAssets/RawAnimation.h"
#include "Tools/Core/TimelineEditor/TimelineData.h"
#include "Engine/Animation/AnimationClip.h"
#include "System/Core/FileSystem/FileSystem.h"
#include "System/Core/Serialization/BinaryArchive.h"

//-------------------------------------------------------------------------

namespace KRG::Animation
{
    AnimationClipCompiler::AnimationClipCompiler()
        : Resource::Compiler( "AnimationCompiler", s_version )
    {
        m_outputTypes.push_back( AnimationClip::GetStaticResourceTypeID() );
    }

    Resource::CompilationResult AnimationClipCompiler::Compile( Resource::CompileContext const& ctx ) const
    {
        AnimationClipResourceDescriptor resourceDescriptor;
        if ( !ctx.TryReadResourceDescriptor( resourceDescriptor ) )
        {
            return Error( "Failed to read resource descriptor from input file: %s", ctx.m_inputFilePath.c_str() );
        }

        // Read Skeleton Data
        //-------------------------------------------------------------------------

        // Convert the skeleton resource path to a physical file path
        if ( !resourceDescriptor.m_pSkeleton.GetResourceID().IsValid() )
        {
            return Error( "Invalid skeleton resource ID" );
        }

        DataPath const& skeletonDataPath = resourceDescriptor.m_pSkeleton.GetResourceID().GetDataPath();
        FileSystem::Path skeletonDescriptorFilePath;
        if ( !ctx.ConvertDataPathToFilePath( skeletonDataPath, skeletonDescriptorFilePath ) )
        {
            return Error( "Invalid skeleton data path: %s", skeletonDataPath.c_str() );
        }

        if ( !skeletonDescriptorFilePath.Exists() )
        {
            return Error( "Invalid skeleton descriptor file path: %s", skeletonDescriptorFilePath.ToString().c_str() );
        }

        SkeletonResourceDescriptor skeletonResourceDescriptor;
        if ( !ctx.TryReadResourceDescriptorFromFile( skeletonDescriptorFilePath, skeletonResourceDescriptor ) )
        {
            return Error( "Failed to read skeleton resource descriptor from input file: %s", ctx.m_inputFilePath.c_str() );
        }

        FileSystem::Path skeletonFilePath;
        if ( !ctx.ConvertDataPathToFilePath( skeletonResourceDescriptor.m_skeletonDataPath, skeletonFilePath ) )
        {
            return Error( "Invalid skeleton FBX data path: %s", skeletonResourceDescriptor.m_skeletonDataPath.GetString().c_str() );
        }

        RawAssets::ReaderContext readerCtx = { [this]( char const* pString ) { Warning( pString ); }, [this] ( char const* pString ) { Error( pString ); } };
        auto pRawSkeleton = RawAssets::ReadSkeleton( readerCtx, skeletonFilePath, skeletonResourceDescriptor.m_skeletonRootBoneName );
        if ( pRawSkeleton == nullptr && !pRawSkeleton->IsValid() )
        {
            return Error( "Failed to read skeleton file: %s", skeletonFilePath.ToString().c_str() );
        }

        // Read animation data
        //-------------------------------------------------------------------------

        FileSystem::Path animationFilePath;
        if ( !ctx.ConvertDataPathToFilePath( resourceDescriptor.m_animationDataPath, animationFilePath ) )
        {
            return Error( "Invalid animation data path: %s", resourceDescriptor.m_animationDataPath.c_str() );
        }

        if ( !animationFilePath.Exists() )
        {
            return Error( "Invalid animation file path: %s", animationFilePath.ToString().c_str() );
        }

        TUniquePtr<RawAssets::RawAnimation> pRawAnimation = RawAssets::ReadAnimation( readerCtx, animationFilePath, *pRawSkeleton, resourceDescriptor.m_animationName );
        if ( pRawAnimation == nullptr )
        {
            return Error( "Failed to read animation from source file" );
        }

        // Reflect raw animation data into runtime format
        //-------------------------------------------------------------------------

        AnimationClip animData;
        animData.m_pSkeleton = resourceDescriptor.m_pSkeleton;
        TransferAndCompressAnimationData( *pRawAnimation, animData );
        SetRootMotionData( *pRawAnimation, animData );

        // Handle events
        //-------------------------------------------------------------------------

        AnimationEventData eventData;
        if ( !CreateEventsData( ctx, resourceDescriptor, *pRawAnimation, eventData ) )
        {
            return CompilationFailed( ctx );
        }

        // Serialize animation data
        //-------------------------------------------------------------------------

        FileSystem::EnsurePathExists( ctx.m_outputFilePath );
        Serialization::BinaryFileArchive archive( Serialization::Mode::Write, ctx.m_outputFilePath );
        if ( archive.IsValid() )
        {
            Resource::ResourceHeader hdr( s_version, AnimationClip::GetStaticResourceTypeID() );
            hdr.AddInstallDependency( resourceDescriptor.m_pSkeleton.GetResourceID() );
            archive << hdr << animData;

            // Event Data
            archive << eventData.m_syncEventMarkers;
            archive << eventData.m_collection;

            if ( pRawAnimation->HasWarnings() )
            {
                return CompilationSucceededWithWarnings( ctx );
            }
            else
            {
                return CompilationSucceeded( ctx );
            }
        }
        else
        {
            return CompilationFailed( ctx );
        }
    }

    //-------------------------------------------------------------------------

    void AnimationClipCompiler::TransferAndCompressAnimationData( RawAssets::RawAnimation const& rawAnimData, AnimationClip& animClip ) const
    {
        auto const& rawTrackData = rawAnimData.GetTrackData();
        int32 const numBones = rawAnimData.GetNumBones();

        // Transfer basic anim data
        //-------------------------------------------------------------------------

        animClip.m_numFrames = rawAnimData.GetNumFrames();
        animClip.m_duration = ( animClip.IsSingleFrameAnimation() ) ? 0.0f : rawAnimData.GetDuration();

        // Compress raw data
        //-------------------------------------------------------------------------

        static constexpr float const defaultQuantizationRangeLength = 0.1f;

        for ( int32 boneIdx = 0; boneIdx < numBones; boneIdx++ )
        {
            TrackCompressionSettings trackSettings;

            // Record offset into data for this track
            trackSettings.m_trackStartIndex = (uint32) animClip.m_compressedPoseData.size();

            //-------------------------------------------------------------------------
            // Rotation
            //-------------------------------------------------------------------------

            for ( uint32 frameIdx = 0; frameIdx < animClip.m_numFrames; frameIdx++ )
            {
                Transform const& rawBoneTransform = rawTrackData[boneIdx].m_transforms[frameIdx];
                Quaternion const rotation = rawBoneTransform.GetRotation();

                Quantization::EncodedQuaternion const encodedQuat( rotation );
                animClip.m_compressedPoseData.push_back( encodedQuat.GetData0() );
                animClip.m_compressedPoseData.push_back( encodedQuat.GetData1() );
                animClip.m_compressedPoseData.push_back( encodedQuat.GetData2() );
            }

            //-------------------------------------------------------------------------
            // Translation
            //-------------------------------------------------------------------------

            auto const& rawTranslationValueRangeX = rawTrackData[boneIdx].m_translationValueRangeX;
            auto const& rawTranslationValueRangeY = rawTrackData[boneIdx].m_translationValueRangeY;
            auto const& rawTranslationValueRangeZ = rawTrackData[boneIdx].m_translationValueRangeZ;

            float const& rawTranslationValueRangeLengthX = rawTranslationValueRangeX.GetLength();
            float const& rawTranslationValueRangeLengthY = rawTranslationValueRangeY.GetLength();
            float const& rawTranslationValueRangeLengthZ = rawTranslationValueRangeZ.GetLength();

            // We could arguably compress more by saving each component individually at the cost of sampling performance. If we absolutely need more compression, we can do it here
            bool const isTranslationConstant = Math::IsNearZero( rawTranslationValueRangeLengthX ) && Math::IsNearZero( rawTranslationValueRangeLengthY ) && Math::IsNearZero( rawTranslationValueRangeLengthZ );
            if ( isTranslationConstant )
            {
                Transform const& rawBoneTransform = rawTrackData[boneIdx].m_transforms[0];
                Vector const& translation = rawBoneTransform.GetTranslation();

                trackSettings.m_translationRangeX = { translation.m_x, defaultQuantizationRangeLength };
                trackSettings.m_translationRangeY = { translation.m_y, defaultQuantizationRangeLength };
                trackSettings.m_translationRangeZ = { translation.m_z, defaultQuantizationRangeLength };
                trackSettings.m_isTranslationStatic = true;
            }
            else
            {
                trackSettings.m_translationRangeX = { rawTranslationValueRangeX.m_start, rawTranslationValueRangeLengthX };
                trackSettings.m_translationRangeY = { rawTranslationValueRangeY.m_start, rawTranslationValueRangeLengthY };
                trackSettings.m_translationRangeZ = { rawTranslationValueRangeZ.m_start, rawTranslationValueRangeLengthZ };
            }

            //-------------------------------------------------------------------------

            if ( trackSettings.IsTranslationTrackStatic() )
            {
                Transform const& rawBoneTransform = rawTrackData[boneIdx].m_transforms[0];
                Vector const& translation = rawBoneTransform.GetTranslation();

                uint16 const m_x = Quantization::EncodeFloat( translation.m_x, trackSettings.m_translationRangeX.m_rangeStart, trackSettings.m_translationRangeX.m_rangeLength );
                uint16 const m_y = Quantization::EncodeFloat( translation.m_y, trackSettings.m_translationRangeY.m_rangeStart, trackSettings.m_translationRangeY.m_rangeLength );
                uint16 const m_z = Quantization::EncodeFloat( translation.m_z, trackSettings.m_translationRangeZ.m_rangeStart, trackSettings.m_translationRangeZ.m_rangeLength );

                animClip.m_compressedPoseData.push_back( m_x );
                animClip.m_compressedPoseData.push_back( m_y );
                animClip.m_compressedPoseData.push_back( m_z );
            }
            else // Store all frames
            {
                for ( uint32 frameIdx = 0; frameIdx < animClip.m_numFrames; frameIdx++ )
                {
                    Transform const& rawBoneTransform = rawTrackData[boneIdx].m_transforms[frameIdx];
                    Vector const& translation = rawBoneTransform.GetTranslation();

                    uint16 const m_x = Quantization::EncodeFloat( translation.m_x, trackSettings.m_translationRangeX.m_rangeStart, trackSettings.m_translationRangeX.m_rangeLength );
                    uint16 const m_y = Quantization::EncodeFloat( translation.m_y, trackSettings.m_translationRangeY.m_rangeStart, trackSettings.m_translationRangeY.m_rangeLength );
                    uint16 const m_z = Quantization::EncodeFloat( translation.m_z, trackSettings.m_translationRangeZ.m_rangeStart, trackSettings.m_translationRangeZ.m_rangeLength );

                    animClip.m_compressedPoseData.push_back( m_x );
                    animClip.m_compressedPoseData.push_back( m_y );
                    animClip.m_compressedPoseData.push_back( m_z );
                }
            }

            //-------------------------------------------------------------------------
            // Scale
            //-------------------------------------------------------------------------

            auto const& rawScaleValueRangeX = rawTrackData[boneIdx].m_scaleValueRangeX;
            auto const& rawScaleValueRangeY = rawTrackData[boneIdx].m_scaleValueRangeY;
            auto const& rawScaleValueRangeZ = rawTrackData[boneIdx].m_scaleValueRangeZ;

            float const& rawScaleValueRangeLengthX = rawScaleValueRangeX.GetLength();
            float const& rawScaleValueRangeLengthY = rawScaleValueRangeY.GetLength();
            float const& rawScaleValueRangeLengthZ = rawScaleValueRangeZ.GetLength();

            // We could arguably compress more by saving each component individually at the cost of sampling performance. If we absolutely need more compression, we can do it here
            bool const isScaleConstant = Math::IsNearZero( rawScaleValueRangeLengthX ) && Math::IsNearZero( rawScaleValueRangeLengthY ) && Math::IsNearZero( rawScaleValueRangeLengthZ );
            if ( isScaleConstant )
            {
                Transform const& rawBoneTransform = rawTrackData[boneIdx].m_transforms[0];
                Vector const& scale = rawBoneTransform.GetScale();

                trackSettings.m_scaleRangeX = { scale.m_x, defaultQuantizationRangeLength };
                trackSettings.m_scaleRangeY = { scale.m_y, defaultQuantizationRangeLength };
                trackSettings.m_scaleRangeZ = { scale.m_z, defaultQuantizationRangeLength };
                trackSettings.m_isScaleStatic = true;
            }
            else
            {
                trackSettings.m_scaleRangeX = { rawScaleValueRangeX.m_start, rawScaleValueRangeLengthX };
                trackSettings.m_scaleRangeY = { rawScaleValueRangeY.m_start, rawScaleValueRangeLengthY };
                trackSettings.m_scaleRangeZ = { rawScaleValueRangeZ.m_start, rawScaleValueRangeLengthZ };
            }

            //-------------------------------------------------------------------------

            if ( trackSettings.IsScaleTrackStatic() )
            {
                Transform const& rawBoneTransform = rawTrackData[boneIdx].m_transforms[0];
                Vector const& scale = rawBoneTransform.GetScale();

                uint16 const m_x = Quantization::EncodeFloat( scale.m_x, trackSettings.m_scaleRangeX.m_rangeStart, trackSettings.m_scaleRangeX.m_rangeLength );
                uint16 const m_y = Quantization::EncodeFloat( scale.m_y, trackSettings.m_scaleRangeY.m_rangeStart, trackSettings.m_scaleRangeY.m_rangeLength );
                uint16 const m_z = Quantization::EncodeFloat( scale.m_z, trackSettings.m_scaleRangeZ.m_rangeStart, trackSettings.m_scaleRangeZ.m_rangeLength );

                animClip.m_compressedPoseData.push_back( m_x );
                animClip.m_compressedPoseData.push_back( m_y );
                animClip.m_compressedPoseData.push_back( m_z );
            }
            else // Store all frames
            {
                for ( uint32 frameIdx = 0; frameIdx < animClip.m_numFrames; frameIdx++ )
                {
                    Transform const& rawBoneTransform = rawTrackData[boneIdx].m_transforms[frameIdx];
                    Vector const& scale = rawBoneTransform.GetScale();

                    uint16 const m_x = Quantization::EncodeFloat( scale.m_x, trackSettings.m_scaleRangeX.m_rangeStart, trackSettings.m_scaleRangeX.m_rangeLength );
                    uint16 const m_y = Quantization::EncodeFloat( scale.m_y, trackSettings.m_scaleRangeY.m_rangeStart, trackSettings.m_scaleRangeY.m_rangeLength );
                    uint16 const m_z = Quantization::EncodeFloat( scale.m_z, trackSettings.m_scaleRangeZ.m_rangeStart, trackSettings.m_scaleRangeZ.m_rangeLength );

                    animClip.m_compressedPoseData.push_back( m_x );
                    animClip.m_compressedPoseData.push_back( m_y );
                    animClip.m_compressedPoseData.push_back( m_z );
                }
            }

            //-------------------------------------------------------------------------

            animClip.m_trackCompressionSettings.emplace_back( trackSettings );
        }
    }

    void AnimationClipCompiler::SetRootMotionData( RawAssets::RawAnimation const& rawAnimData, AnimationClip& animClip ) const
    {
        if ( animClip.GetNumFrames() <= 1 )
        {
            animClip.m_totalRootMotionDelta = Transform::Identity;
            animClip.m_averageLinearVelocity = 0;
            animClip.m_averageAngularVelocity = 0;
            return;
        }

        //-------------------------------------------------------------------------

        float distanceCovered = 0.0f;
        float rotationCovered = 0.0f;

        animClip.m_rootMotionTrack.reserve( animClip.GetNumFrames() );

        for ( auto i = 0u; i < animClip.GetNumFrames(); i++ )
        {
            // TEMP
            static Quaternion const tempRotation( EulerAngles( 0, 0, -180 ) );
            animClip.m_rootMotionTrack.emplace_back( Transform( tempRotation ) );

            // Track deltas
            if ( i > 0 )
            {
                Transform const deltaRoot = Transform::DeltaNoScale( animClip.m_rootMotionTrack[i - 1], animClip.m_rootMotionTrack[i] );
                distanceCovered += deltaRoot.GetTranslation().GetLength3();

                // We use the negative world forward since deltas are relative to the identity transform
                Vector const deltaForward2D = deltaRoot.GetForwardVector().GetNormalized2();
                Radians const deltaAngle = Vector::GetAngleBetweenVectors( deltaForward2D, Vector::WorldBackward ).GetClamped();
                rotationCovered += Math::Abs( deltaAngle );
            }
        }

        animClip.m_totalRootMotionDelta = Transform::DeltaNoScale( animClip.m_rootMotionTrack.front(), animClip.m_rootMotionTrack.back() );
        animClip.m_averageLinearVelocity = distanceCovered / animClip.GetDuration();
        animClip.m_averageAngularVelocity = rotationCovered / animClip.GetDuration();
    }

    //-------------------------------------------------------------------------

    bool AnimationClipCompiler::CreateEventsData( Resource::CompileContext const& ctx, AnimationClipResourceDescriptor const& animResourceDesc, RawAssets::RawAnimation const& rawAnimData, AnimationEventData& outEventData ) const
    {
        // Events are optional
        if ( !animResourceDesc.m_animationEventData.IsValid() )
        {
            return true;
        }

        // If the event data path is set ensure that it exists
        FileSystem::Path const eventFilePath = animResourceDesc.m_animationEventData.ToFileSystemPath( ctx.m_sourceDataPath );
        if ( !eventFilePath.Exists() )
        {
            Error( "Referenced event data file doesnt exist: %s", eventFilePath.c_str() );
            return false;
        }

        // Read event track data
        //-------------------------------------------------------------------------

        JsonFileReader jsonReader;
        if ( !jsonReader.ReadFromFile( eventFilePath ) )
        {
            Error( "Failed to read event track file: %s", eventFilePath.c_str() );
            return false;
        }

        TimelineData data;
        if ( !data.Load( ctx.m_typeRegistry, jsonReader.GetDocument() ) )
        {
            Error( "Malformed event track file: %s", eventFilePath.c_str() );
            return false;
        }

        // Reflect into runtime events
        //-------------------------------------------------------------------------

        int32 numSyncTracks = 0;
        TVector<Event*> events;
        FloatRange const animationTimeRange( 0, rawAnimData.GetDuration() );
        for ( auto pTrack : data.m_tracks )
        {
            auto pEventTrack = SafeCast<EventTrack>( pTrack );

            if ( pEventTrack->IsSyncTrack() )
            {
                numSyncTracks++;
            }

            for ( auto pItem : pTrack->m_items )
            {
                auto pEvent = SafeCast<EventItem>( pItem )->GetEvent();

                // Add event
                //-------------------------------------------------------------------------

                if ( !animationTimeRange.ContainsInclusive( pEvent->GetTimeRange() ) )
                {
                    Warning( "Event detected outside animation time range, event will be ignored" );
                    continue;
                }

                // TODO: Clamp event to animation length
                events.emplace_back( pEvent );

                // Create sync event
                //-------------------------------------------------------------------------

                if ( pEventTrack->IsSyncTrack() && numSyncTracks == 1 )
                {
                    outEventData.m_syncEventMarkers.emplace_back( SyncTrack::EventMarker( Percentage( pEvent->GetStartTime() / rawAnimData.GetDuration() ), pEvent->GetSyncEventID() ) );
                }
            }
        }

        if ( numSyncTracks > 1 )
        {
            Warning( "Multiple sync tracks detected, using the first one encountered!" );
        }

        // Transfer sorted events
        //-------------------------------------------------------------------------

        auto sortPredicate = [] ( Event* const& pEventA, Event* const& pEventB )
        {
            return pEventA->GetStartTime() < pEventB->GetStartTime();
        };

        eastl::sort( events.begin(), events.end(), sortPredicate );

        for ( auto const& pEvent : events )
        {
            TypeSystem::TypeDescriptor desc;
            TypeSystem::Serialization::CreateTypeDescriptorFromNativeType( ctx.m_typeRegistry, pEvent, desc );

            outEventData.m_collection.m_descriptors.emplace_back( desc );
        }

        eastl::sort( outEventData.m_syncEventMarkers.begin(), outEventData.m_syncEventMarkers.end() );

        // Free allocated memory
        //-------------------------------------------------------------------------

        data.Reset();

        return true;
    }
}