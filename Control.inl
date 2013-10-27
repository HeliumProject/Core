Helium::Inspect::ControlChangingArgs::ControlChangingArgs( class Control* control, Reflect::Data newValue, bool preview )
	: m_Control( control )
	, m_NewValue( newValue )
	, m_Preview( preview )
	, m_Veto( false )
{
}

Helium::Inspect::ControlChangedArgs::ControlChangedArgs(class Control* control)
	: m_Control (control)
{
}

Helium::Inspect::PopulateItem::PopulateItem(const std::string& key, const std::string& data)
	: m_Key( key )
	, m_Data( data )
{
}

Helium::Inspect::PopulateLinkArgs::PopulateLinkArgs(uint32_t type)
	: m_Type( type )
{
}

Helium::Inspect::SelectLinkArgs::SelectLinkArgs(const std::string& id)
	: m_ID( id )
{
}

Helium::Inspect::PickLinkArgs::PickLinkArgs(const DataBindingPtr& data)
	: m_DataBinding( data )
{
}

Helium::Inspect::ClientData::ClientData( Control* control )
	: m_Control ( control )
{
}

Helium::Inspect::Control* Helium::Inspect::ClientData::GetControl() const
{
	return m_Control;
}

void Helium::Inspect::ClientData::SetControl( Control* control )
{
	m_Control = control;
}

Helium::Inspect::Widget::Widget()
	: m_Control( NULL )
{

}

Helium::Inspect::Control* Helium::Inspect::Widget::GetControl() const
{
	return m_Control;
}

void Helium::Inspect::Widget::SetControl( Control* control )
{
	m_Control = control;
}

Helium::Inspect::Canvas* Helium::Inspect::Control::GetCanvas() const
{
	return m_Canvas;
}

Helium::Inspect::Container* Helium::Inspect::Control::GetParent() const
{
	return m_Parent;
}

Helium::Inspect::Widget* Helium::Inspect::Control::GetWidget() const
{
	return m_Widget;
}

void Helium::Inspect::Control::SetWidget( Widget* widget )
{
	m_Widget = widget;
}

Helium::Inspect::ClientData* Helium::Inspect::Control::GetClientData() const
{
	return m_ClientData;
}
void Helium::Inspect::Control::SetClientData( ClientData* clientData )
{
	m_ClientData = clientData;
}

bool Helium::Inspect::Control::IsBound() const
{
	return m_DataBinding.ReferencesObject();
}

const Helium::Inspect::DataBinding* Helium::Inspect::Control::GetBinding() const
{
	return m_DataBinding;
}

template<class T>
bool Helium::Inspect::Control::ReadTypedData(const typename DataBindingTemplate<T>::Ptr& data, T& val)
{
	if (data)
	{
		T currentValue;
		data->Get( currentValue );
	}

	HELIUM_BREAK(); // you should not call this, your control is using custom data
	return false;
}

template<class T>
bool Helium::Inspect::Control::WriteTypedData(const T& val, const typename DataBindingTemplate<T>::Ptr& dataBinding, bool preview)
{
	if (dataBinding)
	{
		T currentValue;
		dataBinding->Get( currentValue );
		if ( val == currentValue )
		{
			return true;
		}

		AutoPtr< Reflect::Translator > translator( Reflect::AllocateTranslator< T >() );
		if ( !PreWrite( Reflect::Data( Reflect::Pointer( &currentValue ), translator.Ptr() ), preview ) )
		{
			Read();
			return false;
		}

		m_IsWriting = true;
		bool result = dataBinding->Set( val );
		m_IsWriting = false;

		if (result)
		{
			PostWrite();
			return true;
		}
	}

	HELIUM_BREAK(); // you should not call this, your control is using custom data
	return false;
}

const std::string& Helium::Inspect::Control::GetProperty( const std::string& key ) const
{
	std::map< std::string, std::string >::const_iterator found = m_Properties.find( key );
	if ( found != m_Properties.end() )
	{
		return found->second;
	}

	static std::string empty;
	return empty;
}

template<class T>
bool Helium::Inspect::Control::GetProperty( const std::string& key, T& value ) const
{
	std::string strValue;
	bool result = GetProperty<std::string>( key, strValue );

	if ( result )
	{
		std::istringstream str( strValue );
		str >> value;
		return !str.fail();
	}

	return false;
}

template<class T>
void Helium::Inspect::Control::SetProperty( const std::string& key, const T& value )
{
	std::ostringstream str;
	str << value;

	if ( !str.fail() )
	{
		SetProperty<std::string>( key, str.str() );
	}
}
