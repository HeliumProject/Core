#include "Precompile.h"

#include "Platform/Console.h"
#include "Platform/Assert.h"

#include "gtest/gtest.h"

#include <fstream>
#include <iostream>
#include <stdio.h>

using namespace Helium;

void setupFileScanTest(const char* const fileName, const char* strings[], size_t stringCount)
{
	std::ofstream fileToCheck(fileName);
	
	for (size_t i = 0; i < stringCount; i++)
	{
		fileToCheck << strings[i];
	}

	fileToCheck.close();
}

 TEST(PlatformConsoleTest, CheckScanFromFile )
 {
	 const char* const fileName = "scan_test1.txt";
	 const char* strings[] = { "testing123", "\n", "54321" };
	 setupFileScanTest(fileName, strings, 3);

	 FILE* fileToScan = fopen(fileName, "r");

	 char scanedText[64];
	 int res = FileScan(fileToScan, "%s", scanedText);
	 ASSERT_TRUE(res == 1);
	 ASSERT_TRUE(strcmp(strings[0], scanedText) == 0);

	 int num = 0;
	 res = FileScan(fileToScan, "%d", &num);
	 ASSERT_TRUE(res == 1);
	 ASSERT_TRUE(num == 54321);
	 fclose(fileToScan);

	 remove(fileName);
 }

 TEST(PlatformConsoleTest, CheckScanFromFile2)
 {
	 const char* const fileName = "scan_test2.txt";

	 // the only diferentce between CheckScanFromFile test
	 const char* strings[] = { "testing123", " " /*no new line*/, "54321" };
	 setupFileScanTest(fileName, strings, 3);

	 FILE* fileToScan = fopen(fileName, "r");

	 char scanedText[64];
	 int res = FileScan(fileToScan, "%s", scanedText);
	 ASSERT_TRUE(res == 1);
	 ASSERT_TRUE(strcmp(strings[0], scanedText) == 0);

	 int num = 0;
	 FileScan(fileToScan, "%d", &num);
	 ASSERT_TRUE(num == 0);
	 fclose(fileToScan);

	 remove(fileName);
 }

 TEST(PlatformConsoleTest, CheckScanFromFile3)
 {
	 const char* const fileName = "scan_test2.txt";

	 // the only diferentce between CheckScanFromFile test
	 const char* strings[] = { "testing123", " " /*no new line*/, "54321" };
	 setupFileScanTest(fileName, strings, 3);

	 FILE* fileToScan = fopen(fileName, "r");

	 char scanedText[64];
	 int res = FileScan(fileToScan, "", scanedText);
	 fclose(fileToScan);

	 remove(fileName);
 }

 TEST(PlatformConsoleTest, CheckStringScan)
 {
	 int num = 0;
	 char str[10];
	 StringScan("9876 foobar", "%d %s", &num, str);

	 ASSERT_TRUE(num == 9876);
	 ASSERT_TRUE(strcmp(str, "foobar") == 0);
 }

 TEST(PlatformConsoleTest, CheckFilePrint)
 {
	 const char* const fileName = "print_test.txt";

	 FILE* file = fopen(fileName, "w");
	 ASSERT_TRUE(file != NULL);

	 int res = FilePrint(file, L"testing %d", 1234);
	 ASSERT_TRUE(res != 0);
	 ASSERT_TRUE(fclose(file) == 0);

	 std::wifstream fileToCheck(fileName);

	 ASSERT_TRUE(fileToCheck.is_open());
	 std::wstring text(L"");
	 int num = 0;
	 fileToCheck >> text >> num;

	 fileToCheck.close();

	 ASSERT_TRUE(num == 1234);
	 ASSERT_TRUE(wcscmp(text.c_str(), L"testing") == 0);
 }

 TEST(PlatformConsoleTest, CheckStringPrint)
 {
	 const int size = 10;
	 char dest[size];

	 int res = StringPrint(dest, size, "foobar %d", size);
	 ASSERT_TRUE(res != 0);

	 int num = 0;
	 char str[size];
	 StringScan(dest, "%s %d", str, &num);

	 ASSERT_TRUE(num == size);
	 ASSERT_TRUE(strcmp(str, "foobar") == 0);
 }

 TEST(PlatformConsoleTest, CheckStringPrintWithSizeDeduction)
 {
	 const int size = 10;
	 char dest[size];

	 int res = StringPrint(dest, "barbaz %d", size);
	 ASSERT_TRUE(res != 0);

	 int num = 0;
	 char str[size];
	 StringScan(dest, "%s %d", str, &num);

	 ASSERT_TRUE(num == size);
	 ASSERT_TRUE(strcmp(str, "barbaz") == 0);
 }

 TEST(PlatformConsoleTest, CheckStringPrintOverSize)
 {
	 const int size = 10;
	 char dest[size];

	 int res = StringPrint(dest, size, "foobar bar baz %d", 33);

	 char str1[size];
	 char str2[size];
	 StringScan(dest, "%s %s", str1, str2);

	 ASSERT_TRUE(strcmp(str1, "foobar") == 0);
	 ASSERT_TRUE(strcmp(str2, "ba") == 0);
 }


#if HELIUM_ASSERT_ENABLED 
TEST(PlatformConsoleTest, FailOnWCharScan)
{
	ASSERT_DEATH(Scan(L""), "");
}

TEST(PlatformConsoleTest, CheckScanFromNullFile)
{
	FILE* fileToScan = nullptr;
	char scanedText[64];
	ASSERT_DEATH(FileScan(fileToScan, "%s", scanedText), "");
}

TEST(PlatformConsoleTest, CheckPrintToNullFile)
{
	FILE* file = nullptr;
	char scanedText[64];
	ASSERT_DEATH(FilePrint(file, "123 %s", scanedText), "");
}

TEST(PlatformConsoleTest, FailOnWCharFileScan)
{
	ASSERT_DEATH(FileScan(nullptr, L""), "");
}
#endif