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

newoption
{
	trigger = "monolithic",
	description = "Build monolithic lib",
}

if not _OPTIONS["monolithic"] then

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

		configuration "SharedLib"
			links
			{
				"Foundation",
				"Platform",
			}

		configuration {}

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

		configuration "SharedLib"
			links
			{
				"Foundation",
				"Platform",
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

		excludes
		{
			"Source/Persist/*Tests.*",
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
			"mongo-c",
		}

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

		configuration "SharedLib"
			links
			{
				"Math",
				"Persist",
				"Reflect",
				"Application",
				"Foundation",
				"Platform",
			}

		configuration {}

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

		configuration "SharedLib"
			links
			{
				"Reflect",
				"Foundation",
				"Platform",
			}

		configuration {}

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
end

if _OPTIONS["monolithic"] then
	project( "Core" )

		if _OPTIONS["pch"] then
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
			"HELIUM_MONOLITHIC=1",
			"HELIUM_MONOLITHIC_EXPORTS",
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
			"googletest",
			"mongo-c",
		}

		configuration { "SharedLib", "linux" }
			links
			{
				"pthread",
				"dl",
				"rt",
				"m",
				"stdc++",
				"c",
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
			"HELIUM_MONOLITHIC=1",
		}

		files
		{
			"Source/**Tests.*",
		}

		links
		{
			"Core",
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
				"c",
			}

		configuration {}

		postbuildcommands
		{
			"\"%{cfg.linktarget.abspath}\""
		}
end