#include "Platform/Assert.h"

#include "gtest/gtest.h"

int main(int argc, char** argv) {
	printf("Running main() from MainTests.cpp\n");
	
	Helium::Assert::SetAssertIsFatal(true);

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
