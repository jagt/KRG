#pragma once
#include "_Module/API.h"
#include "System/Core/Math/Transform.h"
#include "System/Core/Types/Color.h"
#include "System/Core/Memory/Memory.h"
#include "System/Core/Debug/DebugDrawingSystem.h"

#include <bfxSystem.h>
#include <bfxPlanner.h>

//-------------------------------------------------------------------------

namespace KRG::Debug { class DrawingSystem; }

//-------------------------------------------------------------------------

namespace KRG::Navmesh
{
    //-------------------------------------------------------------------------
    // Type Conversion
    //-------------------------------------------------------------------------

    KRG_FORCE_INLINE Float3 FromBfx( bfx::Vector3 const& v )
    {
        return Float3( v.m_x, v.m_y, v.m_z );
    }

    KRG_FORCE_INLINE Quaternion FromBfx( bfx::Quaternion const& v )
    {
        return Quaternion( v.m_x, v.m_y, v.m_z, v.m_w );
    }

    KRG_FORCE_INLINE Color FromBfx( bfx::Color const& color )
    {
        return Color( U8( color.m_r * 255 ), U8( color.m_g * 255 ), U8( color.m_b ), U8( color.m_a * 255 ) );
    }

    //-------------------------------------------------------------------------

    KRG_FORCE_INLINE bfx::Vector3 ToBfx( Float3 const& v )
    {
        return bfx::Vector3( v.x, v.y, v.z );
    }

    KRG_FORCE_INLINE bfx::Vector3 ToBfx( Float4 const& v )
    {
        return bfx::Vector3( v.x, v.y, v.z );
    }

    KRG_FORCE_INLINE bfx::Vector3 ToBfx( Vector const& v )
    {
        return bfx::Vector3( v.x, v.y, v.z );
    }

    KRG_FORCE_INLINE bfx::Quaternion ToBfx( Quaternion const& q )
    {
        bfx::Quaternion quaternion;
        quaternion.m_x = q.x;
        quaternion.m_y = q.y;
        quaternion.m_z = q.z;
        quaternion.m_w = q.w;
        return quaternion;
    }

    KRG_FORCE_INLINE bfx::Color ToBfx( Color const& color )
    {
        return bfx::Color( color.m_byteColor.m_r / 255.0f, color.m_byteColor.m_g / 255.0f, color.m_byteColor.m_b / 255.0f, color.m_byteColor.m_a / 255.0f );
    }

    //-------------------------------------------------------------------------
    // Integration
    //-------------------------------------------------------------------------

    class NavPowerAllocator final : public bfx::CustomAllocator
    {
        virtual void* CustomMalloc( U32 size ) override final { return KRG::Alloc( size ); }
        virtual void* CustomAlignedMalloc( U32 alignment, U32 size ) override final { return KRG::Alloc( size, alignment ); }
        virtual void CustomFree( void* ptr ) override final { KRG::Free( ptr ); }
        virtual bool IsThreadSafe() const override final { return true; }
        virtual const char* GetName() const override { return "KRUGER"; }
    };

    //-------------------------------------------------------------------------

    #if KRG_DEVELOPMENT_TOOLS
    class NavPowerRenderer final : public bfx::Renderer
    {

    public:

        void SetDebugDrawingSystem( Debug::DrawingSystem* pDebugDrawingSystem );
        void Reset();

    private:

        virtual void DrawLineList( bfx::LineSegment const* pLines, U32 numLines, bfx::Color const& color ) override;
        virtual void DrawTriList( bfx::Triangle const* pTris, U32 numTris, bfx::Color const& color ) override;
        virtual void DrawString( bfx::Color const& color, char const* str ) override;
        virtual void DrawString( bfx::Color const& color, bfx::Vector3 const& pos, char const* str ) override;

    private:

        Debug::DrawingSystem*                       m_pDebugDrawingSystem = nullptr;
        Float2                                      m_statsPos = Float2::Zero;
    };
    #endif
}