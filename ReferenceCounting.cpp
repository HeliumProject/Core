#include "Precompile.h"
#include "Foundation/ReferenceCounting.h"

#include "Foundation/ConcurrentHashSet.h"
#include "Foundation/ObjectPool.h"

#if !HELIUM_RELEASE

using namespace Helium;

class Object;

void TestRefCountingFwd()
{
	WeakPtr< Object > weak;
	StrongPtr< Object > strong;
}

class ObjectRefCountSupport
{
public:
	typedef Object BaseType;

	inline static void PreDestroy( Object* pObject );
	inline static void Destroy( Object* pObject );

	static RefCountProxy< Object >* Allocate();
	static void Release( RefCountProxy< Object >* pProxy );
	static void Shutdown();

#if HELIUM_ENABLE_MEMORY_TRACKING
	static size_t GetActiveProxyCount();
	static bool GetFirstActiveProxy( ConcurrentHashSet< RefCountProxy< Object >* >::ConstAccessor& rAccessor );
#endif

private:
	struct StaticTranslator;
	static StaticTranslator* sm_pStaticTranslator;
};

class Object
{
protected:
	HELIUM_DECLARE_REF_COUNT( Object, ObjectRefCountSupport );

public:
	virtual void PreDestroy();
	virtual void Destroy();
};

struct ObjectRefCountSupport::StaticTranslator
{
	static const size_t POOL_BLOCK_SIZE = 1024*1024;

	ObjectPool< RefCountProxy< Object > > proxyPool;

#if HELIUM_ENABLE_MEMORY_TRACKING
	ConcurrentHashSet< RefCountProxy< Object >* > activeProxySet;
#endif

	StaticTranslator();
};

ObjectRefCountSupport::StaticTranslator* ObjectRefCountSupport::sm_pStaticTranslator = NULL;

void ObjectRefCountSupport::PreDestroy( Object* pObject )
{
    HELIUM_ASSERT( pObject );

    pObject->PreDestroy();
}

void ObjectRefCountSupport::Destroy( Object* pObject )
{
    HELIUM_ASSERT( pObject );

    pObject->Destroy();
}

RefCountProxy< Object >* ObjectRefCountSupport::Allocate()
{
	StaticTranslator* pStaticTranslator = sm_pStaticTranslator;
	if( !pStaticTranslator )
	{
		pStaticTranslator = new StaticTranslator;
		HELIUM_ASSERT( pStaticTranslator );
		sm_pStaticTranslator = pStaticTranslator;
	}

	RefCountProxy< Object >* pProxy = pStaticTranslator->proxyPool.Allocate();
	HELIUM_ASSERT( pProxy );

#if HELIUM_ENABLE_MEMORY_TRACKING
	ConcurrentHashSet< RefCountProxy< Object >* >::Accessor activeProxySetAccessor;
	HELIUM_VERIFY( pStaticTranslator->activeProxySet.Insert( activeProxySetAccessor, pProxy ) );
#endif

	return pProxy;
}

void ObjectRefCountSupport::Release( RefCountProxy< Object >* pProxy )
{
	HELIUM_ASSERT( pProxy );

	StaticTranslator* pStaticTranslator = sm_pStaticTranslator;
	HELIUM_ASSERT( pStaticTranslator );

#if HELIUM_ENABLE_MEMORY_TRACKING
	HELIUM_VERIFY( pStaticTranslator->activeProxySet.Remove( pProxy ) );
#endif

	pStaticTranslator->proxyPool.Release( pProxy );
}

void ObjectRefCountSupport::Shutdown()
{
#if HELIUM_ENABLE_MEMORY_TRACKING
	ConcurrentHashSet< RefCountProxy< Object >* >::ConstAccessor refCountProxyAccessor;
	if( ObjectRefCountSupport::GetFirstActiveProxy( refCountProxyAccessor ) )
	{
		HELIUM_TRACE(
			TraceLevels::Error,
			"%" PRIuSZ " reference counted object(s) still active during shutdown!\n",
			ObjectRefCountSupport::GetActiveProxyCount() );
  
		ObjectRefCountSupport::GetFirstActiveProxy( refCountProxyAccessor );
		while( refCountProxyAccessor.IsValid() )
		{
			RefCountProxy< Object >* pProxy = *refCountProxyAccessor;
			HELIUM_ASSERT( pProxy );

			HELIUM_TRACE(
				TraceLevels::Error,
				"   - 0x%" HELIUM_PRINT_POINTER ": (%" PRIu16 " strong ref(s), %" PRIu16 " weak ref(s))\n",
				pProxy,
				pProxy->GetStrongRefCount(),
				pProxy->GetWeakRefCount() );

			++refCountProxyAccessor;
		}
		refCountProxyAccessor.Release();
	}
#endif  // HELIUM_ENABLE_MEMORY_TRACKING

	delete sm_pStaticTranslator;
	sm_pStaticTranslator = NULL;
}

#if HELIUM_ENABLE_MEMORY_TRACKING
size_t ObjectRefCountSupport::GetActiveProxyCount()
{
	HELIUM_ASSERT( sm_pStaticTranslator );

	return sm_pStaticTranslator->activeProxySet.GetSize();
}

bool ObjectRefCountSupport::GetFirstActiveProxy(
	ConcurrentHashSet< RefCountProxy< Object >* >::ConstAccessor& rAccessor )
{
	HELIUM_ASSERT( sm_pStaticTranslator );

	return sm_pStaticTranslator->activeProxySet.First( rAccessor );
}
#endif

ObjectRefCountSupport::StaticTranslator::StaticTranslator()
: proxyPool( POOL_BLOCK_SIZE )
{
}

void Object::PreDestroy()
{
}

void Object::Destroy()
{
	delete this;
}

void TestRefCountingFull()
{
	StrongPtr< Object > strong1 = new Object ();
	StrongPtr< Object > strong2 = strong1;
	bool equal = strong1 == strong2;
	equal = !(strong1 != strong2);

	WeakPtr< Object > weak1 = WeakPtr< Object >( strong1 );
	strong1 = weak1;
	WeakPtr< Object > weak2 = WeakPtr< Object >( strong2 );
	equal = weak1 == weak2;
	equal = !(weak1 != weak2);

	equal = weak1 == strong2;
	equal = !(weak1 != strong2);

	equal = strong1 == weak2;
	equal = !(strong1 != weak2);
}

#endif