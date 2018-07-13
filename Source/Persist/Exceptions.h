#pragma once

#include "Platform/Exception.h"

namespace Helium
{
	namespace Persist
	{
		//
		// Basic exception
		//

		class Exception : public Helium::Exception
		{
		public:
			Exception( const char *msgFormat, ... )
			{
				va_list msgArgs;
				va_start( msgArgs, msgFormat );
				SetMessage( msgFormat, msgArgs );
				va_end( msgArgs );
			}

		protected:
			Exception() throw() {} // hide default c_tor
		};

		//
		// There is a problem dealing with a opening/closing reading/writing to a file or stream
		//

		class StreamException : public Persist::Exception
		{
		public:
			StreamException( const char* msgFormat, ... )
			{
				va_list msgArgs;
				va_start( msgArgs, msgFormat );
				SetMessage( msgFormat, msgArgs );
				va_end( msgArgs );
			}

		protected:
			StreamException() throw() {} // hide default c_tor
		};
	}
}