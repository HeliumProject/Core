#include "Precompile.h"
#include "Inspect/DataBinding.h"

using namespace Helium;
using namespace Helium::Inspect;

template<>
void Inspect::Extract(std::istream& stream, std::string* val)
{
	std::streamsize size = stream.rdbuf()->in_avail();
	if ( size == 0 )
	{
		val->clear();
	}
	else
	{
		HELIUM_ASSERT(size < 0xFFFFFFFFL);
		val->resize( std::string::size_type(size) );
		stream.read( const_cast< char* >( val->c_str() ), size );
	}
}

template<>
void Inspect::Extract(std::istream& stream, uint8_t* val)
{
	uint16_t tmp;
	stream >> tmp;

	if (!stream.fail())
	{
		*val = (uint8_t)tmp;
	}
}

template<>
void Inspect::Insert(std::ostream& stream, const uint8_t* val)
{
	uint16_t tmp = *val;
	stream << tmp;
}

template<>
void Inspect::Extract(std::istream& stream, int8_t* val)
{
	int16_t tmp;
	stream >> tmp;

	if (!stream.fail())
	{
		*val = (uint8_t)tmp;
	}
}

template<>
void Inspect::Insert(std::ostream& stream, const int8_t* val)
{
	int16_t tmp = *val;
	stream << tmp;
}

template<>
void Inspect::Insert(std::ostream& stream, const float32_t* val)
{
	float32_t tmp = *val;
	stream << std::fixed << std::setprecision(6) << tmp;
}

template<>
void Inspect::Insert(std::ostream& stream, const float64_t* val)
{
	float64_t tmp = *val;
	stream << std::fixed << std::setprecision(6) << tmp;
}

template<>
void Inspect::Extract(std::istream& stream, Reflect::Data* val)
{
	Reflect::ScalarTranslator* scalar = Reflect::ReflectionCast< Reflect::ScalarTranslator >( val->m_Translator );
	if ( scalar )
	{
		std::string str;
		Extract( stream, &str );
		scalar->Parse( String( str.c_str() ), val->m_Pointer, NULL, true );
	}
}

template<>
void Inspect::Insert(std::ostream& stream, const Reflect::Data* val)
{
	Reflect::ScalarTranslator* scalar = Reflect::ReflectionCast< Reflect::ScalarTranslator >( val->m_Translator );
	if ( scalar )
	{
		String str;
		scalar->Print( val->m_Pointer, str );
		std::string str2 ( str.GetData() );
		Insert( stream, &str2 );
	}
}
