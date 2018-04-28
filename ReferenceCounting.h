#pragma once

#include "Foundation/API.h"

#include "Platform/Atomic.h"
#include "Platform/Utility.h"

/// Utility macro for declaring common functions and variables for an object with strong/weak reference counting
/// support.
///
/// @param[in] CLASS         Class type being declared.
/// @param[in] SUPPORT_TYPE  Reference counting support type.
#define HELIUM_DECLARE_REF_COUNT( CLASS, SUPPORT_TYPE ) \
	public: \
	typedef SUPPORT_TYPE RefCountSupportType; \
	Helium::RefCountProxy< CLASS >* GetRefCountProxy() const \
	{ \
		return m_refCountProxyContainer.Get( const_cast< CLASS* >( this ) ); \
	} \
	void SetRefCountProxy( Helium::RefCountProxy< CLASS >* pProxy ) const \
	{ \
		return m_refCountProxyContainer.Set( pProxy ); \
	} \
	private: \
	mutable Helium::RefCountProxyContainer< CLASS > m_refCountProxyContainer;

namespace Helium
{
	template< typename BaseT > class RefCountProxy;
	template< typename PointerT > class StrongPtr;
	template< typename PointerT > class WeakPtr;

	/// Base type for reference counting object proxies.
	template< typename T >
	class RefCountProxyBase
	{
	public:
		int32_t GetRefCounts() const;

	protected:
		/// Reference-counted object.
		T* volatile m_pObject;
		/// Reference counts (strong references in lower 16-bits, weak references in upper 16-bits).
		volatile int32_t m_refCounts;
	};

	/// Reference counting object proxy.
	template< typename BaseT >
	class RefCountProxy : public RefCountProxyBase< void >
	{
	public:
		/// @name Initialization
		//@{
		void Initialize( BaseT* pObject );
		//@}

		/// @name Object Access
		//@{
		BaseT* GetObject() const;
		void SetObject( BaseT* object );
		//@}

		/// @name Reference Counting
		//@{
		void AddStrongRef();
		bool RemoveStrongRef();
		uint16_t GetStrongRefCount() const;

		void AddWeakRef();
		bool RemoveWeakRef();
		uint16_t GetWeakRefCount() const;
		//@}

		void Swap( Helium::RefCountProxy< BaseT >* object );

	private:
		/// @name Private Utility Functions
		//@{
		void DestroyObject();
		//@}
	};

	/// Reference counting object proxy container.  This is provided to ease the management of a reference count proxy
	/// for an object (i.e. don't need to initialize in the constructor).
	template< typename BaseT >
	class RefCountProxyContainer
	{
	public:
		/// Reference count support type.
		typedef typename BaseT::RefCountSupportType SupportType;

		/// @name Construction/Destruction
		//@{
		RefCountProxyContainer();
		//@}

		/// @name Access
		//@{
		RefCountProxy< BaseT >* Get( BaseT* pObject );
		void Set( RefCountProxy< BaseT >* pProxy );
		//@}

		void Swap( RefCountProxyContainer< BaseT >* pOther );

	private:
		/// Reference counting proxy instance.
		RefCountProxy< BaseT >* volatile m_pProxy;
	};

	/// Strong pointer for reference-counted objects.
	template< typename PointerT >
	class StrongPtr
	{
		friend class WeakPtr< PointerT >;

	public:
		/// @name Construction/Destruction
		//@{
		StrongPtr();
		StrongPtr( PointerT* pObject );
		explicit StrongPtr( const WeakPtr< PointerT >& rPointer );
		StrongPtr( const StrongPtr& rPointer );
		~StrongPtr();
		//@}

		/// @name Object Referencing
		//@{
		PointerT* Get() const;
		PointerT* Ptr() const;
		void Set( PointerT* pObject );
		void Release();
		bool ReferencesObject() const;
		//@}

		/// @name Proxy Referencing
		//@{
		RefCountProxyBase< PointerT >* GetProxy() const;
		void SetProxy( RefCountProxyBase< PointerT >* pProxy );
		//@}

		/// @name Overloaded Operators
		//@{
		operator PointerT*() const;
		template< typename BaseT > operator const StrongPtr< BaseT >&() const;

		PointerT& operator*() const;
		PointerT* operator->() const;

		StrongPtr& operator=( PointerT* pObject );
		StrongPtr& operator=( const WeakPtr< PointerT >& rOther );
		StrongPtr& operator=( const StrongPtr& rOther );

		bool operator==( const WeakPtr< PointerT >& rPointer ) const;
		bool operator==( const StrongPtr& rPointer ) const;
		bool operator!=( const WeakPtr< PointerT >& rPointer ) const;
		bool operator!=( const StrongPtr& rPointer ) const;
		//@}

	private:
		/// Proxy object (cast to a RefCountProxyBase pointer to allow for declaring smart pointers to forward-declared types).
		RefCountProxyBase< PointerT >* m_pProxy;

		/// @name Conversion Utility Functions, Private
		//@{
		template< typename BaseT > const StrongPtr< BaseT >& ImplicitUpCast( const std::true_type& rIsProperBase ) const;
		//@}
	};

	/// Weak pointer for reference-counted objects.
	template< typename PointerT >
	class WeakPtr
	{
		friend class StrongPtr< PointerT >;

	public:
		/// @name Construction/Destruction
		//@{
		WeakPtr();
		WeakPtr( PointerT* pObject );
		explicit WeakPtr( const StrongPtr< PointerT >& rPointer );
		WeakPtr( const WeakPtr& rPointer );
		~WeakPtr();
		//@}

		/// @name Object Referencing
		//@{
		PointerT* Get() const;
		PointerT* Ptr() const;
		void Set( PointerT* pObject );
		void Release();
		bool ReferencesObject() const;
		//@}

		/// @name Proxy Referencing
		//@{
		RefCountProxyBase< PointerT >* GetProxy() const;
		void SetProxy( RefCountProxyBase< PointerT >* pProxy );
		//@}

		/// @name Reference Count Proxy Matching
		//@{
		bool HasObjectProxy( const PointerT* pObject ) const;
		//@}

		/// @name Overloaded Operators
		//@{
		operator PointerT*() const;
		template< typename BaseT > operator const WeakPtr< BaseT >&() const;

		PointerT& operator*() const;
		PointerT* operator->() const;

		WeakPtr& operator=( PointerT* pObject );
		WeakPtr& operator=( const StrongPtr< PointerT >& rOther );
		WeakPtr& operator=( const WeakPtr& rOther );

		bool operator==( const StrongPtr< PointerT >& rPointer ) const;
		bool operator==( const WeakPtr& rPointer ) const;
		bool operator!=( const StrongPtr< PointerT >& rPointer ) const;
		bool operator!=( const WeakPtr& rPointer ) const;
		//@}

	private:
		/// Proxy object (cast to a RefCountProxyBase pointer to allow for declaring smart pointers to forward-declared types).
		RefCountProxyBase< PointerT >* m_pProxy;

		/// @name Conversion Utility Functions, Private
		//@{
		template< typename BaseT > const WeakPtr< BaseT >& ImplicitUpCast( const std::true_type& rIsProperBase ) const;
		//@}
	};
}

#include "Foundation/ReferenceCounting.inl"
