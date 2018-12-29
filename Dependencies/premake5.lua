require( './premake' )

workspace "Dependencies"
Helium.DoBasicWorkspaceSettings()

configuration "Debug"
	targetdir( "../Bin/Debug/" )

configuration "Intermediate"
	targetdir( "../Bin/Intermediate/" )

configuration "Profile"
	targetdir( "../Bin/Profile/" )

configuration "Release"
	targetdir( "../Bin/Release/" )

project "googletest"
	uuid "1DCBDADD-043A-4853-8118-5D437106309A"
	kind "StaticLib"
	language "C++"
	includedirs
	{
		"googletest/googletest/include",
		"googletest/googletest/include/internal",
		"googletest/googletest",
	}
	files
	{
		"googletest/googletest/include/**.h",
		"googletest/googletest/src/**.cc",
	}
	excludes
	{
		"googletest/googletest/src/gtest-all.cc",
	}

project "mongo-c"
	uuid "2704694D-D087-4703-9D4F-124D56E17F3F"
	kind "StaticLib"
	language "C"
	defines
	{
		"MONGO_HAVE_STDINT=1",
		"MONGO_STATIC_BUILD=1",
	}
	files
	{
		"mongo-c/src/*.h",
		"mongo-c/src/*.c",
	}

-- These are breadcrumbs for the travis scripts

print("Writing solution.txt...")
local file = io.open("./Build/solution.txt", "w");
file:write("Dependencies\n");
file:close();

print("Writing platform.txt...")
local file = io.open("./Build/platform.txt", "w");
if _OPTIONS[ "architecture" ] == 'x86_64' then
	file:write("x64\n");
else
	file:write("Win32\n");
end
file:close();

