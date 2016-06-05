template<class T>
void Helium::Inspect::Extract(std::istream& stream, T* val)
{
	stream >> *val;
}

template<class T>
void Helium::Inspect::Insert(std::ostream& stream, const T* val)
{
	stream << *val;
}

Helium::Inspect::DataChangingArgs::DataChangingArgs( const DataBinding* data, Reflect::Data value )
	: m_Data ( data )
	, m_NewValue( value )
	, m_Veto ( false )
{
}

Helium::Inspect::DataChangedArgs::DataChangedArgs( const DataBinding* data )
	: m_Data ( data )
{
}

Helium::Inspect::DataBinding::DataBinding()
	: m_Significant(true)
{

}

Helium::Inspect::DataBinding::~DataBinding()
{

}

void Helium::Inspect::DataBinding::SetSignificant(bool significant)
{
	m_Significant = significant; 
}
bool Helium::Inspect::DataBinding::IsSignificant() const
{
	return m_Significant; 
}

void Helium::Inspect::DataBinding::AddChangingListener( const DataChangingSignature::Delegate& listener ) const
{
	m_Changing.Add( listener );
}
void Helium::Inspect::DataBinding::RemoveChangingListener( const DataChangingSignature::Delegate& listener ) const
{
	m_Changing.Remove( listener );
}

void Helium::Inspect::DataBinding::AddChangedListener( const DataChangedSignature::Delegate& listener ) const
{
	m_Changed.Add( listener );
}
void Helium::Inspect::DataBinding::RemoveChangedListener( const DataChangedSignature::Delegate& listener ) const
{
	m_Changed.Remove( listener );
}

template< typename T, Helium::Inspect::DataBindingTypes::DataBindingType type >
T* Helium::Inspect::CastDataBinding( DataBinding* data )
{
	return data ? (data->HasType( type ) ? static_cast<T*>( data ) : NULL) : NULL;
}

template< class T >
void Helium::Inspect::DataBindingTemplate<T>::Refresh()
{
	T temp;
	Get( temp );
	Set( temp );
}

template< class T >
Helium::UndoCommandPtr Helium::Inspect::DataBindingTemplate<T>::GetUndoCommand() const
{
	return new DataBindingCommand<T>( this );
}

template< class T >
bool Helium::Inspect::DataBindingTemplate<T>::SetAll(const std::vector<T>& s, const DataChangedSignature::Delegate& emitter)
{
	bool result = false;
	HELIUM_ASSERT( s.size() == 1 ); // this means you did not override this function for data objects that support multi
	if ( s.size() > 0 )
	{
		result = Set( s.back(), emitter );
	}
	return result;
}

template< class T >
void Helium::Inspect::DataBindingTemplate<T>::GetAll(std::vector<T>& s) const
{
	s.clear();
	T value;
	Get( value );
	s.push_back( value );
}

template< class T >
Helium::Inspect::DataBindingCommand<T>::DataBindingCommand( const typename DataBindingTemplate<T>::Ptr& data )
	: m_Data ( data )
{
	if ( m_Data.ReferencesObject() )
	{
		m_Data->GetAll(m_Values);
	}
}

template< class T >
void Helium::Inspect::DataBindingCommand<T>::Undo()
{
	Swap();
}

template< class T >
void Helium::Inspect::DataBindingCommand<T>::Redo()
{
	Swap();
}

template< class T >
bool Helium::Inspect::DataBindingCommand<T>::IsSignificant() const
{
	if( m_Data )
	{
		return m_Data->IsSignificant(); 
	}
	else
	{
		return false; 
	}
}

template< class T >
void Helium::Inspect::DataBindingCommand<T>::Swap()
{
	std::vector<T> temp;

	// read current state into temp
	m_Data->GetAll( temp );

	// set previous state
	m_Data->SetAll( m_Values );

	// cache previously current state
	m_Values = temp;
}

template< class T >
Helium::Inspect::StringFormatter<T>::StringFormatter(T* data, bool perishable)
	: m_Data (data)
	, m_Perishable (perishable)
{

}

template< class T >
Helium::Inspect::StringFormatter<T>::~StringFormatter()
{
	if (m_Perishable)
	{
		delete m_Data;
	}
}

template< class T >
bool Helium::Inspect::StringFormatter<T>::Set(const std::string& s, const DataChangedSignature::Delegate& emitter)
{
	bool result = false;

	AutoPtr< Reflect::Translator > translator( Reflect::AllocateTranslator< std::string >() );
	Reflect::Data data ( Reflect::Pointer( const_cast< std::string* >( &s ) ), translator.Ptr() );
	DataChangingArgs args ( this, data );
	m_Changing.Raise( args );
	if ( !args.m_Veto )
	{
		std::stringstream str ( s );
		Extract< T >( str, m_Data );
		m_Changed.RaiseWithEmitter( this, emitter );
		result = true;
	}

	return result;
}

template< class T >
void Helium::Inspect::StringFormatter<T>::Get(std::string& s) const
{
	std::stringstream stream;
	Insert<T>(stream, m_Data);
	s = stream.str();
}

template< class T >
Helium::Inspect::MultiStringFormatter<T>::MultiStringFormatter( const std::vector<T*>& data, bool perishable)
	: m_Data (data)
	, m_Perishable (perishable)
{

}

template< class T >
Helium::Inspect::MultiStringFormatter<T>::~MultiStringFormatter()
{
	if (m_Perishable)
	{
		typename std::vector<T*>::iterator itr = m_Data.begin();
		typename std::vector<T*>::iterator end = m_Data.end();
		for ( ; itr != end; ++itr )
		{
			delete (*itr);
		}
	}
}

template< class T >
bool Helium::Inspect::MultiStringFormatter<T>::Set(const std::string& s, const DataChangedSignature::Delegate& emitter)
{
	bool result = false;

	AutoPtr< Reflect::Translator > translator( Reflect::AllocateTranslator< std::string >() );
	Reflect::Data data ( Reflect::Pointer( const_cast< std::string* >( &s ) ), translator.Ptr() );
	DataChangingArgs args ( this, data );
	m_Changing.Raise( args );
	if ( !args.m_Veto )
	{
		typename std::vector<T*>::iterator itr = m_Data.begin();
		typename std::vector<T*>::iterator end = m_Data.end();
		for ( ; itr != end; ++itr )
		{
			std::stringstream str ( s );
			Extract<T>( str, *itr );
			result = true;
		}

		m_Changed.RaiseWithEmitter( this, emitter );
	}

	return result;
}

template< class T >
bool Helium::Inspect::MultiStringFormatter<T>::SetAll(const std::vector< std::string >& values, const DataChangedSignature::Delegate& emitter)
{
	bool result = false;

	if ( values.size() == m_Data.size() )
	{
		std::vector< std::string >::const_iterator itr = values.begin();
		std::vector< std::string >::const_iterator end = values.end();
		for ( size_t index = 0; itr != end; ++itr, ++index )
		{
			AutoPtr< Reflect::Translator > translator( Reflect::AllocateTranslator< std::string >() );
			Reflect::Data data ( Reflect::Pointer( const_cast< std::string* >( &*itr ) ), translator.Ptr() );
			DataChangingArgs args ( this, data );
			m_Changing.Raise( args );
			if ( !args.m_Veto )
			{
				std::stringstream str ( *itr );
				Extract<T>( str, m_Data[ index ] );
				result = true;
			}
		}

		m_Changed.RaiseWithEmitter( this, emitter );
	}
	else
	{
		HELIUM_BREAK();
	}

	return result;
}

template< class T >
void Helium::Inspect::MultiStringFormatter<T>::Get(std::string& s) const
{
	T* value = NULL;
	std::stringstream stream;

	//
	// Scan for equality
	//

	typename std::vector<T*>::const_iterator itr = m_Data.begin();
	typename std::vector<T*>::const_iterator end = m_Data.end();
	for ( ; itr != end; ++itr )
	{
		// grab the first one if we don't have a value yet
		if (value == NULL)
		{
			value = *itr;
			continue;
		}

		// do equality
		if (*value != *(*itr))
		{
			// we are not equal, break
			value = NULL;
			break;
		}
	}

	// if we were equal
	if (value != NULL)
	{
		// do insert
		Insert<T>(stream, value);
	}
	// else we are unequal
	else
	{
		HELIUM_ASSERT( m_Data.size() );

		// if we have data
		if (m_Data.size() > 0)
		{
			// we are a multi
			stream << MULTI_VALUE_STRING;
		}
		// we have no data
		else
		{
			// god help you if you hit this!
			stream << UNDEF_VALUE_STRING;
		}
	}

	// set the result
	s = stream.str();
}

template< class T >
void Helium::Inspect::MultiStringFormatter<T>::GetAll(std::vector< std::string >& s) const
{
	s.resize( m_Data.size() );
	typename std::vector<T*>::const_iterator itr = m_Data.begin();
	typename std::vector<T*>::const_iterator end = m_Data.end();
	for ( size_t index = 0; itr != end; ++itr, ++index )
	{
		T* value = *itr;
		std::stringstream stream;
		Insert<T>( stream, value );
		s[ index ] = stream.str();
	}
}

template< class T >
Helium::Inspect::PropertyStringFormatter<T>::PropertyStringFormatter(const Helium::SmartPtr< Helium::Property<T> >& property)
	: m_Property(property)
{
}

template< class T >
Helium::Inspect::PropertyStringFormatter<T>::~PropertyStringFormatter()
{
}

template< class T >
bool Helium::Inspect::PropertyStringFormatter<T>::Set(const std::string& s, const DataChangedSignature::Delegate& emitter)
{
	bool result = false;

	AutoPtr< Reflect::Translator > translator( Reflect::AllocateTranslator< std::string >() );
	Reflect::Data data ( Reflect::Pointer( const_cast< std::string* >( &s ) ), translator.Ptr() );
	DataChangingArgs args ( this, data );
	m_Changing.Raise( args );
	if ( !args.m_Veto )
	{
		T value;
		std::stringstream str ( s );
		Extract< T >( str, &value );
		m_Property->Set( value );
		m_Changed.RaiseWithEmitter( this, emitter );
		result = true;
	}

	return result;
}

template< class T >
void Helium::Inspect::PropertyStringFormatter<T>::Get(std::string& s) const
{
	std::stringstream stream;
	T val = m_Property->Get();
	Insert<T>( stream, &val );
	s = stream.str();
}

template< class T >
Helium::Inspect::MultiPropertyStringFormatter<T>::MultiPropertyStringFormatter(const std::vector< Helium::SmartPtr< Helium::Property<T> > >& properties)
	: m_Properties (properties)
{
}

template< class T >
Helium::Inspect::MultiPropertyStringFormatter<T>::~MultiPropertyStringFormatter()
{
}

template< class T >
bool Helium::Inspect::MultiPropertyStringFormatter<T>::Set(const std::string& s, const DataChangedSignature::Delegate& emitter)
{
	bool result = false;

	AutoPtr< Reflect::Translator > translator( Reflect::AllocateTranslator< std::string >() );
	Reflect::Data data ( Reflect::Pointer( const_cast< std::string* >( &s ) ), translator.Ptr() );
	DataChangingArgs args ( this, data );
	m_Changing.Raise( args );
	if ( !args.m_Veto )
	{
		T value;
		std::stringstream str ( s );
		Extract< T >( str, &value );

		typename std::vector< Helium::SmartPtr< Helium::Property<T> > >::iterator itr = m_Properties.begin();
		typename std::vector< Helium::SmartPtr< Helium::Property<T> > >::iterator end = m_Properties.end();
		for ( ; itr != end; ++itr )
		{
			(*itr)->Set( value );
			result = true;
		}

		m_Changed.RaiseWithEmitter( this, emitter );
	}

	return result;
}

template< class T >
bool Helium::Inspect::MultiPropertyStringFormatter<T>::SetAll(const std::vector< std::string >& s, const DataChangedSignature::Delegate& emitter)
{
	bool result = false;

	if ( s.size() == m_Properties.size() )
	{
		std::vector< std::string >::const_iterator itr = s.begin();
		std::vector< std::string >::const_iterator end = s.end();
		for ( size_t index = 0; itr != end; ++itr, ++index )
		{
			AutoPtr< Reflect::Translator > translator( Reflect::AllocateTranslator< std::string >() );
			Reflect::Data data ( Reflect::Pointer( const_cast< std::string* >( &*itr ) ), translator.Ptr() );
			DataChangingArgs args ( this, data );
			m_Changing.Raise( args );
			if ( !args.m_Veto )
			{
				T value;
				std::stringstream str ( *itr );
				Extract<T>( str, &value );
				m_Properties[ index ]->Set(value);
				result = true;
			}
		}

		m_Changed.RaiseWithEmitter( this, emitter );
	}
	else
	{
		HELIUM_BREAK();
	}

	return result;
}

template< class T >
void Helium::Inspect::MultiPropertyStringFormatter<T>::Get(std::string& s) const
{
	std::stringstream stream;

	//
	// Scan for equality
	//

	typename std::vector< Helium::SmartPtr< Helium::Property<T> > >::const_iterator itr = m_Properties.begin();
	typename std::vector< Helium::SmartPtr< Helium::Property<T> > >::const_iterator end = m_Properties.end();
	for ( ; itr != end; ++itr )
	{
		// grab the first one if we don't have a value yet
		if ( itr == m_Properties.begin() )
		{
			T val = (*itr)->Get();
			Insert<T>( stream, &val );
			continue;
		}
		else
		{
			T val = (*itr)->Get();
			std::stringstream temp;
			Insert<T>( temp, &val );

			if (temp.str() != stream.str())
			{
				break;
			}
		}
	}

	// if we were not equal
	if (itr == end)
	{
		s = stream.str();
	}
	else
	{
		// if we have data
		if (m_Properties.size() > 0)
		{
			// we are a multi
			s = MULTI_VALUE_STRING;
		}
		// we have no data
		else
		{
			// god help you if you hit this!
			s = UNDEF_VALUE_STRING;
		}
	}
}

template< class T >
void Helium::Inspect::MultiPropertyStringFormatter<T>::GetAll(std::vector< std::string >& s) const
{
	s.resize( m_Properties.size() );
	typename std::vector< Helium::SmartPtr< Helium::Property<T> > >::const_iterator itr = m_Properties.begin();
	typename std::vector< Helium::SmartPtr< Helium::Property<T> > >::const_iterator end = m_Properties.end();
	for ( size_t index = 0 ; itr != end; ++itr, ++index )
	{
		T val = (*itr)->Get();
		std::stringstream stream;
		Insert<T>( stream, &val );
		s[ index ] = stream.str();
	}
}

template< class T >
Helium::Inspect::TypedPropertyFormatter<T>::TypedPropertyFormatter(const Helium::SmartPtr< Helium::Property< T > >& property)
	: m_Property(property)
{

}

template< class T >
Helium::Inspect::TypedPropertyFormatter<T>::~TypedPropertyFormatter()
{

}

template< class T >
bool Helium::Inspect::TypedPropertyFormatter<T>::Set(const T& value, const DataChangedSignature::Delegate& emitter)
{
	bool result = false;

	AutoPtr< Reflect::Translator > translator( Reflect::AllocateTranslator< T >() );
	Reflect::Data data ( Reflect::Pointer( const_cast< T* >( &value ) ), translator.Ptr() );
	DataChangingArgs args ( this, data );
	this->m_Changing.Raise( args );
	if ( !args.m_Veto )
	{
		m_Property->Set( value );
		this->m_Changed.RaiseWithEmitter( this, emitter );
		result = true;
	}

	return result;
}

template< class T >
void Helium::Inspect::TypedPropertyFormatter<T>::Get(T& value) const
{
	value = m_Property->Get();
}
