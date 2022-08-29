require( './premake' )

workspace "Dependencies"
Helium.DoBasicWorkspaceSettings()

filter "configurations:Debug"
	targetdir( "../Bin/Debug/" )

filter "configurations:Intermediate"
	targetdir( "../Bin/Intermediate/" )

filter "configurations:Profile"
	targetdir( "../Bin/Profile/" )

filter "configurations:Release"
	targetdir( "../Bin/Release/" )

filter {}

print( "We don't have any premake-built projects at the moment. Thanks vcpkg!" )
os.exit(0)