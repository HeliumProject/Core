require( './premake' )

workspace "Core"
Helium.DoBasicWorkspaceSettings()

filter "configurations:Debug"
	targetdir( "Bin/Debug/" )
	libdirs { "Bin/Debug/" }

filter "configurations:Intermediate"
	targetdir( "Bin/Intermediate/" )
	libdirs { "Bin/Intermediate/" }

filter "configurations:Profile"
	targetdir( "Bin/Profile/" )
	libdirs { "Bin/Profile/" }

filter "configurations:Release"
	targetdir( "Bin/Release/" )
	libdirs { "Bin/Release/" }

filter {}

project( "Platform" )

	Helium.DoModuleProjectSettings( "Source", "HELIUM", "Platform", "PLATFORM" )

	files
	{
		"Source/Platform/*.cpp",
		"Source/Platform/*.h",
		"Source/Platform/*.inl",
	}

	excludes
	{
		"Source/Platform/*Tests.*",
	}

	filter "system:windows"
		excludes
		{
			"Source/Platform/*Posix.*",
			"Source/Platform/*Mac.*",
			"Source/Platform/*Lin.*",
		}

	filter "system:macosx"
		excludes
		{
			"Source/Platform/*Win.*",
			"Source/Platform/*Lin.*",
		}

	filter "system:linux"
		excludes
		{
			"Source/Platform/*Win.*",
			"Source/Platform/*Mac.*",
		}

	filter { "kind:SharedLib", "system:linux" }
		links
		{
			"pthread",
			"dl",
		}

	filter {}

project( "PlatformTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Platform/*Tests.*",
	}

	links
	{
		"Platform"
	}

project( "Foundation" )

	Helium.DoModuleProjectSettings( "Source", "HELIUM", "Foundation", "FOUNDATION" )

	files
	{
		"Source/Foundation/**",
	}

	excludes
	{
		"Source/Foundation/*Tests.*",
	}

	filter "kind:SharedLib"
		links
		{
			"Platform",
		}

	filter {}

project( "FoundationTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Foundation/*Tests.*",
	}

	links
	{
		"Foundation",
		"Platform",
	}

project( "Application" )

	Helium.DoModuleProjectSettings( "Source", "HELIUM", "Application", "APPLICATION" )

	files
	{
		"Source/Application/**",
	}

	excludes
	{
		"Source/Application/*Tests.*",
	}

	filter "kind:SharedLib"
		links
		{
			"Foundation",
			"Platform",
		}

	filter {}

project( "ApplicationTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Application/*Tests.*",
	}

	links
	{
		"Application",
		"Foundation",
		"Platform",
	}

project( "Reflect" )

	Helium.DoModuleProjectSettings( "Source", "HELIUM", "Reflect", "REFLECT" )

	files
	{
		"Source/Reflect/**",
	}

	excludes
	{
		"Source/Reflect/*Tests.*",
	}

	filter "kind:SharedLib"
		links
		{
			"Foundation",
			"Platform",
		}

	filter {}

project( "ReflectTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Reflect/*Tests.*",
	}

	links
	{
		"Reflect",
		"Foundation",
		"Platform",
	}

project( "Persist" )

	Helium.DoModuleProjectSettings( "Source", "HELIUM", "Persist", "PERSIST" )

	files
	{
		"Source/Persist/**",
	}

	excludes
	{
		"Source/Persist/*Tests.*",
	}

	filter "kind:SharedLib"
		links
		{
			"Platform",
			"Foundation",
			"Reflect",
			"bson-static-1.0",
		}

	filter {}

project( "PersistTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Persist/*Tests.*",
	}

	links
	{
		"Persist",
		"Reflect",
		"Foundation",
		"Platform",
		"bson-static-1.0",
	}

project( "Mongo" )

	Helium.DoModuleProjectSettings( "Source", "HELIUM", "Mongo", "MONGO" )

	files
	{
		"Source/Mongo/**",
	}

	excludes
	{
		"Source/Mongo/*Tests.*",
	}

	filter "kind:SharedLib"
		links
		{
			"Platform",
			"Foundation",
			"Reflect",
			"Persist",
			"bson-static-1.0",
			"mongoc-static-1.0",
		}

	filter { "kind:SharedLib", "configurations:Debug" }
		links
		{
			"zlibd",
		}

	filter { "kind:SharedLib", "configurations:not Debug" }
		links
		{
			"zlib",
		}

	filter { "kind:SharedLib", "system:windows" }
		links
		{
			"bcrypt",
			"crypt32",
			"dnsapi",
			"secur32",
		}

	filter {}

project( "MongoTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Mongo/*Tests.*",
	}

	links
	{
		"Mongo",
		"Persist",
		"Reflect",
		"Foundation",
		"Platform",
		"bson-static-1.0",
		"mongoc-static-1.0",
	}

	filter "configurations:Debug"
		links
		{
			"zlibd",
		}

	filter "configurations:not Debug"
		links
		{
			"zlib",
		}

	filter "system:windows"
		links
		{
			"bcrypt",
			"crypt32",
			"dnsapi",
			"secur32",
		}

	filter {}

project( "Inspect" )

	Helium.DoModuleProjectSettings( "Source", "HELIUM", "Inspect", "INSPECT" )

	files
	{
		"Source/Inspect/**",
	}

	excludes
	{
		"Source/Inspect/*Tests.*",
	}

	filter "kind:SharedLib"
		links
		{
			"Math",
			"Persist",
			"Reflect",
			"Application",
			"Foundation",
			"Platform",
		}

	filter {}

project( "InspectTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Inspect/*Tests.*",
	}

	links
	{
		"Inspect",
		"Math",
		"Persist",
		"Reflect",
		"Application",
		"Foundation",
		"Platform",
	}

project( "Math" )

	Helium.DoModuleProjectSettings( "Source", "HELIUM", "Math", "MATH" )

	files
	{
		"Source/Math/**",
	}

	excludes
	{
		"Source/Math/*Tests.*",
	}

	filter "kind:SharedLib"
		links
		{
			"Reflect",
			"Foundation",
			"Platform",
		}

	filter {}

project( "MathTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Math/*Tests.*",
	}

	links
	{
		"Math",
		"Reflect",
		"Foundation",
		"Platform",
	}
