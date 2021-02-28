#pragma once
#include "CoreTypeIDs.h"
#include "System/Resource/ResourceTypeID.h"
#include "System/Resource/ResourceID.h"
#include "System/Resource/ResourcePtr.h"
#include "System/Core/Types/UUID.h"
#include "System/Core/Types/Color.h"
#include "System/Core/Types/StringID.h"
#include "System/Core/Math/Math.h"
#include "System/Core/FileSystem/DataPath.h"
#include "System/Core/Types/Containers.h"
#include "System/Core/Types/Percentage.h"
#include "System/Core/Math/Vector.h"
#include "System/Core/Math/Matrix.h"
#include "System/Core/Math/Transform.h"
#include "System/Core/Time/Time.h"

//-------------------------------------------------------------------------

namespace KRG::TypeSystem
{
    template<typename T> inline bool IsCoreType() { return false; }
    template<template<typename> typename C> inline bool IsCoreType() { return false; }

    //-------------------------------------------------------------------------\

    template<> inline bool IsCoreType<bool>() { return true; }
    template<> inline bool IsCoreType<S8>() { return true; }
    template<> inline bool IsCoreType<S16>() { return true; }
    template<> inline bool IsCoreType<S32>() { return true; }
    template<> inline bool IsCoreType<S64>() { return true; }
    template<> inline bool IsCoreType<U8>() { return true; }
    template<> inline bool IsCoreType<U16>() { return true; }
    template<> inline bool IsCoreType<U32>() { return true; }
    template<> inline bool IsCoreType<U64>() { return true; }
    template<> inline bool IsCoreType<F32>() { return true; }
    template<> inline bool IsCoreType<F64>() { return true; }
    template<> inline bool IsCoreType<UUID>() { return true; }
    template<> inline bool IsCoreType<StringID>() { return true; }
    template<> inline bool IsCoreType<String>() { return true; }
    template<> inline bool IsCoreType<Color>() { return true; }
    template<> inline bool IsCoreType<Float2>() { return true; }
    template<> inline bool IsCoreType<Float3>() { return true; }
    template<> inline bool IsCoreType<Float4>() { return true; }
    template<> inline bool IsCoreType<Vector>() { return true; }
    template<> inline bool IsCoreType<Quaternion>() { return true; }
    template<> inline bool IsCoreType<Matrix>() { return true; }
    template<> inline bool IsCoreType<Transform>() { return true; }
    template<> inline bool IsCoreType<Microseconds>() { return true; }
    template<> inline bool IsCoreType<Milliseconds>() { return true; }
    template<> inline bool IsCoreType<Seconds>() { return true; }
    template<> inline bool IsCoreType<Percentage>() { return true; }
    template<> inline bool IsCoreType<Degrees>() { return true; }
    template<> inline bool IsCoreType<Radians>() { return true; }
    template<> inline bool IsCoreType<EulerAngles >() { return true; }
    template<> inline bool IsCoreType<DataPath>() { return true; }
    template<> inline bool IsCoreType<ResourceTypeID>() { return true; }
    template<> inline bool IsCoreType<ResourceID>() { return true; }
    template<> inline bool IsCoreType<Resource::ResourcePtr>() { return true; }
    template<> inline bool IsCoreType<TVector>() { return true; }
    template<> inline bool IsCoreType<TResourcePtr>() { return true; }

    //-------------------------------------------------------------------------

    template<typename T> inline TypeID GetCoreType() { return TypeID(); }
    template<template<typename> typename C> inline TypeID GetCoreType() { return TypeID(); }

    template<> inline TypeID GetCoreType<bool>() { return GetCoreTypeID( CoreTypes::Bool ); }
    template<> inline TypeID GetCoreType<S8>() { return GetCoreTypeID( CoreTypes::S8 ); }
    template<> inline TypeID GetCoreType<S16>() { return GetCoreTypeID( CoreTypes::S16 ); }
    template<> inline TypeID GetCoreType<S32>() { return GetCoreTypeID( CoreTypes::S32 ); }
    template<> inline TypeID GetCoreType<S64>() { return GetCoreTypeID( CoreTypes::S64 ); }
    template<> inline TypeID GetCoreType<U8>() { return GetCoreTypeID( CoreTypes::U8 ); }
    template<> inline TypeID GetCoreType<U16>() { return GetCoreTypeID( CoreTypes::U16 ); }
    template<> inline TypeID GetCoreType<U32>() { return GetCoreTypeID( CoreTypes::U32 ); }
    template<> inline TypeID GetCoreType<U64>() { return GetCoreTypeID( CoreTypes::U64 ); }
    template<> inline TypeID GetCoreType<F32>() { return GetCoreTypeID( CoreTypes::F32 ); }
    template<> inline TypeID GetCoreType<F64>() { return GetCoreTypeID( CoreTypes::F64 ); }
    template<> inline TypeID GetCoreType<UUID>() { return GetCoreTypeID( CoreTypes::UUID ); }
    template<> inline TypeID GetCoreType<StringID>() { return GetCoreTypeID( CoreTypes::StringID ); }
    template<> inline TypeID GetCoreType<String>() { return GetCoreTypeID( CoreTypes::String ); }
    template<> inline TypeID GetCoreType<Color>() { return GetCoreTypeID( CoreTypes::Color ); }
    template<> inline TypeID GetCoreType<Float2>() { return GetCoreTypeID( CoreTypes::Float2 ); }
    template<> inline TypeID GetCoreType<Float3>() { return GetCoreTypeID( CoreTypes::Float3 ); }
    template<> inline TypeID GetCoreType<Float4>() { return GetCoreTypeID( CoreTypes::Float4 ); }
    template<> inline TypeID GetCoreType<Vector>() { return GetCoreTypeID( CoreTypes::Vector ); }
    template<> inline TypeID GetCoreType<Quaternion>() { return GetCoreTypeID( CoreTypes::Quaternion ); }
    template<> inline TypeID GetCoreType<Matrix>() { return GetCoreTypeID( CoreTypes::Matrix ); }
    template<> inline TypeID GetCoreType<Transform>() { return GetCoreTypeID( CoreTypes::Transform ); }
    template<> inline TypeID GetCoreType<Microseconds>() { return GetCoreTypeID( CoreTypes::Microseconds ); }
    template<> inline TypeID GetCoreType<Milliseconds>() { return GetCoreTypeID( CoreTypes::Milliseconds ); }
    template<> inline TypeID GetCoreType<Seconds>() { return GetCoreTypeID( CoreTypes::Seconds ); }
    template<> inline TypeID GetCoreType<Percentage>() { return GetCoreTypeID( CoreTypes::Percentage ); }
    template<> inline TypeID GetCoreType<Degrees>() { return GetCoreTypeID( CoreTypes::Degrees ); }
    template<> inline TypeID GetCoreType<Radians>() { return GetCoreTypeID( CoreTypes::Radians ); }
    template<> inline TypeID GetCoreType<EulerAngles >() { return GetCoreTypeID( CoreTypes::EulerAngles ); }
    template<> inline TypeID GetCoreType<DataPath>() { return GetCoreTypeID( CoreTypes::DataPath ); }
    template<> inline TypeID GetCoreType<ResourceTypeID>() { return GetCoreTypeID( CoreTypes::ResourceTypeID ); }
    template<> inline TypeID GetCoreType<ResourceID>() { return GetCoreTypeID( CoreTypes::ResourceID ); }
    template<> inline TypeID GetCoreType<Resource::ResourcePtr>() { return GetCoreTypeID( CoreTypes::ResourcePtr ); }
    template<> inline TypeID GetCoreType<TVector>() { return GetCoreTypeID( CoreTypes::TVector ); }
    template<> inline TypeID GetCoreType<TResourcePtr>() { return GetCoreTypeID( CoreTypes::TResourcePtr ); }
}