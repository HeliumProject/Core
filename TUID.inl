Helium::TUID::TUID()
	: m_ID( 0x0 )
{

}

Helium::TUID::TUID( tuid id )
	: m_ID( id )
{

}

Helium::TUID::TUID(const TUID &id)
	: m_ID( id.m_ID )
{

}

Helium::TUID::TUID( const tstring& id )
{
	FromString( id );
}

Helium::TUID& Helium::TUID::operator=(const TUID &rhs)
{
	m_ID = rhs.m_ID;
	return *this;
}

bool Helium::TUID::operator==(const TUID &rhs) const
{
	return m_ID == rhs.m_ID;
}

bool Helium::TUID::operator==( const tuid& rhs ) const
{
	return m_ID == rhs;
}

bool Helium::TUID::operator!=(const TUID &rhs) const
{
	return m_ID != rhs.m_ID;
}

bool Helium::TUID::operator!=( const tuid &rhs ) const
{
	return m_ID != rhs;
}

bool Helium::TUID::operator<(const TUID &rhs) const
{
	return m_ID < rhs.m_ID;
}

Helium::TUID::operator tuid() const
{
	return m_ID;
}

void Helium::TUID::Reset()
{
	m_ID = 0x0;
}

size_t Helium::TUIDHasher::operator()( const TUID& tuid ) const
{
	return stdext::hash_compare< uint64_t >::operator()( 0 );
}

bool Helium::TUIDHasher::operator()( const TUID& tuid1, const TUID& tuid2 ) const
{
	return stdext::hash_compare< uint64_t >::operator()( tuid1, tuid2 );
}

template<>
inline void Helium::Swizzle< Helium::TUID >(TUID& val, bool swizzle)
{
	// operator tuid() const will handle the conversion into the other swizzle func
	val = ConvertEndian(val, swizzle);
}