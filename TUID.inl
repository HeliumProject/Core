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

Helium::TUID::TUID( const std::string& id )
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

template<>
inline void Helium::Swizzle< Helium::TUID >(TUID& val, bool swizzle)
{
	// operator tuid() const will handle the conversion into the other swizzle func
	val = ConvertEndian(val, swizzle);
}