#include "Precompile.h"

#include "Mongo/Mongo.h"

#include "gtest/gtest.h"

using namespace Helium;

TEST( Mongo, MongoStartupShutdown )
{
	Mongo::Initialize();
	Mongo::Cleanup();
}
