#pragma once

#include "Vector.h"

//-------------------------------------------------------------------------
// The Kruger math library is heavily based on the DirectX math library
//-------------------------------------------------------------------------
// https://github.com/Microsoft/DirectXMath
//-------------------------------------------------------------------------

namespace KRG
{
    class KRG_SYSTEM_CORE_API alignas( 16 ) Quaternion
    {
        KRG_SERIALIZE_MEMBERS( m_x, m_y, m_z, m_w );

    public:

        static Quaternion const Identity;

        // Calculate the rotation required to align the source vector to the target vector (shortest path)
        inline static Quaternion FromRotationBetweenNormalizedVectors( Vector const& sourceVector, Vector const& targetVector );
        
        // Calculate the rotation required to align one vector onto another but also taking account a fallback rotation axis for opposite parallel vectors
        inline static Quaternion FromRotationBetweenNormalizedVectors( Vector const& sourceVector, Vector const& targetVector, Vector const& fallbackRotationAxis );

        // Calculate the rotation required to align the source vector to the target vector (shortest path)
        KRG_FORCE_INLINE static Quaternion FromRotationBetweenVectors( Vector const& sourceVector, Vector const& targetVector ) { return FromRotationBetweenNormalizedVectors( sourceVector.GetNormalized3(), targetVector.GetNormalized3() ); }

        inline static Quaternion NLerp( Quaternion const& from, Quaternion const& to, float t );
        inline static Quaternion SLerp( Quaternion const& from, Quaternion const& to, float t );
        inline static Quaternion SQuad( Quaternion const& q0, Quaternion const& q1, Quaternion const& q2, Quaternion const& q3, float t );

        // Calculate the shortest delta quaternion needed to rotate 'from' onto 'to'
        KRG_FORCE_INLINE static Quaternion Delta( Quaternion const& from, Quaternion const& to ) { return to * from.GetInverse(); }

        // Simple vector dot product between two quaternions
        inline static Vector Dot( Quaternion const& q0, Quaternion const& q1 ) { return Vector::Dot4( q0.AsVector(), q1.AsVector() ); }

        // Calculate the angular distance between two quaternions
        inline static Radians Distance( Quaternion const& q0, Quaternion const& q1 );

    public:

        inline Quaternion() = default;
        inline explicit Quaternion( NoInit_t ) {}
        inline explicit Quaternion( IdentityInit_t ) : m_data( Vector::UnitW.m_data ) {}
        inline explicit Quaternion( Vector const& v ) : m_data( v.m_data ) {}
        inline explicit Quaternion( float ix, float iy, float iz, float iw ) { m_data = _mm_set_ps( iw, iz, iy, ix ); }
        inline explicit Quaternion( Float4 const& v ) : Quaternion( v.m_x, v.m_y, v.m_z, v.m_w ) {}

        inline explicit Quaternion( Vector const& axis, Radians angle );
        inline explicit Quaternion( AxisAngle axisAngle ) : Quaternion( Vector( axisAngle.m_axis ), axisAngle.m_angle ) {}

        inline explicit Quaternion( EulerAngles const& eulerAngles );
        inline explicit Quaternion( Radians rotX, Radians rotY, Radians rotZ ) : Quaternion( EulerAngles( rotX, rotY, rotZ ) ) {}

        KRG_FORCE_INLINE operator __m128&( ) { return m_data; }
        KRG_FORCE_INLINE operator __m128 const&() const { return m_data; }

        inline Vector Length() { return ToVector().Length4(); }
        inline float GetLength() const { return ToVector().GetLength4(); }

        inline Float4 ToFloat4() const { return Float4( m_x, m_y, m_z, m_w ); }
        inline Vector const& ToVector() const { return reinterpret_cast<Vector const&>( *this ); }
        inline AxisAngle ToAxisAngle() const;
        EulerAngles ToEulerAngles() const;

        inline Vector RotateVector( Vector const& vector ) const;
        inline Vector RotateVectorInverse( Vector const& vector ) const;

        // Operations
        inline Quaternion& Conjugate();
        inline Quaternion GetConjugate() const;

        inline Quaternion& Negate();
        inline Quaternion GetNegated() const;

        inline Quaternion& Invert();
        inline Quaternion GetInverse() const;

        inline Quaternion& Normalize();
        inline Quaternion GetNormalized() const;

        // This function will return the estimated normalized quaternion, this is not super accurate but a lot faster (use with care)
        inline Quaternion& NormalizeInaccurate();

        // This function will return the estimated normalized quaternion, this is not super accurate but a lot faster (use with care)
        inline Quaternion GetNormalizedInaccurate() const;

        inline bool IsNormalized() const { return AsVector().IsNormalized4(); }
        inline bool IsIdentity() const { return AsVector().IsEqual3( Vector::UnitW ); }

        // Concatenate the rotation of this onto rhs and return the result i.e. first rotate by rhs then by this
        // This means order of rotation is right-to-left: child-rotation * parent-rotation
        inline Quaternion operator*( Quaternion const& rhs ) const;
        inline Quaternion& operator*=( Quaternion const& rhs ) { *this = *this * rhs; return *this; }

        inline bool operator==( Quaternion const& rhs ) const { return m_x == rhs.m_x && m_y == rhs.m_y && m_z == rhs.m_z && m_w == rhs.m_w; }
        inline bool operator!=( Quaternion const& rhs ) const { return m_x != rhs.m_x || m_y != rhs.m_y || m_z != rhs.m_z || m_w != rhs.m_w; }

    private:

        inline Vector& AsVector() { return reinterpret_cast<Vector&>( *this ); }
        inline Vector const& AsVector() const { return reinterpret_cast<Vector const&>( *this ); }

        Quaternion& operator=( Vector const& v ) = delete;

    public:

        union
        {
            struct { float m_x, m_y, m_z, m_w; };
            __m128 m_data;
        };
    };

    static_assert( sizeof( Vector ) == 16, "Quaternion size must be 16 bytes!" );

    //-------------------------------------------------------------------------

    inline Radians Quaternion::Distance( Quaternion const& q0, Quaternion const& q1 )
    {
        return Radians( 2 * Math::ACos( Math::Abs( Dot( q0, q1 ).ToFloat() ) ) );
    }

    inline Quaternion& Quaternion::Conjugate()
    {
        static __m128 const conj = { -1.0f, -1.0f, -1.0f, 1.0f };
        m_data = _mm_mul_ps( *this, conj );
        return *this;
    }

    inline Quaternion Quaternion::GetConjugate() const
    {
        Quaternion q = *this;
        q.Conjugate();
        return q;
    }

    inline Quaternion& Quaternion::Negate()
    {
        m_data = _mm_mul_ps( *this, Vector::NegativeOne );
        return *this;
    }

    inline Quaternion Quaternion::GetNegated() const
    {
        Quaternion q = *this;
        q.Negate();
        return q;
    }

    inline Quaternion& Quaternion::Invert()
    {
        Vector const conjugate = GetConjugate().AsVector();
        Vector const length = AsVector().Length4();
        Vector const mask = length.LessThanEqual( Vector::Epsilon );
        Vector const result = conjugate / length;
        AsVector() = result.Select( result, Vector::Zero, mask );
        return *this;
    }

    inline Quaternion Quaternion::GetInverse() const
    {
        Quaternion q = *this;
        q.Invert();
        return q;
    }

    inline Quaternion& Quaternion::Normalize()
    {
        AsVector().Normalize4();
        return *this;
    }

    inline Quaternion Quaternion::GetNormalized() const
    {
        Quaternion q = *this;
        q.Normalize();
        return q;
    }

    inline Quaternion& Quaternion::NormalizeInaccurate()
    {
        *this = GetNormalizedInaccurate();
        return *this;
    }

    inline Quaternion Quaternion::GetNormalizedInaccurate() const
    {
        __m128 vLengthSq = _mm_mul_ps( m_data, m_data );
        __m128 vTemp = _mm_shuffle_ps( vLengthSq, vLengthSq, _MM_SHUFFLE( 3, 2, 3, 2 ) );
        vLengthSq = _mm_add_ps( vLengthSq, vTemp );
        vLengthSq = _mm_shuffle_ps( vLengthSq, vLengthSq, _MM_SHUFFLE( 1, 0, 0, 0 ) );
        vTemp = _mm_shuffle_ps( vTemp, vLengthSq, _MM_SHUFFLE( 3, 3, 0, 0 ) );
        vLengthSq = _mm_add_ps( vLengthSq, vTemp );
        vLengthSq = _mm_shuffle_ps( vLengthSq, vLengthSq, _MM_SHUFFLE( 2, 2, 2, 2 ) );
        
        // Get the reciprocal and mul to perform the normalization
        Quaternion result;
        result.m_data = _mm_rsqrt_ps( vLengthSq );
        result.m_data = _mm_mul_ps( result.m_data, m_data );
        return result;
    }

    //-------------------------------------------------------------------------

    inline Quaternion::Quaternion( Vector const& axis, Radians angle )
    {
        KRG_ASSERT( axis.IsNormalized3() );

        auto N = _mm_and_ps( axis, SIMD::g_maskXYZ0 );
        N = _mm_or_ps( N, Vector::UnitW );
        auto scale = _mm_set_ps1( 0.5f * (float) angle );

        Vector sine, cosine;
        Vector::SinCos( sine, cosine, scale );

        scale = _mm_and_ps( sine, SIMD::g_maskXYZ0 );
        cosine = _mm_and_ps( cosine, SIMD::g_mask000W );
        scale = _mm_or_ps( scale, cosine );

        N = _mm_mul_ps( N, scale );
        m_data = N;
    }

    inline Quaternion::Quaternion( EulerAngles const& eulerAngles )
    {
        auto const rotationX = Quaternion( Vector::UnitX, eulerAngles.m_x );
        auto const rotationY = Quaternion( Vector::UnitY, eulerAngles.m_y );
        auto const rotationZ = Quaternion( Vector::UnitZ, eulerAngles.m_z );

        // Rotation order is XYZ - all in global space, hence the order is reversed
        m_data = ( rotationX * rotationY * rotationZ ).GetNormalized().m_data;
    }

    inline Quaternion Quaternion::FromRotationBetweenNormalizedVectors( Vector const& from, Vector const& to )
    {
        KRG_ASSERT( from.IsNormalized3() && to.IsNormalized3() );

        Quaternion result;

        // Parallel vectors - return zero rotation
        Vector const dot = Vector::Dot3( from, to );
        if ( dot.IsGreaterThanEqual4( Vector::OneMinusEpsilon ) )
        {
            result = Quaternion::Identity;
        }
        // Opposite vectors - return 180 rotation around any orthogonal axis
        else if ( dot.IsLessThanEqual4( Vector::EpsilonMinusOne ) )
        {
            result = Quaternion( -from.m_z, from.m_y, from.m_x, 0 );
            result.Normalize();
        }
        else // Calculate quaternion rotation
        {
            Vector const cross = Vector::Cross3( from, to );
            Vector Q = Vector::Select( cross, dot, Vector::Select0001 );
            Q += Vector::Select( Vector::Zero, Q.Length4(), Vector::Select0001 );
            result = Quaternion( Q );
            result.Normalize();
        }

        return result;
    }

    inline Quaternion Quaternion::FromRotationBetweenNormalizedVectors( Vector const& from, Vector const& to, Vector const& fallbackRotationAxis )
    {
        KRG_ASSERT( from.IsNormalized3() && to.IsNormalized3() );

        Quaternion Q( NoInit );

        Vector rotationAxis = from.Cross3( to ).GetNormalized3();
        if ( rotationAxis.GetLengthSquared3() == 0 )
        {
            rotationAxis = fallbackRotationAxis;
        }

        float const dot = from.GetDot3( to );
        if ( dot >= ( 1.0f - Math::Epsilon ) )
        {
            Q = Quaternion::Identity;
        }
        else
        {
            float const angle = Math::ACos( dot );
            Q = Quaternion( rotationAxis, angle );
        }

        return Q;
    }

    //-------------------------------------------------------------------------

    inline AxisAngle Quaternion::ToAxisAngle() const
    {
        return AxisAngle( AsVector(), Radians( 2.0f * Math::ACos( m_w ) ) );
    }

    inline Vector Quaternion::RotateVector( Vector const& vector ) const
    {
        Quaternion const A( Vector::Select( Vector::Select1110, vector, Vector::Select1110 ) );
        Quaternion const Result = GetConjugate() * A;
        return ( Result * *this ).AsVector();
    }

    inline Vector Quaternion::RotateVectorInverse( Vector const& vector ) const
    {
        Quaternion const A( Vector::Select( Vector::Select1110, vector, Vector::Select1110 ) );
        Quaternion const Result = *this * A;
        return ( Result * GetConjugate() ).AsVector();
    }

    inline Quaternion Quaternion::operator*( Quaternion const& rhs ) const
    {
        static const __m128 ControlWZYX = { 1.0f,-1.0f, 1.0f,-1.0f };
        static const __m128 ControlZWXY = { 1.0f, 1.0f,-1.0f,-1.0f };
        static const __m128 ControlYXWZ = { -1.0f, 1.0f, 1.0f,-1.0f };

        // Copy to SSE registers and use as few as possible for x86
        __m128 Q2X = rhs;
        __m128 Q2Y = rhs;
        __m128 Q2Z = rhs;
        __m128 vResult = rhs;
        // Splat with one instruction
        vResult = _mm_shuffle_ps( vResult, vResult, _MM_SHUFFLE( 3, 3, 3, 3 ) );
        Q2X = _mm_shuffle_ps( Q2X, Q2X, _MM_SHUFFLE( 0, 0, 0, 0 ) );
        Q2Y = _mm_shuffle_ps( Q2Y, Q2Y, _MM_SHUFFLE( 1, 1, 1, 1 ) );
        Q2Z = _mm_shuffle_ps( Q2Z, Q2Z, _MM_SHUFFLE( 2, 2, 2, 2 ) );
        // Retire Q1 and perform Q1*Q2W
        vResult = _mm_mul_ps( vResult, *this );
        __m128 Q1Shuffle = *this;
        // Shuffle the copies of Q1
        Q1Shuffle = _mm_shuffle_ps( Q1Shuffle, Q1Shuffle, _MM_SHUFFLE( 0, 1, 2, 3 ) );
        // Mul by Q1WZYX
        Q2X = _mm_mul_ps( Q2X, Q1Shuffle );
        Q1Shuffle = _mm_shuffle_ps( Q1Shuffle, Q1Shuffle, _MM_SHUFFLE( 2, 3, 0, 1 ) );
        // Flip the signs on m_y and m_z
        Q2X = _mm_mul_ps( Q2X, ControlWZYX );
        // Mul by Q1ZWXY
        Q2Y = _mm_mul_ps( Q2Y, Q1Shuffle );
        Q1Shuffle = _mm_shuffle_ps( Q1Shuffle, Q1Shuffle, _MM_SHUFFLE( 0, 1, 2, 3 ) );
        // Flip the signs on m_z and m_w
        Q2Y = _mm_mul_ps( Q2Y, ControlZWXY );
        // Mul by Q1YXWZ
        Q2Z = _mm_mul_ps( Q2Z, Q1Shuffle );
        vResult = _mm_add_ps( vResult, Q2X );
        // Flip the signs on m_x and m_w
        Q2Z = _mm_mul_ps( Q2Z, ControlYXWZ );
        Q2Y = _mm_add_ps( Q2Y, Q2Z );
        vResult = _mm_add_ps( vResult, Q2Y );

        return Quaternion( vResult );
    }

    //-------------------------------------------------------------------------

    inline Quaternion Quaternion::NLerp( Quaternion const& from, Quaternion const& to, float T )
    {
        KRG_ASSERT( T >= 0.0f && T <= 1.0f );

        Quaternion adjustedFrom( from );

        // Ensure that the rotations are in the same direction
        if ( Quaternion::Dot( from, to ).IsLessThan4( Vector::Zero ) )
        {
            adjustedFrom.Negate();
        }

        Quaternion result( Vector::Lerp( adjustedFrom.ToVector(), to.ToVector(), T ) );
        result.Normalize();
        return result;
    }

    inline Quaternion Quaternion::SLerp( Quaternion const& from, Quaternion const& to, float T )
    {
        KRG_ASSERT( T >= 0.0f && T <= 1.0f );

        static SIMD::UIntMask const maskSign = { 0x80000000,0x00000000,0x00000000,0x00000000 };
        static __m128 const oneMinusEpsilon = { 1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f };

        Vector const VecT( T );

        Vector CosOmega = Quaternion::Dot( from, to );

        Vector Control = CosOmega.LessThan( Vector::Zero );
        Vector Sign = Vector::Select( Vector::One, Vector::NegativeOne, Control );

        CosOmega = _mm_mul_ps( CosOmega, Sign );
        Control = CosOmega.LessThan( oneMinusEpsilon );

        Vector SinOmega = _mm_mul_ps( CosOmega, CosOmega );
        SinOmega = _mm_sub_ps( Vector::One, SinOmega );
        SinOmega = _mm_sqrt_ps( SinOmega );

        Vector Omega = Vector::ATan2( SinOmega, CosOmega );

        Vector V01 = _mm_shuffle_ps( VecT, VecT, _MM_SHUFFLE( 2, 3, 0, 1 ) );
        V01 = _mm_and_ps( V01, SIMD::g_maskXY00 );
        V01 = _mm_xor_ps( V01, maskSign );
        V01 = _mm_add_ps( Vector::UnitX, V01 );

        Vector S0 = _mm_mul_ps( V01, Omega );
        S0 = Vector::Sin( S0 );
        S0 = _mm_div_ps( S0, SinOmega );
        S0 = Vector::Select( V01, S0, Control );

        Vector S1 = S0.GetSplatY();
        S0 = S0.GetSplatX();

        S1 = _mm_mul_ps( S1, Sign );
        Vector result = _mm_mul_ps( from, S0 );
        S1 = _mm_mul_ps( S1, to );
        result = _mm_add_ps( result, S1 );

        return Quaternion( result );
    }

    inline Quaternion Quaternion::SQuad( Quaternion const& q0, Quaternion const& q1, Quaternion const& q2, Quaternion const& q3, float t )
    {
        KRG_ASSERT( t >= 0.0f && t <= 1.0f );

        Quaternion const q03 = Quaternion::SLerp( q0, q3, t );
        Quaternion const q12 = Quaternion::SLerp( q1, q2, t );
        t = ( t - ( t * t ) ) * 2;
        Quaternion const result = Quaternion::SLerp( q03, q12, t );
        return result;
    }
}