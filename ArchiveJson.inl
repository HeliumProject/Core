Helium::Persist::RapidJsonOutputStream::RapidJsonOutputStream()
	: m_Stream( NULL )
{
}

void Helium::Persist::RapidJsonOutputStream::SetStream( Stream* stream )
{
	m_Stream = stream;
}

void Helium::Persist::RapidJsonOutputStream::Put( Ch c )
{
	m_Stream->Write< Ch >( c );
}

void Helium::Persist::RapidJsonOutputStream::Flush()
{
	m_Stream->Flush();
}
