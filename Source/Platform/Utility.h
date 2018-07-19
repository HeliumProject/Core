#pragma once

#include "Platform/API.h"
#include "Platform/Types.h"
#include "Platform/Assert.h"
#include "Platform/System.h"

#include <type_traits>
#include <cstdarg>
#include <cstring>

/// @defgroup utilitymacros General Utility Macros
//@{

/// Specify a variable as purposefully unreferenced.
///
/// @param[in] VAR  Variable that may be unreferenced across its scoped.
#define HELIUM_UNREF( VAR ) ( void )( VAR )

/// Get the number of elements in a static array.
///
/// @param[in] ARRAY  Static array.
///
/// @return  Number of elements in the array.
#define HELIUM_ARRAY_COUNT( ARRAY ) ( sizeof( ARRAY ) / sizeof( ARRAY[ 0 ] ) )

/// Get the byte offset of a member of a struct or class.
///
/// @param[in] TYPE    Struct or class type name.
/// @param[in] MEMBER  Member name.
///
/// @return  Byte offset of the specified member.
#define HELIUM_OFFSET_OF( TYPE, MEMBER ) ( reinterpret_cast< size_t >( &static_cast< TYPE* >( NULL )->MEMBER ) )

/// Get the bitfield with only the most significant bit set.
///
/// @param[in] TYPE    Any type (should be numeric).
///
/// @return  Constant number of the highest bit set, all other bits zero.
#define HELIUM_HIGH_BIT( TYPE ) ( 1 << ( sizeof( TYPE ) * 8 ) )

//@}

namespace Helium
{
    /// @defgroup utilitymem Memory Utility Functions
    //@{
    inline void MemoryCopy( void* pDest, const void* pSource, size_t size );
    inline void MemoryMove( void* pDest, const void* pSource, size_t size );
    inline void MemorySet( void* pDest, int value, size_t size );
    inline void MemoryZero( void* pDest, size_t size );
    inline int MemoryCompare( const void* pMemory0, const void* pMemory1, size_t size );

    template< typename T > void InPlaceConstruct( void* pMemory );
    template< typename T > void InPlaceDestruct( void* pMemory );

	template< typename T > void ArrayCopy( T* pDest, const T* pSource, size_t count );
    template< typename T > void ArrayMove( T* pDest, const T* pSource, size_t count );
    template< typename T > void ArraySet( T* pDest, const T& rValue, size_t count );

    template< typename T > T* ArrayInPlaceConstruct( void* pMemory, size_t count );
    template< typename T > void ArrayInPlaceDestruct( T* pMemory, size_t count );
    template< typename T > void ArrayUninitializedCopy( T* pDest, const T* pSource, size_t count );
    template< typename T > void ArrayUninitializedFill( T* pDest, const T& rValue, size_t count );

    template< typename T > T Align( const T& rValue, size_t alignment );

    template< typename T > void Swap( T& rValue0, T& rValue1 );
    //@}

    /// @defgroup utilitystring String Utility Functions
    //@{
    template< typename T > uint32_t StringHash( const T* pString );
    template< typename T > size_t StringLength( const T* pString );

	template< class T, size_t N >
	inline void CopyString( T (&dest)[N], const T* src, size_t count = 0 );
	template< class T >
	inline void CopyString( T* dest, size_t destCount, const T* src, size_t count = 0 );

	template< class T, size_t N >
	inline void AppendString( T (&dest)[N], const T* src, size_t count = 0 );
	template< class T >
	inline void AppendString( T* dest, size_t destCount, const T* src, size_t count = 0 );

	template < class T >
	inline int CompareString( const T* a, const T* b, size_t count = 0 );
	template < class T >
	inline int CaseInsensitiveCompareString( const T* a, const T* b, size_t count = 0 );

	template< class T >
	inline const T* FindCharacter( const T* data, const T value, size_t count = 0 );
	template< class T >
	inline const T* FindNextToken( const T* data, const T delim, size_t count = 0 );
	//@}

    /// @defgroup utilityindex Index Utility Functions
    //@{
    template< typename IndexType > IndexType Invalid();
    template< typename IndexType > bool IsValid( IndexType index );
    template< typename IndexType > bool IsInvalid( IndexType index );
    template< typename IndexType > void SetInvalid( IndexType& rIndex );
    template< typename DestIndexType, typename SourceIndexType > DestIndexType CastIndex( SourceIndexType index );
    //@}

    /// @defgroup utilitybit Bit Manipulation Utility Functions
    //@{
    template< typename ElementType > void GetBitElementAndMaskIndex( size_t bitIndex, size_t& rElementIndex, size_t& rMaskIndex );

    template< typename ElementType > bool GetBit( const ElementType& rElement, size_t maskIndex );
    template< typename ElementType > void SetBit( ElementType& rElement, size_t maskIndex );
    template< typename ElementType > void ClearBit( ElementType& rElement, size_t maskIndex );
    template< typename ElementType > void ToggleBit( ElementType& rElement, size_t maskIndex );

    template< typename ElementType > void SetBitRange( ElementType* pElements, size_t bitStart, size_t bitCount );
    template< typename ElementType > void ClearBitRange( ElementType* pElements, size_t bitStart, size_t bitCount );
    template< typename ElementType > void ToggleBitRange( ElementType* pElements, size_t bitStart, size_t bitCount );
    //@}

    /// @defgroup utilitydatareading Data Reading & Byte-swapping Utility Functions
    /// Note that all of these functions can perform in-place byte swapping (source and destination arguments are the same).
    //@{
    template< typename T > const void* LoadValue( T& rDest, const void* pSource );
    template< typename T > const void* LoadValueSwapped( T& rDest, const void* pSource );

    inline void ReverseByteOrder( void* pDest, const void* pSource, size_t size );
    //@}

    /// Non-copyable base class.
    class HELIUM_PLATFORM_API NonCopyable
    {
    public:
        /// @name Construction/Destruction
        //@{
        inline NonCopyable();
        //@}

    private:
        /// @name Construction/Destruction, Private
        //@{
        NonCopyable( const NonCopyable& );  // Not implemented.
        //@}

        /// @name Overloaded Operators
        //@{
        NonCopyable& operator=( const NonCopyable& );  // Not implemented.
        //@}
    };
}

#include "Platform/Utility.inl"
