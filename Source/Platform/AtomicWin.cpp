#include "Precompile.h"
#include "Atomic.h"

#include "Platform/Types.h"
#include "Platform/Assert.h"
#include "Platform/SystemWin.h"

// Windows (non-Itanium platforms) doesn't reorder reads and writes, so memory barriers aren't needed.  As such, all
// variants of each Atomic*() function are the same.  On the downside, we can't easily debug these functions when using
// macros to generate them, and build times may be somewhat slower.
#define _GENERATE_ATOMIC_WORKER( PREFIX, OPERATION, PARAM_LIST, ACTION ) \
    PREFIX Helium::Atomic##OPERATION PARAM_LIST ACTION \
    PREFIX Helium::Atomic##OPERATION##Acquire PARAM_LIST ACTION \
    PREFIX Helium::Atomic##OPERATION##Release PARAM_LIST ACTION \
    PREFIX Helium::Atomic##OPERATION##Unsafe PARAM_LIST ACTION

HELIUM_COMPILE_ASSERT( sizeof(LONG) == sizeof(int32_t) );

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Exchange,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
        return _InterlockedExchange( reinterpret_cast< volatile LONG* >( &rAtomic ), value );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    CompareExchange,
    ( int32_t volatile & rAtomic, int32_t value, int32_t compare ),
    {
        return _InterlockedCompareExchange( reinterpret_cast< volatile LONG* >( &rAtomic ), value, compare );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Increment,
    ( int32_t volatile & rAtomic ),
    {
        return _InterlockedIncrement( reinterpret_cast< volatile LONG* >( &rAtomic ) );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Decrement,
    ( int32_t volatile & rAtomic ),
    {
        return _InterlockedDecrement( reinterpret_cast< volatile LONG* >( &rAtomic ) );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Add,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
        return _InterlockedExchangeAdd( reinterpret_cast< volatile LONG* >( &rAtomic ), value );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Subtract,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
        return _InterlockedExchangeAdd( reinterpret_cast< volatile LONG* >( &rAtomic ), -value );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    And,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
        int32_t originalValueNew = rAtomic;
        int32_t originalValueOld;
        do
        {
            originalValueOld = originalValueNew;
            originalValueNew = _InterlockedCompareExchange(
                reinterpret_cast< volatile LONG* >( &rAtomic ),
                originalValueOld & value,
                originalValueOld );
        } while( originalValueNew != originalValueOld );

        return originalValueNew;
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Or,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
        int32_t originalValueNew = rAtomic;
        int32_t originalValueOld;
        do
        {
            originalValueOld = originalValueNew;
            originalValueNew = _InterlockedCompareExchange(
                reinterpret_cast< volatile LONG* >( &rAtomic ),
                originalValueOld | value,
                originalValueOld );
        } while( originalValueNew != originalValueOld );

        return originalValueNew;
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Xor,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
        int32_t originalValueNew = rAtomic;
        int32_t originalValueOld;
        do
        {
            originalValueOld = originalValueNew;
            originalValueNew = _InterlockedCompareExchange(
                reinterpret_cast< volatile LONG* >( &rAtomic ),
                originalValueOld ^ value,
                originalValueOld );
        } while( originalValueNew != originalValueOld );

        return originalValueNew;
    } )

#if defined( HELIUM_CPU_X86_32 )

_GENERATE_ATOMIC_WORKER(
    void*,
    ExchangePointer,
    ( void* volatile & rAtomic, void* value ),
    {
        return reinterpret_cast< void* >( _InterlockedExchange(
            reinterpret_cast< volatile long* >( &rAtomic ),
            reinterpret_cast< long >( value ) ) );
    } )

_GENERATE_ATOMIC_WORKER(
    void*,
    CompareExchangePointer,
    ( void* volatile & rAtomic, void* value, void* compare ),
    {
        return reinterpret_cast< void* >( _InterlockedCompareExchange(
            reinterpret_cast< volatile long* >( &rAtomic ),
            reinterpret_cast< long >( value ),
            reinterpret_cast< long >( compare ) ) );
    } )

#else

_GENERATE_ATOMIC_WORKER(
    void*,
    ExchangePointer,
    ( void* volatile & rAtomic, void* value ),
    {
        return static_cast< void* >( _InterlockedExchangePointer(
            reinterpret_cast< void* volatile * >( &rAtomic ),
            value ) );
    } )

_GENERATE_ATOMIC_WORKER(
    void*,
    CompareExchangePointer,
    ( void* volatile & rAtomic, void* value, void* compare ),
    {
        return static_cast< void* >( _InterlockedCompareExchangePointer(
            reinterpret_cast< void* volatile * >( &rAtomic ),
            value,
            compare ) );
    } )

#endif

#undef _GENERATE_ATOMIC_WORKER
