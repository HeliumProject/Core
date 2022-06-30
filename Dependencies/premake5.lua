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
