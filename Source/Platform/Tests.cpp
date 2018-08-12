#include "Precompile.h"
#include "Platform/File.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <fstream>
#include <stdio.h>
#include <vector>


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
