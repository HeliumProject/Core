#include "Precompile.h"
#include "RPC.h"

#include "Platform/Assert.h"

#include "Foundation/IPC.h"

#include <string.h>

using namespace Helium;
using namespace Helium::RPC;

// Debug printing
//#define RPC_DEBUG
//#define RPC_DEBUG_MSG

Interface::Interface(const char* name)
	: m_Name (name)
	, m_Host (NULL)
	, m_InvokerCount (0)
{

}

void Interface::AddInvoker(InvokerPtr invoker)
{
	if (m_InvokerCount >= MAX_INVOKERS)
	{
		HELIUM_BREAK();
		return;
	}

	m_Invokers[m_InvokerCount++] = invoker;
}

Invoker* Interface::GetInvoker(const char* name)
{
	for ( uint32_t i=0; i<m_InvokerCount; i++ )
	{
		if ( !strcmp( m_Invokers[i]->GetName(), name ) )
		{
			return m_Invokers[i];
		}
	}

	return NULL;
}

Host::Stack::Stack()
{
	Reset();
}

void Host::Stack::Reset()
{
	memset(m_Frames, 0, sizeof(Host::Frame) * MAX_STACK);
	m_Size = 0;
}

int32_t Host::Stack::Size()
{
	return m_Size;
}

Host::Frame* Host::Stack::Push()
{
	HELIUM_ASSERT(m_Size < MAX_STACK);

	m_Size++;

	return Top();
}

Host::Frame* Host::Stack::Top()
{
	HELIUM_ASSERT(m_Size > 0);

	return &m_Frames[m_Size-1];
}

void Host::Stack::Pop()
{
	memset(Top(), 0, sizeof(Host::Frame));

	m_Size--;
}

Host::Host()
{
	Reset();
}

Host::~Host()
{

}

void Host::Reset()
{
	m_Connection = NULL;
	m_ConnectionCount = 0;

	memset(m_Interfaces, 0, sizeof(Interface*) * MAX_INTERFACES);
}

void Host::AddInterface(Interface* iface)
{
	if (m_InterfaceCount >= MAX_INTERFACES)
	{
		HELIUM_BREAK();
		return;
	}

	m_Interfaces[m_InterfaceCount++] = iface;

	iface->SetHost( this );
}

Interface* Host::GetInterface(const char* name)
{
	for ( uint32_t i=0; i<m_InterfaceCount; i++ )
	{
		if ( !strcmp( m_Interfaces[i]->GetName(), name ) )
		{
			return m_Interfaces[i];
		}
	}

	return NULL;
}

void Host::SetConnection(IPC::Connection* con)
{
	m_Connection = con;
	m_ConnectionCount = m_Connection->GetConnectCount();
}

IPC::Message* Host::Create(Invoker* invoker, uint32_t size, int32_t transaction)
{
	// TODO: Pack the invoker and interface name into the message data
	invoker->GetName();
	invoker->GetInterface()->GetName();

	if (transaction != 0)
	{
		return m_Connection->CreateMessage(0, size, transaction);
	}
	else
	{
		return m_Connection->CreateMessage(0, size);
	}
}

void Host::Emit(Invoker* invoker, Args* args, uint32_t size, SwizzleFunc swizzler)
{
	if (Connected())
	{
		if (m_ConnectionCount != m_Connection->GetConnectCount())
		{
#ifdef RPC_DEBUG
			printf("RPC::Connection cycled, resetting stack\n");
#endif
			m_ConnectionCount = m_Connection->GetConnectCount();
			m_Stack.Reset();
		}

		uint32_t size = 0;

		if (args != NULL)
		{
			HELIUM_ASSERT(size > 0);
			size += size;
		}

		if (args->m_Payload != NULL)
		{
			HELIUM_ASSERT(args->m_PayloadSize > 0);
			size += args->m_PayloadSize;
		}

		IPC::Message* message = Create(invoker, size);

		uint8_t* ptr = message->GetData();

		if (args != NULL)
		{
			swizzler(args);

			memcpy(ptr, args, size);
			ptr += size;

			swizzler(args);
		}

		if (args->m_Payload != NULL)
		{
			memcpy(ptr, args->m_Payload, args->m_PayloadSize);
			ptr += args->m_PayloadSize;
		}

		HELIUM_ASSERT((uint32_t)(ptr - message->GetData()) == size);

#ifdef RPC_DEBUG_MSG
		uint32_t msg_id = message->GetID();
		uint32_t msg_size = message->GetSize();
#endif

		int32_t msg_transaction = message->GetTransaction();
		if (m_Connection->Send(message)!=IPC::ConnectionStates::Active)
		{
			delete message;
			return;
		}

#ifdef RPC_DEBUG_MSG
		printf("RPC::Put message id 0x%" HELIUM_PRINT_POINTER ", size %d, transaction %d\n", msg_id, msg_size, msg_transaction);
#endif

		message = NULL; // assume its GONE

		if (args->m_Flags & RPC::Flags::NonBlocking)
		{
#ifdef RPC_DEBUG
			printf("RPC::Emitting async transaction %d\n", msg_transaction);
#endif
		}
		else
		{
			// create frame for call
			Frame* frame = m_Stack.Push();

			// set the transaction we are blocking on
			frame->m_ReplyTransaction = msg_transaction;

#ifdef RPC_DEBUG
			printf("RPC::Emitting transaction %d, stack size %d\n", msg_transaction, m_Stack.Size());
#endif

			// process until we retrieve it
			frame->m_Replied = false;

			// process messages until we receive our reply
			while (Process(true) && !frame->m_Replied);

			// if we did not get our reply
			if (!frame->m_Replied)
			{
#ifdef RPC_DEBUG
				printf("RPC::Emit failed for transaction %d, stack size %d\n", msg_transaction, m_Stack.Size());
#endif

				// if we didn't reset and the call timed out, pop
				if (m_Stack.Size())
				{
					m_Stack.Pop();
				}
			}
			else
			{
#ifdef RPC_DEBUG
				printf("RPC::Emit success for transaction %d, stack size %d\n", msg_transaction, m_Stack.Size());
#endif

				size = 0;
				ptr = frame->m_ReplyData;

				if (args != NULL)
				{
					HELIUM_ASSERT(size > 0);
					size += size;
				}

				if (args->m_Payload != NULL)
				{
					HELIUM_ASSERT(args->m_PayloadSize > 0);
					size += args->m_PayloadSize;
				}

				// do ref args processing here
				if (args->m_Flags & RPC::Flags::ReplyWithArgs && args != NULL)
				{
					swizzler(ptr);

					// copy our data BACK
					memcpy(args, ptr, size);

					ptr += size;
				}

				// do ref payload processing here
				if (args->m_Flags & RPC::Flags::ReplyWithPayload && args->m_Payload != NULL)
				{
					// copy our data BACK
					memcpy(args->m_Payload, ptr, args->m_PayloadSize);
					ptr += args->m_PayloadSize;
				}

				// clean up our reply message's memory
				delete[] frame->m_ReplyData;

				// call complete, pop
				m_Stack.Pop();
			}
		}
	}
}

bool Host::Invoke(IPC::Message* msg)
{
	// TODO: Unpack the invoker and interface name from the message data
	const char* invokerName = NULL;
	const char* interfaceName = NULL;

	// find the interface
	Interface* iface = GetInterface(interfaceName);
	if (iface == NULL)
	{
		printf("RPC::Unable to find interface '%s'\n", interfaceName);
		delete msg;
		return true;
	}

	// find the invoker
	Invoker* invoker = iface->GetInvoker(invokerName);
	if (invoker == NULL)
	{
		printf("RPC::Unable to find invoker '%s' in interface '%s'\n", invokerName, interfaceName);
		delete msg;
		return true;
	}

	// get our frame from the top of the stack
	Frame* frame = m_Stack.Top();

	HELIUM_ASSERT(frame->m_Message == NULL);
	frame->m_Message = msg;

	// call the function
	frame->m_MessageTaken = false;
	invoker->Invoke(msg->GetData(), msg->GetSize());

	HELIUM_ASSERT(frame->m_Message != NULL);
	frame->m_Message = NULL;

	Args* args = (Args*)msg->GetData();

	if (args->m_Flags & RPC::Flags::NonBlocking)
	{
		if (!frame->m_MessageTaken)
		{
			delete msg;
		}

		return true; // async call, we are done
	}

	// our reply
	IPC::Message* reply = NULL;

	// if we have data, and a reference args or payload
	if (msg->GetSize() > 0 && args->m_Flags & (RPC::Flags::ReplyWithArgs | RPC::Flags::ReplyWithPayload))
	{
		// total size of reply
		uint32_t size = 0;

		// size of args section
		uint32_t argSize = invoker->GetArgsSize();

		// size of payload section
		uint32_t payload_size = msg->GetSize() - argSize;

		// if we have a ref args
		if (args->m_Flags & RPC::Flags::ReplyWithArgs)
		{
			// alloc for args block
			size += argSize;
		}

		// if we have a ref payload
		if (args->m_Flags & RPC::Flags::ReplyWithPayload)
		{
			// alloc for payload block
			size += payload_size;
		}

		// create message
		reply = Create(invoker, size, msg->GetTransaction());

		// where to write
		uint8_t* ptr = reply->GetData();

		// if we have a ref args
		if (args->m_Flags & RPC::Flags::ReplyWithArgs)
		{
			invoker->Swizzle( msg->GetData() );

			// write to ptr
			memcpy(ptr, msg->GetData(), argSize);  

			// incr ptr by amount written
			ptr += argSize;
		}

		// if we have a ref payload
		if (args->m_Flags & RPC::Flags::ReplyWithPayload)
		{
			// write to ptr
			memcpy(ptr, msg->GetData() + argSize, payload_size);

			// incr ptr by amount written
			ptr += payload_size;
		}

		// assert we did not overrun message size
		HELIUM_ASSERT((uint32_t)(ptr - reply->GetData()) == size);
	}
	else // no data, or no ref args or payload
	{
		// just create an empty reply, the other side is blocking
		reply = Create(invoker, 0, msg->GetTransaction());
	}

	if (m_Connection->Send(reply)!= IPC::ConnectionStates::Active)
	{
		delete reply;
	}

#ifdef RPC_DEBUG_MSG
	printf("RPC::Put message id 0x%" HELIUM_PRINT_POINTER ", size %d, transaction %d\n", reply->GetID(), reply->GetSize(), reply->GetTransaction());
#endif

	if (!frame->m_MessageTaken)
	{
		delete msg;
	}

	return true;
}

uint8_t* Host::TakeData()
{
	Frame* frame = m_Stack.Top();

	frame->m_MessageTaken = true;

	return frame->m_Message->GetData();
}

bool Host::Connected()
{
	if (m_Connection == NULL)
	{
		return false; // no connection to use
	}

	if (m_Connection->GetState()!=IPC::ConnectionStates::Active)
	{
		return false; // no connection to use
	}

	return true;
}

bool Host::Dispatch()
{
	return Process(false);
}

bool Host::Process(bool wait)
{
	bool result = true;

	if (Connected())
	{
		if (m_ConnectionCount != m_Connection->GetConnectCount())
		{
#ifdef RPC_DEBUG
			printf("RPC::Connection cycled, resetting stack\n");
#endif
			m_ConnectionCount = m_Connection->GetConnectCount();
			m_Stack.Reset();
		}

		if (m_Connection->GetState() != IPC::ConnectionStates::Active)
		{
			result = false;
		}

		while (result)
		{
			IPC::Message* msg = NULL;
			IPC::ConnectionState state = m_Connection->Receive(&msg, wait);

			if (state != IPC::ConnectionStates::Active || msg == NULL)
			{
				result = false;
				break;
			}

#ifdef RPC_DEBUG_MSG
			printf("RPC::Got message id 0x%" HELIUM_PRINT_POINTER ", size %d, transaction %d\n", msg->GetID(), msg->GetSize(), msg->GetTransaction());
#endif

			bool is_reply = m_Connection->CreatedMessage(msg->GetTransaction());
			if (is_reply && m_Stack.Size() > 0)
			{
				Frame* top = m_Stack.Top();

				bool is_current = msg->GetTransaction() == top->m_ReplyTransaction;
				if (is_current)
				{
#ifdef RPC_DEBUG
					printf("RPC::Got reply to transaction %d\n", msg->GetTransaction());
#endif

					// subsume the message into the frame
					top->m_Replied = true;
					top->m_ReplyID = msg->GetID();
					top->m_ReplyData = msg->TakeData();  // taking this will disconnect it from the message, making delete below *safe*
					top->m_ReplySize = msg->GetSize();

					// free msg
					delete msg;

					// we have our reply, break out of processing messages
					break;
				}
				else
				{
					printf("RPC::Got reply to transaction %d, however its not a reply for the top of the stack (stack size: %d)\n", msg->GetTransaction(), m_Stack.Size());
				}
			}
			else // else this is not a reply, meaning this is a new invocation
			{
				int32_t size HELIUM_ASSERT_ONLY = m_Stack.Size();

				// allocate a frame for this local call
				Frame* frame = m_Stack.Push();

				frame->m_ReplyTransaction = msg->GetTransaction();

#ifdef RPC_DEBUG
				printf("RPC::Pushing invocation transaction %d, stack size %d\n", frame->m_ReplyTransaction, m_Stack.Size());
#endif

				// the one and only call to invoke, this expects our frame to be allocated
				if (Invoke(msg))
				{
#ifdef RPC_DEBUG
					printf("RPC::Popping invocation transaction %d, stack size %d\n", frame->m_ReplyTransaction, m_Stack.Size());
#endif

					// success, pop the call
					m_Stack.Pop();

					HELIUM_ASSERT(size == m_Stack.Size());
				}
				else
				{
					printf("RPC::Invocation failed, resetting stack\n");
					m_Stack.Reset();
				}
			}
		}
	}
	else
	{
		result = false;
	}

	return result;
}

using namespace Helium::RPC::Test;

Test::TestInterface g_TestInterface;

TestInterface::TestInterface()
	: Interface ("Test")
{
	Helium::Signature< TestArgs&>::Delegate delegate ( this, &TestInterface::Test );

	AddInvoker( new InvokerTemplate<TestArgs> ( this, delegate ) );
}

void TestInterface::Test( TestArgs& args )
{

}

void RPC::Test::AddInterface( RPC::Host& host )
{
	host.AddInterface( &g_TestInterface );
}