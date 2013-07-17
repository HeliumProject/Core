#pragma once

#include "Platform/Types.h"

#include <string> 
#include <sstream>

#if HELIUM_CC_GCC

#include <tr1/regex>

namespace std
{
    using std::tr1::regex;
    using std::tr1::smatch;
    using std::tr1::match_results;
    using std::tr1::sregex_token_iterator;
}

#else
#include <regex>
#endif

//--------------------------------------------------------------
// helper functions for using std::tr1::regex
// 
// some of these could be made template functions on their return types
// and moved to std::stringstream; if we did that we would need to decide what
// kind of error semantics we want. currently this is just adopted from 
// existing code that requires this functionality, and has WEAK error checking
// to say the least. 
// 
// currently they are templated on their input result
// good candidate result types are: 
//   std::tr1::smatch (when you match a std::string)
//   std::tr1::cmatch (when you match a const char*) 
// 
// this templatization should be transparent to the user, since 
// cmatch and smatch are not ambiguous at all. 
// 

namespace Helium
{
    template <class MatchT>
    inline std::string MatchResultAsString( const std::match_results<MatchT>& results, int i )
    {
        return std::string ( results[i].first, results[i].second ); 
    }

    template <class T, class MatchT>
    inline T MatchResult( const std::match_results<MatchT>& results, int i )
    {
        std::istringstream str ( MatchResultAsString<MatchT>(results, i) );

        T result;
        str >> result;
        HELIUM_ASSERT( !str.fail() );

        return result;
    }
}
