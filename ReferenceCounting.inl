/// Fetch the references counts for an objects.
///
/// @return The reference counts, high short are weak references, low short are strong references
template< typename T >
int32_t Helium::RefCountProxyBase< T >::GetRefCounts() const
{
	return m_refCounts;
}

/// Initialize this reference count proxy object.
///
/// @param[in] pObject           Object for which to manage reference counting.
/// @param[in] pDestroyCallback  Callback to execute when the object needs to be destroyed.
template< typename BaseT >
void Helium::RefCountProxy< BaseT >::Initialize( BaseT* pObject )
{
    HELIUM_ASSERT( pObject );

    m_pObject = pObject;
    m_refCounts = 0;
}

/// Get the pointer to the object managed by this proxy.
///
/// @return  Pointer to the managed object.
template< typename BaseT >
BaseT* Helium::RefCountProxy< BaseT >::GetObject() const
{
    return static_cast< BaseT* >( m_pObject );
}

/// Set the pointer to the object (if it hasn't been setup already), used for deferred fixups.
///
/// @see GetObject()
template< typename BaseT >
void Helium::RefCountProxy< BaseT >::SetObject( BaseT* object )
{
	if ( HELIUM_VERIFY( m_pObject == NULL ) )
	{
		m_pObject = object;
	}
}

/// Increment the strong reference count.
///
/// @see RemoveStrongRef(), GetStrongRefCount(), AddWeakRef(), RemoveWeakRef(), GetWeakRefCount()
template< typename BaseT >
void Helium::RefCountProxy< BaseT >::AddStrongRef()
{
    AtomicIncrementAcquire( m_refCounts );
}

/// Decrement the strong reference count.
///
/// @return  True if there are no more strong or weak references, false otherwise.
///
/// @see AddStrongRef(), GetStrongRefCount(), AddWeakRef(), RemoveWeakRef(), GetWeakRefCount()
template< typename BaseT >
bool Helium::RefCountProxy< BaseT >::RemoveStrongRef()
{
    int32_t newRefCounts = AtomicDecrementRelease( m_refCounts );
    HELIUM_ASSERT( ( static_cast< uint32_t >( newRefCounts ) & 0xffff ) != 0xffff );
    if( ( static_cast< uint32_t >( newRefCounts ) & 0xffff ) == 0 )
    {
        DestroyObject();
    }

    return ( newRefCounts == 0 );
}

/// Get the current number of strong references for this proxy.
///
/// @return  Strong reference count.
///
/// @see AddStrongRef(), RemoveStrongRef(), AddWeakRef(), RemoveWeakRef(), GetWeakRefCount()
template< typename BaseT >
uint16_t Helium::RefCountProxy< BaseT >::GetStrongRefCount() const
{
    return static_cast< uint16_t >( static_cast< uint32_t >( m_refCounts ) & 0xffff );
}

/// Increment the weak reference count.
///
/// @see RemoveWeakRef(), GetWeakRefCount(), AddStrongRef(), RemoveStrongRef(), GetStrongRefCount()
template< typename BaseT >
void Helium::RefCountProxy< BaseT >::AddWeakRef()
{
    AtomicAddAcquire( m_refCounts, 0x10000 );
}

/// Decrement the weak reference count.
///
/// @return  True if there are no more strong or weak references, false otherwise.
///
/// @see AddWeakRef(), GetWeakRefCount(), AddStrongRef(), RemoveStrongRef(), GetStrongRefCount()
template< typename BaseT >
bool Helium::RefCountProxy< BaseT >::RemoveWeakRef()
{
    // Remember: AtomicSubtractRelease() returns the original value, not the new value.
    int32_t oldRefCounts = AtomicSubtractRelease( m_refCounts, 0x10000 );
    HELIUM_ASSERT( ( static_cast< uint32_t >( oldRefCounts ) >> 16 ) != 0 );

    return ( oldRefCounts == 0x10000 );
}

/// Get the current number of weak references for this proxy.
///
/// @return  Weak reference count.
///
/// @see AddWeakRef(), RemoveWeakRef(), AddStrongRef(), RemoveStrongRef(), GetStrongRefCount()
template< typename BaseT >
uint16_t Helium::RefCountProxy< BaseT >::GetWeakRefCount() const
{
    return static_cast< uint16_t >( static_cast< uint32_t >( m_refCounts ) >> 16 );
}

template< typename BaseT >
void Helium::RefCountProxy< BaseT >::Swap( Helium::RefCountProxy< BaseT >* other )
{
	Helium::Swap( m_pObject, other->m_pObject );
}

/// Helper function for performing proper object destruction upon its strong reference count reaching zero.
template< typename BaseT >
void Helium::RefCountProxy< BaseT >::DestroyObject()
{
    BaseT* pObject = static_cast< BaseT* >( m_pObject );
    if ( pObject )
	{
		BaseT::RefCountSupportType::PreDestroy( pObject );

		BaseT* pAtomicObjectOld = static_cast< BaseT* >( AtomicExchangeRelease< void >( m_pObject, NULL ) );
		HELIUM_ASSERT( pAtomicObjectOld == pObject );
		HELIUM_UNREF( pAtomicObjectOld );

		BaseT::RefCountSupportType::Destroy( pObject );
	}
}

/// Constructor.
template< typename BaseT >
Helium::RefCountProxyContainer< BaseT >::RefCountProxyContainer()
    : m_pProxy( NULL )
{
}

/// Get the reference count proxy, initializing it if necessary.
///
/// @param[in] pObject  Object to which the proxy should be initialized if needed.
///
/// @return  Pointer to the reference count proxy instance.
template< typename BaseT >
Helium::RefCountProxy< BaseT >* Helium::RefCountProxyContainer< BaseT >::Get( BaseT* pObject )
{
    RefCountProxy< BaseT >* pProxy = m_pProxy;
    if( !pProxy )
    {
        pProxy = SupportType::Allocate();
        HELIUM_ASSERT( pProxy );
        pProxy->Initialize( pObject );

        // Atomically set the existing proxy, making sure the proxy is still null.  If another proxy was swapped in
        // first, release the proxy we just tried to allocate.
        RefCountProxy< BaseT >* pExistingProxy = AtomicCompareExchangeRelease< RefCountProxy< BaseT > >( m_pProxy, pProxy, NULL );
        if( pExistingProxy )
        {
            SupportType::Release( pProxy );
            pProxy = pExistingProxy;
        }
    }

    return pProxy;
}

/// Set the reference count proxy manually.
///
/// @param[in] proxy  The proxy to use for reference counting.
///
/// @return  Pointer to the reference count proxy instance.
template< typename BaseT >
void Helium::RefCountProxyContainer< BaseT >::Set( RefCountProxy< BaseT >* pProxy )
{
	m_pProxy = pProxy;
}

/// Set the reference count proxy manually.
///
/// @param[in] proxy  The proxy to use for reference counting.
///
/// @return  Pointer to the reference count proxy instance.
template< typename BaseT >
void Helium::RefCountProxyContainer< BaseT >::Swap( RefCountProxyContainer< BaseT >* pOther )
{
	Helium::Swap( m_pProxy, pOther->m_pProxy );
}

/// Constructor.
template< typename PointerT >
Helium::StrongPtr< PointerT >::StrongPtr()
    : m_pProxy( NULL )
{
}

/// Constructor.
///
/// @param[in] pObject  Object to initially assign.
template< typename PointerT >
Helium::StrongPtr< PointerT >::StrongPtr( PointerT* pObject )
    : m_pProxy( NULL )
{
    if( pObject )
    {
        m_pProxy = reinterpret_cast< RefCountProxyBase< PointerT >* >( pObject->GetRefCountProxy() );
        HELIUM_ASSERT( m_pProxy );
        reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy )->AddStrongRef();
    }
}

/// Constructor.
///
/// @param[in] rPointer  Weak pointer from which to copy.
template< typename PointerT >
Helium::StrongPtr< PointerT >::StrongPtr( const WeakPtr< PointerT >& rPointer )
    : m_pProxy( rPointer.m_pProxy )
{
    // Note that a weak pointer can have a reference count proxy set to null, so we need to check for and handle
    // that case as well.
    if( m_pProxy )
    {
        RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pProxy =
            static_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy );
        if( pProxy->GetObject() )
        {
            pProxy->AddStrongRef();
        }
        else
        {
            m_pProxy = NULL;
        }
    }
}

/// Copy constructor.
///
/// @param[in] rPointer  Strong pointer from which to copy.
template< typename PointerT >
Helium::StrongPtr< PointerT >::StrongPtr( const StrongPtr& rPointer )
    : m_pProxy( rPointer.m_pProxy )
{
    if( m_pProxy )
    {
        reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy )->AddStrongRef();
    }
}

/// Destructor.
template< typename PointerT >
Helium::StrongPtr< PointerT >::~StrongPtr()
{
    Release();
}

/// Get the object referenced by this smart pointer.
///
/// @return  Pointer to the referenced object.
///
/// @see Set(), Release()
template< typename PointerT >
PointerT* Helium::StrongPtr< PointerT >::Get() const
{
    typedef typename PointerT::RefCountSupportType RefCountSupportType;
    return ( m_pProxy
        ? static_cast< PointerT* >( reinterpret_cast< RefCountProxy< typename RefCountSupportType::BaseType >* >( m_pProxy )->GetObject() )
        : NULL );
}

/// Get the object referenced by this smart pointer.
///
/// @return  Pointer to the referenced object.
///
/// @see Set(), Release()
template< typename PointerT >
PointerT* Helium::StrongPtr< PointerT >::Ptr() const
{
    return Get();
}

/// Set the object referenced by this smart pointer.
///
/// @param[in] pObject  Object to reference.
///
/// @see Get(), Release()
template< typename PointerT >
void Helium::StrongPtr< PointerT >::Set( PointerT* pObject )
{
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy );
    if( pProxy )
    {
        if( pProxy->GetObject() == pObject )
        {
            return;
        }

        m_pProxy = NULL;

        if( pProxy->RemoveStrongRef() )
        {
            typedef typename PointerT::RefCountSupportType RefCountSupportType;
            RefCountSupportType::Release( pProxy );
        }
    }

    if( pObject )
    {
        m_pProxy = reinterpret_cast< RefCountProxyBase< PointerT >* >( pObject->GetRefCountProxy() );
        HELIUM_ASSERT( m_pProxy );
        reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy )->AddStrongRef();
    }
}

/// Release any object referenced by this smart pointer.
///
/// @see Get(), Set()
template< typename PointerT >
void Helium::StrongPtr< PointerT >::Release()
{
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pProxy =
        reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy );
    m_pProxy = NULL;

    if( pProxy && pProxy->RemoveStrongRef() )
    {
        PointerT::RefCountSupportType::Release( pProxy );
    }
}

/// Get whether this smart pointer references an object.
///
/// @return  True if this smart pointer is set to a non-null pointer, false if it is null.
template< typename PointerT >
bool Helium::StrongPtr< PointerT >::ReferencesObject() const
{
    // Proxy object should never be holding a null reference for strong pointers, so we should only have to check
    // whether we have a proxy object set.
    HELIUM_ASSERT(
        !m_pProxy ||
        reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy )->GetObject() );

    return ( m_pProxy != NULL );
}

/// Get the proxy object in use by this pointer.
///
/// @return  The proxy object in use by this pointer.
/// @see SetProxy()
template< typename PointerT >
Helium::RefCountProxyBase< PointerT >* Helium::StrongPtr< PointerT >::GetProxy() const
{
	return m_pProxy;
}

/// Set the proxy object in use by this pointer (if it hasn't been set already), used for deferred fixups.
///
/// @see GetProxy()
template< typename PointerT >
void Helium::StrongPtr< PointerT >::SetProxy( RefCountProxyBase< PointerT >* pProxy )
{
	if ( HELIUM_VERIFY( m_pProxy == NULL ) )
	{
		m_pProxy = pProxy;
	}
}

/// Get the object referenced by this smart pointer.
///
/// @return  Pointer to the referenced object.
///
/// @see Set(), Release()
template< typename PointerT >
Helium::StrongPtr< PointerT >::operator PointerT*() const
{
    return Get();
}

/// Implicit cast to a strong pointer of a base type.
///
/// @return  Constant reference to the cast strong pointer.
template< typename PointerT >
template< typename BaseT >
Helium::StrongPtr< PointerT >::operator const Helium::StrongPtr< BaseT >&() const
{
    return ImplicitUpCast< BaseT >( std::is_base_of< BaseT, PointerT >() );
}

/// Dereference this pointer.
///
/// @return  Reference to the actual object.
template< typename PointerT >
PointerT& Helium::StrongPtr< PointerT >::operator*() const
{
    PointerT* pObject = Get();
    HELIUM_ASSERT( pObject );

    return *pObject;
}

/// Dereference this pointer.
///
/// @return  Pointer to the actual object.
template< typename PointerT >
PointerT* Helium::StrongPtr< PointerT >::operator->() const
{
    PointerT* pObject = Get();
    HELIUM_ASSERT( pObject );

    return pObject;
}

/// Assignment operator.
///
/// @param[in] pObject  Object to assign.
///
/// @return  Reference to this object.
template< typename PointerT >
Helium::StrongPtr< PointerT >& Helium::StrongPtr< PointerT >::operator=( PointerT* pObject )
{
    Set( pObject );
    return *this;
}

/// Assignment operator.
///
/// @param[in] rPointer  Smart pointer from which to copy.
///
/// @return  Reference to this object.
template< typename PointerT >
Helium::StrongPtr< PointerT >& Helium::StrongPtr< PointerT >::operator=( const WeakPtr< PointerT >& rPointer )
{
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pThisProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy );
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pOtherProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( rPointer.m_pProxy );

    // Note that a weak pointer can have a reference count proxy whose object is set to null, so we need to check for and handle that case as well.
    if( pOtherProxy && !pOtherProxy->GetObject() )
    {
        pOtherProxy = NULL;
    }

    if( pThisProxy != pOtherProxy )
    {
        Release();

        m_pProxy = reinterpret_cast< RefCountProxyBase< PointerT >* >( pOtherProxy );
        if( pOtherProxy )
        {
            pOtherProxy->AddStrongRef();
        }
    }

    return *this;
}

/// Assignment operator.
///
/// @param[in] rPointer  Smart pointer from which to copy.
///
/// @return  Reference to this object.
template< typename PointerT >
Helium::StrongPtr< PointerT >& Helium::StrongPtr< PointerT >::operator=( const StrongPtr& rPointer )
{
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pThisProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy );
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pOtherProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( rPointer.m_pProxy );
    if( pThisProxy != pOtherProxy )
    {
        Release();

        m_pProxy = reinterpret_cast< RefCountProxyBase< PointerT >* >( pOtherProxy );
        if( pOtherProxy )
        {
            pOtherProxy->AddStrongRef();
        }
    }

    return *this;
}

/// Equality comparison operator.
///
/// @param[in] rPointer  Smart pointer with which to compare.
///
/// @return  True if this smart pointer and the given smart pointer reference the same object, false if not.
template< typename PointerT >
bool Helium::StrongPtr< PointerT >::operator==( const WeakPtr< PointerT >& rPointer ) const
{
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pThisProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy );
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pOtherProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( rPointer.m_pProxy );

	// Note that a weak pointer can have a reference count proxy whose object is set to null, so we need to check
    // for and handle that case as well.
    if( pOtherProxy && !pOtherProxy->GetObject() )
    {
        pOtherProxy = NULL;
    }

    return ( pThisProxy == pOtherProxy );
}

/// Equality comparison operator.
///
/// @param[in] rPointer  Smart pointer with which to compare.
///
/// @return  True if this smart pointer and the given smart pointer reference the same object, false if not.
template< typename PointerT >
bool Helium::StrongPtr< PointerT >::operator==( const StrongPtr& rPointer ) const
{
    return ( m_pProxy == rPointer.m_pProxy );
}

/// Inequality comparison operator.
///
/// @param[in] rPointer  Smart pointer with which to compare.
///
/// @return  True if this smart pointer and the given smart pointer do not reference the same object, false if they do.
template< typename PointerT >
bool Helium::StrongPtr< PointerT >::operator!=( const WeakPtr< PointerT >& rPointer ) const
{
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pThisProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy );
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pOtherProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( rPointer.m_pProxy );

    // Note that a weak pointer can have a reference count proxy whose object is set to null, so we need to check for and handle that case as well.
    if( pOtherProxy && !pOtherProxy->GetObject() )
    {
        pOtherProxy = NULL;
    }

    return ( pThisProxy != pOtherProxy );
}

/// Inequality comparison operator.
///
/// @param[in] rPointer  Smart pointer with which to compare.
///
/// @return  True if this smart pointer and the given smart pointer do not reference the same object, false if they do.
template< typename PointerT >
bool Helium::StrongPtr< PointerT >::operator!=( const StrongPtr& rPointer ) const
{
    return ( m_pProxy != rPointer.m_pProxy );
}

/// Helper function for performing a compile-time verified up-cast of a StrongPtr.
///
/// @param[in] rIsProperBase  Instance of std::is_base_of< BaseT, PointerT > if it inherits from std::true_type.
///
/// @return  Constant reference to the cast strong pointer.
template< typename PointerT >
template< typename BaseT >
const Helium::StrongPtr< BaseT >& Helium::StrongPtr< PointerT >::ImplicitUpCast( const std::true_type& /*rIsProperBase*/ ) const
{
    return *reinterpret_cast< const StrongPtr< BaseT >* >( this );
}

/// Constructor.
template< typename PointerT >
Helium::WeakPtr< PointerT >::WeakPtr()
    : m_pProxy( NULL )
{
}

/// Constructor.
///
/// @param[in] pObject  Object to initially assign.
template< typename PointerT >
Helium::WeakPtr< PointerT >::WeakPtr( PointerT* pObject )
    : m_pProxy( NULL )
{
    if( pObject )
    {
        m_pProxy = reinterpret_cast< RefCountProxyBase< PointerT >* >( pObject->GetRefCountProxy() );
        HELIUM_ASSERT( m_pProxy );
        reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy )->AddWeakRef();
    }
}

/// Constructor.
///
/// @param[in] rPointer  Strong pointer from which to copy.
template< typename PointerT >
Helium::WeakPtr< PointerT >::WeakPtr( const StrongPtr< PointerT >& rPointer )
    : m_pProxy( rPointer.m_pProxy )
{
    if( m_pProxy )
    {
        reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy )->AddWeakRef();
    }
}

/// Copy constructor.
///
/// @param[in] rPointer  Weak pointer from which to copy.
template< typename PointerT >
Helium::WeakPtr< PointerT >::WeakPtr( const WeakPtr& rPointer )
    : m_pProxy( rPointer.m_pProxy )
{
    // Note that a weak pointer can have a reference count proxy set to null, so we need to check for and handle
    // that case as well.
    if( m_pProxy )
    {
        RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy );
        if( pProxy->GetObject() )
        {
            pProxy->AddWeakRef();
        }
        else
        {
            m_pProxy = NULL;
        }
    }
}

/// Destructor.
template< typename PointerT >
Helium::WeakPtr< PointerT >::~WeakPtr()
{
    Release();
}

/// Get the object referenced by this smart pointer.
///
/// @return  Pointer to the referenced object.
///
/// @see Set(), Release()
template< typename PointerT >
PointerT* Helium::WeakPtr< PointerT >::Get() const
{
    return m_pProxy ? static_cast< PointerT* >( reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy )->GetObject() ) : NULL;
}

/// Get the object referenced by this smart pointer.
///
/// @return  Pointer to the referenced object.
///
/// @see Set(), Release()
template< typename PointerT >
PointerT* Helium::WeakPtr< PointerT >::Ptr() const
{
    return Get();
}

/// Set the object referenced by this smart pointer.
///
/// @param[in] pObject  Object to reference.
///
/// @see Get(), Release()
template< typename PointerT >
void Helium::WeakPtr< PointerT >::Set( PointerT* pObject )
{
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy );
    if( pProxy )
    {
        // Note that a weak pointer can have a reference count proxy set to null, so we need to check for and handle
        // that case as well.
        if( pObject && pProxy->GetObject() == pObject )
        {
            return;
        }

        m_pProxy = NULL;

        if( pProxy->RemoveWeakRef() )
        {
            typedef typename PointerT::RefCountSupportType RefCountSupportType;
            RefCountSupportType::Release( pProxy );
        }
    }

    if( pObject )
    {
        m_pProxy = reinterpret_cast< RefCountProxyBase< PointerT >* >( pObject->GetRefCountProxy() );
        HELIUM_ASSERT( m_pProxy );
        reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy )->AddWeakRef();
    }
}

/// Release any object referenced by this smart pointer.
///
/// @see Get(), Set()
template< typename PointerT >
void Helium::WeakPtr< PointerT >::Release()
{
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy );
    m_pProxy = NULL;

    if( pProxy && pProxy->RemoveWeakRef() )
    {
        typedef typename PointerT::RefCountSupportType RefCountSupportType;
        RefCountSupportType::Release( pProxy );
    }
}

/// Get whether this smart pointer references an object.
///
/// @return  True if this smart pointer is set to a non-null pointer, false if it is null.
template< typename PointerT >
bool Helium::WeakPtr< PointerT >::ReferencesObject() const
{
    // Proxy object can be holding a null reference for weak pointers, so make sure the actual object reference exists.
    return ( Get() != NULL );
}

/// Get the proxy object in use by this pointer.
///
/// @return  The proxy object in use by this pointer.
/// @see SetProxy()
template< typename PointerT >
Helium::RefCountProxyBase< PointerT >* Helium::WeakPtr< PointerT >::GetProxy() const
{
	return m_pProxy;
}

/// Set the proxy object in use by this pointer (if it hasn't been set already), used for deferred fixups.
///
/// @see GetProxy()
template< typename PointerT >
void Helium::WeakPtr< PointerT >::SetProxy( RefCountProxyBase< PointerT >* pProxy )
{
	if ( HELIUM_VERIFY( m_pProxy == NULL ) )
	{
		m_pProxy = pProxy;
	}
}
/// Get whether this weak pointer is holding onto the reference counting proxy object for the given object.
///
/// @return  True if this object is holding onto the given object's reference counting proxy, false if not.
template< typename PointerT >
bool Helium::WeakPtr< PointerT >::HasObjectProxy( const PointerT* pObject ) const
{
    HELIUM_ASSERT( pObject );
    return ( m_pProxy == reinterpret_cast< RefCountProxyBase< PointerT >* >( pObject->GetRefCountProxy() ) );
}

/// Get the object referenced by this smart pointer.
///
/// @return  Pointer to the referenced object.
///
/// @see Set(), Release()
template< typename PointerT >
Helium::WeakPtr< PointerT >::operator PointerT*() const
{
    return Get();
}

/// Implicit cast to a weak pointer of a base type.
///
/// @return  Constant reference to the cast weak pointer.
template< typename PointerT >
template< typename BaseT >
Helium::WeakPtr< PointerT >::operator const Helium::WeakPtr< BaseT >&() const
{
    return ImplicitUpCast< BaseT >( std::is_base_of< BaseT, PointerT >() );
}

/// Dereference this pointer.
///
/// @return  Reference to the actual object.
template< typename PointerT >
PointerT& Helium::WeakPtr< PointerT >::operator*() const
{
    PointerT* pObject = Get();
    HELIUM_ASSERT( pObject );
    return *pObject;
}

/// Dereference this pointer.
///
/// @return  Pointer to the actual object.
template< typename PointerT >
PointerT* Helium::WeakPtr< PointerT >::operator->() const
{
    PointerT* pObject = Get();
    HELIUM_ASSERT( pObject );
    return pObject;
}

/// Assignment operator.
///
/// @param[in] pObject  Object to assign.
///
/// @return  Reference to this object.
template< typename PointerT >
Helium::WeakPtr< PointerT >& Helium::WeakPtr< PointerT >::operator=( PointerT* pObject )
{
    Set( pObject );
    return *this;
}

/// Assignment operator.
///
/// @param[in] rPointer  Smart pointer from which to copy.
///
/// @return  Reference to this object.
template< typename PointerT >
Helium::WeakPtr< PointerT >& Helium::WeakPtr< PointerT >::operator=( const StrongPtr< PointerT >& rPointer )
{
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pThisProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy );
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pOtherProxy = static_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( rPointer.m_pProxy );
    if( pThisProxy != pOtherProxy )
    {
        Release();

        m_pProxy = reinterpret_cast< RefCountProxyBase< PointerT >* >( pOtherProxy );
        if( pOtherProxy )
        {
            pOtherProxy->AddWeakRef();
        }
    }

    return *this;
}

/// Assignment operator.
///
/// @param[in] rPointer  Smart pointer from which to copy.
///
/// @return  Reference to this object.
template< typename PointerT >
Helium::WeakPtr< PointerT >& Helium::WeakPtr< PointerT >::operator=( const WeakPtr& rPointer )
{
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pThisProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy );
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pOtherProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( rPointer.m_pProxy );

    // Note that a weak pointer can have a reference count proxy whose object is set to null, so we need to check for and handle that case as well.
    if( pOtherProxy && !pOtherProxy->GetObject() )
    {
        pOtherProxy = NULL;
    }

    if( pThisProxy != pOtherProxy )
    {
        Release();

        m_pProxy = reinterpret_cast< RefCountProxyBase< PointerT >* >( pOtherProxy );
        if( pOtherProxy )
        {
            pOtherProxy->AddWeakRef();
        }
    }

    return *this;
}

/// Equality comparison operator.
///
/// @param[in] rPointer  Smart pointer with which to compare.
///
/// @return  True if this smart pointer and the given smart pointer reference the same object, false if not.
template< typename PointerT >
bool Helium::WeakPtr< PointerT >::operator==( const StrongPtr< PointerT >& rPointer ) const
{
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pThisProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy );
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pOtherProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( rPointer.m_pProxy );

    // Note that a weak pointer can have a reference count proxy whose object is set to null, so we need to check for and handle that case as well.
    if( pThisProxy && !pThisProxy->GetObject() )
    {
        pThisProxy = NULL;
    }

    return ( pThisProxy == pOtherProxy );
}

/// Equality comparison operator.
///
/// @param[in] rPointer  Smart pointer with which to compare.
///
/// @return  True if this smart pointer and the given smart pointer reference the same object, false if not.
template< typename PointerT >
bool Helium::WeakPtr< PointerT >::operator==( const WeakPtr& rPointer ) const
{
    // Note that a weak pointer can have a reference count proxy whose object is set to null, so we need to check for and handle that case as well.
    return ( Get() == rPointer.Get() );
}

/// Inequality comparison operator.
///
/// @param[in] rPointer  Smart pointer with which to compare.
///
/// @return  True if this smart pointer and the given smart pointer do not reference the same object, false if they do.
template< typename PointerT >
bool Helium::WeakPtr< PointerT >::operator!=( const StrongPtr< PointerT >& rPointer ) const
{
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pThisProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( m_pProxy );
    RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* pOtherProxy = reinterpret_cast< RefCountProxy< typename PointerT::RefCountSupportType::BaseType >* >( rPointer.m_pProxy );

    // Note that a weak pointer can have a reference count proxy whose object is set to null, so we need to check for and handle that case as well.
    if( pThisProxy && !pThisProxy->GetObject() )
    {
        pThisProxy = NULL;
    }

    return ( pThisProxy != pOtherProxy );
}

/// Inequality comparison operator.
///
/// @param[in] rPointer  Smart pointer with which to compare.
///
/// @return  True if this smart pointer and the given smart pointer do not reference the same object, false if they do.
template< typename PointerT >
bool Helium::WeakPtr< PointerT >::operator!=( const WeakPtr& rPointer ) const
{
    // Note that a weak pointer can have a reference count proxy whose object is set to null, so we need to check for and handle that case as well.
    return ( Get() != rPointer.Get() );
}

/// Helper function for performing a compile-time verified up-cast of a WeakPtr.
///
/// @param[in] rIsProperBase  Instance of std::is_base_of< BaseT, PointerT > if it inherits from std::true_type.
///
/// @return  Constant reference to the cast weak pointer.
template< typename PointerT >
template< typename BaseT >
const Helium::WeakPtr< BaseT >& Helium::WeakPtr< PointerT >::ImplicitUpCast( const std::true_type& /*rIsProperBase*/ ) const
{
    return *reinterpret_cast< const WeakPtr< BaseT >* >( this );
}
