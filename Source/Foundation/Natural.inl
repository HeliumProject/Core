int Helium::NaturalCompareString( char const* a, char const* b )
{
	return strnatcmp( a, b );
}

int Helium::CaseInsensitiveNaturalCompareString( char const* a, char const* b )
{
	return strinatcmp( a, b );
}

bool Helium::NaturalStringComparitor::operator()( const std::string& str1, const std::string& str2 ) const
{
	return ( strnatcmp( str1.c_str(), str2.c_str() ) < 0 );
}

bool Helium::CaseInsensitiveNaturalStringComparitor::operator()( const std::string& str1, const std::string& str2 ) const
{
	return ( strinatcmp( str1.c_str(), str2.c_str() ) < 0 );
}