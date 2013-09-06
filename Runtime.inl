const char* Helium::Platform::GetTypeString(Type t)
{
	if (t >= 0 && t<Types::Count)
	{
		return Types::Strings[t];
	}
	else
	{
		return Types::Strings[0];
	}
}

const char* Helium::Platform::GetEndiannessString(Endianness e)
{
	if (e >= 0 && e<Endiannesses::Count)
	{
		return Endiannesses::Strings[e];
	}
	else
	{
		return Endiannesses::Strings[0];
	}
}

