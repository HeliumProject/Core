#define _GENERATE_ATOMIC_WORKER( PREFIX, OPERATION, PARAM_LIST, ACTION ) \
    PREFIX Helium::Atomic##OPERATION PARAM_LIST ACTION \
    PREFIX Helium::Atomic##OPERATION##Acquire PARAM_LIST ACTION \
    PREFIX Helium::Atomic##OPERATION##Release PARAM_LIST ACTION \
    PREFIX Helium::Atomic##OPERATION##Unsafe PARAM_LIST ACTION

#if !defined( HELIUM_CC_GCC )
#error "implement atomics for this compiler"
#else

/* this is a (maybe too) clever trick for using the right keywords on gcc 4.6 and earlier */
#if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
# define __atomic_add_fetch __sync_add_and_fetch
# define __atomic_sub_fetch __sync_sub_and_fetch
# define __atomic_or_fetch __sync_or_and_fetch
# define __atomic_xor_fetch __sync_xor_and_fetch
# define __atomic_and_fetch __sync_and_and_fetch
# define __atomic_fetch_add __sync_fetch_and_add
# define __atomic_fetch_sub __sync_fetch_and_sub
# define __atomic_fetch_or __sync_fetch_and_or
# define __atomic_fetch_xor __sync_fetch_and_xor
# define __atomic_fetch_and __sync_fetch_and_and
#endif

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Exchange,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
#if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
        return __sync_val_compare_and_swap( reinterpret_cast< volatile int32_t* >( &rAtomic ), reinterpret_cast< volatile int32_t* >( &rAtomic ), value );
#else
        return __atomic_exchange_n( reinterpret_cast< volatile int32_t* >( &rAtomic ), value, __ATOMIC_SEQ_CST );
#endif
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    CompareExchange,
    ( int32_t volatile & rAtomic, int32_t value, int32_t compare ),
    {
#if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
        return __sync_val_compare_and_swap( reinterpret_cast< volatile int32_t* >( &rAtomic ), compare, value );
#else
        return __atomic_compare_exchange_n( reinterpret_cast< volatile int32_t* >( &rAtomic ), &compare, value, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST );
#endif
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Increment,
    ( int32_t volatile & rAtomic ),
    {
        return __atomic_add_fetch( reinterpret_cast< volatile int32_t* >( &rAtomic ), 1, __ATOMIC_SEQ_CST );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Decrement,
    ( int32_t volatile & rAtomic ),
    {
        return __atomic_sub_fetch( reinterpret_cast< volatile int32_t* >( &rAtomic ), 1, __ATOMIC_SEQ_CST );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Add,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
        return __atomic_fetch_add( reinterpret_cast< volatile int32_t* >( &rAtomic ), value, __ATOMIC_SEQ_CST );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Subtract,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
        return __atomic_fetch_sub( reinterpret_cast< volatile long* >( &rAtomic ), value, __ATOMIC_SEQ_CST );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    And,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
        return __atomic_fetch_and( reinterpret_cast< volatile int32_t* >( &rAtomic ), value, __ATOMIC_SEQ_CST );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Or,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
        return __atomic_fetch_or( reinterpret_cast< volatile int32_t* >( &rAtomic ), value, __ATOMIC_SEQ_CST );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Xor,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
        return __atomic_fetch_xor( reinterpret_cast< volatile int32_t* >( &rAtomic ), value, __ATOMIC_SEQ_CST );
    } )

_GENERATE_ATOMIC_WORKER(
    template< typename T > T*,
    Exchange,
    ( T* volatile & rAtomic, T* value ),
    {
#if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
        return __sync_val_compare_and_swap( reinterpret_cast< volatile T* >( &rAtomic ), reinterpret_cast< volatile T* >( &rAtomic ), value );
#else
        return __atomic_exchange_n( reinterpret_cast< volatile T* >( &rAtomic ), value, __ATOMIC_SEQ_CST );
#endif
    } )

_GENERATE_ATOMIC_WORKER(
    template< typename T > T*,
    CompareExchange,
    ( T* volatile & rAtomic, T* value, T* compare ),
    {
#if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
        return __sync_val_compare_and_swap( rAtomic, compare, value );
#else
        return __atomic_compare_exchange_n( rAtomic, &compare, value, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST );
#endif
    } )

#endif //HELIUM_CC_GCC

#undef _GENERATE_ATOMIC_WORKER
