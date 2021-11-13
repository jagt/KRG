#include "CoreTypeConversions.h"
#include "TypeRegistry.h"
#include "TypeInfo.h"
#include "System/Resource/ResourcePtr.h"
#include "System/Core/Serialization/BinaryArchive.h"
#include "System/Core/Time/Time.h"
#include "System/Core/Types/Color.h"
#include "System/Core/Types/UUID.h"
#include "System/Core/Types/StringID.h"
#include "System/Core/Types/BitFlags.h"
#include "System/Core/Types/Percentage.h"
#include "System/Core/Math/Transform.h"
#include "System/Core/Logging/Log.h"

//-------------------------------------------------------------------------

namespace KRG::TypeSystem::Conversion
{
    void StringToFloatArray( String const& str, int32 const numFloats, float* pFloats )
    {
        char substr[128] = { 0 };
        int32 resIdx = 0;

        bool complete = false;
        size_t startIdx = 0;
        while ( resIdx < numFloats && !complete )
        {
            size_t endIdx = str.find_first_of( ',', startIdx );
            if ( endIdx == String::npos )
            {
                endIdx = str.length();
                complete = true;
            }

            size_t sizeToCopy = endIdx - startIdx;
            KRG_ASSERT( sizeToCopy < 128 );
            memcpy( substr, &str.c_str()[startIdx], sizeToCopy );
            substr[sizeToCopy] = '\0';

            pFloats[resIdx++] = std::strtof( substr, nullptr );
            startIdx = endIdx + 1;
        }
    }

    void FloatArrayToString( float const* pFloats, int32 const numFloats, String& strValue )
    {
        strValue.clear();

        for ( int32 i = 0; i < numFloats; i++ )
        {
            strValue += eastl::to_string( pFloats[i] );

            if ( i != ( numFloats - 1 ) )
            {
                strValue += ',';
            }
        }
    }

    void StringToIntArray( String const& str, int32 const numInts, int32* pInts )
    {
        char substr[128] = { 0 };
        int32 resIdx = 0;

        bool complete = false;
        size_t startIdx = 0;
        while ( resIdx < numInts && !complete )
        {
            size_t endIdx = str.find_first_of( ',', startIdx );
            if ( endIdx == String::npos )
            {
                endIdx = str.length();
                complete = true;
            }

            size_t sizeToCopy = endIdx - startIdx;
            KRG_ASSERT( sizeToCopy < 128 );
            memcpy( substr, &str.c_str()[startIdx], sizeToCopy );
            substr[sizeToCopy] = '\0';

            pInts[resIdx++] = std::strtoul( substr, nullptr, 0 );
            startIdx = endIdx + 1;
        }
    }

    void IntArrayToString( int32 const* pInts, int32 const numInts, String& strValue )
    {
        strValue.clear();

        for ( int32 i = 0; i < numInts; i++ )
        {
            strValue += eastl::to_string( pInts[i] );

            if ( i != ( numInts - 1 ) )
            {
                strValue += ',';
            }
        }
    }

    inline static bool ConvertStringToBitFlags( EnumInfo const& enumInfo, String const& str, BitFlags& outFlags )
    {
        outFlags.ClearAllFlags();

        if ( str.empty() )
        {
            return true;
        }

        // Handle hex format explicitly
        //-------------------------------------------------------------------------

        if ( str.find_first_of( "0x" ) == 0 || str.find_first_of( "0X" ) == 0 )
        {
            uint32 value = std::strtoul( str.c_str(), nullptr, 16 );
            outFlags.Set( value );
            return true;
        }

        //-------------------------------------------------------------------------

        char buffer[256] = { 0 };
        size_t bufferIdx = 0;
        bool isReadingEnumValue = false;
        bool hasReadWhiteSpace = false;

        //-------------------------------------------------------------------------

        auto ProcessReadValue = [&] ()
        {
            if ( isReadingEnumValue )
            {
                buffer[bufferIdx] = 0;
                bufferIdx = 0;
                isReadingEnumValue = false;

                int64 flag;
                if ( enumInfo.TryGetConstantValue( StringID( buffer ), flag ) )
                {
                    outFlags.SetFlag( (uint8) flag, true );
                    return true;
                }
            }

            return false;
        };

        //-------------------------------------------------------------------------

        size_t const length = str.length();
        for ( auto i = 0u; i < length; i++ )
        {
            if ( str[i] == '|' )
            {
                if ( !ProcessReadValue() )
                {
                    outFlags.ClearAllFlags();
                    return false;
                }
                hasReadWhiteSpace = false;
            }
            else if ( str[i] == ' ' )
            {
                if ( isReadingEnumValue )
                {
                    hasReadWhiteSpace = true;
                }
            }
            else
            {
                // We read a white space while reading an enum value
                if ( hasReadWhiteSpace )
                {
                    outFlags.ClearAllFlags();
                    return false;
                }

                isReadingEnumValue = true;
                buffer[bufferIdx] = str[i];
                bufferIdx++;
                KRG_ASSERT( bufferIdx < 256 );
            }
        }

        //-------------------------------------------------------------------------

        if ( !ProcessReadValue() )
        {
            outFlags.ClearAllFlags();
            return false;
        }

        //-------------------------------------------------------------------------

        return true;
    }

    inline static bool ConvertBitFlagsToString( EnumInfo const& enumInfo, BitFlags const& flags, String& strValue )
    {
        strValue.clear();

        for ( auto i = 0u; i < 32; i++ )
        {
            if ( flags.IsFlagSet( (uint8) i ) )
            {
                StringID label;
                if ( !enumInfo.TryGetConstantLabel( i, label ) )
                {
                    strValue.clear();
                    return false;
                }

                strValue += label.ToString();
                strValue += "|";
            }
        }

        //-------------------------------------------------------------------------

        if ( !strValue.empty() )
        {
            strValue.erase( strValue.end() - 1, strValue.end() );
        }

        return true;
    }

    //-------------------------------------------------------------------------

    void ConvertStringToNativeType( TypeRegistry const& typeRegistry, TypeID typeID, TypeID templateArgumentTypeID, String const& str, void* pValue )
    {
        // Enums
        if( !IsCoreType( typeID ) ) 
        {
            EnumInfo const* pEnumInfo = typeRegistry.GetEnumInfo( typeID );
            KRG_ASSERT( pEnumInfo != nullptr );
            
            StringID const enumID( str );
            int64 const enumValue = pEnumInfo->GetConstantValue( enumID );

            // We only support up to 32 bit enum types...
            switch ( pEnumInfo->m_underlyingType )
            {
                case CoreTypeID::Uint8:
                {
                    *( (uint8*) pValue ) = (uint8) enumValue;
                }
                break;

                case CoreTypeID::Int8:
                {
                    *( (int8*) pValue ) = (int8) enumValue;
                }
                break;

                case CoreTypeID::Uint16:
                {
                    *( (uint16*) pValue ) = (uint16) enumValue;
                }
                break;

                case CoreTypeID::Int16:
                {
                    *( (int16*) pValue ) = (int16) enumValue;
                }
                break;

                case CoreTypeID::Uint32:
                {
                    *( (uint32*) pValue ) = (uint32) enumValue;
                }
                break;

                case CoreTypeID::Int32:
                {
                    *( (int32*) pValue ) = (int32) enumValue;
                }
                break;

                default:
                {
                    KRG_UNREACHABLE_CODE();
                }
                break;
            }
        }
        else // Real core types
        {
            CoreTypeID const typeToConvert = GetCoreType( typeID );
            switch ( typeToConvert )
            {
                case CoreTypeID::Bool :
                {
                    String lowerString = str;
                    lowerString.make_lower();
                    *reinterpret_cast<bool*>( pValue ) = ( lowerString == "true" );
                }
                break;

                case CoreTypeID::Uint8 :
                {
                    *reinterpret_cast<uint8*>( pValue ) = (uint8) std::strtoul( str.c_str(), nullptr, 0 );
                }
                break;

                case CoreTypeID::Int8 :
                {
                    *reinterpret_cast<int8*>( pValue ) = (int8) std::strtol( str.c_str(), nullptr, 0 );
                }
                break;

                case CoreTypeID::Uint16 :
                {
                    *reinterpret_cast<uint16*>( pValue ) = (uint16) std::strtoul( str.c_str(), nullptr, 0 );
                }
                break;

                case CoreTypeID::Int16 :
                {
                    *reinterpret_cast<int16*>( pValue ) = (int16) std::strtol( str.c_str(), nullptr, 0 );
                }
                break;

                case CoreTypeID::Uint32 :
                {
                    *reinterpret_cast<uint32*>( pValue ) = std::strtoul( str.c_str(), nullptr, 0 );
                }
                break;

                case CoreTypeID::Int32 :
                {
                    *reinterpret_cast<int32*>( pValue ) = std::strtol( str.c_str(), nullptr, 0 );
                }
                break;

                case CoreTypeID::Uint64 :
                {
                    *reinterpret_cast<uint64*>( pValue ) = std::strtoull( str.c_str(), nullptr, 0 );
                }
                break;

                case CoreTypeID::Int64:
                {
                    *reinterpret_cast<int64*>( pValue ) = std::strtol( str.c_str(), nullptr, 0 );
                }
                break;

                case CoreTypeID::Float :
                {
                    *reinterpret_cast<float*>( pValue ) = std::strtof( str.c_str(), nullptr );
                }
                break;

                case CoreTypeID::Double :
                {
                    *reinterpret_cast<double*>( pValue ) = std::strtod( str.c_str(), nullptr );
                }
                break;

                case CoreTypeID::String :
                {
                    *reinterpret_cast<String*>( pValue ) = str;
                }
                break;

                case CoreTypeID::StringID :
                {
                    *reinterpret_cast<StringID*>( pValue ) = StringID( str.c_str() );
                }
                break;

                case CoreTypeID::TypeID:
                {
                    *reinterpret_cast<TypeID*>( pValue ) = TypeID( str.c_str() );
                }
                break;

                case CoreTypeID::UUID :
                {
                    *reinterpret_cast<UUID*>( pValue ) = UUID( str );
                }
                break;

                case CoreTypeID::Color :
                {
                    uint32 const colorType = std::strtoul( str.c_str(), nullptr, 16 );
                    *reinterpret_cast<Color*>( pValue ) = Color( colorType );
                }
                break;

                case CoreTypeID::Float2 :
                {
                    StringToFloatArray( str, 2, &reinterpret_cast<Float2*>( pValue )->m_x );
                }
                break;

                case CoreTypeID::Float3:
                {
                    StringToFloatArray( str, 3, &reinterpret_cast<Float3*>( pValue )->m_x );
                }
                break;

                case CoreTypeID::Float4:
                {
                    StringToFloatArray( str, 4, &reinterpret_cast<Float4*>( pValue )->m_x );
                }
                break;

                case CoreTypeID::Vector:
                {
                    StringToFloatArray( str, 4, &reinterpret_cast<Vector*>( pValue )->m_x );
                }
                break;

                case CoreTypeID::Quaternion:
                {
                    StringToFloatArray( str, 4, &reinterpret_cast<Quaternion*>( pValue )->m_x );
                }
                break;

                case CoreTypeID::Matrix:
                {
                    float floatData[9];
                    StringToFloatArray( str, 9, floatData );

                    EulerAngles const rotation( floatData[0], floatData[1], floatData[2] );
                    Vector const translation = Vector( floatData[3], floatData[4], floatData[5] );
                    Vector const scale = Vector( floatData[6], floatData[7], floatData[8] );
                    *reinterpret_cast<Matrix*>( pValue ) = Matrix( Quaternion( rotation ), translation, scale );
                }
                break;

                case CoreTypeID::Transform:
                {
                    float floatData[9];
                    StringToFloatArray( str, 9, floatData );

                    EulerAngles const rotation( floatData[0], floatData[1], floatData[2] );
                    Vector const translation( floatData[3], floatData[4], floatData[5] );
                    Vector const scale( floatData[6], floatData[7], floatData[8] );
                    *reinterpret_cast<Transform*>( pValue ) = Transform( Quaternion( rotation ), translation, scale );
                }
                break;

                case CoreTypeID::EulerAngles:
                {
                    Float3 angles;
                    StringToFloatArray( str, 3, &angles.m_x );
                    *reinterpret_cast<EulerAngles*>( pValue ) = EulerAngles( angles.m_x, angles.m_y, angles.m_z );
                }
                break;

                case CoreTypeID::Microseconds:
                {
                    *reinterpret_cast<Microseconds*>( pValue ) = Microseconds( std::strtof( str.c_str(), nullptr ) );
                }
                break;

                case CoreTypeID::Milliseconds:
                {
                    *reinterpret_cast<Milliseconds*>( pValue ) = Milliseconds( std::strtof( str.c_str(), nullptr ) );
                }
                break;

                case CoreTypeID::Seconds:
                {
                    *reinterpret_cast<Seconds*>( pValue ) = Seconds( std::strtof( str.c_str(), nullptr ) );
                }
                break;

                case CoreTypeID::Percentage:
                {
                    *reinterpret_cast<Percentage*>( pValue ) = Percentage( std::strtof( str.c_str(), nullptr ) );
                }
                break;

                case CoreTypeID::Degrees:
                {
                    *reinterpret_cast<Degrees*>( pValue ) = Degrees( std::strtof( str.c_str(), nullptr ) );
                }
                break;

                case CoreTypeID::Radians:
                {
                    *reinterpret_cast<Radians*>( pValue ) = Radians( std::strtof( str.c_str(), nullptr ) );
                }
                break;

                case CoreTypeID::ResourcePath:
                {
                    *reinterpret_cast<ResourcePath*>( pValue ) = ResourcePath( str );
                }
                break;

                case CoreTypeID::IntRange:
                {
                    int32 intData[2];
                    StringToIntArray( str, 2, intData );
                    *reinterpret_cast<IntRange*>( pValue ) = IntRange( intData[0], intData[1] );
                }
                break;

                case CoreTypeID::FloatRange:
                {
                    float floatData[2];
                    StringToFloatArray( str, 2, floatData );
                    *reinterpret_cast<FloatRange*>( pValue ) = FloatRange( floatData[0], floatData[1] );
                }
                break;

                case CoreTypeID::ResourceTypeID:
                {
                    *reinterpret_cast<ResourceTypeID*>( pValue ) = ResourceTypeID( str );
                }
                break;

                case CoreTypeID::ResourcePtr:
                case CoreTypeID::TResourcePtr:
                {
                    Resource::ResourcePtr& resourcePtr = *reinterpret_cast<Resource::ResourcePtr*>( pValue );
                    if ( str.empty() )
                    {
                        resourcePtr = Resource::ResourcePtr();
                    }
                    else
                    {
                        KRG_ASSERT( ResourceID::IsValidResourceIDString( str ) );
                        ResourceID const ID( str );
                        resourcePtr = Resource::ResourcePtr( ID );
                    }
                }
                break;

                case CoreTypeID::ResourceID:
                {
                    if ( str.empty() )
                    {
                        *reinterpret_cast<ResourceID*>( pValue ) = ResourceID();
                    }
                    else
                    {
                        KRG_ASSERT( ResourceID::IsValidResourceIDString( str ) );
                        *reinterpret_cast<ResourceID*>( pValue ) = ResourceID( str );
                    }
                }
                break;

                case CoreTypeID::BitFlags:
                {
                    reinterpret_cast<BitFlags*>( pValue )->Set( std::strtol( str.c_str(), nullptr, 0 ) );
                }
                break;

                case CoreTypeID::TBitFlags:
                {
                    EnumInfo const* pEnumInfo = typeRegistry.GetEnumInfo( templateArgumentTypeID );
                    if ( pEnumInfo == nullptr )
                    {
                        KRG_LOG_WARNING( "TypeSystem", "Unknown enum class (%s) for TBitFlags", templateArgumentTypeID.ToStringID().c_str() );
                        return;
                    }

                    if ( !ConvertStringToBitFlags( *pEnumInfo, str, *reinterpret_cast<BitFlags*>( pValue ) ) )
                    {
                        KRG_LOG_WARNING( "TypeSystem", "Failed to convert string (%s) into valid TBitFlags<%s>", str.c_str(), templateArgumentTypeID.ToStringID().c_str() );
                    }
                }
                break;

                default:
                {
                    KRG_UNREACHABLE_CODE();
                }
                break;
            }
        }
    }

    void ConvertNativeTypeToString( TypeRegistry const& typeRegistry, TypeID typeID, TypeID templateArgumentTypeID, void const* pValue, String & strValue )
    {
        // Enums
        if ( !IsCoreType( typeID ) )
        {
            EnumInfo const* pEnumInfo = typeRegistry.GetEnumInfo( typeID );
            KRG_ASSERT( pEnumInfo != nullptr );

            // We only support up to 32 bit enum types...
            switch ( pEnumInfo->m_underlyingType )
            {
                case CoreTypeID::Uint8:
                {
                    strValue = pEnumInfo->GetConstantLabel( *( (uint8*) pValue ) ).c_str();
                }
                break;

                case CoreTypeID::Int8:
                {
                    strValue = pEnumInfo->GetConstantLabel( *( (int8*) pValue ) ).c_str();
                }
                break;

                case CoreTypeID::Uint16:
                {
                    strValue = pEnumInfo->GetConstantLabel( *( (uint16*) pValue ) ).c_str();
                }
                break;

                case CoreTypeID::Int16:
                {
                    strValue = pEnumInfo->GetConstantLabel( *( (int16*) pValue ) ).c_str();
                }
                break;

                case CoreTypeID::Uint32:
                {
                    strValue = pEnumInfo->GetConstantLabel( *( (uint32*) pValue ) ).c_str();
                }
                break;

                case CoreTypeID::Int32:
                {
                    strValue = pEnumInfo->GetConstantLabel( *( (int32*) pValue ) ).c_str();
                }
                break;

                default:
                {
                    KRG_UNREACHABLE_CODE();
                }
                break;
            }
        }
        else  // Real core types
        {
            CoreTypeID const typeToConvert = GetCoreType( typeID );
            switch ( typeToConvert )
            {
                case CoreTypeID::Bool:
                {
                    strValue = *reinterpret_cast<bool const*>( pValue ) ? "True" : "False";
                }
                break;

                case CoreTypeID::Uint8:
                {
                    strValue = eastl::to_string( *reinterpret_cast<uint8 const*>( pValue ) );
                }
                break;

                case CoreTypeID::Int8:
                {
                    strValue = eastl::to_string( *reinterpret_cast<int8 const*>( pValue ) );
                }
                break;

                case CoreTypeID::Uint16:
                {
                    strValue = eastl::to_string( *reinterpret_cast<uint16 const*>( pValue ) );
                }
                break;

                case CoreTypeID::Int16:
                {
                    strValue = eastl::to_string( *reinterpret_cast<int16 const*>( pValue ) );
                }
                break;

                case CoreTypeID::Uint32:
                {
                    strValue = eastl::to_string( *reinterpret_cast<uint32 const*>( pValue ) );
                }
                break;

                case CoreTypeID::Int32:
                {
                    strValue = eastl::to_string( *reinterpret_cast<int32 const*>( pValue ) );
                }
                break;

                case CoreTypeID::Uint64:
                {
                    strValue = eastl::to_string( *reinterpret_cast<uint64 const*>( pValue ) );
                }
                break;

                case CoreTypeID::Int64:
                {
                    strValue = eastl::to_string( *reinterpret_cast<int64 const*>( pValue ) );
                }
                break;

                case CoreTypeID::Float:
                {
                    strValue = eastl::to_string( *reinterpret_cast<float const*>( pValue ) );
                }
                break;

                case CoreTypeID::Double:
                {
                    strValue = eastl::to_string( *reinterpret_cast<double const*>( pValue ) );
                }
                break;

                case CoreTypeID::String:
                {
                    strValue = *reinterpret_cast<String const*>( pValue );
                }
                break;

                case CoreTypeID::StringID:
                {
                    char const* pStr = reinterpret_cast<StringID const*>( pValue )->ToString();
                    KRG_ASSERT( pStr != nullptr );
                    strValue = pStr;
                }
                break;

                case CoreTypeID::TypeID:
                {
                    char const* pStr = reinterpret_cast<TypeID const*>( pValue )->ToStringID().c_str();
                    KRG_ASSERT( pStr != nullptr );
                    strValue = pStr;
                }
                break;

                case CoreTypeID::UUID:
                {
                    strValue = reinterpret_cast<UUID const*>( pValue )->ToString();
                }
                break;

                case CoreTypeID::Color:
                {
                    Color const& value = *reinterpret_cast<Color const*>( pValue );
                    strValue.sprintf( "%02X%02X%02X%02X", value.m_byteColor.m_r, value.m_byteColor.m_g, value.m_byteColor.m_b, value.m_byteColor.m_a );
                }
                break;

                case CoreTypeID::Float2:
                {
                    FloatArrayToString( &reinterpret_cast<Float2 const*>( pValue )->m_x, 2, strValue );
                }
                break;

                case CoreTypeID::Float3:
                {
                    FloatArrayToString( &reinterpret_cast<Float3 const*>( pValue )->m_x, 3, strValue );
                }
                break;

                case CoreTypeID::Float4:
                {
                    FloatArrayToString( &reinterpret_cast<Float4 const*>( pValue )->m_x, 4, strValue );
                }
                break;

                case CoreTypeID::Vector:
                {
                    FloatArrayToString( &reinterpret_cast<Vector const*>( pValue )->m_x, 4, strValue );
                }
                break;

                case CoreTypeID::Quaternion:
                {
                    FloatArrayToString( &reinterpret_cast<Quaternion const*>( pValue )->m_x, 4, strValue );
                }
                break;

                case CoreTypeID::Matrix:
                {
                    Matrix const& value = *reinterpret_cast<Matrix const*>( pValue );

                    // Handle uninitialized matrix
                    if ( value.IsOrthonormal() )
                    {
                        auto eulerAngles = value.ToEulerAngles();

                        float floatData[9];
                        (Float3&) floatData = Float3( (float) eulerAngles.m_x.ToDegrees(), (float) eulerAngles.m_y.ToDegrees(), (float) eulerAngles.m_z.ToDegrees() );
                        (Float3&) floatData[3] = value.GetTranslation().ToFloat3();
                        (Float3&) floatData[6] = value.GetScale().ToFloat3();

                        FloatArrayToString( floatData, 9, strValue );
                    }
                    else
                    {
                        strValue = "0,0,0,0,0,0,0,0,0";
                    }
                }
                break;

                case CoreTypeID::Transform:
                {
                    Transform const& transform = *reinterpret_cast<Transform const*>( pValue );
                    auto eulerAngles = transform.GetRotation().ToEulerAngles();

                    float floatData[9];

                    floatData[0] = (float) eulerAngles.m_x.ToDegrees();
                    floatData[1] = (float) eulerAngles.m_y.ToDegrees();
                    floatData[2] = (float) eulerAngles.m_z.ToDegrees();

                    floatData[3] = transform.GetTranslation().m_x;
                    floatData[4] = transform.GetTranslation().m_y;
                    floatData[5] = transform.GetTranslation().m_z;

                    floatData[6] = transform.GetScale().m_x;
                    floatData[7] = transform.GetScale().m_y;
                    floatData[8] = transform.GetScale().m_z;

                    //-------------------------------------------------------------------------

                    FloatArrayToString( floatData, 9, strValue );
                }
                break;

                case CoreTypeID::EulerAngles:
                {
                    Float3 const angles = reinterpret_cast<EulerAngles const*>( pValue )->GetAsDegrees();
                    FloatArrayToString( &angles.m_x, 3, strValue );
                }
                break;

                case CoreTypeID::Microseconds:
                {
                    strValue = eastl::to_string( reinterpret_cast<Microseconds const*>( pValue )->ToFloat() );
                }
                break;

                case CoreTypeID::Milliseconds:
                {
                    strValue = eastl::to_string( reinterpret_cast<Milliseconds const*>( pValue )->ToFloat() );
                }
                break;

                case CoreTypeID::Seconds:
                {
                    strValue = eastl::to_string( reinterpret_cast<Seconds const*>( pValue )->ToFloat() );
                }
                break;

                case CoreTypeID::Percentage:
                {
                    strValue = eastl::to_string( reinterpret_cast<Percentage const*>( pValue )->ToFloat() );
                }
                break;

                case CoreTypeID::Degrees:
                {
                    strValue = eastl::to_string( reinterpret_cast<Degrees const*>( pValue )->ToFloat() );
                }
                break;

                case CoreTypeID::Radians:
                {
                    strValue = eastl::to_string( reinterpret_cast<Radians const*>( pValue )->ToFloat() );
                }
                break;

                case CoreTypeID::ResourcePath:
                {
                    strValue = reinterpret_cast<ResourcePath const*>( pValue )->c_str();
                }
                break;

                case CoreTypeID::IntRange:
                {
                    IntRange const* pRange = reinterpret_cast<IntRange const*>( pValue );
                    IntArrayToString( &pRange->m_start, 3, strValue );
                }
                break;

                case CoreTypeID::FloatRange:
                {
                    FloatRange const* pRange = reinterpret_cast<FloatRange const*>( pValue );
                    FloatArrayToString( &pRange->m_start, 3, strValue );
                }
                break;

                case CoreTypeID::ResourceTypeID:
                {
                    strValue = reinterpret_cast<ResourceTypeID const*>( pValue )->ToString();
                }
                break;

                case CoreTypeID::ResourcePtr:
                case CoreTypeID::TResourcePtr:
                {
                    strValue = reinterpret_cast<Resource::ResourcePtr const*>( pValue )->GetResourceID().ToString();
                }
                break;

                case CoreTypeID::ResourceID:
                {
                    strValue = reinterpret_cast<ResourceID const*>( pValue )->ToString();
                }
                break;

                case CoreTypeID::BitFlags:
                {
                    strValue = eastl::to_string( reinterpret_cast<BitFlags const*>( pValue )->Get() );
                }
                break;

                case CoreTypeID::TBitFlags:
                {
                    EnumInfo const* pEnumInfo = typeRegistry.GetEnumInfo( templateArgumentTypeID );
                    if ( pEnumInfo == nullptr )
                    {
                        KRG_LOG_WARNING( "TypeSystem", "Unknown enum class (%s) for TBitFlags", templateArgumentTypeID.ToStringID().c_str() );
                        return;
                    }

                    if ( !ConvertBitFlagsToString( *pEnumInfo, *reinterpret_cast<BitFlags const*>( pValue ), strValue ) )
                    {
                        KRG_LOG_WARNING( "TypeSystem", "Failed to convert string (%s) into valid TBitFlags<%s>", strValue.c_str(), templateArgumentTypeID.ToStringID().c_str() );
                    }
                }
                break;

                default:
                {
                    KRG_UNREACHABLE_CODE();
                }
                break;
            }
        }
    }

    void ConvertNativeTypeToBinary( TypeRegistry const& typeRegistry, TypeID typeID, TypeID templateArgumentTypeID, void const* pValue, TVector<Byte>& byteArray )
    {
        Serialization::BinaryMemoryArchive archive( Serialization::Mode::Write, byteArray );

        // Enums
        if ( !IsCoreType( typeID ) )
        {
            EnumInfo const* pEnumInfo = typeRegistry.GetEnumInfo( typeID );
            KRG_ASSERT( pEnumInfo != nullptr );

            // We only support up to 32 bit enum types...
            switch ( pEnumInfo->m_underlyingType )
            {
                case CoreTypeID::Uint8:
                {
                    archive << *reinterpret_cast<uint8 const*>( pValue );
                }
                break;

                case CoreTypeID::Int8:
                {
                    archive << *reinterpret_cast<int8 const*>( pValue );
                }
                break;

                case CoreTypeID::Uint16:
                {
                    archive << *reinterpret_cast<uint16 const*>( pValue );
                }
                break;

                case CoreTypeID::Int16:
                {
                    archive << *reinterpret_cast<int16 const*>( pValue );
                }
                break;

                case CoreTypeID::Uint32:
                {
                    archive << *reinterpret_cast<uint32 const*>( pValue );
                }
                break;

                case CoreTypeID::Int32:
                {
                    archive << *reinterpret_cast<int32 const*>( pValue );
                }
                break;

                default:
                {
                    KRG_UNREACHABLE_CODE();
                }
                break;
            }
        }
        else  // Real core types
        {
            CoreTypeID const typeToConvert = GetCoreType( typeID );
            switch ( typeToConvert )
            {
                case CoreTypeID::Bool:
                {
                    archive << *reinterpret_cast<bool const*>( pValue );
                }
                break;

                case CoreTypeID::Uint8:
                {
                    archive << *reinterpret_cast<uint8 const*>( pValue );
                }
                break;

                case CoreTypeID::Int8:
                {
                    archive << *reinterpret_cast<int8 const*>( pValue );
                }
                break;

                case CoreTypeID::Uint16:
                {
                    archive << *reinterpret_cast<uint16 const*>( pValue );
                }
                break;

                case CoreTypeID::Int16:
                {
                    archive << *reinterpret_cast<int16 const*>( pValue );
                }
                break;

                case CoreTypeID::Uint32:
                {
                    archive << *reinterpret_cast<uint32 const*>( pValue );
                }
                break;

                case CoreTypeID::Int32:
                {
                    archive << *reinterpret_cast<int32 const*>( pValue );
                }
                break;

                case CoreTypeID::Uint64:
                {
                    archive << *reinterpret_cast<uint64 const*>( pValue );
                }
                break;

                case CoreTypeID::Int64:
                {
                    archive << *reinterpret_cast<int64 const*>( pValue );
                }
                break;

                case CoreTypeID::Float:
                {
                    archive << *reinterpret_cast<float const*>( pValue );
                }
                break;

                case CoreTypeID::Double:
                {
                    archive << *reinterpret_cast<double const*>( pValue );
                }
                break;

                case CoreTypeID::String:
                {
                    archive << *reinterpret_cast<String const*>( pValue );
                }
                break;

                case CoreTypeID::StringID:
                {
                    archive << *reinterpret_cast<StringID const*>( pValue );
                }
                break;

                case CoreTypeID::TypeID:
                {
                    archive << reinterpret_cast<TypeID const*>( pValue )->ToStringID();
                }
                break;

                case CoreTypeID::UUID:
                {
                    archive << *reinterpret_cast<UUID const*>( pValue );
                }
                break;

                case CoreTypeID::Color:
                {
                    archive << *reinterpret_cast<Color const*>( pValue );
                }
                break;

                case CoreTypeID::Float2:
                {
                    archive << *reinterpret_cast<Float2 const*>( pValue );
                }
                break;

                case CoreTypeID::Float3:
                {
                    archive << *reinterpret_cast<Float3 const*>( pValue );
                }
                break;

                case CoreTypeID::Float4:
                {
                    archive << *reinterpret_cast<Float4 const*>( pValue );
                }
                break;

                case CoreTypeID::Vector:
                {
                    archive << *reinterpret_cast<Vector const*>( pValue );
                }
                break;

                case CoreTypeID::Quaternion:
                {
                    archive << *reinterpret_cast<Quaternion const*>( pValue );
                }
                break;

                case CoreTypeID::Matrix:
                {
                    archive << *reinterpret_cast<Matrix const*>( pValue );
                }
                break;

                case CoreTypeID::Transform:
                {
                    archive << *reinterpret_cast<Transform const*>( pValue );
                }
                break;

                case CoreTypeID::EulerAngles:
                {
                    archive << *reinterpret_cast<EulerAngles const*>( pValue );
                }
                break;

                case CoreTypeID::Microseconds:
                {
                    archive << *reinterpret_cast<Microseconds const*>( pValue );
                }
                break;

                case CoreTypeID::Milliseconds:
                {
                    archive << *reinterpret_cast<Milliseconds const*>( pValue );
                }
                break;

                case CoreTypeID::Seconds:
                {
                    archive << *reinterpret_cast<Seconds const*>( pValue );
                }
                break;

                case CoreTypeID::Percentage:
                {
                    archive << *reinterpret_cast<Percentage const*>( pValue );
                }
                break;

                case CoreTypeID::Degrees:
                {
                    archive << *reinterpret_cast<Degrees const*>( pValue );
                }
                break;

                case CoreTypeID::Radians:
                {
                    archive << *reinterpret_cast<Radians const*>( pValue );
                }
                break;

                case CoreTypeID::ResourcePath:
                {
                    archive << *reinterpret_cast<ResourcePath const*>( pValue );
                }
                break;

                case CoreTypeID::IntRange:
                {
                    archive << *reinterpret_cast<IntRange const*>( pValue );
                }
                break;

                case CoreTypeID::FloatRange:
                {
                    archive << *reinterpret_cast<FloatRange const*>( pValue );
                }
                break;

                case CoreTypeID::ResourceTypeID:
                {
                    archive << *reinterpret_cast<ResourceTypeID const*>( pValue );
                }
                break;

                case CoreTypeID::ResourcePtr:
                case CoreTypeID::TResourcePtr:
                {
                    archive << *reinterpret_cast<Resource::ResourcePtr const*>( pValue );
                }
                break;

                case CoreTypeID::ResourceID:
                {
                    archive << *reinterpret_cast<ResourceID const*>( pValue );
                }
                break;

                case CoreTypeID::BitFlags:
                case CoreTypeID::TBitFlags:
                {
                    archive << *reinterpret_cast<BitFlags const*>( pValue );
                }
                break;

                default:
                {
                    KRG_UNREACHABLE_CODE();
                }
                break;
            }
        }
    }

    void ConvertBinaryToNativeType( TypeRegistry const& typeRegistry, TypeID typeID, TypeID templateArgumentTypeID, TVector<Byte> const& byteArray, void* pValue )
    {
        Serialization::BinaryMemoryArchive archive( Serialization::Mode::Read, const_cast<TVector<Byte>&>( byteArray ) );

        // Enums
        if ( !IsCoreType( typeID ) )
        {
            EnumInfo const* pEnumInfo = typeRegistry.GetEnumInfo( typeID );
            KRG_ASSERT( pEnumInfo != nullptr );

            // We only support up to 32 bit enum types...
            switch ( pEnumInfo->m_underlyingType )
            {
                case CoreTypeID::Uint8:
                {
                    archive >> *reinterpret_cast<uint8*>( pValue );
                }
                break;

                case CoreTypeID::Int8:
                {
                    archive >> *reinterpret_cast<int8*>( pValue );
                }
                break;

                case CoreTypeID::Uint16:
                {
                    archive >> *reinterpret_cast<uint16*>( pValue );
                }
                break;

                case CoreTypeID::Int16:
                {
                    archive >> *reinterpret_cast<int16*>( pValue );
                }
                break;

                case CoreTypeID::Uint32:
                {
                    archive >> *reinterpret_cast<uint32*>( pValue );
                }
                break;

                case CoreTypeID::Int32:
                {
                    archive >> *reinterpret_cast<int32*>( pValue );
                }
                break;

                default:
                {
                    KRG_UNREACHABLE_CODE();
                }
                break;
            }
        }
        else  // Real core types
        {
            CoreTypeID const typeToConvert = GetCoreType( typeID );
            switch ( typeToConvert )
            {
                case CoreTypeID::Bool:
                {
                    archive >> *reinterpret_cast<bool*>( pValue );
                }
                break;

                case CoreTypeID::Uint8:
                {
                    archive >> *reinterpret_cast<uint8*>( pValue );
                }
                break;

                case CoreTypeID::Int8:
                {
                    archive >> *reinterpret_cast<int8*>( pValue );
                }
                break;

                case CoreTypeID::Uint16:
                {
                    archive >> *reinterpret_cast<uint16*>( pValue );
                }
                break;

                case CoreTypeID::Int16:
                {
                    archive >> *reinterpret_cast<int16*>( pValue );
                }
                break;

                case CoreTypeID::Uint32:
                {
                    archive >> *reinterpret_cast<uint32*>( pValue );
                }
                break;

                case CoreTypeID::Int32:
                {
                    archive >> *reinterpret_cast<int32*>( pValue );
                }
                break;

                case CoreTypeID::Uint64:
                {
                    archive >> *reinterpret_cast<uint64*>( pValue );
                }
                break;

                case CoreTypeID::Int64:
                {
                    archive >> *reinterpret_cast<int64*>( pValue );
                }
                break;

                case CoreTypeID::Float:
                {
                    archive >> *reinterpret_cast<float*>( pValue );
                }
                break;

                case CoreTypeID::Double:
                {
                    archive >> *reinterpret_cast<double*>( pValue );
                }
                break;

                case CoreTypeID::String:
                {
                    archive >> *reinterpret_cast<String*>( pValue );
                }
                break;

                case CoreTypeID::StringID:
                {
                    archive >> *reinterpret_cast<StringID*>( pValue );
                }
                break;

                case CoreTypeID::TypeID:
                {
                    StringID ID;
                    archive >> ID;
                    *reinterpret_cast<TypeID*>( pValue ) = TypeID( ID );
                }
                break;

                case CoreTypeID::UUID:
                {
                    archive >> *reinterpret_cast<UUID*>( pValue );
                }
                break;

                case CoreTypeID::Color:
                {
                    archive >> *reinterpret_cast<Color*>( pValue );
                }
                break;

                case CoreTypeID::Float2:
                {
                    archive >> *reinterpret_cast<Float2*>( pValue );
                }
                break;

                case CoreTypeID::Float3:
                {
                    archive >> *reinterpret_cast<Float3*>( pValue );
                }
                break;

                case CoreTypeID::Float4:
                {
                    archive >> *reinterpret_cast<Float4*>( pValue );
                }
                break;

                case CoreTypeID::Vector:
                {
                    archive >> *reinterpret_cast<Vector*>( pValue );
                }
                break;

                case CoreTypeID::Quaternion:
                {
                    archive >> *reinterpret_cast<Quaternion*>( pValue );
                }
                break;

                case CoreTypeID::Matrix:
                {
                    archive >> *reinterpret_cast<Matrix*>( pValue );
                }
                break;

                case CoreTypeID::Transform:
                {
                    archive >> *reinterpret_cast<Transform*>( pValue );
                }
                break;

                case CoreTypeID::EulerAngles:
                {
                    archive >> *reinterpret_cast<EulerAngles*>( pValue );
                }
                break;

                case CoreTypeID::Microseconds:
                {
                    archive >> *reinterpret_cast<Microseconds*>( pValue );
                }
                break;

                case CoreTypeID::Milliseconds:
                {
                    archive >> *reinterpret_cast<Milliseconds*>( pValue );
                }
                break;

                case CoreTypeID::Seconds:
                {
                    archive >> *reinterpret_cast<Seconds*>( pValue );
                }
                break;

                case CoreTypeID::Percentage:
                {
                    archive >> *reinterpret_cast<Percentage*>( pValue );
                }
                break;

                case CoreTypeID::Degrees:
                {
                    archive >> *reinterpret_cast<Degrees*>( pValue );
                }
                break;

                case CoreTypeID::Radians:
                {
                    archive >> *reinterpret_cast<Radians*>( pValue );
                }
                break;

                case CoreTypeID::ResourcePath:
                {
                    archive >> *reinterpret_cast<ResourcePath*>( pValue );
                }
                break;

                case CoreTypeID::IntRange:
                {
                    archive >> *reinterpret_cast<IntRange*>( pValue );
                }
                break;

                case CoreTypeID::FloatRange:
                {
                    archive >> *reinterpret_cast<FloatRange*>( pValue );
                }
                break;

                case CoreTypeID::ResourceTypeID:
                {
                    archive >> *reinterpret_cast<ResourceTypeID*>( pValue );
                }
                break;

                case CoreTypeID::ResourcePtr:
                case CoreTypeID::TResourcePtr:
                {
                    archive >> *reinterpret_cast<Resource::ResourcePtr*>( pValue );
                }
                break;

                case CoreTypeID::ResourceID:
                {
                    archive >> *reinterpret_cast<ResourceID*>( pValue );
                }
                break;

                case CoreTypeID::BitFlags:
                case CoreTypeID::TBitFlags:
                {
                    archive >> *reinterpret_cast<BitFlags*>( pValue );
                }
                break;

                default:
                {
                    KRG_UNREACHABLE_CODE();
                }
                break;
            }
        }
    }

    void ConvertStringToBinary( TypeRegistry const& typeRegistry, TypeID typeID, TypeID templateArgumentTypeID, String const& strValue, TVector<Byte>& byteArray )
    {
        byteArray.clear();

        //-------------------------------------------------------------------------

        // Enums
        if ( !IsCoreType( typeID ) )
        {
            EnumInfo const* pEnumInfo = typeRegistry.GetEnumInfo( typeID );
            KRG_ASSERT( pEnumInfo != nullptr );

            // We only support up to 32 bit enum types...
            switch ( pEnumInfo->m_underlyingType )
            {
                case CoreTypeID::Uint8:
                {
                    uint8 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Int8:
                {
                    int8 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Uint16:
                {
                    uint16 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Int16:
                {
                    int16 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Uint32:
                {
                    uint32 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Int32:
                {
                    int32 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                default:
                {
                    KRG_UNREACHABLE_CODE();
                }
                break;
            }
        }
        else  // Real core types
        {
            CoreTypeID const typeToConvert = GetCoreType( typeID );
            switch ( typeToConvert )
            {
                case CoreTypeID::Bool:
                {
                    bool value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Uint8:
                {
                    uint8 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Int8:
                {
                    int8 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Uint16:
                {
                    uint16 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Int16:
                {
                    int16 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Uint32:
                {
                    uint32 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Int32:
                {
                    int32 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Uint64:
                {
                    uint64 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Int64:
                {
                    int64 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Float:
                {
                    float value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Double:
                {
                    double value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::String:
                {
                    String value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::StringID:
                {
                    StringID value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::TypeID:
                {
                    TypeID value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::UUID:
                {
                    UUID value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Color:
                {
                    Color value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Float2:
                {
                    Float2 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Float3:
                {
                    Float3 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Float4:
                {
                    Float4 value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Vector:
                {
                    Vector value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Quaternion:
                {
                    Quaternion value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Matrix:
                {
                    Matrix value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Transform:
                {
                    Transform value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::EulerAngles:
                {
                    EulerAngles value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Microseconds:
                {
                    Microseconds value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Milliseconds:
                {
                    Milliseconds value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Seconds:
                {
                    Seconds value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Percentage:
                {
                    Percentage value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Degrees:
                {
                    Degrees value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::Radians:
                {
                    Radians value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::ResourcePath:
                {
                    ResourcePath value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::IntRange:
                {
                    IntRange value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::FloatRange:
                {
                    FloatRange value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::ResourceTypeID:
                {
                    ResourceTypeID value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::ResourcePtr:
                case CoreTypeID::TResourcePtr:
                {
                    Resource::ResourcePtr value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::ResourceID:
                {
                    ResourceID value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                case CoreTypeID::BitFlags:
                case CoreTypeID::TBitFlags:
                {
                    BitFlags value;
                    ConvertStringToNativeType( typeRegistry, typeID, templateArgumentTypeID, strValue, &value );
                    ConvertNativeTypeToBinary( typeRegistry, typeID, templateArgumentTypeID, &value, byteArray );
                }
                break;

                default:
                {
                    KRG_UNREACHABLE_CODE();
                }
                break;
            }
        }
    }

    //-------------------------------------------------------------------------

    bool IsValidStringValueForType( TypeRegistry const& typeRegistry, TypeID typeID, TypeID templateArgumentTypeID, String const& strValue )
    {
        // Special cases: Enums and TBitFlags
        if ( typeID == CoreTypeID::TBitFlags )
        {
            EnumInfo const* pEnumInfo = typeRegistry.GetEnumInfo( templateArgumentTypeID );
            KRG_ASSERT( pEnumInfo != nullptr );
            BitFlags flags;
            return ConvertStringToBitFlags( *pEnumInfo, strValue, flags );
        }
        else if ( !IsCoreType( typeID ) ) // Enums has unique typeIDs
        {
            auto const pEnumInfo = typeRegistry.GetEnumInfo( typeID );
            KRG_ASSERT( pEnumInfo != nullptr );
            return pEnumInfo->IsValidValue( StringID( strValue ) );
        }
        else // Real core types
        {
            // TODO
            return true;
        }
    }
}