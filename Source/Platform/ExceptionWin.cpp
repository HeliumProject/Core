#include "Precompile.h"
#include "Exception.h"

#include "Platform/SystemWin.h"
#undef Yield

#include "Platform/Types.h"
#include "Platform/Locks.h"
#include "Platform/Error.h"
#include "Platform/Assert.h"
#include "Platform/Encoding.h"
#include "Platform/Exception.h"
#include "Platform/Trace.h"
#include "Platform/Console.h"
#include "Platform/Process.h"

#include <map>
#include <time.h>
#include <shlobj.h>
#include <tlhelp32.h>

#define DBGHELP_TRANSLATE_TCHAR
#pragma warning( push )
#pragma warning( disable : 4091 )
#include <dbghelp.h>
#pragma warning( pop )

#pragma comment ( lib, "dbghelp.lib" )

using namespace Helium;

//#define DEBUG_SYMBOLS

// disable the optimizer if debugging in release
#if defined(DEBUG_SYMBOLS) && defined(NDEBUG)
#pragma optimize("", off)
#endif

// loaded flag
static bool g_Initialized = false;

// Utility to print to a string
static void PrintToString(std::string& str, const char* fmt, ...)
{
	va_list args;

	va_start(args, fmt);

	size_t needed = StringPrintArgs(NULL, 0, fmt, args) + 1;
	char* buf = (char*)alloca( sizeof(char) * needed );

	StringPrintArgs(buf, sizeof(buf) * needed, fmt, args);
	buf[ needed - 1] = 0; 

	va_end(args);

	str += buf;
}

// Callback that loads symbol data from loaded dll into DbgHelp system, dumping error info
static BOOL CALLBACK EnumerateLoadedModulesProc(PCTSTR name, DWORD64 base, ULONG size, PVOID data)
{
	if (SymLoadModuleEx(GetCurrentProcess(), 0, name, 0, base, size, NULL, 0))
	{
		HELIUM_CONVERT_TO_CHAR( name, convertedName );

		IMAGEHLP_MODULE64 moduleInfo;
		ZeroMemory(&moduleInfo, sizeof(IMAGEHLP_MODULE64));
		moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
		if ( SymGetModuleInfo64( GetCurrentProcess(), base, &moduleInfo ) )
		{
			if ( moduleInfo.LoadedPdbName[0] != '\0' )
			{
				HELIUM_CONVERT_TO_CHAR( moduleInfo.LoadedPdbName, convertedPdbName ); 
				Print( "Success loading symbols for module: %s, base: 0x%" HELIUM_PRINT_POINTER ", size: %u: %s\n", convertedName, base, size, convertedPdbName );
			}
			else
			{
				Print( "Success loading symbols for module: %s, base: 0x%" HELIUM_PRINT_POINTER ", size: %u\n", convertedName, base, size );
			}
		}
		else
		{
			Print( "Failure loading symbols for module: %s: %s\n", convertedName, Helium::GetErrorString().c_str() );
		}
	}

	return TRUE;
}

// Load debugging information for the currently loaded dlls, call this each time you need to use Sym*() API.  Keep calling it
static void EnumerateLoadedModules()
{
	// load debugging information
	//  this is probably slow, but SYMOPT_DEFERRED_LOADS doesn't always work with 
	//  dynamically loaded dlls so we do this instead of SYMOPT_DEFERRED_LOADS and 
	//  invading the process during SymInitialize
	EnumerateLoadedModules64(GetCurrentProcess(), &EnumerateLoadedModulesProc, NULL);
}

bool Helium::IsDebuggerPresent()
{
	return ::IsDebuggerPresent() != 0;
}

bool Helium::InitializeSymbols(const std::string& path)
{
	if ( !g_Initialized )
	{
		std::wstring dir;

		if ( path.empty() )
		{
			wchar_t module[MAX_PATH];
			wchar_t drive[MAX_PATH];
			wchar_t path[MAX_PATH];
			wchar_t file[MAX_PATH];
			wchar_t ext[MAX_PATH];
			GetModuleFileName(0,module,MAX_PATH);
			_wsplitpath(module,drive,path,file,ext);

			dir = drive;
			dir += path;
		}
		else
		{
			ConvertString(path, dir);
		}

		DWORD options = SYMOPT_FAIL_CRITICAL_ERRORS |
			SYMOPT_LOAD_LINES |
			SYMOPT_UNDNAME;

		SymSetOptions(options);

		// initialize symbols (dbghelp.dll)
		if ( SymInitialize(GetCurrentProcess(), dir.c_str(), FALSE) == 0 )
		{
			Print( "Failure initializing symbol API: %s\n", Helium::GetErrorString().c_str() );
			return false;
		}

		// success
		g_Initialized = true;
	}

	return true;
}

bool Helium::GetSymbolsInitialized()
{
    return g_Initialized;
}

std::string Helium::GetSymbolInfo(uintptr_t adr, bool enumLoadedModules)
{
	HELIUM_ASSERT( g_Initialized );

	if ( enumLoadedModules )
	{
		// load debugging information
		EnumerateLoadedModules();
	}

	// module image name "reflect.dll"
	wchar_t module[MAX_PATH];
	ZeroMemory(&module, sizeof(module));
	wchar_t extension[MAX_PATH];
	ZeroMemory(&extension, sizeof(extension));

	// symbol name "Reflect::Class::AddSerializer + 0x16d"
	char symbol[MAX_SYM_NAME+16];
	ZeroMemory(&symbol, sizeof(symbol));

	// source file name "typeinfo.cpp"
	wchar_t filename[MAX_PATH];
	ZeroMemory(&filename, sizeof(filename));

	// line number in source "246"
	DWORD line = 0xFFFFFFFF;

	// resulting line is worst case of all components
	char result[sizeof(module) + sizeof(symbol) + sizeof(filename) + 64];
	ZeroMemory(&result, sizeof(result));


	//
	// Start by finding the module the address is in
	//

	IMAGEHLP_MODULE64 moduleInfo;
	ZeroMemory(&moduleInfo, sizeof(IMAGEHLP_MODULE64));
	moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
	if (SymGetModuleInfo64(GetCurrentProcess(), adr, &moduleInfo))
	{
		// success, copy the module info
		_wsplitpath(moduleInfo.ImageName, NULL, NULL, module, extension);
		wcscat(module, extension);

		//
		// Now find symbol information
		//

		// displacement of the symbol
		DWORD64 disp;

		// okay, the name runs off the end of the structure, so setup a buffer and cast it
		ULONG64 buffer[(sizeof(SYMBOL_INFO) + MAX_SYM_NAME*sizeof(TCHAR) + sizeof(ULONG64) - 1) / sizeof(ULONG64)];
		PSYMBOL_INFO symbolInfo = (PSYMBOL_INFO)buffer;
		symbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbolInfo->MaxNameLen = MAX_SYM_NAME;

		if ( SymFromAddr(GetCurrentProcess(), adr, &disp, symbolInfo) != 0 )
		{
			HELIUM_WIDE_TO_TCHAR(symbolInfo->Name, convertedSymbolName);
			// success, copy the symbol info
			StringPrint(symbol, "%s + 0x%" HELIUM_PRINT_POINTER, convertedSymbolName, disp);

			//
			// Now find source line information
			//

			DWORD d;
			IMAGEHLP_LINE64 l;
			ZeroMemory(&l,sizeof(l));
			l.SizeOfStruct = sizeof(l);
			if ( SymGetLineFromAddr64(GetCurrentProcess(), adr, &d, &l) !=0 )
			{
				// success, copy the source file name
				wcscpy(filename, l.FileName);
				wchar_t ext[MAX_PATH];
				wchar_t file[MAX_PATH];
				_wsplitpath(filename, NULL, NULL, file, ext);

				HELIUM_WIDE_TO_TCHAR(module, convertedModule);
				HELIUM_WIDE_TO_TCHAR(file, convertedFile);
				HELIUM_WIDE_TO_TCHAR(ext, convertedExt);
				StringPrint(result, "%s, %s : %s%s(%d)", convertedModule, symbol, convertedFile, convertedExt, l.LineNumber);
				return result;
			}

			StringPrint(result, "%s, %s", module, symbol);
			return result;
		}

		StringPrint(result, "%s", module);
		return result;
	}
	else
	{
		DWORD err = GetLastError();
		return "Unknown";
	}
}

Helium::Exception* Helium::GetHeliumException(uintptr_t addr)
{
	Helium::Exception* cppException = (Helium::Exception*)addr;

	__try
	{
		// if its non-null
		if (cppException)
		{
			// this will explode if the address isn't really a c++ exception (std::exception)
			cppException->What();
		}

		// i guess we lived!
		return cppException;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// uh oh, somebody is throwing register types or another root struct or class
		return NULL;
	}
}

std::exception* Helium::GetStandardException(uintptr_t addr)
{
	std::exception* cppException = (std::exception*)addr;

	__try
	{
		// if its non-null
		if (cppException)
		{
			// this will explode if the address isn't really a c++ exception (std::exception)
			(void)cppException->what();
		}

		// i guess we lived!
		return cppException;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// uh oh, somebody is throwing register types or another root struct or class
		return NULL;
	}
}

bool Helium::GetStackTrace(std::vector<uintptr_t>& trace, unsigned omitFrames)
{
	CONTEXT context;
	ZeroMemory( &context, sizeof( context ) );
	RtlCaptureContext( &context );
	return GetStackTrace(&context, trace, omitFrames+1);
}

bool Helium::GetStackTrace(LPCONTEXT context, std::vector<uintptr_t>& stack, unsigned omitFrames)
{
	HELIUM_ASSERT( g_Initialized );

	// load debugging information
	EnumerateLoadedModules();

	if (omitFrames == 0)
	{
		// our our current location as the top of the stack
		stack.push_back( context->IPREG );
	}
	else
	{
		omitFrames--;
	}

	// this is for handling stack overflows
	std::map<int64_t, uint32_t> visited;

	// setup the stack frame to use for traversal
	STACKFRAME64 frame;
	memset(&frame,0,sizeof(frame));
	frame.AddrPC.Offset = context->IPREG;
	frame.AddrPC.Segment = 0;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrStack.Offset = context->SPREG;
	frame.AddrStack.Segment = 0;
	frame.AddrStack.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = context->BPREG;
	frame.AddrFrame.Segment = 0;
	frame.AddrFrame.Mode = AddrModeFlat;

	// make a copy here because StackWalk64 can modify the one you give it
	CONTEXT context_copy;
	memcpy(&context_copy, context, sizeof(context_copy));

	while(1)
	{
		if (!StackWalk64(IMAGE_FILE_ARCH,
			GetCurrentProcess(),
			GetCurrentThread(),
			&frame,
			&context_copy,
			NULL,
			SymFunctionTableAccess64,
			SymGetModuleBase64,
			NULL))
		{
			break;
		}

		if (frame.AddrReturn.Offset == 0x0 || frame.AddrReturn.Offset == 0xffffffffcccccccc)
		{
			break;
		}

		if (visited[ frame.AddrReturn.Offset ]++ >= 8192)
		{
			break;
		}

#ifdef DEBUG_SYMBOLS
		Print( "0x%" HELIUM_PRINT_POINTER " - %s\n", frame.AddrReturn.Offset, GetSymbolInfo(frame.AddrReturn.Offset, false).c_str() );
#endif

		if (omitFrames == 0)
		{
			stack.push_back( (uintptr_t)frame.AddrReturn.Offset );
		}
		else
		{
			omitFrames--;
		}
	}

	return !stack.empty();
}

void Helium::TranslateStackTrace(const std::vector<uintptr_t>& trace, std::string& buffer)
{
	std::vector<uintptr_t>::const_iterator itr = trace.begin();
	std::vector<uintptr_t>::const_iterator end = trace.end();
	for ( ; itr != end; ++itr )
	{
		PrintToString(buffer, "0x%" HELIUM_PRINT_POINTER " - %s\n", *itr, GetSymbolInfo(*itr, false).c_str() );
	}
}

const char* Helium::GetExceptionClass(uint32_t exceptionCode)
{
	const char* ex_name = NULL;

	switch (exceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
		ex_name = "EXCEPTION_ACCESS_VIOLATION";
		break;

	case EXCEPTION_BREAKPOINT:
		ex_name = "EXCEPTION_BREAKPOINT";
		break;

	case EXCEPTION_SINGLE_STEP:
		ex_name = "EXCEPTION_SINGLE_STEP";
		break;

	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		ex_name = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
		break;

	case EXCEPTION_FLT_DENORMAL_OPERAND:
		ex_name = "EXCEPTION_FLT_DENORMAL_OPERAND";
		break;

	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		ex_name = "EXCEPTION_FLT_DIVIDE_BY_ZERO";
		break;

	case EXCEPTION_FLT_INEXACT_RESULT:
		ex_name = "EXCEPTION_FLT_INEXACT_RESULT";
		break;

	case EXCEPTION_FLT_INVALID_OPERATION:
		ex_name = "EXCEPTION_FLT_INVALID_OPERATION";
		break;

	case EXCEPTION_FLT_OVERFLOW:
		ex_name = "EXCEPTION_FLT_OVERFLOW";
		break;

	case EXCEPTION_FLT_STACK_CHECK:
		ex_name = "EXCEPTION_FLT_STACK_CHECK";
		break;

	case EXCEPTION_FLT_UNDERFLOW:
		ex_name = "EXCEPTION_FLT_UNDERFLOW";
		break;

	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		ex_name = "EXCEPTION_INT_DIVIDE_BY_ZERO";
		break;

	case EXCEPTION_INT_OVERFLOW:
		ex_name = "EXCEPTION_INT_OVERFLOW";
		break;

	case EXCEPTION_PRIV_INSTRUCTION:
		ex_name = "EXCEPTION_PRIV_INSTRUCTION";
		break;

	case EXCEPTION_IN_PAGE_ERROR:
		ex_name = "EXCEPTION_IN_PAGE_ERROR";
		break;

	case EXCEPTION_ILLEGAL_INSTRUCTION:
		ex_name = "EXCEPTION_ILLEGAL_INSTRUCTION";
		break;

	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		ex_name = "EXCEPTION_NONCONTINUABLE_EXCEPTION";
		break;

	case EXCEPTION_STACK_OVERFLOW:
		ex_name = "EXCEPTION_STACK_OVERFLOW";
		break;

	case EXCEPTION_INVALID_DISPOSITION:
		ex_name = "EXCEPTION_INVALID_DISPOSITION";
		break;

	case EXCEPTION_GUARD_PAGE:
		ex_name = "EXCEPTION_GUARD_PAGE";
		break;

	case EXCEPTION_INVALID_HANDLE:
		ex_name = "EXCEPTION_INVALID_HANDLE";
		break;

	case 0xC00002B5:
		ex_name = "Multiple floating point traps";
		break;
	}

	if (ex_name == NULL)
	{
		ex_name = "Unknown Exception";
	}

	return ex_name;
}

void Helium::GetExceptionDetails( LPEXCEPTION_POINTERS info, ExceptionArgs& args )
{
	static Helium::Mutex s_ExceptionMutex;
	Helium::MutexScopeLock mutex ( s_ExceptionMutex );

	typedef std::vector< std::pair<DWORD, HANDLE> > V_ThreadHandles;
	V_ThreadHandles threads;

	// the first thing we do is check out other threads, since they are still running!
	HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if ( snapshot != INVALID_HANDLE_VALUE )
	{
		THREADENTRY32 thread;
		memset(&thread, 0, sizeof( thread ));
		thread.dwSize = sizeof( thread );
		if ( Thread32First(snapshot, &thread) )
		{
			do
			{
				if ( thread.th32OwnerProcessID == GetCurrentProcessId() && thread.th32ThreadID != GetCurrentThreadId() )
				{
					HANDLE handle = ::OpenThread( THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, thread.th32ThreadID );
					if ( handle )
					{
						::SuspendThread( handle );
						threads.push_back( std::make_pair( thread.th32ThreadID, handle ) );
					}
				}
			}
			while ( Thread32Next( snapshot, &thread ) );
		}

		::CloseHandle( snapshot );
	}

	V_ThreadHandles::const_iterator itr = threads.begin();
	V_ThreadHandles::const_iterator end = threads.end();
	for ( ; itr != end; ++itr )
	{
		DWORD id = itr->first;
		HANDLE handle = itr->second;

		args.m_Threads.push_back( "" );
		PrintToString( args.m_Threads.back(), "Thread %d:\n", id );
		std::string::size_type size = args.m_Threads.back().size();

		CONTEXT context;
		memset(&context, 0, sizeof( context ));
		context.ContextFlags = CONTEXT_FULL;
		if ( ::GetThreadContext( handle, &context ) )
		{
			PrintToString( args.m_Threads.back(), 
				"\nControl Registers:\n"
				"EIP = 0x%" HELIUM_PRINT_POINTER "  ESP = 0x%" HELIUM_PRINT_POINTER "\n"
				"EBP = 0x%" HELIUM_PRINT_POINTER "  EFL = 0x%" HELIUM_PRINT_POINTER "\n",
				info->ContextRecord->IPREG,
				info->ContextRecord->SPREG,
				info->ContextRecord->BPREG,
				info->ContextRecord->EFlags );

			PrintToString( args.m_Threads.back(), 
				"\nInteger Registers:\n"
				"EAX = 0x%" HELIUM_PRINT_POINTER "  EBX = 0x%" HELIUM_PRINT_POINTER "\n"
				"ECX = 0x%" HELIUM_PRINT_POINTER "  EDX = 0x%" HELIUM_PRINT_POINTER "\n"
				"ESI = 0x%" HELIUM_PRINT_POINTER "  EDI = 0x%" HELIUM_PRINT_POINTER "\n",
				info->ContextRecord->AXREG,
				info->ContextRecord->BXREG,
				info->ContextRecord->CXREG,
				info->ContextRecord->DXREG,
				info->ContextRecord->SIREG,
				info->ContextRecord->DIREG );

			PrintToString( args.m_Threads.back(), "\nCallstack:\n" );

			std::vector<uintptr_t> trace;
			if ( GetStackTrace( &context, trace ) )
			{
				TranslateStackTrace( trace, args.m_Threads.back() );
			}
		}

		if ( args.m_Threads.back().size() == size )
		{
			args.m_Threads.back() += "No thread info\n";
		}

		::ResumeThread( handle );
		::CloseHandle( handle );
	}

	threads.clear();

	args.m_SEHCode = info->ExceptionRecord->ExceptionCode;
	args.m_SEHClass = GetExceptionClass( info->ExceptionRecord->ExceptionCode );

	if (info->ExceptionRecord->ExceptionCode == 0xE06D7363) // Microsoft C++ Exception code
	{
		args.m_Type = ExceptionTypes::CPP;

		Helium::Exception* ex = GetHeliumException(info->ExceptionRecord->ExceptionInformation[1]);
		if (ex)
		{
			const char* cppClass = "Unknown";
#ifdef _CPPRTTI
			try
			{
				cppClass = typeid(*ex).name();
			}
			catch (...)
			{
			}
#else
			cppClass = "Unknown, no RTTI";
#endif

			Helium::ConvertString( cppClass, args.m_CPPClass );
			Helium::ConvertString( ex->What(), args.m_Message );
		}
		else
		{
			std::exception* ex = GetStandardException(info->ExceptionRecord->ExceptionInformation[1]);
			if ( ex )
			{
				const char* cppClass = "Unknown";
#ifdef _CPPRTTI
				try
				{
					cppClass = typeid(*ex).name();
				}
				catch (...)
				{
					cppClass = "Unknown";
				}
#else
				cppClass = "Unknown, no RTTI";
#endif

				Helium::ConvertString( cppClass, args.m_CPPClass );
				Helium::ConvertString( ex->what(), args.m_Message );
			}
		}

		if ( args.m_CPPClass.empty() && args.m_Message.empty() )
		{
			args.m_Message = "Thrown object is not a known type of C++ exception";
		}

		info->ContextRecord->IPREG = (DWORD)info->ExceptionRecord->ExceptionInformation[2];
	}
	else
	{
		args.m_Type = ExceptionTypes::Structured;

		if (args.m_SEHCode == EXCEPTION_ACCESS_VIOLATION)
		{
			PrintToString( args.m_Message, "Attempt to %s address 0x%" HELIUM_PRINT_POINTER "", (info->ExceptionRecord->ExceptionInformation[0]==1) ? "write to" : "read from", info->ExceptionRecord->ExceptionInformation[1] );
		}

		if (info->ContextRecord->ContextFlags & CONTEXT_CONTROL)
		{
			PrintToString( args.m_SEHControlRegisters, 
				"Control Registers:\n"
				"EIP = 0x%" HELIUM_PRINT_POINTER "  ESP = 0x%" HELIUM_PRINT_POINTER "\n"
				"EBP = 0x%" HELIUM_PRINT_POINTER "  EFL = 0x%" HELIUM_PRINT_POINTER "\n",
				info->ContextRecord->IPREG,
				info->ContextRecord->SPREG,
				info->ContextRecord->BPREG,
				info->ContextRecord->EFlags );
		}

		if ( info->ContextRecord->ContextFlags & CONTEXT_INTEGER )
		{
			PrintToString( args.m_SEHIntegerRegisters, 
				"Integer Registers:\n"
				"EAX = 0x%" HELIUM_PRINT_POINTER "  EBX = 0x%" HELIUM_PRINT_POINTER "\n"
				"ECX = 0x%" HELIUM_PRINT_POINTER "  EDX = 0x%" HELIUM_PRINT_POINTER "\n"
				"ESI = 0x%" HELIUM_PRINT_POINTER "  EDI = 0x%" HELIUM_PRINT_POINTER "\n",
				info->ContextRecord->AXREG,
				info->ContextRecord->BXREG,
				info->ContextRecord->CXREG,
				info->ContextRecord->DXREG,
				info->ContextRecord->SIREG,
				info->ContextRecord->DIREG );
		}
	}

	std::vector<uintptr_t> trace;
	if ( GetStackTrace( info->ContextRecord, trace ) )
	{
		TranslateStackTrace( trace, args.m_Callstack );
	}
}

std::string Helium::GetExceptionInfo(LPEXCEPTION_POINTERS info)
{
	ExceptionArgs args ( ExceptionTypes::Structured, false );
	GetExceptionDetails( info, args );

	std::string buffer;
	buffer += "An exception has occurred\n";

	switch ( args.m_Type )
	{
	case ExceptionTypes::CPP:
		{
			PrintToString( buffer, "Type:    C++ Exception\n" );
			PrintToString( buffer, "Class:   %s\n", args.m_CPPClass.c_str() );

			if ( !args.m_Message.empty() )
			{
				PrintToString( buffer, "Message:\n%s\n", args.m_Message.c_str() );
			}

			break;
		}

	case ExceptionTypes::Structured:
		{
			PrintToString( buffer, "Type:    SEH Exception\n" );
			PrintToString( buffer, "Code:    0x%" HELIUM_PRINT_POINTER "\n", args.m_SEHCode );
			PrintToString( buffer, "Class:   %s\n", args.m_SEHClass.c_str() );

			if ( !args.m_Message.empty() )
			{
				PrintToString( buffer, "Message:\n%s\n", args.m_Message.c_str() );
			}

			if ( !args.m_SEHControlRegisters.empty() )
			{
				PrintToString( buffer, "\n%s", args.m_SEHControlRegisters.c_str() );
			}

			if ( !args.m_SEHIntegerRegisters.empty() )
			{
				PrintToString( buffer, "\n%s", args.m_SEHIntegerRegisters.c_str() );
			}

			break;
		}
	}

	buffer += "\nCallstack:\n";

	if ( !args.m_Callstack.empty() )
	{
		PrintToString( buffer, "%s", args.m_Callstack.c_str() );
	}
	else
	{
		buffer += "No call stack info\n";
	}

	std::vector< std::string >::const_iterator itr = args.m_Threads.begin();
	std::vector< std::string >::const_iterator end = args.m_Threads.end();
	for ( ; itr != end; ++itr )
	{
		PrintToString( buffer, "\n%s", itr->c_str() );
	}

	return buffer;
}

#if !HELIUM_RELEASE && !HELIUM_PROFILE

static Mutex& GetStackWalkMutex()
{
	static Mutex stackWalkMutex;

	return stackWalkMutex;
}

static void ConditionalSymInitialize()
{
	static volatile bool bSymInitialized = false;
	if( !bSymInitialized )
	{
		HELIUM_TRACE( TraceLevels::Info, "Initializing symbol handler for the current process...\n" );

		HANDLE hProcess = GetCurrentProcess();
		HELIUM_ASSERT( hProcess );

		BOOL bInitialized = SymInitialize( hProcess, NULL, TRUE );
		if( bInitialized )
		{
			HELIUM_TRACE( TraceLevels::Info, "Symbol handler initialization successful!\n" );
		}
		else
		{
			HELIUM_TRACE(
				TraceLevels::Info,
				"Symbol handler initialization failed (error code %u).\n",
				::GetLastError() );
		}

		bSymInitialized = true;
	}
}

/// Get the current stack trace.
///
/// @param[out] ppStackTraceArray    Array in which to store the backtrace of program counter addresses.
/// @param[in]  stackTraceArraySize  Maximum number of addresses to fill in the output array.
/// @param[in]  skipCount            Number of stack levels to skip before filling the output array, counting the stack
///                                  level for this function call.  By default, this is one, meaning that only the call
///                                  to this function is skipped.
///
/// @return  Number of addresses stored in the output array.
size_t Helium::GetStackTrace( void** ppStackTraceArray, size_t stackTraceArraySize, size_t skipCount )
{
	HELIUM_ASSERT( ppStackTraceArray || stackTraceArraySize == 0 );

	MutexScopeLock scopeLock( GetStackWalkMutex() );
	ConditionalSymInitialize();

	// Get the current context.
	CONTEXT context;
	RtlCaptureContext( &context );

	// Initialize the stack frame structure for the first call to StackWalk64().
	STACKFRAME64 stackFrame;
	MemoryZero( &stackFrame, sizeof( stackFrame ) );
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Mode = AddrModeFlat;

#if HELIUM_OS_WIN32
	const DWORD machineType = IMAGE_FILE_MACHINE_I386;
	stackFrame.AddrPC.Offset = context.Eip;
	stackFrame.AddrFrame.Offset = context.Ebp;
	stackFrame.AddrStack.Offset = context.Esp;
#else
	// Assuming x86-64 (likely not supporting Itanium).
	const DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
	stackFrame.AddrPC.Offset = context.Rip;
	stackFrame.AddrFrame.Offset = context.Rdi/*Rbp*/;
	stackFrame.AddrStack.Offset = context.Rsp;
#endif

	HANDLE hProcess = GetCurrentProcess();
	HELIUM_ASSERT( hProcess );

	HANDLE hThread = GetCurrentThread();
	HELIUM_ASSERT( hThread );

	// Skip addresses first.
	for( size_t skipIndex = 0; skipIndex < skipCount; ++skipIndex )
	{
		BOOL bResult = StackWalk64(
			machineType,
			hProcess,
			hThread,
			&stackFrame,
			&context,
			NULL,
			SymFunctionTableAccess64,
			SymGetModuleBase64,
			NULL );
		if( !bResult || stackFrame.AddrPC.Offset == 0 )
		{
			return 0;
		}
	}

	// Fill out the remaining stack frame addresses up to the output array limit.
	for( size_t traceIndex = 0; traceIndex < stackTraceArraySize; ++traceIndex )
	{
		BOOL bResult = StackWalk64(
			machineType,
			hProcess,
			hThread,
			&stackFrame,
			&context,
			NULL,
			SymFunctionTableAccess64,
			SymGetModuleBase64,
			NULL );
		if( !bResult || stackFrame.AddrPC.Offset == 0 )
		{
			return traceIndex;
		}

		ppStackTraceArray[ traceIndex ] =
			reinterpret_cast< void* >( static_cast< uintptr_t >( stackFrame.AddrPC.Offset ) );
	}

	return stackTraceArraySize;
}

/// Get the symbol for the specified address.
///
/// @param[out] rSymbol   Address symbol.
/// @param[in]  pAddress  Address to translate.
///
/// @return  True if the address was successfully resolved, false if not.
void Helium::GetAddressSymbol( std::string& rSymbol, void* pAddress )
{
	HELIUM_ASSERT( pAddress );

	MutexScopeLock scopeLock( GetStackWalkMutex() );
	ConditionalSymInitialize();

//    rSymbol.Remove( 0, rSymbol.GetSize() );
	rSymbol.clear();

	HANDLE hProcess = GetCurrentProcess();
	HELIUM_ASSERT( hProcess );

	bool bAddedModuleName = false;

	DWORD64 moduleBase = SymGetModuleBase64( hProcess, reinterpret_cast< uintptr_t >( pAddress ) );
	if( moduleBase )
	{
		IMAGEHLP_MODULE64 moduleInfo;
		MemoryZero( &moduleInfo, sizeof( moduleInfo ) );
		moduleInfo.SizeOfStruct = sizeof( moduleInfo );
		if( SymGetModuleInfo64( hProcess, moduleBase, &moduleInfo ) )
		{
			rSymbol += "(";
			HELIUM_WIDE_TO_TCHAR( moduleInfo.ModuleName, convertedModuleName );
			rSymbol += convertedModuleName;
			rSymbol += ") ";

			bAddedModuleName = true;
		}
	}

	if( !bAddedModuleName )
	{
		rSymbol += "(???) ";
	}

	uint64_t symbolInfoBuffer[
		( sizeof( SYMBOL_INFO ) + sizeof( char ) * ( MAX_SYM_NAME - 1 ) + sizeof( uint64_t ) - 1 ) /
		sizeof( uint64_t ) ];
	MemoryZero( symbolInfoBuffer, sizeof( symbolInfoBuffer ) );

	SYMBOL_INFO& rSymbolInfo = *reinterpret_cast< SYMBOL_INFO* >( &symbolInfoBuffer[ 0 ] );
	rSymbolInfo.SizeOfStruct = sizeof( SYMBOL_INFO );
	rSymbolInfo.MaxNameLen = MAX_SYM_NAME;
	if( SymFromAddr( hProcess, reinterpret_cast< uintptr_t >( pAddress ), NULL, &rSymbolInfo ) )
	{
		rSymbolInfo.Name[ MAX_SYM_NAME - 1 ] = '\0';
		HELIUM_WIDE_TO_TCHAR( rSymbolInfo.Name, convertedName );
		rSymbol += convertedName;
		rSymbol += " ";
	}
	else
	{
		rSymbol += "??? ";
	}

	DWORD displacement = 0;
	IMAGEHLP_LINE64 lineInfo;
	MemoryZero( &lineInfo, sizeof( lineInfo ) );
	lineInfo.SizeOfStruct = sizeof( lineInfo );
	if( SymGetLineFromAddr64( hProcess, reinterpret_cast< uintptr_t >( pAddress ), &displacement, &lineInfo ) )
	{
		char lineNumberBuffer[ 32 ];
		StringPrint( lineNumberBuffer, HELIUM_ARRAY_COUNT( lineNumberBuffer ), "%u", lineInfo.LineNumber );
		lineNumberBuffer[ HELIUM_ARRAY_COUNT( lineNumberBuffer ) - 1 ] = '\0';

		rSymbol += "(";
		HELIUM_WIDE_TO_TCHAR( lineInfo.FileName, convertedFileName );
		rSymbol += convertedFileName;
		rSymbol += ", line ";
		rSymbol += lineNumberBuffer;
		rSymbol += ")";
	}
	else
	{
		rSymbol += "(???, line ?)";
	}
}

/// Write a string to any platform-specific debug log output.
///
/// @param[in] pMessage  Message string to write to the debug log.
void Helium::DebugLog( const char* pMessage )
{
	HELIUM_ASSERT( pMessage );
	HELIUM_TCHAR_TO_WIDE( pMessage, convertedMessage );
	OutputDebugStringW( convertedMessage );
}

#endif  // !HELIUM_RELEASE && !HELIUM_PROFILE
