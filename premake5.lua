require( './premake' )

workspace "Core"

Helium.DoBasicWorkspaceSettings()

configuration "Debug"
	targetdir( "Bin/Debug/" )
	libdirs { "Bin/Debug/" }

configuration "Intermediate"
	targetdir( "Bin/Intermediate/" )
	libdirs { "Bin/Intermediate/" }

configuration "Profile"
	targetdir( "Bin/Profile/" )
	libdirs { "Bin/Profile/" }

configuration "Release"
	targetdir( "Bin/Release/" )
	libdirs { "Bin/Release/" }

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

	configuration "windows"
		excludes
		{
			"Source/Platform/*Posix.*",
			"Source/Platform/*Mac.*",
			"Source/Platform/*Lin.*",
		}

	configuration "macosx"
		excludes
		{
			"Source/Platform/*Win.*",
			"Source/Platform/*Lin.*",
		}

	configuration "linux"
		excludes
		{
			"Source/Platform/*Win.*",
			"Source/Platform/*Mac.*",
		}

	configuration { "SharedLib", "linux" }
		links
		{
			"pthread",
			"dl",
		}

	configuration {}

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

	configuration "SharedLib"
		links
		{
			"Platform",
		}

	configuration {}

project( "FoundationTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Foundation/*Tests.*",
	}

	links
	{
		"Platform",
		"Foundation"
	}

project( "Application" )

	Helium.DoModuleProjectSettings( "Source", "HELIUM", "Application", "APPLICATION" )

	files
	{
		"Source/Application/**",
	}

	configuration "SharedLib"
		links
		{
			"Platform",
			"Foundation",
		}

	configuration {}

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

	configuration "SharedLib"
		links
		{
			"Platform",
			"Foundation",
		}

	configuration {}

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

	configuration "SharedLib"
		links
		{
			"Platform",
			"Foundation",
			"Reflect",
			"mongo-c",
		}

	configuration {}

project( "Mongo" )

	Helium.DoModuleProjectSettings( "Source", "HELIUM", "Mongo", "MONGO" )

	files
	{
		"Source/Mongo/**",
	}

	configuration "SharedLib"
		links
		{
			"Platform",
			"Foundation",
			"Reflect",
			"Persist",
			"mongo-c",
		}

	configuration {}

project( "Inspect" )

	Helium.DoModuleProjectSettings( "Source", "HELIUM", "Inspect", "INSPECT" )

	files
	{
		"Source/Inspect/**",
	}

	includedirs
	{
		"Source/Inspect",
	}

	configuration "SharedLib"
		links
		{
			"Platform",
			"Foundation",
			"Application",
			"Reflect",
			"Persist",
			"Math",
		}

	configuration {}

project( "Math" )

	Helium.DoModuleProjectSettings( "Source", "HELIUM", "Math", "MATH" )

	files
	{
		"Source/Math/**",
	}

	configuration "SharedLib"
		links
		{
			"Platform",
			"Foundation",
			"Reflect",
			"Persist",
		}

	configuration {}

project( "Core" )

	if os.host() == "windows" then
		pchheader( "Precompile.h" )
		pchsource( "Source/Monolithic/Precompile.cpp" )
	end

	Helium.DoBasicProjectSettings()

	if _OPTIONS['shared'] then
		kind "SharedLib"
	else
		kind "StaticLib"
	end

	defines
	{
		"HELIUM_HEAP=0",
	}

	includedirs
	{
		"Source/Monolithic"
	}

	files
	{
		"Source/**.cpp",
		"Source/**.h",
		"Source/**.inl",
	}

	excludes
	{
		"Source/**Tests.*",
	}

	configuration "windows"
		excludes
		{
			"Source/Platform/*Posix.*",
			"Source/Platform/*Mac.*",
			"Source/Platform/*Lin.*",
		}

	configuration "macosx"
		excludes
		{
			"Source/Platform/*Win.*",
			"Source/Platform/*Lin.*",
		}

	configuration "linux"
		excludes
		{
			"Source/Platform/*Win.*",
			"Source/Platform/*Mac.*",
		}

	configuration {}

	links
	{
		"Persist",
		"Mongo",
		"Inspect",
		"Math",
		"Reflect",
		"Foundation",
		"Platform",
		"googletest",
		"mongo-c",
	}

	configuration { "SharedLib", "linux" }
		links
		{
			"pthread",
			"dl",
		}

	configuration {}

project( "CoreTests" )

	configuration {}

	kind "ConsoleApp"

	Helium.DoBasicProjectSettings()

	includedirs
	{
		".",
		"Dependencies/googletest/googletest/include"
	}

	defines
	{
		"HELIUM_HEAP=0",
	}

	files
	{
		"Source/**Tests.*",
	}

	links
	{
		"Persist",
		"Mongo",
		"Inspect",
		"Math",
		"Reflect",
		"Foundation",
		"Platform",
		"googletest",
		"mongo-c",
	}

	configuration "linux"
		links
		{
			"pthread",
			"dl",
			"rt",
			"m",
			"stdc++",
		}

	configuration {}

	postbuildcommands
	{
		"\"%{cfg.linktarget.abspath}\""
	}
