#pragma once

#include "Platform/Types.h"
#include "Platform/Assert.h"

#include "API.h"

#define F32_MIN     1.175494351e-38F
#define F32_MAX     3.402823466e+38F

#define F64_MIN     2.2250738585072014e-308
#define F64_MAX     1.7976931348623158e+308

namespace Helium
{
    template< class T >
    struct NumericLimits
    {
        const static T Minimum;
        const static T Maximum;
    };

    template<> const int8_t NumericLimits<int8_t>::Minimum = INT8_MIN;
    template<> const int8_t NumericLimits<int8_t>::Maximum = INT8_MAX;

    template<> const uint8_t NumericLimits<uint8_t>::Minimum = 0;
    template<> const uint8_t NumericLimits<uint8_t>::Maximum = UINT8_MAX;

    template<> const int16_t NumericLimits<int16_t>::Minimum = INT16_MIN;
    template<> const int16_t NumericLimits<int16_t>::Maximum = INT16_MAX;

    template<> const uint16_t NumericLimits<uint16_t>::Minimum = 0;
    template<> const uint16_t NumericLimits<uint16_t>::Maximum = UINT16_MAX;

    template<> const int32_t NumericLimits<int32_t>::Minimum = INT32_MIN;
    template<> const int32_t NumericLimits<int32_t>::Maximum = INT32_MAX;

    template<> const uint32_t NumericLimits<uint32_t>::Minimum = 0;
    template<> const uint32_t NumericLimits<uint32_t>::Maximum = UINT32_MAX;

    template<> const int64_t NumericLimits<int64_t>::Minimum = INT64_MIN;
    template<> const int64_t NumericLimits<int64_t>::Maximum = INT64_MAX;

    template<> const uint64_t NumericLimits<uint64_t>::Minimum = 0;
    template<> const uint64_t NumericLimits<uint64_t>::Maximum = UINT64_MAX;

    template<> const float32_t NumericLimits<float32_t>::Minimum = -F32_MIN;
    template<> const float32_t NumericLimits<float32_t>::Maximum = F32_MAX;

    template<> const float64_t NumericLimits<float64_t>::Minimum = -F64_MIN;
    template<> const float64_t NumericLimits<float64_t>::Maximum = F64_MAX;

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
        else if ( source < 0 )
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
        else if ( source < 0 )
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

    template <> inline bool RangeCast( const int8_t source, int8_t& dest, bool clamp )          { dest = source; return true; }
    template <> inline bool RangeCast( const int8_t source, int16_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int8_t source, int32_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int8_t source, int64_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int8_t source, uint8_t& dest, bool clamp )          { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int8_t source, uint16_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int8_t source, uint32_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int8_t source, uint64_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int8_t source, float32_t& dest, bool clamp )         { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const int8_t source, float64_t& dest, bool clamp )         { return RangeCastFloat( source, dest, clamp ); }

    template <> inline bool RangeCast( const uint8_t source, int8_t& dest, bool clamp )          { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint8_t source, int16_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint8_t source, int32_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint8_t source, int64_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint8_t source, uint8_t& dest, bool clamp )          { dest = source; return true; }
    template <> inline bool RangeCast( const uint8_t source, uint16_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint8_t source, uint32_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint8_t source, uint64_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint8_t source, float32_t& dest, bool clamp )         { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint8_t source, float64_t& dest, bool clamp )         { return RangeCastFloat( source, dest, clamp ); }

    template <> inline bool RangeCast( const int16_t source, int8_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int16_t source, int16_t& dest, bool clamp )        { dest = source; return true; }
    template <> inline bool RangeCast( const int16_t source, int32_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int16_t source, int64_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int16_t source, uint8_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int16_t source, uint16_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int16_t source, uint32_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int16_t source, uint64_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int16_t source, float32_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const int16_t source, float64_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }

    template <> inline bool RangeCast( const uint16_t source, int8_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint16_t source, int16_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint16_t source, int32_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint16_t source, int64_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint16_t source, uint8_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint16_t source, uint16_t& dest, bool clamp )        { dest = source; return true; }
    template <> inline bool RangeCast( const uint16_t source, uint32_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint16_t source, uint64_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint16_t source, float32_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint16_t source, float64_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }

    template <> inline bool RangeCast( const int32_t source, int8_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int32_t source, int16_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int32_t source, int32_t& dest, bool clamp )        { dest = source; return true; }
    template <> inline bool RangeCast( const int32_t source, int64_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int32_t source, uint8_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int32_t source, uint16_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int32_t source, uint32_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int32_t source, uint64_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int32_t source, float32_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const int32_t source, float64_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }

    template <> inline bool RangeCast( const uint32_t source, int8_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint32_t source, int16_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint32_t source, int32_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint32_t source, int64_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint32_t source, uint8_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint32_t source, uint16_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint32_t source, uint32_t& dest, bool clamp )        { dest = source; return true; }
    template <> inline bool RangeCast( const uint32_t source, uint64_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint32_t source, float32_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint32_t source, float64_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }

    template <> inline bool RangeCast( const int64_t source, int8_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int64_t source, int16_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int64_t source, int32_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int64_t source, int64_t& dest, bool clamp )        { dest = source; return true; }
    template <> inline bool RangeCast( const int64_t source, uint8_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int64_t source, uint16_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int64_t source, uint32_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int64_t source, uint64_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const int64_t source, float32_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const int64_t source, float64_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }

    template <> inline bool RangeCast( const uint64_t source, int8_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint64_t source, int16_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint64_t source, int32_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint64_t source, int64_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint64_t source, uint8_t& dest, bool clamp )         { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint64_t source, uint16_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint64_t source, uint32_t& dest, bool clamp )        { return RangeCastInteger( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint64_t source, uint64_t& dest, bool clamp )        { dest = source; return true; }
    template <> inline bool RangeCast( const uint64_t source, float32_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const uint64_t source, float64_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }

    template <> inline bool RangeCast( const float32_t source, int8_t& dest, bool clamp )         { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float32_t source, int16_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float32_t source, int32_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float32_t source, int64_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float32_t source, uint8_t& dest, bool clamp )         { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float32_t source, uint16_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float32_t source, uint32_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float32_t source, uint64_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float32_t source, float32_t& dest, bool clamp )        { dest = source; return true; }
    template <> inline bool RangeCast( const float32_t source, float64_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }

    template <> inline bool RangeCast( const float64_t source, int8_t& dest, bool clamp )         { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float64_t source, int16_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float64_t source, int32_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float64_t source, int64_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float64_t source, uint8_t& dest, bool clamp )         { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float64_t source, uint16_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float64_t source, uint32_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float64_t source, uint64_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float64_t source, float32_t& dest, bool clamp )        { return RangeCastFloat( source, dest, clamp ); }
    template <> inline bool RangeCast( const float64_t source, float64_t& dest, bool clamp )        { dest = source; return true; }
}