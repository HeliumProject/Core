const std::string& Helium::Directory::GetPath()
{
	return m_Path;
}

void Helium::Directory::SetPath( const std::string& path )
{
	Close();
	m_Path = path;
}
