#include "Precompile.h"
#include "Log.h"

#include "Platform/Assert.h"
#include "Platform/Locks.h"
#include "Platform/Thread.h"
#include "Platform/File.h"
#include "Platform/Console.h"
#include "Platform/Encoding.h"

#include "Foundation/String.h"

#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>

#include <map>

#if HELIUM_OS_WIN
# include <crtdbg.h>
# include <shlobj.h>
#endif

using namespace Helium;
using namespace Helium::Log;

static uint32_t      g_LogFileCount = 20;
static Helium::Mutex g_Mutex;

typedef std::map<std::string, File*> M_Files;

class FileManager
{
private:
	M_Files m_Files;

public:
	FileManager()
	{

	}

	~FileManager()
	{
		M_Files::const_iterator itr = m_Files.begin();
		M_Files::const_iterator end = m_Files.end();
		for ( ; itr != end; ++itr )
		{
			itr->second->Close();
			delete itr->second;
		}

		m_Files.clear();
	}

	bool Opened(const std::string& fileName, File* f)
	{
		return m_Files.insert( M_Files::value_type (fileName, f) ).second;
	}

	File* Find(const std::string& fileName)
	{
		M_Files::const_iterator found = m_Files.find(fileName.c_str());
		if (found != m_Files.end())
		{
			return found->second;
		}
		else
		{
			return NULL;
		}
	}

	void Close(const std::string& fileName)
	{
		M_Files::iterator found = m_Files.find( fileName );
		if (found != m_Files.end())
		{
			found->second->Close();
			delete found->second;
			m_Files.erase( found );
		}
		else
		{
			HELIUM_BREAK();
		}
	}
};

FileManager g_FileManager;

struct OutputFile
{
	Channel       m_ChannelType;
	int          m_RefCount;
	ThreadId m_ThreadId;

	OutputFile()
		: m_ChannelType( Channels::Normal )
		, m_RefCount( 0 )
		, m_ThreadId()
	{

	}
};

typedef std::map< std::string, OutputFile > M_OutputFile;
static M_OutputFile g_TraceFiles;

static uint32_t g_Channels = Channels::Normal | Channels::Warning | Channels::Error;
static Level g_Level = Levels::Default;
static int g_Indent = 0;

static ListenerSignature::Event g_LogEvent;

void Log::Statement::ApplyIndent( const char* string, std::string& output )
{
	if ( m_Indent > 0 )
	{
		char m_IndentString[64] = "";
		if(m_Indent >= sizeof(m_IndentString))
		{
			m_Indent = sizeof(m_IndentString)-1;
		}

		for (int i=0; i<m_Indent; i++)
		{
			m_IndentString[i] = ' ';
		}
		m_IndentString[m_Indent] = '\0';

		// insert the indtent string after newlines and before non-newlines
		const char* pos = string;
		char previous = '\n';
		for ( ; *pos != '\0'; previous = *pos++ )
		{
			if ( *pos == '\r' )
			{
				continue;
			}

			if ( previous == '\n' && *pos != '\n' )
			{
				// start of a new line, apply m_Indent
				output += m_IndentString;
			}

			// copy the char to the statement
			output += *pos;
		}
	}
	else
	{
		output = string;
	}
}

void Log::AddListener(const ListenerSignature::Delegate& listener)
{
	Helium::MutexScopeLock mutex (g_Mutex);

	g_LogEvent.Add(listener);
}

void Log::RemoveListener(const ListenerSignature::Delegate& listener)
{
	Helium::MutexScopeLock mutex (g_Mutex);

	g_LogEvent.Remove(listener);
}

void Redirect(const std::string& fileName, const char* str, bool stampNewLine = true )
{
	File* f = g_FileManager.Find(fileName);
	if (f)
	{
		size_t length = 0;
		size_t count = ( StringLength( str ) + 128 );
		char* temp = (char*)alloca( sizeof(char) * count );
		if ( stampNewLine )
		{
			uint32_t timestamp = 0;

#if defined(HELIUM_OS_WIN)
			_timeb currentTime;
			_ftime( &currentTime );
			timestamp = (uint32_t) currentTime.time;
#else
			timestamp = time(nullptr);
#endif

			uint32_t sec = timestamp % 60; timestamp /= 60;
			uint32_t min = timestamp % 60; timestamp /= 60;
			uint32_t hour = timestamp % 24;
			length = StringPrint( temp, count, "[%02d:%02d:%02d TID:%d] %s", hour, min, sec, Thread::GetCurrentId(), str );
		}
		else
		{
			length = StringPrint( temp, count, "%s", str );
		}

		f->Write( temp, length );
		f->Flush();
	}
}

bool AddFile( M_OutputFile& files, const std::string& fileName, Channel channel, ThreadId threadId, bool append )
{
	Helium::MutexScopeLock mutex (g_Mutex);

	M_OutputFile::iterator found = files.find( fileName );
	if ( found != files.end() )
	{
		if ( found->second.m_ChannelType != channel )
		{
			HELIUM_BREAK(); // trying to add the same file, but with a different channel type
		}

		if ( found->second.m_ThreadId != threadId )
		{
			HELIUM_BREAK(); // trying to add the same file with a different thread id
		}

		found->second.m_RefCount++; // another reference
	}
	else
	{
		if (fileName != "")
		{
			File* f = new File;
			if ( f->Open( fileName.c_str(), FileModes::Write ) )
			{
				if ( append )
				{
					f->Seek( 0, SeekOrigins::End );
				}

				g_FileManager.Opened( fileName, f );
				OutputFile info;
				info.m_ChannelType = channel;
				info.m_RefCount = 1;
				info.m_ThreadId = threadId;
				files[ fileName ] = info;
				return true;
			}
			else
			{
				delete f;
				f = NULL;
			}
		}
	}

	return false;
}

void RemoveFile( M_OutputFile& files, const std::string& fileName )
{
	Helium::MutexScopeLock mutex (g_Mutex);

	M_OutputFile::iterator found = files.find( fileName );
	if ( found != files.end() )
	{
		found->second.m_RefCount--;

		if ( found->second.m_RefCount == 0 )
		{
			g_FileManager.Close( fileName );
			files.erase( found );
		}
	}
}

bool Log::AddTraceFile( const std::string& fileName, Channel channel, ThreadId threadId, bool append )
{
	return AddFile( g_TraceFiles, fileName, channel, threadId, append );
}

void Log::RemoveTraceFile( const std::string& fileName )
{
	RemoveFile( g_TraceFiles, fileName );
}

void Log::Indent(int col)
{
	if ( Thread::IsMain() )
	{
		g_Indent += (col < 0 ? 2 : col);
	}
}

void Log::UnIndent(int col)
{
	if ( Thread::IsMain() )
	{
		g_Indent -= (col < 0 ? 2 : col);
		if (g_Indent < 0)
		{
			g_Indent = 0;
		}
	}
}

Level Log::GetLevel()
{
	return g_Level;
}

void Log::SetLevel(Level level)
{
	g_Level = level;
}

bool Log::IsChannelEnabled( Channel channel )
{
	return ( g_Channels & channel ) == channel;
}

void Log::EnableChannel( Channel channel, bool enable )
{
	if ( enable )
	{
		g_Channels |= channel;
	}
	else
	{
		g_Channels &= ~channel;
	}
}

ConsoleColor Log::GetChannelColor( Log::Channel channel )
{
	switch (channel)
	{
	case Channels::Normal:
		return ConsoleColors::None;

	case Channels::Debug:
		return ConsoleColors::Aqua;

	case Channels::Profile:
		return ConsoleColors::Green;

	case Channels::Warning:
		return ConsoleColors::Yellow;

	case Channels::Error:
		return ConsoleColors::Red;

	default:
		HELIUM_BREAK();
		break;
	}

	return ConsoleColors::None;
}

void Log::LockMutex()
{
	g_Mutex.Lock();
}

void Log::UnlockMutex()
{
	g_Mutex.Unlock();
}

void Log::PrintString(const char* string, Channel channel, Level level, ConsoleColor color, int indent, char* output, uint32_t outputSize)
{
	Helium::MutexScopeLock mutex (g_Mutex);

	// check trace files
	bool trace = false;
	M_OutputFile::iterator itr = g_TraceFiles.begin();
	M_OutputFile::iterator end = g_TraceFiles.end();
	for( ; itr != end; ++itr )
	{
		if ( ( (*itr).second.m_ChannelType & channel ) == channel
			&& ( (*itr).second.m_ThreadId == ThreadId () || (*itr).second.m_ThreadId == Thread::GetCurrentId() ) )
		{
			trace = true;
		}
	}

	// determine if we should be displayed
	bool display = ( g_Channels & channel ) == channel && level <= g_Level;

	// check for nothing to do
	if ( trace || display || output )
	{
		if ( indent < 0 )
		{
			indent = g_Indent;
		}

		// the statement
		Statement statement ( string, channel, level, indent );

		// construct the print statement
		ListenerArgs args ( statement );

		// is this statement to be output via normal channels
		if ( display )
		{
			// raise the printing event
			g_LogEvent.Raise( args );
		}

		// only process this string if it was not handled by a handler
		if ( !args.m_Skip )
		{
			// apply indentation
			statement.m_String.clear();
			statement.ApplyIndent( string, statement.m_String );

			// output to screen window
			if ( display )
			{
				// deduce the color if we were told to do so
				if ( color == ConsoleColors::None )
				{
					color = GetChannelColor( channel );
				}

				// print the statement to the window
				Helium::PrintString(color, channel == Channels::Error ? stderr : stdout, statement.m_String);
			}

			// send the text to the debugger, if no debugger nothing happens
#if HELIUM_OS_WIN
			HELIUM_CONVERT_TO_WIDE( statement.m_String.c_str(), convertedStatement );
			OutputDebugStringW( convertedStatement );
#endif
			// output to trace file(s)
			static bool stampNewLine = true;

			itr = g_TraceFiles.begin();
			end = g_TraceFiles.end();
			for( ; itr != end; ++itr )
			{
				if ( ( (*itr).second.m_ChannelType & channel ) == channel
					&& ( (*itr).second.m_ThreadId == ThreadId () || (*itr).second.m_ThreadId == Thread::GetCurrentId() ) )
				{
					Redirect( (*itr).first, statement.m_String.c_str(), stampNewLine );
				}
			}

			// update stampNewLine
			if ( !statement.m_String.empty() )
			{
				stampNewLine = ( *statement.m_String.rbegin() == '\n' ) ? true : false ;
			}

			// output to buffer
			if (output && outputSize > 0)
			{
				CopyString( output, outputSize - 1, statement.m_String.c_str() );
			}
		}
	}
}

void Log::PrintStatement(const Statement& statement)
{
	Helium::MutexScopeLock mutex (g_Mutex);

	PrintString( statement.m_String.c_str(), statement.m_Channel, statement.m_Level, GetChannelColor( statement.m_Channel ), statement.m_Indent );
}

void Log::PrintStatements(const std::vector< Statement >& statements, uint32_t channelFilter)
{
	Helium::MutexScopeLock mutex (g_Mutex);

	std::vector< Statement >::const_iterator itr = statements.begin();
	std::vector< Statement >::const_iterator end = statements.end();
	for ( ; itr != end; ++itr )
	{
		if ( itr->m_Channel & channelFilter )
		{
			PrintStatement( *itr );
		}
	}
}

void Log::PrintColor(ConsoleColor color, const char* fmt, ...)
{
	Helium::MutexScopeLock mutex (g_Mutex);

	va_list args;
	va_start(args, fmt); 
	static char string[MAX_PRINT_SIZE];
	int size = StringPrintArgs(string, fmt, args);
	string[ sizeof(string)/sizeof(string[0]) - 1] = 0; 
	HELIUM_ASSERT(size >= 0);
	PrintString(string, Channels::Normal, Levels::Default, color); 
	va_end(args); 
}

void Log::Print(const char *fmt,...) 
{
	Helium::MutexScopeLock mutex (g_Mutex);

	va_list args;
	va_start(args, fmt); 
	static char string[MAX_PRINT_SIZE];
	int size = StringPrintArgs(string, fmt, args);
	string[ sizeof(string)/sizeof(string[0]) - 1] = 0; 
	HELIUM_ASSERT(size >= 0);

	PrintString(string, Channels::Normal, Levels::Default, Log::GetChannelColor( Channels::Normal ));
	va_end(args);      
}

void Log::Print(Level level, const char *fmt,...) 
{
	Helium::MutexScopeLock mutex (g_Mutex);

	va_list args;
	va_start(args, fmt); 
	static char string[MAX_PRINT_SIZE];
	int size = StringPrintArgs(string, fmt, args);
	string[ sizeof(string)/sizeof(string[0]) - 1] = 0; 
	HELIUM_ASSERT(size >= 0);

	PrintString(string, Channels::Normal, level, Log::GetChannelColor( Channels::Normal ));
	va_end(args);       
}

void Log::Debug(const char *fmt,...) 
{
	Helium::MutexScopeLock mutex (g_Mutex);

	static char format[MAX_PRINT_SIZE];
	StringPrint(format, "DEBUG: ");
	AppendString(format, fmt);

	va_list args;
	va_start(args, fmt); 
	static char string[MAX_PRINT_SIZE];
	int size = StringPrintArgs(string, format, args);
	string[ sizeof(string)/sizeof(string[0]) - 1] = 0; 
	HELIUM_ASSERT(size >= 0);

	PrintString(string, Channels::Debug, Levels::Default, Log::GetChannelColor( Channels::Debug ), 0);
	va_end(args);
}

void Log::Debug(Level level, const char *fmt,...) 
{
	Helium::MutexScopeLock mutex (g_Mutex);

	static char format[MAX_PRINT_SIZE];
	StringPrint(format, "DEBUG: ");
	AppendString(format, fmt);

	va_list args;
	va_start(args, fmt); 
	static char string[MAX_PRINT_SIZE];
	int size = StringPrintArgs(string, format, args);
	string[ sizeof(string)/sizeof(string[0]) - 1] = 0; 
	HELIUM_ASSERT(size >= 0);

	PrintString(string, Channels::Debug, level, Log::GetChannelColor( Channels::Debug ), 0);
	va_end(args);
}

void Log::Profile(const char *fmt,...) 
{
	Helium::MutexScopeLock mutex (g_Mutex);

	static char format[MAX_PRINT_SIZE];
	StringPrint(format, "PROFILE: ");
	AppendString(format, fmt);

	va_list args;
	va_start(args, fmt); 
	static char string[MAX_PRINT_SIZE];
	int size = StringPrintArgs(string, format, args);
	string[ sizeof(string)/sizeof(string[0]) - 1] = 0; 
	HELIUM_ASSERT(size >= 0);

	PrintString(string, Channels::Profile, Levels::Default, Log::GetChannelColor( Channels::Profile ), 0);
	va_end(args);
}

void Log::Profile(Level level, const char *fmt,...) 
{
	Helium::MutexScopeLock mutex (g_Mutex);

	static char format[MAX_PRINT_SIZE];
	StringPrint(format, "PROFILE: ");
	AppendString(format, fmt);

	va_list args;
	va_start(args, fmt); 
	static char string[MAX_PRINT_SIZE];
	int size = StringPrintArgs(string, format, args);
	string[ sizeof(string)/sizeof(string[0]) - 1] = 0; 
	HELIUM_ASSERT(size >= 0);

	PrintString(string, Channels::Profile, level, Log::GetChannelColor( Channels::Profile ), 0);
	va_end(args);
}

void Log::Warning(const char *fmt,...) 
{
	Helium::MutexScopeLock mutex (g_Mutex);

	static char format[MAX_PRINT_SIZE];
	StringPrint(format, "WARNING: ");
	AppendString(format, fmt);

	va_list args;
	va_start(args, fmt); 
	static char string[MAX_PRINT_SIZE];
	int size = StringPrintArgs(string, format, args);
	string[ sizeof(string)/sizeof(string[0]) - 1] = 0; 
	HELIUM_ASSERT(size >= 0);

	PrintString(string, Channels::Warning, Levels::Default, Log::GetChannelColor( Channels::Warning ), 0);
	va_end(args);      
}

void Log::Warning(Level level, const char *fmt,...) 
{
	Helium::MutexScopeLock mutex (g_Mutex);

	static char format[MAX_PRINT_SIZE];
	StringPrint(format, "WARNING: ");
	AppendString(format, fmt);

	va_list args;
	va_start(args, fmt); 
	static char string[MAX_PRINT_SIZE];
	int size = StringPrintArgs(string, format, args);
	string[ sizeof(string)/sizeof(string[0]) - 1] = 0; 
	HELIUM_ASSERT(size >= 0);

	PrintString(string, Channels::Warning, level, Log::GetChannelColor( Channels::Warning ), 0);
	va_end(args);      
}

void Log::Error(const char *fmt,...) 
{
	Helium::MutexScopeLock mutex (g_Mutex);

	static char format[MAX_PRINT_SIZE];
	StringPrint(format, "ERROR: ");
	AppendString(format, fmt);

	va_list args;
	va_start(args, fmt); 
	static char string[MAX_PRINT_SIZE];
	int size = StringPrintArgs(string, format, args);
	string[ sizeof(string)/sizeof(string[0]) - 1] = 0; 
	HELIUM_ASSERT(size >= 0);

	PrintString(string, Channels::Error, Levels::Default, Log::GetChannelColor( Channels::Error ), 0);
	va_end(args);
}

void Log::Error(Level level, const char *fmt,...) 
{
	Helium::MutexScopeLock mutex (g_Mutex);

	static char format[MAX_PRINT_SIZE];
	StringPrint(format, "ERROR: ");
	AppendString(format, fmt);

	va_list args;
	va_start(args, fmt); 
	static char string[MAX_PRINT_SIZE];
	int size = StringPrintArgs(string, format, args);
	string[ sizeof(string)/sizeof(string[0]) - 1] = 0; 
	HELIUM_ASSERT(size >= 0);

	PrintString(string, Channels::Error, level, Log::GetChannelColor( Channels::Error ), 0);
	va_end(args);
}

Log::Heading::Heading(const char *fmt, ...)
{
	Helium::MutexScopeLock mutex (g_Mutex);

	// do a basic print
	va_list args;
	va_start(args, fmt); 
	static char string[MAX_PRINT_SIZE];
	int size = StringPrintArgs(string, fmt, args);
	string[ sizeof(string)/sizeof(string[0]) - 1] = 0; 
	HELIUM_ASSERT(size >= 0);

	PrintString(string, Channels::Normal, Levels::Default, Log::GetChannelColor( Channels::Normal ));
	va_end(args);      

	// now indent
	Indent();
}

Log::Heading::~Heading()
{
	Helium::MutexScopeLock mutex (g_Mutex);

	// unindent
	UnIndent();
}

std::vector<std::string> g_OutlineState;

Log::Bullet::Bullet(const char *fmt, ...)
: m_Channel( Channels::Normal )
, m_Level( Log::Levels::Default )
, m_Valid( fmt != NULL )
{
	if (m_Valid)
	{
		Helium::MutexScopeLock mutex (g_Mutex);

		va_list args;
		va_start(args, fmt); 
		CreateBullet( fmt, args );
		va_end(args);
	}
}

Log::Bullet::Bullet(Channel channel, Log::Level level, const char *fmt, ...)
: m_Channel( channel )
, m_Level( level )
, m_Valid( fmt != NULL )
{
	if (m_Valid)
	{
		Helium::MutexScopeLock mutex (g_Mutex);

		va_list args;
		va_start(args, fmt); 
		CreateBullet( fmt, args );
		va_end(args);
	}
}

Log::Bullet::~Bullet()
{
	if (m_Valid)
	{
		Helium::MutexScopeLock mutex (g_Mutex);

		// this gates the output to the console for channels and levels that the user did not elect to see on in the console
		bool print = ( ( g_Channels & m_Channel ) == m_Channel ) && ( m_Level <= g_Level );

		if ( print )
		{
			// unindent
			UnIndent();

			if (g_Indent == 1)
			{
				UnIndent(1);
			}
			else
			{
				Indent(1);
			}
		}

		g_OutlineState.pop_back();
	}
}

void Log::Bullet::CreateBullet(const char *fmt, va_list args)
{
	static char delims[] = { 'o', '*', '>', '-' };

	// this gates the output to the console for channels and levels that the user did not elect to see on in the console
	bool print = ( ( g_Channels & m_Channel ) == m_Channel ) && ( m_Level <= g_Level );

	if ( print )
	{
		if (g_Indent == 0)
		{
			Indent(1);
		}
		else
		{
			UnIndent(1);
		}
	}

	// build the format string
	static char format[MAX_PRINT_SIZE];
	if (g_Indent == 1)
	{
		format[0] = delims[ 0 ];
		format[1] = ' ';
		format[2] = '\0';
	}
	else
	{
		format[0] = delims[ (g_Indent-1 + sizeof(delims)) % sizeof(delims) ];
		format[1] = ' ';
		format[2] = '\0';
	}
	AppendString(format, fmt);

	// format the bullet string
	static char string[MAX_PRINT_SIZE];
	int size = StringPrintArgs(string, format, args);
	string[ sizeof(string)/sizeof(string[0]) - 1] = 0; 
	HELIUM_ASSERT(size >= 0);

	// do the print and capture the output
	static char output[MAX_PRINT_SIZE];
	if (g_Indent == 1)
	{
		PrintString( "\n", m_Channel, m_Level, Log::GetChannelColor( m_Channel ), -1, output, sizeof( output ) );
	}
	PrintString( string, m_Channel, m_Level, Log::GetChannelColor( m_Channel ), -1, output, sizeof( output ) );

	// push state
	g_OutlineState.push_back( output );

	if ( print )
	{
		// now indent
		Indent();
	}
}

std::string Log::GetOutlineState()
{
	Helium::MutexScopeLock mutex (g_Mutex);

	std::string state;

	std::vector<std::string>::const_iterator itr = g_OutlineState.begin();
	std::vector<std::string>::const_iterator end = g_OutlineState.end();
	for ( ; itr != end; ++itr )
	{
		if ( itr == g_OutlineState.begin() )
		{
			state = *itr;
		}
		else
		{
			state += *itr;
		}
	}

	return state;
}

Listener::Listener( uint32_t throttle, uint32_t* errorCount, uint32_t* warningCount, std::vector< Statement >* consoleOutput )
: m_Thread( Thread::GetCurrentId() )
, m_Throttle( throttle )
, m_WarningCount( warningCount )
, m_ErrorCount( errorCount )
, m_LogOutput( consoleOutput )
{
	Start();
}

Listener::~Listener()
{
	Stop();
}

void Listener::Start()
{
	Log::AddListener( Log::ListenerSignature::Delegate( this, &Listener::Print ) );
}

void Listener::Stop()
{
	Log::RemoveListener( Log::ListenerSignature::Delegate( this, &Listener::Print ) );
}

void Listener::Dump(bool stop)
{
	if ( stop )
	{
		Stop();
	}

	if ( m_LogOutput )
	{
		Log::PrintStatements( *m_LogOutput );
	}
}

uint32_t Listener::GetWarningCount()
{
	return *m_WarningCount;
}

uint32_t Listener::GetErrorCount()
{
	return *m_ErrorCount;
}

void Listener::Print( ListenerArgs& args )
{
	if ( m_Thread == Thread::GetCurrentId() )
	{
		if ( args.m_Statement.m_Channel == Log::Channels::Warning && m_WarningCount )
		{
			(*m_WarningCount)++;
		}

		if ( args.m_Statement.m_Channel == Log::Channels::Error && m_ErrorCount )
		{
			(*m_ErrorCount)++;
		}

		if ( m_LogOutput )
		{
			m_LogOutput->push_back( args.m_Statement );
		}

		if ( m_Throttle & args.m_Statement.m_Channel )
		{
			args.m_Skip = true;
		}
	}
}
