#include "PlatformPch.h"
#include "Platform/File.h"

using namespace Helium;

void Helium::SplitDirectories( const std::string& path, std::vector< std::string >& output )
{ 
	static char pathSep[ 2 ] = { Helium::PathSeparator, '\0' };

	std::string::size_type start = 0, end = 0;
	for ( ; ( end = path.find( Helium::PathSeparator, start ) ) != std::string::npos; start = end + 1 )
	{ 
		if ( start != end )
		{
			std::string substr = path.substr( start, end - start );
			if ( substr != pathSep )
			{
				output.push_back( substr );
			}
		}
	}

	std::string substr = path.substr( start );
	if ( substr != pathSep )
	{
		output.push_back( substr ); 
	}
}
