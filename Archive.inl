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
