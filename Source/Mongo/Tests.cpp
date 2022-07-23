#include "Precompile.h"

#include "Mongo/Mongo.h"
#include "gtest/gtest.h"

#include "Platform/Process.h"
#include "Foundation/FilePath.h"

using namespace Helium;

class MongoSession
{
public:
	static void Startup();
	static void Shutdown();

	void Initialize();
	void Cleanup();

private:
	ProcessHandle mongod;
	Mongo::Database admin;
};

void MongoSession::Startup()
{
	Mongo::Startup();
}

void MongoSession::Shutdown()
{
	Mongo::Shutdown();
}

void MongoSession::Initialize()
{
	FilePath pathToMongo;
	pathToMongo = GetProcessPath();
	pathToMongo = pathToMongo.Directory();
	pathToMongo = pathToMongo.Parent(); // strip BuildConfig
	pathToMongo = pathToMongo.Parent(); // strip Bin
	pathToMongo.Append( "Dependencies" );
	pathToMongo.Append( "mongodb" );

	FilePath pathToMongoData = pathToMongo;
	pathToMongoData.Append( "data" );

	FilePath pathToMongoBin = pathToMongo;
	pathToMongoBin.Append( "bin" );
	pathToMongoBin.Append( "mongod" );

	std::string commandLine;
	commandLine = pathToMongoBin.Get();
	commandLine += " --dbpath=";
	commandLine += pathToMongoData.Get();
	mongod = Helium::Spawn( commandLine );
	std::string error = Helium::GetErrorString();
	EXPECT_TRUE( mongod != HELIUM_INVALID_PROCESS ) << error;

	// Give it some time to start up
	Thread::Sleep(1000);

	EXPECT_TRUE( admin.Connect( "mongodb://localhost/admin" ) );
}

void MongoSession::Cleanup()
{
	admin.RequestShutdown();

	int mongoResult = Helium::SpawnResult( mongod );
	EXPECT_EQ( mongoResult, 0 );

	// Give the kernel some time to release the port
	Thread::Sleep(1000);
}

class MongoEnvironment : public testing::Environment {
public:
	virtual void SetUp() override;
	virtual void TearDown() override;
};

void MongoEnvironment::SetUp()
{
	MongoSession::Startup();
}

void MongoEnvironment::TearDown()
{
	MongoSession::Shutdown();
}

// will be called before main() and deleted before returning from main()
testing::Environment* const mongoEnvironment = testing::AddGlobalTestEnvironment( new MongoEnvironment );

TEST( Mongo, InitAndCleanup )
{
	MongoSession session;
	session.Initialize();
	session.Cleanup();
}

TEST( Mongo, CreateDeleteDatabase )
{
	MongoSession session;
	session.Initialize();

	// Mongo::Database test;
	// bool connectToTest = test.Connect( "mongodb://localhost/test" );
	// EXPECT_FALSE( connectToTest );

	session.Cleanup();
}

TEST( Mongo, CreateDeleteCollection )
{
	MongoSession session;
	session.Initialize();
	session.Cleanup();
}
