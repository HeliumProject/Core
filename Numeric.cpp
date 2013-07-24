#include "FoundationPch.h"
#include "Numeric.h"

using namespace Helium;

template<> HELIUM_FOUNDATION_API const int8_t Helium::NumericLimits<int8_t>::Minimum = INT8_MIN;
template<> HELIUM_FOUNDATION_API const int8_t Helium::NumericLimits<int8_t>::Maximum = INT8_MAX;

template<> HELIUM_FOUNDATION_API const uint8_t Helium::NumericLimits<uint8_t>::Minimum = 0;
template<> HELIUM_FOUNDATION_API const uint8_t Helium::NumericLimits<uint8_t>::Maximum = UINT8_MAX;

template<> HELIUM_FOUNDATION_API const int16_t Helium::NumericLimits<int16_t>::Minimum = INT16_MIN;
template<> HELIUM_FOUNDATION_API const int16_t Helium::NumericLimits<int16_t>::Maximum = INT16_MAX;

template<> HELIUM_FOUNDATION_API const uint16_t Helium::NumericLimits<uint16_t>::Minimum = 0;
template<> HELIUM_FOUNDATION_API const uint16_t Helium::NumericLimits<uint16_t>::Maximum = UINT16_MAX;

template<> HELIUM_FOUNDATION_API const int32_t Helium::NumericLimits<int32_t>::Minimum = INT32_MIN;
template<> HELIUM_FOUNDATION_API const int32_t Helium::NumericLimits<int32_t>::Maximum = INT32_MAX;

template<> HELIUM_FOUNDATION_API const uint32_t Helium::NumericLimits<uint32_t>::Minimum = 0;
template<> HELIUM_FOUNDATION_API const uint32_t Helium::NumericLimits<uint32_t>::Maximum = UINT32_MAX;

template<> HELIUM_FOUNDATION_API const int64_t Helium::NumericLimits<int64_t>::Minimum = INT64_MIN;
template<> HELIUM_FOUNDATION_API const int64_t Helium::NumericLimits<int64_t>::Maximum = INT64_MAX;

template<> HELIUM_FOUNDATION_API const uint64_t Helium::NumericLimits<uint64_t>::Minimum = 0;
template<> HELIUM_FOUNDATION_API const uint64_t Helium::NumericLimits<uint64_t>::Maximum = UINT64_MAX;

template<> HELIUM_FOUNDATION_API const float32_t Helium::NumericLimits<float32_t>::Minimum = F32_MIN;
template<> HELIUM_FOUNDATION_API const float32_t Helium::NumericLimits<float32_t>::Maximum = F32_MAX;

template<> HELIUM_FOUNDATION_API const float64_t Helium::NumericLimits<float64_t>::Minimum = F64_MIN;
template<> HELIUM_FOUNDATION_API const float64_t Helium::NumericLimits<float64_t>::Maximum = F64_MAX;

template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int8_t source, int8_t& dest, bool clamp )          { dest = source; return true; }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int8_t source, int16_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int8_t source, int32_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int8_t source, int64_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int8_t source, uint8_t& dest, bool clamp )          { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int8_t source, uint16_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int8_t source, uint32_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int8_t source, uint64_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int8_t source, float32_t& dest, bool clamp )         { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int8_t source, float64_t& dest, bool clamp )         { return Helium::RangeCastFloat( source, dest, clamp ); }

template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint8_t source, int8_t& dest, bool clamp )          { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint8_t source, int16_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint8_t source, int32_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint8_t source, int64_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint8_t source, uint8_t& dest, bool clamp )          { dest = source; return true; }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint8_t source, uint16_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint8_t source, uint32_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint8_t source, uint64_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint8_t source, float32_t& dest, bool clamp )         { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint8_t source, float64_t& dest, bool clamp )         { return Helium::RangeCastFloat( source, dest, clamp ); }

template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int16_t source, int8_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int16_t source, int16_t& dest, bool clamp )        { dest = source; return true; }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int16_t source, int32_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int16_t source, int64_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int16_t source, uint8_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int16_t source, uint16_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int16_t source, uint32_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int16_t source, uint64_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int16_t source, float32_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int16_t source, float64_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }

template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint16_t source, int8_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint16_t source, int16_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint16_t source, int32_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint16_t source, int64_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint16_t source, uint8_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint16_t source, uint16_t& dest, bool clamp )        { dest = source; return true; }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint16_t source, uint32_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint16_t source, uint64_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint16_t source, float32_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint16_t source, float64_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }

template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int32_t source, int8_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int32_t source, int16_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int32_t source, int32_t& dest, bool clamp )        { dest = source; return true; }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int32_t source, int64_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int32_t source, uint8_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int32_t source, uint16_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int32_t source, uint32_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int32_t source, uint64_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int32_t source, float32_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int32_t source, float64_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }

template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint32_t source, int8_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint32_t source, int16_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint32_t source, int32_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint32_t source, int64_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint32_t source, uint8_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint32_t source, uint16_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint32_t source, uint32_t& dest, bool clamp )        { dest = source; return true; }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint32_t source, uint64_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint32_t source, float32_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint32_t source, float64_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }

template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int64_t source, int8_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int64_t source, int16_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int64_t source, int32_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int64_t source, int64_t& dest, bool clamp )        { dest = source; return true; }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int64_t source, uint8_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int64_t source, uint16_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int64_t source, uint32_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int64_t source, uint64_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int64_t source, float32_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const int64_t source, float64_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }

template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint64_t source, int8_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint64_t source, int16_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint64_t source, int32_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint64_t source, int64_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint64_t source, uint8_t& dest, bool clamp )         { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint64_t source, uint16_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint64_t source, uint32_t& dest, bool clamp )        { return Helium::RangeCastInteger( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint64_t source, uint64_t& dest, bool clamp )        { dest = source; return true; }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint64_t source, float32_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const uint64_t source, float64_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }

template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float32_t source, int8_t& dest, bool clamp )         { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float32_t source, int16_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float32_t source, int32_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float32_t source, int64_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float32_t source, uint8_t& dest, bool clamp )         { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float32_t source, uint16_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float32_t source, uint32_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float32_t source, uint64_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float32_t source, float32_t& dest, bool clamp )        { dest = source; return true; }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float32_t source, float64_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }

template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float64_t source, int8_t& dest, bool clamp )         { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float64_t source, int16_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float64_t source, int32_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float64_t source, int64_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float64_t source, uint8_t& dest, bool clamp )         { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float64_t source, uint16_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float64_t source, uint32_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float64_t source, uint64_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float64_t source, float32_t& dest, bool clamp )        { return Helium::RangeCastFloat( source, dest, clamp ); }
template <> HELIUM_FOUNDATION_API bool Helium::RangeCast( const float64_t source, float64_t& dest, bool clamp )        { dest = source; return true; }

#if HELIUM_DEBUG

static void Test( bool clamp )
{
    // 32->32 int
    {
        int32_t source = NumericLimits<int32_t>::Maximum;
        uint32_t dest = 0;
        RangeCast( source, dest, clamp );
        HELIUM_ASSERT( dest == NumericLimits<int32_t>::Maximum );
    }

    // 32->32 int
    {
        uint32_t source = NumericLimits<uint32_t>::Maximum;
        int32_t dest = 0;
        RangeCast( source, dest, clamp );
        HELIUM_ASSERT( clamp ? dest == NumericLimits<int32_t>::Maximum : dest == 0 );
    }

    // 32->64 int
    {
        int32_t source = NumericLimits<int32_t>::Maximum;
        int64_t dest = 0;
        RangeCast( source, dest, clamp );
        HELIUM_ASSERT( dest == NumericLimits<int32_t>::Maximum );
    }

    // 32->64 float
    {
        float32_t source = NumericLimits<float32_t>::Maximum;
        float64_t dest = 0;
        RangeCast( source, dest, clamp );
        HELIUM_ASSERT( dest == NumericLimits<float32_t>::Maximum );
    }

    // 64->32 int
    {
        int64_t source = NumericLimits<int64_t>::Maximum;
        int32_t dest = 0;
        RangeCast( source, dest, clamp );
        HELIUM_ASSERT( clamp ? dest == NumericLimits<int32_t>::Maximum : dest == 0 );
    }

    // 64->32 float
    {
        float64_t source = NumericLimits<float64_t>::Maximum;
        float32_t dest = 0;
        RangeCast( source, dest, clamp );
        HELIUM_ASSERT( clamp ? dest == NumericLimits<float32_t>::Maximum : dest == 0 );
    }

    // int64_t->float32_t
    {
        int64_t source = NumericLimits<int64_t>::Maximum;
        float32_t dest = 0;
        RangeCast( source, dest, clamp );
        float32_t test = (float32_t)NumericLimits<int64_t>::Maximum;
        HELIUM_ASSERT( dest == test );
    }

    // float64_t->int32_t
    {
        float64_t source = NumericLimits<float64_t>::Maximum;
        int32_t dest = 0;
        RangeCast( source, dest, clamp );
        HELIUM_ASSERT( clamp ? dest == NumericLimits<int32_t>::Maximum : dest == 0 );
    }
}

struct NumericTest
{
    NumericTest()
    {
        Test(true);
        Test(false);
    }
} g_Test;

#endif