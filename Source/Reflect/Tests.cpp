#include "Precompile.h"

#include "Reflect/Object.h"
#include "Reflect/TranslatorDeduction.h"
#include "Foundation/Log.h"

#include "gtest/gtest.h"

using namespace Helium;
using namespace Helium::Reflect;

struct TestEnumeration : Reflect::Enum
{
	enum Enum
	{
		ValueOne,
		ValueTwo,
	};

	HELIUM_DECLARE_ENUM(TestEnumeration);
	static void PopulateMetaType(MetaEnum& info);
};

struct TestStructure : Reflect::Struct
{
	uint8_t  m_Uint8;
	uint16_t m_Uint16;
	uint32_t m_Uint32;
	uint64_t m_Uint64;

	int8_t  m_Int8;
	int16_t m_Int16;
	int32_t m_Int32;
	int64_t m_Int64;

	float32_t m_Float32;
	float64_t m_Float64;

	std::vector<uint32_t> m_StdVectorUint32;
	std::set<uint32_t> m_StdSetUint32;
	std::map<uint32_t, uint32_t> m_StdMapUint32;

	DynamicArray<uint32_t> m_FoundationDynamicArrayUint32;
	Set<uint32_t> m_FoundationSetUint32;
	Map<uint32_t, uint32_t> m_FoundationMapUint32;

	TestStructure();

	HELIUM_DECLARE_BASE_STRUCT(TestStructure);
	static void PopulateMetaType(MetaStruct& comp);
};

class TestObject : public Reflect::Object
{
public:
	TestStructure m_Struct;
	TestStructure m_StructArray[8];

	TestEnumeration m_Enumeration;
	TestEnumeration m_EnumerationArray[8];

	TestObject();
	void TestFunction(TestStructure& args);

	HELIUM_DECLARE_CLASS(TestObject, Object);
	static void PopulateMetaType(MetaClass& comp);
};

HELIUM_DEFINE_ENUM( TestEnumeration );
HELIUM_DEFINE_BASE_STRUCT( TestStructure );
HELIUM_DEFINE_CLASS( TestObject );

using namespace Helium;

struct EmptyBaseCheck : Reflect::Struct
{
	uint32_t variable;
};
HELIUM_COMPILE_ASSERT( sizeof( EmptyBaseCheck ) == sizeof( uint32_t ) ); // check for empty-base optimization

void TestEnumeration::PopulateMetaType( MetaEnum& info )
{
	info.AddElement( ValueOne, "Value One" );
	info.AddElement( ValueTwo, "Value Two" );
}

TestStructure::TestStructure()
	: m_Uint8( 0 )
	, m_Uint16( 0 )
	, m_Uint32( 0 )
	, m_Uint64( 0 )
	, m_Int8( 0 )
	, m_Int16( 0 )
	, m_Int32( 0 )
	, m_Int64( 0 )
	, m_Float32( 0 )
	, m_Float64( 0 )
{
}

void TestStructure::PopulateMetaType( Reflect::MetaStruct& comp )
{
	comp.AddField( &TestStructure::m_Uint8,  "Unsigned 8-bit Integer" );
	comp.AddField( &TestStructure::m_Uint16, "Unsigned 16-bit Integer" );
	comp.AddField( &TestStructure::m_Uint32, "Unsigned 32-bit Integer" );
	comp.AddField( &TestStructure::m_Uint64, "Unsigned 64-bit Integer" );

	comp.AddField( &TestStructure::m_Int8,  "Signed 8-bit Integer" );
	comp.AddField( &TestStructure::m_Int16, "Signed 16-bit Integer" );
	comp.AddField( &TestStructure::m_Int32, "Signed 32-bit Integer" );
	comp.AddField( &TestStructure::m_Int64, "Signed 64-bit Integer" );

	comp.AddField( &TestStructure::m_Float32, "32-bit Floating Point" );
	comp.AddField( &TestStructure::m_Float64, "64-bit Floating Point" );
	
	comp.AddField( &TestStructure::m_StdVectorUint32, "std::vector of Signed 32-bit Integers" );
	comp.AddField( &TestStructure::m_StdSetUint32, "std::vector of Unsigned 32-bit Integers" );
	comp.AddField( &TestStructure::m_StdMapUint32, "std::map of Unsigned 32-bit Integers" );
	
	comp.AddField( &TestStructure::m_FoundationDynamicArrayUint32, "Dynamic Array of Signed 32-bit Integers" );
	comp.AddField( &TestStructure::m_FoundationSetUint32, "Set of Unsigned 32-bit Integers" );
	comp.AddField( &TestStructure::m_FoundationMapUint32, "Map of Unsigned 32-bit Integers" );
}

void TestObject::PopulateMetaType( Reflect::MetaClass& comp )
{
	comp.AddField( &TestObject::m_Struct, "MetaStruct" );
	comp.AddField( &TestObject::m_StructArray, "MetaStruct Array" );

	comp.AddField( &TestObject::m_Enumeration, "MetaEnum" );
	comp.AddField( &TestObject::m_EnumerationArray, "MetaEnum Array" );

	comp.AddMethod( &TestObject::TestFunction, "Test Function" );
}

TestObject::TestObject()
{
}

void TestObject::TestFunction( TestStructure& args )
{
	// verify vtable is intact
	HELIUM_ASSERT( this->GetMetaClass() == Reflect::GetMetaClass< This >() );

	// verify argument is intact
	TestStructure* const def = static_cast<TestStructure* const>( GetMetaStruct< TestStructure >()->m_Default );
	HELIUM_ASSERT( def->m_Uint8 == args.m_Uint8 );
	HELIUM_ASSERT( def->m_Uint16 == args.m_Uint16 );
	HELIUM_ASSERT( def->m_Uint32 == args.m_Uint32 );
	HELIUM_ASSERT( def->m_Uint64 == args.m_Uint64 );

	HELIUM_ASSERT( def->m_Int8 == args.m_Int8 );
	HELIUM_ASSERT( def->m_Int16 == args.m_Int16 );
	HELIUM_ASSERT( def->m_Int32 == args.m_Int32 );
	HELIUM_ASSERT( def->m_Int64 == args.m_Int64 );

	HELIUM_ASSERT( def->m_Float32 == args.m_Float32 );
	HELIUM_ASSERT( def->m_Float64 == args.m_Float64 );
}

TEST(Reflect, ReflectStartupShutdown)
{
	Reflect::Startup();
	Reflect::Shutdown();
}

TEST(Reflect, AllocateTestObject)
{
	Reflect::Startup();
	StrongPtr< Object > object = new TestObject ();
	const MetaClass* type = object->GetMetaClass();
	HELIUM_ASSERT( type != nullptr );
	HELIUM_ASSERT( type = Reflect::GetMetaClass<TestObject>() );
	object = nullptr;
	Reflect::Shutdown();
}

#if 0
	const Reflect::Method& m = object->GetMetaClass()->m_Methods.GetFirst();
	void* args = alloca(m.m_Translator->m_Size);
	m.m_Translator->Construct( args );
	static_cast< Invokable* >( m.m_Delegate )->Invoke( args, object.Ptr() );
	m.m_Translator->Destruct( args );
#endif
