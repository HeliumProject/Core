std::string& Helium::Exception::Get()
{
    return m_Message;
}

const std::string& Helium::Exception::Get() const
{
    return m_Message;
}

void Helium::Exception::Set(const std::string& message)
{
    m_Message = message;
}
