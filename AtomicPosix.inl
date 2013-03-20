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
        return __atomic_exchange_n( reinterpret_cast< volatile int32_t* >( &rAtomic ), value );
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
        return __atomic_compare_exchange_n( reinterpret_cast< volatile int32_t* >( &rAtomic ), &compare, value );
#endif
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Increment,
    ( int32_t volatile & rAtomic ),
    {
        return __atomic_add_fetch( reinterpret_cast< volatile int32_t* >( &rAtomic ), 1 );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Decrement,
    ( int32_t volatile & rAtomic ),
    {
        return __atomic_sub_fetch( reinterpret_cast< volatile int32_t* >( &rAtomic ), 1 );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Add,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
        return __atomic_fetch_add( reinterpret_cast< volatile int32_t* >( &rAtomic ), value );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Subtract,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
        return __atomic_fetch_sub( reinterpret_cast< volatile long* >( &rAtomic ), value );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    And,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
        return __atomic_fetch_and( reinterpret_cast< volatile int32_t* >( &rAtomic ), value );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Or,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
        return __atomic_fetch_or( reinterpret_cast< volatile int32_t* >( &rAtomic ), value );
    } )

_GENERATE_ATOMIC_WORKER(
    int32_t,
    Xor,
    ( int32_t volatile & rAtomic, int32_t value ),
    {
        return __atomic_fetch_or( reinterpret_cast< volatile int32_t* >( &rAtomic ), value );
    } )

#if 0
#if HELIUM_CPU_X86

_GENERATE_ATOMIC_WORKER(
    template< typename T > T*,
    Exchange,
    ( T* volatile & rAtomic, T* value ),
    {
        return reinterpret_cast< T* >( _InterlockedExchange(
            reinterpret_cast< volatile long* >( &rAtomic ),
            reinterpret_cast< long >( value ) ) );
    } )

_GENERATE_ATOMIC_WORKER(
    template< typename T > T*,
    CompareExchange,
    ( T* volatile & rAtomic, T* value, T* compare ),
    {
        return reinterpret_cast< T* >( _InterlockedCompareExchange(
            reinterpret_cast< volatile long* >( &rAtomic ),
            reinterpret_cast< long >( value ),
            reinterpret_cast< long >( compare ) ) );
    } )

#else  // HELIUM_CPU_X86

_GENERATE_ATOMIC_WORKER(
    template< typename T > T*,
    Exchange,
    ( T* volatile & rAtomic, T* value ),
    {
        return static_cast< T* >( _InterlockedExchangePointer(
            reinterpret_cast< void* volatile * >( &rAtomic ),
            value ) );
    } )
_GENERATE_ATOMIC_WORKER(
    template< typename T > T*,
    CompareExchange,
    ( T* volatile & rAtomic, T* value, T* compare ),
    {
        return static_cast< T* >( _InterlockedCompareExchangePointer(
            reinterpret_cast< void* volatile * >( &rAtomic ),
            value,
            compare ) );
    } )

#endif // HELIUM_CPU_X86
#endif // 0

#endif //HELIUM_CC_GCC

#undef _GENERATE_ATOMIC_WORKER
