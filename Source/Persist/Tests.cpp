#include "Precompile.h"

#include "Persist/Archive.h"

#include "gtest/gtest.h"

using namespace Helium;

TEST( Persist, PersistStartupShutdown )
{
	Persist::Startup();
	Persist::Shutdown();
}
