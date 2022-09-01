Helium = {}

Helium.RequiredPremakeVersion = '5.0.0-beta1'

Helium.ExecuteAndCapture = function( cmd, raw )
	local f = assert( io.popen( cmd, 'r' ) )
	local s = assert( f:read( '*a' ) )
	f:close()
	if raw then
		return s
	end
	s = string.gsub(s, '^%s+', '')
	s = string.gsub(s, '%s+$', '')
	s = string.gsub(s, '[\n\r]+', ' ')
	return s
end

Helium.ExecuteAndExpect = function( cmd, expectedResult )
	local success, termination, result = os.execute( cmd )
	if result ~= expectedResult then
		premake.error( "'" .. cmd .. "' expected " .. tostring( expectedResult ) .. ", got " .. tostring( result ) )
		os.exit( 1 )
	end
end

Helium.GetPremakeVersion = function()
	if _PREMAKE_VERSION == "HEAD" then
		return 5
	else
		return tonumber( string.sub( _PREMAKE_VERSION, 1, 1 ) )
	end
end

Helium.GetSystemVersion = function()
	local version = 'Unknown'
	if os.host() == "windows" then
		version = Helium.ExecuteAndCapture( "cmd /c ver" )
	else
		version = Helium.ExecuteAndCapture( "uname -r" )
	end

	return version
end

Helium.GetSystemArchitecture = function()
	local arch = Helium.ExecuteAndCapture( "uname -m" )
	
	-- check for rosetta in our process
	if os.host() == "macosx" then
		if Helium.ExecuteAndCapture("sysctl -in sysctl.proc_translated") == "1" then
			return "arm64"
		else
			return "x86_64"
		end
	end

	return arch
end

Helium.GetTargetArchitecture = function()
	local arch = _OPTIONS[ "arch" ]

	if arch == "host" then
		arch = Helium.GetSystemArchitecture()
	end

	return arch
end

Helium.GetProcessorCount = function()
	local result = nil
	if os.host() == "windows" then
		result = os.getenv("NUMBER_OF_PROCESSORS")
	elseif os.host() == "macosx" then
		result = Helium.ExecuteAndCapture("sysctl -n hw.ncpu")
	elseif os.host() == "linux" then
		result = Helium.ExecuteAndCapture( "nproc" )
	end

	result = tonumber( result )
	if result ~= nil then
		return result
	else
		return 4
	end
end

Helium.Sleep = function( seconds )
	if os.host() == "windows" then
		os.execute("ping 127.0.0.1 -n " .. seconds + 1 .. " -w 1000 >:nul 2>&1")
	else
		os.execute("sleep " .. seconds)
	end
end

Helium.Publish = function( files )
	for i,v in pairs(files) do
		-- mkpath the target folder
		os.mkdir( v.target )

		local name = v.name
		if not name then
			name = v.file
		end

		local path = v.source .. v.file
		local exists = os.isfile( path )
		local destination = v.target .. name

		print( path .. "\n\t-> " .. destination )
		os.copyfile( path, destination ) -- linux returns non-zero if the target file is identical (!?)

		-- the files were copied, complete this entry
		files[ i ] = nil
	end
end

Helium.GetVcpkgTriplet = function()
	local triplet = "vcpkg-" .. os.host()

	if os.host() == "windows" then
		triplet = triplet .. "-" .. _ACTION
	else
		triplet = triplet .. "-" .. Helium.GetTargetArchitecture()
	end

	return triplet
end

newoption
{
	trigger = "arch",
	description = "Specify architecture (see premake 'architecture' action for choices)",
	default = "host",
}

Helium.DoBasicWorkspaceSettings = function()

	architecture( Helium.GetTargetArchitecture() )
	location "Build"
	objdir "Build"

	configurations
	{
		"Debug",
		"Intermediate",
		"Profile",
		"Release",
	}

	defines
	{
		"UNICODE=1",
	}

	characterset "Unicode"

	flags
	{
		"NoMinimalRebuild",
	}

	filter "system:windows"
		defines
		{
			"WIN32",
			"_WIN32",
			"_CRT_SECURE_NO_DEPRECATE",
			"_CRT_NON_CONFORMING_SWPRINTFS",
			"_WINSOCK_DEPRECATED_NO_WARNINGS",
		}

	filter "configurations:Debug"
		defines
		{
			"_DEBUG",
			"HELIUM_DEBUG=1",
		}
		symbols "On"

	filter "configurations:Intermediate"
		defines
		{
			"HELIUM_INTERMEDIATE=1",
		}
		symbols "On"
		optimize "Speed"
		editandcontinue "Off"

	filter "configurations:Profile"
		defines
		{
			"NDEBUG",
			"HELIUM_PROFILE=1",
		}
		symbols "On"
		optimize "Speed"
		editandcontinue "Off"
		omitframepointer "On"

	filter "configurations:Release"
		defines
		{
			"NDEBUG",
			"HELIUM_RELEASE=1",
		}
		symbols "On"
		optimize "Speed"
		editandcontinue "Off"
		omitframepointer "On"

	filter "system:windows"
		buildoptions
		{
			"/MP",
			"/Zm256",
			"/d2Zi+", -- http://randomascii.wordpress.com/2013/09/11/debugging-optimized-codenew-in-visual-studio-2012/
		}
		linkoptions
		{
			"/ignore:4221", -- disable warning about linking .obj files with not symbols defined (conditionally compiled away)
		}

	filter { "system:windows", "configurations:Debug" }
		buildoptions
		{
			"/Ob0",
		}

	filter { "system:windows", "configurations:not Debug" }
		buildoptions
		{
			"/Ob2",
			"/Oi",
		}

	filter { "system:macosx" }
		buildoptions
		{
			"-stdlib=libc++", -- clang's stdlib
		}

	filter { "system:linux" }
		buildoptions
		{
			"-fPIC"
		}

	filter {}

end
