Helium::Persist::RapidJsonOutputStream::RapidJsonOutputStream()
	: m_Stream( NULL )
{
}

void Helium::Persist::RapidJsonOutputStream::SetStream( Stream* stream )
{
	m_Stream = stream;
}

void Helium::Persist::RapidJsonOutputStream::Put( char c )
{
	m_Stream->Write< char >( c );
}

void Helium::Persist::RapidJsonOutputStream::Flush()
{
	m_Stream->Flush();
}
