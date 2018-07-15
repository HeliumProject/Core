Helium::Log::Statement::Statement( const std::string& string, Channel channel, Level level, int indent )
    : m_String( string )
    , m_Channel( channel )
    , m_Level( level )
    , m_Indent( indent )
{

}

void Helium::Log::Statement::ApplyIndent()
{
    std::string indented;
    ApplyIndent( m_String.c_str(), indented );
    m_String = indented;
}

Helium::Log::ListenerArgs::ListenerArgs( const Statement& statement )
    : m_Statement ( statement )
    , m_Skip( false )
{

}

template <bool (*AddFunc)(const std::string& fileName, Helium::Log::Channel channel, Helium::ThreadId threadId, bool append), void (*RemoveFunc)(const std::string& fileName)>
Helium::Log::FileHandle< AddFunc, RemoveFunc >::FileHandle(const std::string& file, Channel channel, Helium::ThreadId threadId, bool append  )
    : m_File (file)
{
    if (!m_File.empty())
    {
        AddFunc(m_File, channel, threadId, append);
    }
}

template <bool (*AddFunc)(const std::string& fileName, Helium::Log::Channel channel, Helium::ThreadId threadId, bool append), void (*RemoveFunc)(const std::string& fileName)>
Helium::Log::FileHandle< AddFunc, RemoveFunc >::~FileHandle()
{
    if (!m_File.empty())
    {
        RemoveFunc(m_File);
    }
}

template <bool (*AddFunc)(const std::string& fileName, Helium::Log::Channel channel, Helium::ThreadId threadId, bool append), void (*RemoveFunc)(const std::string& fileName)>
const std::string& Helium::Log::FileHandle< AddFunc, RemoveFunc >::GetFile()
{
    return m_File;
}

Helium::Log::Indentation::Indentation()
{
    Indent();
}

Helium::Log::Indentation::~Indentation()
{
    UnIndent();
}