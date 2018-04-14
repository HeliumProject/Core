const std::vector< Helium::Inspect::ControlPtr >& Helium::Inspect::Container::GetChildren() const
{
	return m_Children;
}

inline std::vector< Helium::Inspect::ControlPtr > Helium::Inspect::Container::ReleaseChildren()
{
	HELIUM_ASSERT( !this->IsRealized() );
	std::vector< ControlPtr > children = m_Children;
	Clear();
	return children;
}

const std::string& Helium::Inspect::Container::GetPath() const
{
	if ( m_Path.empty() )
	{
		BuildPath( m_Path );
	}

	return m_Path;
}

void Helium::Inspect::Container::BuildPath(std::string& path) const
{
	if (m_Parent)
	{
		m_Parent->BuildPath(path);
	}

	path += "|" + a_Name.Get();
}

Helium::Inspect::UIHints Helium::Inspect::Container::GetUIHints() const
{
	return m_UIHints;
}

void Helium::Inspect::Container::SetUIHints( const UIHints hints )
{
	m_UIHints = hints;
}

