#pragma once

#include <string>

#include "Platform/Types.h"
#include "Platform/Thread.h"
#include "Platform/Console.h"

#include "Foundation/API.h"
#include "Foundation/SmartPtr.h"
#include "Foundation/Event.h"

namespace Helium
{
	namespace Log
	{
		const static uint32_t MAX_PRINT_SIZE = 8192;

		//
		// Output channels, these speak to the qualitative value of the print:
		//  - normal is any print that updates the status of a process to a normal user
		//  - debug is any print that is only really meaningful to developers, and not indicative of a problem
		//  - warning is notification that there is a minor problem happening, but things will turn out OK
		//  - error is notification that there is a serious problem and something needs attention to work as expected
		//

		namespace Channels
		{
			enum Channel
			{
				Error    = 1 << 0,
				Warning  = 1 << 1,
				Normal   = 1 << 2,
				Debug    = 1 << 3,
				Profile  = 1 << 4,
				Count    = 5, // careful, this needs to be updated based on the number of channels
				All      = 0xffffffff,
			};
		}

		typedef uint32_t Channel;

		//
		// Verbosity levels, these speak to the quantitative value of the print:
		//  - basic is any print that denotes what is happening on a large scale within the program
		//  - advanced is any print that provides additional detail within a large scale section of code within the program, and is optional
		//  - gratuitous is any print that is so dense in occurance that both users and developers will very rarely need to read it
		//

		namespace Levels
		{
			enum Level
			{
				Default  = 1 << 0, // EG. 'Processing texture...' or 'Copying data...'
				Verbose  = 1 << 1, // EG. 'Adding object ratchet_clone to level data...'
				Extreme  = 1 << 2, // EG. 'Calculating binormal for polygon 3554...'
			};
		}
		typedef Levels::Level Level;

		//
		// Printing event API allows for in-process APIs to handle print events themselves
		//

		struct HELIUM_FOUNDATION_API Statement
		{
			std::string m_String;
			Channel m_Channel;
			Level m_Level;
			int m_Indent;

			inline Statement( const std::string& string, Channel channel = Channels::Normal, Level level = Levels::Default, int indent = 0 );

			inline void ApplyIndent();

			void ApplyIndent( const char* string, std::string& output );
		};

		//
		// Log event
		//

		struct HELIUM_FOUNDATION_API ListenerArgs : NonCopyable
		{
			inline ListenerArgs( const Statement& statement );
			
			const Statement&  m_Statement;
			bool              m_Skip;
		};

		typedef Helium::Signature< ListenerArgs&, Helium::AtomicRefCountBase > ListenerSignature;

		HELIUM_FOUNDATION_API void AddListener(const ListenerSignature::Delegate& listener);
		HELIUM_FOUNDATION_API void RemoveListener(const ListenerSignature::Delegate& listener);

		//
		// Tracing API handles echoing all output to the trace text file associated with the process
		//

		// the trace file gets everything Console delivers to the console and more
		HELIUM_FOUNDATION_API bool AddTraceFile( const std::string& fileName, Channel channel, ThreadId threadId = ThreadId (), bool append = false );
		HELIUM_FOUNDATION_API void RemoveTraceFile( const std::string& fileName );

		template <bool (*AddFunc)(const std::string& fileName, Channel channel, ThreadId threadId, bool append), void (*RemoveFunc)(const std::string& fileName)>
		class FileHandle
		{
		public:
			inline FileHandle(const std::string& file, Channel channel, ThreadId threadId = ThreadId (), bool append = false );
			inline ~FileHandle();

			inline const std::string& GetFile();

		private:
			std::string m_File;
		};

		typedef FileHandle<&AddTraceFile, &RemoveTraceFile> TraceFileHandle;

		//
		// Indenting API causes all output to be offset by whitespace
		//

		// indent all output
		HELIUM_FOUNDATION_API void Indent(int col = -1);

		// unindent all output
		HELIUM_FOUNDATION_API void UnIndent(int col = -1);

		//
		// Tracking APIs configure what channels and levels to use, and allows access to warning/error counters
		//

		// verbosity setting
		HELIUM_FOUNDATION_API Level GetLevel();
		HELIUM_FOUNDATION_API void SetLevel(Level level);

		// enable channel calls
		HELIUM_FOUNDATION_API bool IsChannelEnabled( Channel channel );
		HELIUM_FOUNDATION_API void EnableChannel( Channel channel, bool enable );

		// get the print color for the given channel
		HELIUM_FOUNDATION_API ConsoleColor GetChannelColor(Channel channel);

		// enter/leave this library's section
		HELIUM_FOUNDATION_API void LockMutex();
		HELIUM_FOUNDATION_API void UnlockMutex();

		//
		// Printing APIs are the heart of Console
		//

		// main printing function used by all prototypes
		HELIUM_FOUNDATION_API void PrintString(const char* string,	// the string to print
			Channel channel = Channels::Normal,							// the channel its going into
			Level level = Levels::Default,								// the verbosity level
			ConsoleColor color = ConsoleColors::None,					// the color to use (None for auto)
			int indent = -1,											// the amount to indent
			char* output = NULL,										// the buffer to copy the result string to
			uint32_t outputSize = 0);									// the size of the output buffer

		// print a persisted print statment
		HELIUM_FOUNDATION_API void PrintStatement(const Statement& statement);

		// print several statements
		HELIUM_FOUNDATION_API void PrintStatements(const std::vector< Statement >& statements, uint32_t channels = Channels::All);

		// simple way to print a particular color
		HELIUM_FOUNDATION_API void PrintColor(ConsoleColor color, const char* fmt, ...);

		// make a print statement
		HELIUM_FOUNDATION_API void Print(const char *fmt,...);
		HELIUM_FOUNDATION_API void Print(Level level, const char *fmt,...);

		// make a debug-only statement
		HELIUM_FOUNDATION_API void Debug(const char *fmt,...);
		HELIUM_FOUNDATION_API void Debug(Level level, const char *fmt,...);

		// make a profile-only statement
		HELIUM_FOUNDATION_API void Profile(const char *fmt,...);
		HELIUM_FOUNDATION_API void Profile(Level level, const char *fmt,...);

		// warn the user, increments warning count
		HELIUM_FOUNDATION_API void Warning(const char *fmt,...);
		HELIUM_FOUNDATION_API void Warning(Level level, const char *fmt,...);

		// give an error, increments error count
		HELIUM_FOUNDATION_API void Error(const char *fmt,...);
		HELIUM_FOUNDATION_API void Error(Level level, const char *fmt,...);

		// stack-based indention helper object indents all output while on the stack
		class HELIUM_FOUNDATION_API Indentation
		{
		public:
			inline Indentation();
			inline ~Indentation();
		};

		// like an indentation, but prints to the basic output channel the name of the heading
		class HELIUM_FOUNDATION_API Heading
		{
		public:
			Heading(const char *fmt, ...);
			~Heading();
		};

		// like a heading but preceeded with a delimiter (o, *, -, etc...)
		class HELIUM_FOUNDATION_API Bullet
		{
		private:
			Channel m_Channel;
			Level  m_Level;
			bool   m_Valid;

		public:
			Bullet(const char *fmt, ...);
			Bullet(Channel channel, Level level, const char *fmt, ...);

			~Bullet();

		private:
			void CreateBullet(const char *fmt, va_list args );
		};

		// grab the path to the current bullet
		HELIUM_FOUNDATION_API std::string GetOutlineState();

		class HELIUM_FOUNDATION_API Listener
		{
		public:
			Listener( uint32_t throttle = Channels::All, uint32_t* errorCount = NULL, uint32_t* warningCount = NULL, std::vector< Statement >* consoleOutput = NULL );
			~Listener();

			void Start();
			void Stop();
			void Dump(bool stop = true);

			uint32_t GetWarningCount();
			uint32_t GetErrorCount();

		private:
			void Print( ListenerArgs& args );

		private:
			ThreadId                  m_Thread;
			uint32_t                  m_Throttle;
			uint32_t*                 m_ErrorCount;
			uint32_t*                 m_WarningCount;
			std::vector< Statement >* m_LogOutput;
		};
	}
}

#include "Foundation/Log.inl"