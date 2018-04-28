template< typename T >
void Helium::Tokenize( const std::string& str, std::vector< T >& tokens, const std::string delimiters )
{
    std::regex splitPattern(delimiters); 

    std::sregex_token_iterator i(str.begin(), str.end(), splitPattern, -1); 
    std::sregex_token_iterator end; 

    for(; i != end; i++)
    {
        if( (*i).matched)
        {
            T temp; 
            std::stringstream inStream(*i); 
            inStream >> temp; 

            tokens.push_back(temp);
        }
    }
}

template< typename T >
void Helium::Tokenize( const std::string& str, std::set< T >& tokens, const std::string delimiters )
{
    std::regex splitPattern(delimiters); 

    std::sregex_token_iterator i(str.begin(), str.end(), splitPattern, -1); 
    std::sregex_token_iterator end; 

    for(; i != end; i++)
    {
        if( (*i).matched)
        {
            T temp; 
            std::stringstream inStream(*i); 
            inStream >> temp; 

            tokens.push_back(temp);
        }
    }
}

namespace Helium
{
    template<>
    inline void Tokenize( const std::string& str, std::vector< std::string >& tokens, const std::string delimiters )
    {
        std::regex splitPattern(delimiters); 

        std::sregex_token_iterator i(str.begin(), str.end(), splitPattern, -1); 
        std::sregex_token_iterator end; 

        for(; i != end; i++)
        {
            if( (*i).matched)
            {
                tokens.push_back(*i); 
            }
        }
    }

    template<>
    inline void Tokenize( const std::string& str, std::set< std::string >& tokens, const std::string delimiters )
    {
        std::regex splitPattern(delimiters); 

        std::sregex_token_iterator i(str.begin(), str.end(), splitPattern, -1); 
        std::sregex_token_iterator end; 

        for(; i != end; i++)
        {
            if( (*i).matched )
            {
                tokens.insert( *i ); 
            }
        }
    }
}