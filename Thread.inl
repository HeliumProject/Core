/// Get the name assigned to this thread.
///
/// @return  Thread name.
///
/// @see SetName()
const char* Helium::Thread::GetName() const
{
    return m_Name;
}

// C++ helper (remember, it is valid to pass a member function pointer as a template parameter!)
template< class ObjectT, void (ObjectT::*method)() >
void Helium::CallbackThread::EntryHelper( void* param )
{
    ObjectT* object = (ObjectT*)param;
    (object->*method)();
}

// create and execute a thread with a separate args object
bool Helium::CallbackThread::CreateWithArgs( Entry entry, void* obj, void* args, const char* name, ThreadPriority priority )
{
    ThreadHelperArgs* threadHelperArgs = new ThreadHelperArgs( obj, args );
    return Create(entry, threadHelperArgs, name, priority);
}

// C++ helper (remember, it is valid to pass a member function pointer as a template parameter!)
template< class ObjectT, typename ArgsT, void (ObjectT::*method)( ArgsT& ) >
void Helium::CallbackThread::EntryHelperWithArgs( void* param )
{
    ThreadHelperArgs*   helperArgs = (ThreadHelperArgs*)param;
    ObjectT*            object = (ObjectT*)helperArgs->m_Object;
    ArgsT*              args = (ArgsT*)helperArgs->m_Args;

    (object->*method)( *args );

    delete helperArgs;
    delete args;
}

template< class T >
T* Helium::ThreadLocal< T >::GetPointer() const
{
	return static_cast< T* >( ThreadLocalPointer::GetPointer() );
}

template< class T >
void Helium::ThreadLocal< T >::SetPointer(T* value)
{
	ThreadLocalPointer::SetPointer( value );
}
