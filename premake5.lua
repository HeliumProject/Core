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
			"mongo-c",
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
			"mongo-c",
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

-- These are breadcrumbs for the travis scripts

print("Writing solution.txt...")
local file = io.open("./Build/solution.txt", "w");
file:write("Core\n");
file:close();

print("Writing platform.txt...")
local file = io.open("./Build/platform.txt", "w");
if _OPTIONS[ "arch" ] == 'x86_64' then
	file:write("x64\n");
else
	file:write("Win32\n");
end
file:close();