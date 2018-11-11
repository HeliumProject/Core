#include "Precompile.h"
#include "File.h"

using namespace Helium;

void Helium::SplitDirectories( const std::string& path, std::vector< std::string >& output )
{ 
	static char pathSep[ 2 ] = { Helium::PathSeparator, '\0' };

	output.clear();
	if ( path.empty() )
	{
		return;
	}

	std::string newRoot = path;
	if ( newRoot[0] == Helium::PathSeparator )
	{
		newRoot.erase( 0, 1 );
	}

	std::string::size_type start = 0, end = 0;
	for ( ; ( end = newRoot.find( Helium::PathSeparator, start ) ) != std::string::npos; start = end + 1 )
	{ 
		if ( start != end )
		{
			std::string substr = newRoot.substr( start, end - start );
			if ( substr != pathSep )
			{
				output.push_back( substr );
			}
		}
	}

	std::string substr = newRoot.substr( start );
	if ( substr != pathSep && substr.length() > 0 )
	{
		output.push_back( substr ); 
	}
}
