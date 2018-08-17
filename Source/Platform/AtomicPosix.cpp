#include "Precompile.h"
#include "Atomic.h"

#include "Platform/Types.h"
#include "Platform/Assert.h"
#include "Platform/System.h"

#define _GENERATE_ATOMIC_WORKER( PREFIX, OPERATION, PARAM_LIST, ACTION ) \
    PREFIX Helium::Atomic##OPERATION PARAM_LIST ACTION \
    PREFIX Helium::Atomic##OPERATION##Acquire PARAM_LIST ACTION \
    PREFIX Helium::Atomic##OPERATION##Release PARAM_LIST ACTION \
    PREFIX Helium::Atomic##OPERATION##Unsafe PARAM_LIST ACTION

#if !defined( HELIUM_CC_GCC ) && !defined( HELIUM_CC_CLANG )
#error "implement atomics for this compiler"
#else

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Exchange,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
#if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
        return __sync_val_compare_and_swap( static_cast< int32_t volatile* >( &rAtomic ), *static_cast< int32_t volatile* >( &rAtomic ), value );
#else
        return __atomic_exchange_n( static_cast< int32_t volatile* >( &rAtomic ), value, __ATOMIC_SEQ_CST );
#endif
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    CompareExchange,
    ( int32_t volatile & rAtomic, int32_t value, int32_t compare ),
    {
#if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
        return __sync_val_compare_and_swap( static_cast< int32_t volatile* >( &rAtomic ), compare, value );
#else
        __atomic_compare_exchange_n( static_cast< int32_t volatile* >( &rAtomic ), &compare, value, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST );
        return compare;
#endif
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Increment,
    ( int32_t volatile & rAtomic ),
    {
#if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
        return __sync_add_and_fetch( static_cast< int32_t volatile* >( &rAtomic ), 1 );
#else
        return __atomic_add_fetch( static_cast< int32_t volatile* >( &rAtomic ), 1, __ATOMIC_SEQ_CST );
#endif
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Decrement,
    ( int32_t volatile & rAtomic ),
    {
#if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
        return __sync_sub_and_fetch( static_cast< int32_t volatile* >( &rAtomic ), 1 );
#else
        return __atomic_sub_fetch( static_cast< int32_t volatile* >( &rAtomic ), 1, __ATOMIC_SEQ_CST );
#endif
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Add,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
#if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
        return __sync_fetch_and_add( static_cast< int32_t volatile* >( &rAtomic ), value );
#else
        return __atomic_fetch_add( static_cast< int32_t volatile* >( &rAtomic ), value, __ATOMIC_SEQ_CST );
#endif
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Subtract,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
#if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
        return __sync_fetch_and_sub( static_cast< int32_t volatile* >( &rAtomic ), value );
#else
        return __atomic_fetch_sub( static_cast< int32_t volatile* >( &rAtomic ), value, __ATOMIC_SEQ_CST );
#endif
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    And,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
#if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
        return __sync_fetch_and_and( static_cast< int32_t volatile* >( &rAtomic ), value );
#else
        return __atomic_fetch_and( static_cast< int32_t volatile* >( &rAtomic ), value, __ATOMIC_SEQ_CST );
#endif
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Or,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
#if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
        return __sync_fetch_and_or( static_cast< int32_t volatile* >( &rAtomic ), value );
#else
        return __atomic_fetch_or( static_cast< int32_t volatile* >( &rAtomic ), value, __ATOMIC_SEQ_CST );
#endif
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Xor,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
#if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
        return __sync_fetch_and_xor( static_cast< int32_t volatile* >( &rAtomic ), value );
#else
        return __atomic_fetch_xor( static_cast< int32_t volatile* >( &rAtomic ), value, __ATOMIC_SEQ_CST );
#endif
    } )

_GENERATE_ATOMIC_WORKER(
    void*,
    ExchangePointer,
    ( void* volatile & rAtomic, void* value ),
    {
#if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
        return static_cast<void*>( __sync_val_compare_and_swap( static_cast< void* volatile* >( &rAtomic ), rAtomic, value ) );
#else
        return static_cast<void*>( __atomic_exchange_n( static_cast< void* volatile* >( &rAtomic ), value, __ATOMIC_SEQ_CST ) );
#endif
    } )

_GENERATE_ATOMIC_WORKER(
    void*,
    CompareExchangePointer,
    ( void* volatile & rAtomic, void* value, void* compare ),
    {
#if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
        return static_cast<void*>( __sync_val_compare_and_swap( reinterpret_cast< void* volatile* >( &rAtomic ), compare, value ) );
#else
        __atomic_compare_exchange_n( static_cast< void* volatile* >( &rAtomic ), &compare, value, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST );
        return compare;
#endif
    } )

#endif //HELIUM_CC_GCC

#undef _GENERATE_ATOMIC_WORKER
