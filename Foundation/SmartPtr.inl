template< typename T >
Helium::AutoPtr< T >::AutoPtr( T* ptr )
	: m_Ptr( ptr )
{
	m_Ptr = ptr;
}

template< typename T >
Helium::AutoPtr< T >::AutoPtr( const AutoPtr& rhs )
{
	m_Ptr = AtomicExchange<T>( rhs.m_Ptr, NULL );
}

template< typename T >
template< typename U >
Helium::AutoPtr< T >::AutoPtr( const AutoPtr< U >& rhs )
{
	m_Ptr = rhs.m_Ptr;
	m_Ptr = reinterpret_cast< T* >( AtomicExchange<void>( *reinterpret_cast<void**>( &rhs.m_Ptr ), 0 ) );
}

template< typename T >
Helium::AutoPtr< T >::~AutoPtr()
{
	if ( !IsOrphan() )
	{
		delete Ptr();
	}
}

template< typename T >
T* Helium::AutoPtr< T >::Ptr() const
{
	return reinterpret_cast< T* >( reinterpret_cast< uintptr_t >( m_Ptr ) & ~1 );
}

template< typename T >
T* Helium::AutoPtr< T >::operator->() const
{
	return Ptr();
}

template< typename T >
T& Helium::AutoPtr< T >::operator*() const
{
	return *Ptr();
}

template< typename T >
Helium::AutoPtr< T >::operator T*() const
{
	return Ptr();
}

template< typename T >
Helium::AutoPtr< T >& Helium::AutoPtr< T >::operator=( const AutoPtr& rhs )
{
	m_Ptr = AtomicExchange<T>( rhs.m_Ptr, NULL );
	return *this;
}

template< typename T >
bool Helium::AutoPtr< T >::IsOrphan()
{
	return ( reinterpret_cast< uintptr_t >( m_Ptr ) & 1 ) != 0;
}

template< typename T >
void Helium::AutoPtr< T >::Orphan( bool orphan )
{
	if ( orphan )
	{
		m_Ptr = reinterpret_cast< T* >( reinterpret_cast< uintptr_t >( m_Ptr ) | 1 );
	}
	else
	{
		m_Ptr = reinterpret_cast< T* >( reinterpret_cast< uintptr_t >( m_Ptr ) & ~1 );
	}
}

template< typename T >
void Helium::AutoPtr< T >::Reset(T* ptr, bool carryOrphan)
{
	bool orphan = IsOrphan();
	T* prev = Ptr();
	if (ptr != prev)
	{
		delete prev;
	}
	uintptr_t next = reinterpret_cast<uintptr_t>( ptr );
	HELIUM_ASSERT( ( next & 1 ) == 0 ); // this class only works for non-single-byte-aligned allocations!
	m_Ptr = reinterpret_cast< T* >( next );
	if ( carryOrphan )
	{
		Orphan( orphan );
	}
}

template< typename T >
T* Helium::AutoPtr< T >::Release()
{
	T* return_value = Ptr();
	m_Ptr = NULL;
	return return_value;
}

template< typename T >
Helium::ArrayPtr< T >::ArrayPtr( T* ptr )
	: m_Ptr( ptr )
{
}

template< typename T >
Helium::ArrayPtr< T >::ArrayPtr( const ArrayPtr& rhs )
{
	m_Ptr = AtomicExchange<T>( rhs.m_Ptr, NULL );
}

template< typename T >
Helium::ArrayPtr< T >::~ArrayPtr()
{
	delete [] m_Ptr; 
}

template< typename T >
T* Helium::ArrayPtr< T >::Ptr() const
{
	return m_Ptr; 
}

template< typename T >
T& Helium::ArrayPtr< T >::operator[]( size_t i ) const
{
	return m_Ptr[i];
}

template< typename T >
Helium::ArrayPtr< T >::operator T*() const
{
	return Ptr();
}

template< typename T >
Helium::ArrayPtr< T >& Helium::ArrayPtr< T >::operator=( const ArrayPtr& rhs )
{
	m_Ptr = AtomicExchange<T>( rhs.m_Ptr, NULL );
	return *this;
}

/// Constructor.
template< typename T >
Helium::RefCountBase< T >::RefCountBase()
	: m_RefCount( 0 )
{
}

/// Copy constructor.
///
/// @param[in] rSource  Source object from which to copy.
template< typename T >
Helium::RefCountBase< T >::RefCountBase( const RefCountBase& /*rSource*/ )
	: m_RefCount( 0 )
{
	// Note that the reference count is not copied.
}

/// Destructor.
///
template< typename T >
Helium::RefCountBase< T >::~RefCountBase()
{
}

/// Get the current reference count of this object.
///
/// @return  Current reference count.
template< typename T >
uint32_t Helium::RefCountBase< T >::GetRefCount() const
{
	return m_RefCount;
}

/// Increment the reference count of this object.
///
/// @return  Reference count immediately after incrementing.
///
/// @see DecrRefCount(), GetRefCount()
template< typename T >
uint32_t Helium::RefCountBase< T >::IncrRefCount() const
{
	++m_RefCount;

	// Test for wrapping to zero.
	HELIUM_ASSERT( m_RefCount != 0 );

	return m_RefCount;
}

/// Decrement the reference count of this object, deleting it if the reference count reaches zero.
///
/// @return  Reference count immediately after decrementing.
///
/// @see IncrRefCount(), GetRefCount()
template< typename T >
uint32_t Helium::RefCountBase< T >::DecrRefCount() const
{
	--m_RefCount;
	int32_t newRefCount = m_RefCount;

	if( newRefCount == 0 )
	{
		delete const_cast< T* >( static_cast< const T* >( this ) );
	}

	return newRefCount;
}

/// Assignment operator.
///
/// @param[in] rSource  Source object from which to copy.
template< typename T >
Helium::RefCountBase< T >& Helium::RefCountBase< T >::operator=( const RefCountBase& /*rSource*/ )
{
	// do NOT copy the refcount
	return *this;
}

/// Constructor.
template< typename T >
Helium::AtomicRefCountBase< T >::AtomicRefCountBase()
	: m_RefCount( 0 )
{
}

/// Copy constructor.
///
/// @param[in] rSource  Source object from which to copy.
template< typename T >
Helium::AtomicRefCountBase< T >::AtomicRefCountBase( const AtomicRefCountBase& /*rSource*/ )
	: m_RefCount( 0 )
{
	// Do not copy the reference count.
}

/// Virtual destructor.
template< typename T >
Helium::AtomicRefCountBase< T >::~AtomicRefCountBase()
{
}

/// Get the current reference count of this object.
///
/// @return  Current reference count.
template< typename T >
uint32_t Helium::AtomicRefCountBase< T >::GetRefCount() const
{
	return m_RefCount;
}

/// Increment the reference count of this object.
///
/// @return  Reference count immediately after incrementing.
///
/// @see DecrRefCount(), GetRefCount()
template< typename T >
uint32_t Helium::AtomicRefCountBase< T >::IncrRefCount() const
{
	uint32_t newRefCount = static_cast< uint32_t >( AtomicIncrementUnsafe( m_RefCount ) );

	// Test for wrapping to zero.
	HELIUM_ASSERT( newRefCount != 0 );

	return newRefCount;
}

/// Decrement the reference count of this object, deleting it if the reference count reaches zero.
///
/// @return  Reference count immediately after decrementing.
///
/// @see IncrRefCount(), GetRefCount()
template< typename T >
uint32_t Helium::AtomicRefCountBase< T >::DecrRefCount() const
{
	int32_t newRefCount = AtomicDecrementUnsafe( m_RefCount );
	if( newRefCount == 0 )
	{
		delete this;
	}

	return newRefCount;
}

/// Assignment operator.
///
/// @param[in] rSource  Source object from which to copy.
template< typename T >
Helium::AtomicRefCountBase< T >& Helium::AtomicRefCountBase< T >::operator=( const AtomicRefCountBase& /*rSource*/ )
{
	// Do not copy the reference count.
	return *this;
}

/// Constructor.
template< typename T >
Helium::SmartPtr< T >::SmartPtr()
	: m_Pointer( NULL )
{
}

/// Constructor.
///
/// @param[in] pPointer  Pointer to which this reference should be initialized.
template< typename T >
Helium::SmartPtr< T >::SmartPtr( const T* pPointer )
	: m_Pointer( const_cast< T* >( pPointer ) )
{
	if( pPointer )
	{
		pPointer->IncrRefCount();
	}
}

/// Copy constructor.
///
/// @param[in] rPointer  Reference from which this reference should be initialized.
template< typename T >
Helium::SmartPtr< T >::SmartPtr( const SmartPtr& rPointer )
	: m_Pointer( rPointer.m_Pointer )
{
	if( m_Pointer )
	{
		m_Pointer->IncrRefCount();
	}
}

/// Copy constructor.
///
/// @param[in] rPointer  Reference from which this reference should be initialized.
template< typename T >
template< typename U >
Helium::SmartPtr< T >::SmartPtr( const SmartPtr< U >& rPointer )
	: m_Pointer( rPointer.m_Pointer )
{
	if( m_Pointer )
	{
		m_Pointer->IncrRefCount();
	}
}

/// Destructor.
template< typename T >
Helium::SmartPtr< T >::~SmartPtr()
{
	Release();
}

/// Get the object currently referenced by this smart pointer.
///
/// @return  Pointer to the referenced object
///
/// @see Set()
template< typename T >
T* Helium::SmartPtr< T >::Get() const
{
	return m_Pointer;
}

/// Get the object currently referenced by this smart pointer.
///
/// @return  Pointer to the referenced object
///
/// @see Set()
template< typename T >
T* Helium::SmartPtr< T >::Ptr() const
{
	return m_Pointer;
}

/// Set the object referenced by this smart pointer.
///
/// @param[in] pPointer  Object to set.
///
/// @see Get()
template< typename T >
void Helium::SmartPtr< T >::Set( const T* pPointer )
{
	T* pPointerOld = m_Pointer;
	if( pPointerOld != pPointer )
	{
		m_Pointer = const_cast< T* >( pPointer );

		if( pPointerOld )
		{
			pPointerOld->DecrRefCount();
		}

		if( pPointer )
		{
			pPointer->IncrRefCount();
		}
	}
}

/// Release any currently set object reference, decrementing its reference count.
///
/// @see Set(), Get()
template< typename T >
void Helium::SmartPtr< T >::Release()
{
	T* pPointer = m_Pointer;
	if( pPointer )
	{
		pPointer->DecrRefCount();
		m_Pointer = NULL;
	}
}

/// Get whether this smart pointer references an object.
///
/// @return  True if this smart pointer is set to a non-null pointer, false if it is null.
template< typename T >
bool Helium::SmartPtr< T >::ReferencesObject() const
{
	return ( m_Pointer != NULL );
}

/// Dereference this smart pointer.
///
/// @return  Reference to the currently referenced object.
template< typename T >
T& Helium::SmartPtr< T >::operator*() const
{
	HELIUM_ASSERT( m_Pointer );

	return *m_Pointer;
}

/// Dereference this smart pointer.
///
/// @return  Pointer to the currently referenced object.
template< typename T >
T* Helium::SmartPtr< T >::operator->() const
{
	HELIUM_ASSERT( m_Pointer );

	return m_Pointer;
}

/// Cast this reference to a bare object pointer.
///
/// @return  Pointer to the currently referenced object.
template< typename T >
Helium::SmartPtr< T >::operator T* const&() const
{
	return m_Pointer;
}

/// Assignment operator.
///
/// @param[in] pPointer  Object reference to set.
///
/// @return  Reference to this object.
template< typename T >
Helium::SmartPtr< T >& Helium::SmartPtr< T >::operator=( const T* pPointer )
{
	Set( pPointer );

	return *this;
}

/// Assignment operator.
///
/// @param[in] rPointer  Object reference to set.
///
/// @return  Reference to this object.
template< typename T >
Helium::SmartPtr< T >& Helium::SmartPtr< T >::operator=( const SmartPtr& rSource )
{
	Set( rSource.m_Pointer );

	return *this;
}

/// Assignment operator.
///
/// @param[in] rPointer  Object reference to set.
///
/// @return  Reference to this object.
template< typename T >
template< typename U >
Helium::SmartPtr< T >& Helium::SmartPtr< T >::operator=( const SmartPtr< U >& rSource )
{
	Set( rSource.m_Pointer );

	return *this;
}

template < typename T >
Helium::DeepCompareSmartPtr< T >::DeepCompareSmartPtr()
	: SmartPtr< T >()
{
}

template < typename T >
Helium::DeepCompareSmartPtr< T >::DeepCompareSmartPtr( const Helium::SmartPtr<T>& pPointer )
	: SmartPtr< T >( pPointer )
{
}

template < typename T >
Helium::DeepCompareSmartPtr< T >::DeepCompareSmartPtr( const T* pPointer )
	: SmartPtr< T >( pPointer )
{
}

template < typename T >
bool Helium::DeepCompareSmartPtr< T >::operator<( const DeepCompareSmartPtr& rhs ) const
{
	return (*SmartPtr< T >::Get()) < (*rhs.Get());
}

template < typename T >
bool Helium::DeepCompareSmartPtr< T >::operator==( const DeepCompareSmartPtr& rhs ) const
{
	return (*SmartPtr< T >::Get()) == (*rhs.Get());
}
