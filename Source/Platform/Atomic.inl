template< typename T >
T* Helium::AtomicExchange(T* volatile & rAtomic, T* value)
{
	return reinterpret_cast< T* >(AtomicExchangePointer(
		reinterpret_cast< void* volatile& >(rAtomic),
		reinterpret_cast< void* >(value) ));
}

template< typename T >
T* Helium::AtomicExchangeAcquire( T* volatile & rAtomic, T* value )
{
	return reinterpret_cast< T* >(AtomicExchangePointerAcquire(
		reinterpret_cast< void* volatile& >(rAtomic),
		reinterpret_cast< void* >(value) ));
}

template< typename T >
T* Helium::AtomicExchangeRelease( T* volatile & rAtomic, T* value )
{
	return reinterpret_cast< T* >(AtomicExchangePointerRelease(
		reinterpret_cast< void* volatile& >(rAtomic),
		reinterpret_cast< void* >(value) ));
}

template< typename T >
T* Helium::AtomicExchangeUnsafe( T* volatile & rAtomic, T* value )
{
	return reinterpret_cast< T* >(AtomicExchangePointerUnsafe(
		reinterpret_cast< void* volatile& >(rAtomic),
		reinterpret_cast< void* >(value) ));
}

template< typename T >
T* Helium::AtomicCompareExchange(T* volatile & rAtomic, T* value, T* compare)
{
	return reinterpret_cast< T* >(AtomicCompareExchangePointer(
		reinterpret_cast< void* volatile& >(rAtomic),
		reinterpret_cast< void* >(value),
		reinterpret_cast< void* >(compare) ));
}

template< typename T >
T* Helium::AtomicCompareExchangeAcquire( T* volatile & rAtomic, T* value, T* compare )
{
	return reinterpret_cast< T* >(AtomicCompareExchangePointerAcquire(
		reinterpret_cast< void* volatile& >(rAtomic),
		reinterpret_cast< void* >(value),
		reinterpret_cast< void* >(compare) ));
}

template< typename T >
T* Helium::AtomicCompareExchangeRelease( T* volatile & rAtomic, T* value, T* compare )
{
	return reinterpret_cast< T* >(AtomicCompareExchangePointerRelease(
		reinterpret_cast< void* volatile& >(rAtomic),
		reinterpret_cast< void* >(value),
		reinterpret_cast< void* >(compare) ));
}

template< typename T >
T* Helium::AtomicCompareExchangeUnsafe( T* volatile & rAtomic, T* value, T* compare )
{
	return reinterpret_cast< T* >(AtomicCompareExchangePointerUnsafe(
		reinterpret_cast< void* volatile& >(rAtomic),
		reinterpret_cast< void* >(value),
		reinterpret_cast< void* >(compare) ));
}