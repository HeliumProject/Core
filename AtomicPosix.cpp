#include "Platform/Atomic.h"
#include "Platform/Assert.h"

int32_t Helium::AtomicIncrement( volatile int32_t* value )
{
    HELIUM_ASSERT( false );
}

void Helium::AtomicDecrement( volatile int32_t* value )
{
    HELIUM_ASSERT( false );
}

void Helium::AtomicExchange( volatile int32_t* addr, int32_t value )
{
    HELIUM_ASSERT( false );
}

#ifdef X64

void Helium::AtomicIncrement( volatile int64_t* value )
{
    HELIUM_ASSERT( false );
}

void Helium::AtomicDecrement( volatile int64_t* value )
{
    HELIUM_ASSERT( false );
}

void Helium::AtomicExchange( volatile int64_t* addr, int64_t value )
{
    HELIUM_ASSERT( false );
}

#endif
