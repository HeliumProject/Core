#pragma once 

#include <string>
#include <vector>
#include <set>

#include "Foundation/Regex.h"

////////////////////////////////////////////////////////////////////////
//
// String tokenizer
//
//  This parses delmited values into a typed container
//   EG 1, 2, 3 with delimiter ',' -> std::vector<int> containing
//
// Special delimiters that need to be escaped:
//  - "|" (pipe) should be "\\|"
//
////////////////////////////////////////////////////////////////////////

namespace Helium
{
    template< typename T >
    void Tokenize( const std::string& str, std::vector< T >& tokens, const std::string delimiters );

    template< typename T >
    void Tokenize( const std::string& str, std::set< T >& tokens, const std::string delimiters );
}

#include "Foundation/Tokenize.inl"