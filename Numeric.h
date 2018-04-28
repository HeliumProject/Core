#pragma once

#include "Platform/Types.h"
#include "Platform/Assert.h"

#include "API.h"

#define F32_SMALLEST     1.175494351e-38F
#define F32_BIGGEST      3.402823466e+38F

#define F32_MIN         -F32_BIGGEST
#define F32_MAX          F32_BIGGEST

#define F64_SMALLEST     2.2250738585072014e-308
#define F64_BIGGEST      1.7976931348623158e+308

#define F64_MIN         -F64_BIGGEST
#define F64_MAX          F64_BIGGEST

namespace Helium
{
    template< class T >
    struct HELIUM_FOUNDATION_API NumericLimits
    {
        const static T Minimum;
        const static T Maximum;
    };

    template< class S, class D >
    inline bool RangeCastInteger( const S source, D& dest, bool clamp = false )
    {
        if ( source > 0 )
        {
            if ( (uint64_t)source <= (uint64_t)NumericLimits<D>::Maximum )
            {
                dest = static_cast<D>( source );
                return true;
            }
            else if ( clamp )
            {
                dest = NumericLimits<D>::Maximum;
                return true;
            }
            else
            {
                return false;
            }
        }
        else if ( source != 0 )
        {
            if ( (int64_t)source >= (int64_t)NumericLimits<D>::Minimum )
            {
                dest = static_cast<D>( source );
                return true;
            }
            else if ( clamp )
            {
                dest = NumericLimits<D>::Minimum;
                return true;
            }
            else
            {
                return false;
            }
        }

        dest = 0;
        return true;
    }

    template< class S, class D >
    inline bool RangeCastFloat( const S source, D& dest, bool clamp = false )
    {
        if ( source > 0 )
        {
            if ( source <= NumericLimits<D>::Maximum )
            {
                dest = static_cast<D>( source );
                return true;
            }
            else if ( clamp )
            {
                dest = NumericLimits<D>::Maximum;
                return true;
            }
            else
            {
                return false;
            }
        }
        else if ( source != 0 )
        {
            if ( source >= NumericLimits<D>::Minimum )
            {
                dest = static_cast<D>( source );
                return true;
            }
            else if ( clamp )
            {
                dest = NumericLimits<D>::Minimum;
                return true;
            }
            else
            {
                return false;
            }
        }

        dest = 0;
        return true;
    }

    template< class S, class D >
    inline bool RangeCast( const S source, D& dest, bool clamp = false )
    {
        HELIUM_ASSERT( false );
        return false;
    }

#if HELIUM_CC_CLANG
    template<> HELIUM_FOUNDATION_API const int8_t Helium::NumericLimits<int8_t>::Minimum;
    template<> HELIUM_FOUNDATION_API const int8_t Helium::NumericLimits<int8_t>::Maximum;
    template<> HELIUM_FOUNDATION_API const uint8_t Helium::NumericLimits<uint8_t>::Minimum;
    template<> HELIUM_FOUNDATION_API const uint8_t Helium::NumericLimits<uint8_t>::Maximum;
    template<> HELIUM_FOUNDATION_API const int16_t Helium::NumericLimits<int16_t>::Minimum;
    template<> HELIUM_FOUNDATION_API const int16_t Helium::NumericLimits<int16_t>::Maximum;
    template<> HELIUM_FOUNDATION_API const uint16_t Helium::NumericLimits<uint16_t>::Minimum;
    template<> HELIUM_FOUNDATION_API const uint16_t Helium::NumericLimits<uint16_t>::Maximum;
    template<> HELIUM_FOUNDATION_API const int32_t Helium::NumericLimits<int32_t>::Minimum;
    template<> HELIUM_FOUNDATION_API const int32_t Helium::NumericLimits<int32_t>::Maximum;
    template<> HELIUM_FOUNDATION_API const uint32_t Helium::NumericLimits<uint32_t>::Minimum;
    template<> HELIUM_FOUNDATION_API const uint32_t Helium::NumericLimits<uint32_t>::Maximum;
    template<> HELIUM_FOUNDATION_API const int64_t Helium::NumericLimits<int64_t>::Minimum;
    template<> HELIUM_FOUNDATION_API const int64_t Helium::NumericLimits<int64_t>::Maximum;
    template<> HELIUM_FOUNDATION_API const uint64_t Helium::NumericLimits<uint64_t>::Minimum;
    template<> HELIUM_FOUNDATION_API const uint64_t Helium::NumericLimits<uint64_t>::Maximum;
    template<> HELIUM_FOUNDATION_API const float32_t Helium::NumericLimits<float32_t>::Minimum;
    template<> HELIUM_FOUNDATION_API const float32_t Helium::NumericLimits<float32_t>::Maximum;
    template<> HELIUM_FOUNDATION_API const float64_t Helium::NumericLimits<float64_t>::Minimum;
    template<> HELIUM_FOUNDATION_API const float64_t Helium::NumericLimits<float64_t>::Maximum;
#endif

    template <> HELIUM_FOUNDATION_API bool RangeCast( const int8_t source, int8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int8_t source, int16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int8_t source, int32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int8_t source, int64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int8_t source, uint8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int8_t source, uint16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int8_t source, uint32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int8_t source, uint64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int8_t source, float32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int8_t source, float64_t& dest, bool clamp );

    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint8_t source, int8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint8_t source, int16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint8_t source, int32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint8_t source, int64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint8_t source, uint8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint8_t source, uint16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint8_t source, uint32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint8_t source, uint64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint8_t source, float32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint8_t source, float64_t& dest, bool clamp );

    template <> HELIUM_FOUNDATION_API bool RangeCast( const int16_t source, int8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int16_t source, int16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int16_t source, int32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int16_t source, int64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int16_t source, uint8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int16_t source, uint16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int16_t source, uint32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int16_t source, uint64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int16_t source, float32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int16_t source, float64_t& dest, bool clamp );

    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint16_t source, int8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint16_t source, int16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint16_t source, int32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint16_t source, int64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint16_t source, uint8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint16_t source, uint16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint16_t source, uint32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint16_t source, uint64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint16_t source, float32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint16_t source, float64_t& dest, bool clamp );

    template <> HELIUM_FOUNDATION_API bool RangeCast( const int32_t source, int8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int32_t source, int16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int32_t source, int32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int32_t source, int64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int32_t source, uint8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int32_t source, uint16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int32_t source, uint32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int32_t source, uint64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int32_t source, float32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int32_t source, float64_t& dest, bool clamp );

    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint32_t source, int8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint32_t source, int16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint32_t source, int32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint32_t source, int64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint32_t source, uint8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint32_t source, uint16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint32_t source, uint32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint32_t source, uint64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint32_t source, float32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint32_t source, float64_t& dest, bool clamp );

    template <> HELIUM_FOUNDATION_API bool RangeCast( const int64_t source, int8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int64_t source, int16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int64_t source, int32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int64_t source, int64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int64_t source, uint8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int64_t source, uint16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int64_t source, uint32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int64_t source, uint64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int64_t source, float32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const int64_t source, float64_t& dest, bool clamp );

    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint64_t source, int8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint64_t source, int16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint64_t source, int32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint64_t source, int64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint64_t source, uint8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint64_t source, uint16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint64_t source, uint32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint64_t source, uint64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint64_t source, float32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const uint64_t source, float64_t& dest, bool clamp );

    template <> HELIUM_FOUNDATION_API bool RangeCast( const float32_t source, int8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float32_t source, int16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float32_t source, int32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float32_t source, int64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float32_t source, uint8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float32_t source, uint16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float32_t source, uint32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float32_t source, uint64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float32_t source, float32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float32_t source, float64_t& dest, bool clamp );

    template <> HELIUM_FOUNDATION_API bool RangeCast( const float64_t source, int8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float64_t source, int16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float64_t source, int32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float64_t source, int64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float64_t source, uint8_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float64_t source, uint16_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float64_t source, uint32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float64_t source, uint64_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float64_t source, float32_t& dest, bool clamp );
    template <> HELIUM_FOUNDATION_API bool RangeCast( const float64_t source, float64_t& dest, bool clamp );
}