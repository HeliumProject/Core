#include "Platform/Assert.h"

#include "gtest/gtest.h"

int main(int argc, char** argv) {
	printf("Running main() from MainTests.cpp\n");
	
#if HELIUM_ASSERT_ENABLED 
	Helium::Assert::SetFatal(true);
#endif

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
