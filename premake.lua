require './Dependencies/premake'

newoption
{
	trigger = "pch",
	description = "Build with precompiled headers",
}

newoption
{
	trigger = "shared",
	description = "Build as shared libraries",
}

newoption
{
	trigger = "nortti",
	description = "Disable run-time type information",
}

-- Common settings for projects linking with libraries.
Helium.DoBasicProjectSettings = function()

	filter {}

	language "C++"

	floatingpoint "Fast"

	flags
	{
		"FatalWarnings",
	}

	if _OPTIONS['shared'] then
		defines
		{
			"HELIUM_SHARED=1",
		}
	else
		defines
		{
			"HELIUM_SHARED=0",
		}
	end

	if _OPTIONS['nortti'] then
		rtti "Off"
	end

	includedirs
	{
		"Source",
		"Dependencies/rapidjson/include",
		"Dependencies/mongo-c/src",
	}

	filter { "system:windows", "kind:SharedLib or *App" }
		links
		{
			"dbghelp",
			"ws2_32",
			"wininet",
		}

	filter "system:macosx or linux"
		buildoptions
		{
			"-std=c++11",
		}

	filter { "system:macosx", "kind:SharedLib or *App" }
		linkoptions
		{
			"-stdlib=libc++",
			"-framework CoreFoundation",
			"-framework Carbon",
			"-framework IOKit",
		}

	filter "system:linux"
		buildoptions
		{
			"-pthread",
		}

	filter {}

end

Helium.DoTestsProjectSettings = function()

	filter {}

	kind "ConsoleApp"

	Helium.DoBasicProjectSettings()

	includedirs
	{
		".",
		"Dependencies/googletest/googletest/include"
	}

	links
	{
		"googletest"
	}

	postbuildcommands
	{
		"\"%{cfg.linktarget.abspath}\""
	}

	filter "system:linux"
		links
		{
			"pthread",
			"dl",
			"rt",
			"m",
			"stdc++",
		}

	filter {}

end

Helium.DoModuleProjectSettings = function( baseDirectory, tokenPrefix, moduleName, moduleNameUpper )

	filter {}

	defines
	{
		"HELIUM_HEAP=1",
		"HELIUM_MODULE=" .. moduleName
	}

	if _OPTIONS["pch"] then
		pchheader( "Precompile.h" )

		local source = "Precompile.cpp"
		source = path.join( moduleName, source )
		source = path.join( baseDirectory, source )
		pchsource( source )

		local include = ""
		source = path.join( moduleName, include )
		source = path.join( baseDirectory, include )
		includedirs
		{
			include,
		}
	end

	Helium.DoBasicProjectSettings()

	if _OPTIONS['shared'] then
		kind "SharedLib"
	else
		kind "StaticLib"
	end

	if string.len(tokenPrefix) > 0 then
		tokenPrefix = tokenPrefix .. "_"
	end

	if os.host() == "windows" then
		filter "kind:SharedLib"
			defines
			{
				tokenPrefix .. moduleNameUpper .. "_EXPORTS",
			}
	end

	if os.host() == "macosx" then
		filter "kind:SharedLib"
			linkoptions
			{
				"-Wl,-install_name,@executable_path/lib" .. project().name .. ".dylib",
			}
	end

	filter {}

end
