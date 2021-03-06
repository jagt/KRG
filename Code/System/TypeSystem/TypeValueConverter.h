#pragma once

#include "_Module/API.h"
#include "TypeInfo.h"
#include "System/Resource/ResourcePtr.h"
#include "System/Core/Serialization/BinaryArchive.h"
#include "System/Core/Time/Time.h"
#include "System/Core/Types/Color.h"
#include "System/Core/Types/UUID.h"
#include "System/Core/Types/StringID.h"
#include "System/Core/Math/Transform.h"

//-------------------------------------------------------------------------

namespace KRG
{
    namespace TypeSystem
    {
        class KRG_SYSTEM_TYPESYSTEM_API TypeValueConverter
        {
        public:

            // Typed conversions
            //-------------------------------------------------------------------------

            template<typename T>
            inline static void ValueToString( T const& value, String& outStr ) { KRG_UNIMPLEMENTED_FUNCTION(); }

            template<typename T>
            inline static void StringToValue( String const& str, T& outValue ) { KRG_UNIMPLEMENTED_FUNCTION(); }

            template<typename T>
            inline static void ValueToByteArray( T const& value, TVector<Byte>& outArray )
            {
                Serialization::BinaryMemoryArchive archive( Serialization::Mode::Write, outArray );
                archive << value;
            }

            template<typename T>
            inline static void ByteArrayToValue( TVector<Byte> const& valueByteData, T& outValue )
            {
                Serialization::BinaryMemoryArchive archive( Serialization::Mode::Read, const_cast<TVector<Byte>&>( valueByteData ) );
                archive >> outValue;
            }

            template<typename T>
            inline static void ByteArrayToValue( TVector<Byte> const& valueByteData, T* pOutValue )
            {
                ByteArrayToValue( valueByteData, *pOutValue );
            }

            template<typename T>
            inline static void StringToByteArray( String const& str, TVector<Byte>& outArray )
            {
                T value;
                StringToValue<T>( str, value );
                ValueToByteArray( value, outArray );
            }

            // Untyped conversions
            //-------------------------------------------------------------------------

            static void ConvertValueToString( TypeSystem::TypeID typeID, void const* pValueData, String& strValue );
            static void ConvertValueToByteArray( TypeSystem::TypeID typeID, void const* pValueData, TVector<Byte>& byteArray );
            static void ConvertStringToValue( TypeSystem::TypeID typeID, String const& strValue, void* pValueData );
            static void ConvertStringToByteArray( TypeSystem::TypeID typeID, String const& strValue, TVector<Byte>& byteArray );
            static void ConvertByteArrayToValue( TypeSystem::TypeID typeID, TVector<Byte> const& byteArray, void* pValue );
            static bool IsValidStringValueForType( TypeSystem::TypeID typeID, String const& strValue );

        private:

            static void StringToFloatArray( String const& str, char const* delim, int32 const numFloats, float* pFloats );
            static void FloatArrayToString( float const* pFloats, char const* delim, int32 const numFloats, String& outStr );
        };

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( bool const& value, String& outStr )
        {
            outStr = value ? "True" : "False";
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, bool& value )
        {
            String lowerString = str;
            lowerString.make_lower();
            value = ( lowerString == "true" );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( uint8 const& value, String& outStr )
        {
            outStr = eastl::to_string( value );
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, uint8& value )
        {
            value = (uint8) strtoul( str.c_str(), nullptr, 10 );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( uint16 const& value, String& outStr )
        {
            outStr = eastl::to_string( value );
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, uint16& value )
        {
            value = (uint16) strtoul( str.c_str(), nullptr, 10 );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( uint32 const& value, String& outStr )
        {
            outStr = eastl::to_string( value );
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, uint32& value )
        {
            value = (uint32) strtoul( str.c_str(), nullptr, 10 );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( uint64 const& value, String& outStr )
        {
            outStr = eastl::to_string( value );
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, uint64& value )
        {
            value = (uint64) strtoull( str.c_str(), nullptr, 10 );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( int8 const& value, String& outStr )
        {
            outStr = eastl::to_string( value );
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, int8& value )
        {
            value = (int8) strtol( str.c_str(), nullptr, 10 );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( int16 const& value, String& outStr )
        {
            outStr = eastl::to_string( value );
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, int16& value )
        {
            value = (int16) strtol( str.c_str(), nullptr, 10 );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( int32 const& value, String& outStr )
        {
            outStr = eastl::to_string( value );
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, int32& value )
        {
            value = (int32) strtol( str.c_str(), nullptr, 10 );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( int64 const& value, String& outStr )
        {
            outStr = eastl::to_string( value );
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, int64& value )
        {
            value = (int64) strtoll( str.c_str(), nullptr, 10 );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( float const& value, String& outStr )
        {
            outStr = eastl::to_string( value );
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, float& value )
        {
            value = (float) atof( str.c_str() );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( double const& value, String& outStr )
        {
            outStr = eastl::to_string( value );
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, double& value )
        {
            value = atof( str.c_str() );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( String const& value, String& outStr )
        {
            outStr = value;
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, String& value )
        {
            value = str;
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( StringID const& value, String& outStr )
        {
            char const* pStr = value.ToString();
            KRG_ASSERT( pStr != nullptr );
            outStr = pStr;
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, StringID& value )
        {
            value = StringID( str.c_str() );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( Color const& value, String& outStr )
        {
            outStr.sprintf( "%02X%02X%02X%02X", value.m_byteColor.m_r, value.m_byteColor.m_g, value.m_byteColor.m_b, value.m_byteColor.m_a );
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, Color& value )
        {
            uint32 const colorValue = (uint32) strtoul( str.c_str(), nullptr, 16 );
            value = Color( colorValue );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( UUID const& value, String& outStr )
        {
            outStr = value.ToString();
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, UUID& value )
        {
            value = UUID( str );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( Float2 const& value, String& outStr )
        {
            FloatArrayToString( &value[0], ",", 2, outStr );
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, Float2& value )
        {
            StringToFloatArray( str, ",", 2, &value[0] );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( Float3 const& value, String& outStr )
        {
            FloatArrayToString( &value[0], ",", 3, outStr );
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, Float3& value )
        {
            StringToFloatArray( str, ",", 3, &value[0] );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( Float4 const& value, String& outStr )
        {
            FloatArrayToString( &value[0], ",", 4, outStr );
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, Float4& value )
        {
            StringToFloatArray( str, ",", 4, &value[0] );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( Matrix const& value, String& outStr )
        {
            // Handle uninitialized matrix
            if ( value.IsOrthonormal() )
            {
                auto eulerAngles = value.ToEulerAngles();

                float floatData[9];
                (Float3&) floatData = Float3( (float) eulerAngles.m_x.ToDegrees(), (float) eulerAngles.m_y.ToDegrees(), (float) eulerAngles.m_z.ToDegrees() );
                (Float3&) floatData[3] = value.GetTranslation().ToFloat3();
                (Float3&) floatData[6] = value.GetScale().ToFloat3();

                String str;
                FloatArrayToString( floatData, ",", 9, str );
                outStr = str;
            }
            else
            {
                outStr = "0,0,0,0,0,0,0,0,0";
            }
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, Matrix& value )
        {
            float floatData[9];
            StringToFloatArray( str, ",", 9, floatData );

            EulerAngles const rotation( floatData[0], floatData[1], floatData[2] );
            Vector const translation = Vector( floatData[3], floatData[4], floatData[5] );
            value = Matrix( Quaternion( rotation ), translation );

            Vector const scale = Vector( floatData[6], floatData[7], floatData[8] );
            value.SetScale( scale );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( Transform const& value, String& outStr )
        {
            auto eulerAngles = value.GetRotation().ToEulerAngles();

            float floatData[9];

            floatData[0] = (float) eulerAngles.m_x.ToDegrees();
            floatData[1] = (float) eulerAngles.m_y.ToDegrees();
            floatData[2] = (float) eulerAngles.m_z.ToDegrees();

            floatData[3] = value.GetTranslation().m_x;
            floatData[4] = value.GetTranslation().m_y;
            floatData[5] = value.GetTranslation().m_z;

            floatData[6] = value.GetScale().m_x;
            floatData[7] = value.GetScale().m_y;
            floatData[8] = value.GetScale().m_z;

            //-------------------------------------------------------------------------

            String str;
            FloatArrayToString( floatData, ",", 9, str );
            outStr = str;
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, Transform& value )
        {
            float floatData[9];
            StringToFloatArray( str, ",", 9, floatData );

            // Set rotation
            EulerAngles const rotationEulerAngles( floatData[0], floatData[1], floatData[2] );
            value.SetRotation( Quaternion( rotationEulerAngles ) );

            // Set translation and scale
            value.SetTranslation( Vector( floatData[3], floatData[4], floatData[5], 1.0f ) );
            value.SetScale( Vector( floatData[6], floatData[7], floatData[8], 1.0f ) );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( EulerAngles const& value, String& outStr )
        {
            FloatArrayToString( (float*) &value.m_x, ",", 3, outStr );
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, EulerAngles& value )
        {
            StringToFloatArray( str, ",", 3, (float*) &value.m_x );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( Quaternion const& value, String& outStr )
        {
            EulerAngles angles = value.ToEulerAngles();
            FloatArrayToString( (float*) &angles.m_x, ",", 3, outStr );
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, Quaternion& value )
        {
            EulerAngles angles;
            StringToFloatArray( str, ",", 3, (float*) &angles.m_x );
            value = Quaternion( angles );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( ResourceTypeID const& value, String& outStr )
        {
            outStr = value.ToString();
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, ResourceTypeID& value )
        {
            value = ResourceTypeID( str );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( DataPath const& value, String& outStr )
        {
            outStr = value.c_str();
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, DataPath& value )
        {
            value = DataPath( str );
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( ResourceID const& value, String& outStr )
        {
            outStr = value.ToString();
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, ResourceID& value )
        {
            if ( str.empty() )
            {
                value = ResourceID();
            }
            else
            {
                KRG_ASSERT( ResourceID::IsValidResourceIDString( str ) );
                value = ResourceID( str );
            }
        }

        //-------------------------------------------------------------------------

        template<>
        inline void TypeValueConverter::ValueToString( Resource::ResourcePtr const& value, String& outStr )
        {
            outStr = value.GetResourceID().ToString();
        }

        template<>
        inline void TypeValueConverter::StringToValue( String const& str, Resource::ResourcePtr& value )
        {
            if ( str.empty() )
            {
                value = Resource::ResourcePtr();
            }
            else
            {
                KRG_ASSERT( ResourceID::IsValidResourceIDString( str ) );
                ResourceID const ID( str );
                value = Resource::ResourcePtr( ID );
            }
        }
    }
}