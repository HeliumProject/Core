const Helium::FilePath& Helium::Persist::Archive::GetPath() const
{
	return m_Path;
}

void Helium::Persist::Archive::Get( Reflect::ObjectPtr& object )
{
	object = m_Object;
}

void Helium::Persist::Archive::Put( Reflect::Object* object )
{
	m_Object = object;
}

template <class T>
Helium::StrongPtr<T> Helium::Persist::FromArchive( const FilePath& path, Reflect::ObjectResolver* resolver, ArchiveType archiveType )
{
	return SafeCast< T >( Persist::FroArchive< Reflect::Object >( path, resolver, archiveType ) );
}