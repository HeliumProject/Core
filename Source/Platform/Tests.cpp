#include "Precompile.h"
#include "Platform/Encoding.h"
#include "Platform/File.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <fstream>
#include <stdio.h>
#include <vector>

#ifndef HELIUM_OS_WIN
#include <unistd.h>
#endif

using namespace Helium;


TEST(PlatformFileTest, IsNonInitializedFileOpen)
{
	File f;
	ASSERT_FALSE(f.IsOpen());
}

TEST(PlatformFileTest, OpenNonExistingFileForRead)
{
	File f;
	ASSERT_FALSE(f.Open("foo1.data", FileMode::Read));
}

TEST(PlatformFileTest, OpenNonExistingFileForWrite)
{
	File f;
	const char* fileName = "foo2.data";

	ASSERT_TRUE(f.Open(fileName, FileMode::Write));
	ASSERT_TRUE(f.Close());

	remove(fileName);
}

TEST(PlatformFileTest, CreateCloseOpenForRead)
{
	File f;
	const char* fileName = "foo3.data";

	f.Open(fileName, FileMode::Write);
	f.Close();

	f.Open(fileName, FileMode::Read);
	ASSERT_TRUE(f.IsOpen());
	f.Close();

	remove(fileName);
}

TEST(PlatformFileTest, CreateCloseDelete)
{
	File f;
	const char* fileName = "foo5.data";

	f.Open(fileName, FileMode::Write);
	f.Close();

	ASSERT_TRUE(Delete(fileName));
	ASSERT_FALSE(f.Open(fileName, FileMode::Read));
}

TEST(PlatformFileTest, CheckWriteToFile)
{
	File f;
	const char* fileName = "foo4.data";
	f.Open(fileName, FileMode::Write);

	const size_t DATA_SIZE = 8;
	std::vector<char> inputData = { 1, 2, 3, 4, 5, 6, 7, 8 };

	size_t numberOfBytesWritten = 0;
	f.Write(inputData.data(), DATA_SIZE, &numberOfBytesWritten);

	f.Close();

	std::vector<char> outputData;
	std::ifstream fileToCheck(fileName);
	std::copy(std::istream_iterator<char>(fileToCheck), std::istream_iterator<char>(), std::back_inserter(outputData));
	fileToCheck.close();

	ASSERT_TRUE(outputData.size() == inputData.size());
	ASSERT_TRUE(std::equal(outputData.begin(), outputData.end(), inputData.begin()));

	remove(fileName);
}


class PremadeFileTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		std::ofstream premadeFile(FILE_NAME, std::ios::out | std::ios::binary);
		premadeFile.write(&m_inputData[0], DATA_SIZE);
		premadeFile.close();
	}

	void TearDown() override
	{
		remove(FILE_NAME);
	}

	File m_fileClassUnderTest;

	const char* FILE_NAME = { "foobar.data" };
	static const size_t DATA_SIZE = 8;
	std::vector<char> m_inputData = { 1, 2, 3, 4, 5, 6, 7, 8 };
};

TEST_F(PremadeFileTest, CheckReadFromFile)
{
	File f;
	f.Open(FILE_NAME, FileMode::Read);

	const int64_t fileSize = f.GetSize();
	std::vector<char> readData(static_cast<size_t>(fileSize));

	size_t numberOfBytesWritten = 0;
	f.Read(readData.data(), static_cast<size_t>(fileSize), &numberOfBytesWritten);
	f.Close();

	ASSERT_TRUE(readData.size() == m_inputData.size());
	ASSERT_TRUE(std::equal(readData.begin(), readData.end(), m_inputData.begin()));
}

TEST_F(PremadeFileTest, CheckSeeknReadFromFile)
{
	File f;
	f.Open(FILE_NAME, FileMode::Read);

	const int64_t distanceToMove = 2;
	f.Seek(distanceToMove, SeekOrigins::Begin);

	char readByte;
	size_t numberOfBytesWritten = 0;
	f.Read(&readByte, sizeof(char), &numberOfBytesWritten);
	f.Close();

	ASSERT_TRUE(readByte == m_inputData[distanceToMove]);
}

TEST(PlatformFileTest, FailToGetFullPath)
{
	std::string fullPath = "";
	const char* path = "";
	
	ASSERT_FALSE(GetFullPath(path, fullPath));
}

TEST(PlatformFileTest, PathIsAbsolute)
{
	const char* path = ".";

	ASSERT_FALSE(IsAbsolute(path));
}

TEST(PlatformFileTest, PathIsNotAbsolute)
{
	std::string fullPath = "";
	const char* path = ".";

	bool res = GetFullPath(path, fullPath);

	ASSERT_TRUE(IsAbsolute(fullPath.c_str()));
}

TEST(PlatformFileTest, FailToMakePath)
{
	//ASSERT_DEATH(MakePath(""), ""); 
	//ASSERT_FALSE(MakePath(""));
}

TEST(PlatformFileTest, MakePath)
{
	std::string fullPath = "foo_folder";
	ASSERT_TRUE(MakePath(fullPath.c_str()));

	DirectoryEntry dirEntry(fullPath.c_str());
	Directory dir(fullPath.c_str());
	dir.FindFirst(dirEntry);
	ASSERT_TRUE(dir.IsOpen());
	ASSERT_TRUE(dir.Close());
	
#if HELIUM_OS_WIN
	HELIUM_TCHAR_TO_WIDE(fullPath.c_str(), convertedChars);
	ASSERT_TRUE(::RemoveDirectory(convertedChars));
#else
	ASSERT_FALSE(rmdir(fullPath.c_str()));
#endif
}

TEST(PlatformFileTest, MakePathMakeFileAndMove)
{
	const std::string fullPath = "foo_folder2";
	MakePath(fullPath.c_str());

	File f;
	const std::string fileName("foo3.data");
	const std::string filePath = fullPath + PathSeparator + fileName;

	f.Open(filePath.c_str(), FileMode::Write);
	f.Close();

	const std::string newFullPath = "foo_folder3";
	Move(fullPath.c_str(), newFullPath.c_str());

	const std::string newFilePath = newFullPath + PathSeparator + fileName;
	Move(filePath.c_str(), newFilePath.c_str());

#if HELIUM_OS_WIN
	// stumbled on an issue when calling RemoveDirectory 2 times in a row. The second call would return a Zero (a.k.a. fail) but actually succeed.
	HELIUM_TCHAR_TO_WIDE(fullPath.c_str(), convertedChars3);
	ASSERT_FALSE(::RemoveDirectory(convertedChars3));
#endif

	f.Open(newFilePath.c_str(), FileMode::Read);
	ASSERT_TRUE(f.IsOpen());
	f.Close();

	ASSERT_FALSE(remove(newFilePath.c_str()));

#if HELIUM_OS_WIN
	HELIUM_TCHAR_TO_WIDE(newFullPath.c_str(), convertedChars2);
	ASSERT_TRUE(::RemoveDirectory(convertedChars2));
#else
	ASSERT_FALSE(rmdir(fullPath.c_str()));
	ASSERT_FALSE(rmdir(newFullPath.c_str()));
#endif
}