#include "Precompile.h"
#include "Inspect/Control.h"

#include "Inspect/Canvas.h"

HELIUM_DEFINE_ABSTRACT( Helium::Inspect::ClientData );
HELIUM_DEFINE_ABSTRACT( Helium::Inspect::Widget );
HELIUM_DEFINE_ABSTRACT( Helium::Inspect::Control );

using namespace Helium;
using namespace Helium::Inspect;

#if HELIUM_PROFILE_ENABLE
Profile::Sink Inspect::g_RealizeSink( "Inspect Realize Sink" );
Profile::Sink Inspect::g_UnrealizeSink( "Inspect Unrealize Sink" );
#endif

Control::Control()
	: a_IsEnabled( true )
	, a_IsReadOnly( false )
	, a_IsFrozen( false )
	, a_IsHidden( false )
	, a_ForegroundColor( 0 )
	, a_BackgroundColor( 0 )
	, a_IsFixedWidth( false )
	, a_IsFixedHeight( false )
	, a_ProportionalWidth( 0.f )
	, a_ProportionalHeight( 0.f )
	, m_Canvas( NULL )
	, m_Parent( NULL )
	, m_IsWriting( false )
	, m_IsRealized( false )
{

}

Control::~Control()
{
	if (m_DataBinding)
	{
		m_DataBinding->RemoveChangedListener( DataChangedSignature::Delegate ( this, &Control::DataChanged ) );
	}

	m_IsRealized = false;
}

int Control::GetDepth()
{
	int depth = 0;
	Control* parent = m_Parent;

	while (parent != NULL)
	{
		depth++;
		parent = parent->m_Parent;
	}

	return depth;
}

void Control::SetCanvas( Canvas* canvas )
{
	if ( m_Canvas != canvas )
	{
		HELIUM_ASSERT( (!m_Canvas && canvas) || (m_Canvas && !canvas) );
		m_Canvas = canvas;
	}
}

void Control::SetParent( Container* parent )
{
	if ( m_Parent != parent )
	{
		Container* oldParent = m_Parent;

		m_Parent = parent;

		if ( oldParent )
		{
			oldParent->RemoveChild( this );
		}

		if ( m_Parent )
		{
			m_Parent->AddChild( this );
		}
	}
}

void Control::Bind(const DataBindingPtr& data)
{
	if ( !m_DataBinding.ReferencesObject() || !data.ReferencesObject() )
	{
		if ( m_DataBinding.ReferencesObject() )
		{
			m_DataBinding->RemoveChangedListener( DataChangedSignature::Delegate ( this, &Control::DataChanged ) );
		}

		m_DataBinding = data;

		if ( m_DataBinding.ReferencesObject() )
		{
			m_DataBinding->AddChangedListener( DataChangedSignature::Delegate ( this, &Control::DataChanged ) );
		}
	}
}

bool Control::IsDefault() const
{
	if (a_Default.Get().empty() || m_DataBinding == NULL)
	{
		return false;
	}

	StringDataBinding* data = CastDataBinding<StringDataBinding, DataBindingTypes::String>( m_DataBinding );
	if ( data )
	{
		std::string val;
		data->Get(val);
		return a_Default.Get() == val;
	}

	HELIUM_BREAK(); // you need to override this, your control is using custom data
	return false;
}

bool Control::SetDefault()
{
	if (!a_Default.Get().empty())
	{
		return WriteStringData( a_Default.Get() );
	}
	else
	{
		return false;
	}
}

const ContextMenuPtr& Control::GetContextMenu()
{
	return m_ContextMenu;
}

void Control::SetContextMenu(const ContextMenuPtr& contextMenu)
{
	m_ContextMenu = contextMenu;
}

bool Control::Process(const std::string& key, const std::string& value)
{
	if ( key == ATTR_HELPTEXT )
	{
		a_HelpText.Set(value);

		return true;
	}

	return false;
}

bool Control::IsRealized()
{
	return m_IsRealized;
}

void Control::Realize(Canvas* canvas)
{
#if HELIUM_PROFILE_ENABLE
	Profile::Timer scopeTimer ( g_RealizeSink );
#endif

	if ( !m_IsRealized )
	{
		m_Canvas = canvas;
		m_Canvas->RealizeControl( this );

		m_IsRealized = true;
		e_Realized.Raise(this);
	}
}

void Control::Unrealize()
{
#if HELIUM_PROFILE_ENABLE
	Profile::Timer scopeTimer ( g_UnrealizeSink );
#endif

	if ( m_IsRealized )
	{
		m_Canvas->UnrealizeControl( this );
		m_Canvas = NULL;

		m_IsRealized = false;
		e_Unrealized.Raise(this);
	}
}

void Control::Populate()
{
}

void Control::Read()
{
	if ( m_Widget )
	{
		m_Widget->Read();
	}

	SetDefaultAppearance( IsDefault() );
}

bool Control::ReadStringData(std::string& str) const
{
	StringDataBinding* data = CastDataBinding<StringDataBinding, DataBindingTypes::String>( m_DataBinding );
	if (data)
	{
		str.clear();
		data->Get( str );
		return true;
	}

	HELIUM_BREAK(); // you should not call this, your control is using custom data
	return false;
}

bool Control::ReadAllStringData(std::vector< std::string >& strs) const
{
	StringDataBinding* data = CastDataBinding<StringDataBinding, DataBindingTypes::String>( m_DataBinding );
	if ( data )
	{
		strs.clear();
		data->GetAll( strs );
		return true;
	}

	HELIUM_BREAK(); // you should not call this, your control is using custom data
	return false;
}

void Control::DataChanged(const DataChangedArgs& args)
{
	if ( !m_IsWriting )
	{
		Read();

		e_ControlChanged.Raise( this );
	}
}

bool Control::PreWrite( Reflect::Data newValue, bool preview )
{
	ControlChangingArgs args (this, newValue, preview);
	e_ControlChanging.Raise( args );

	// check to see if a event handler bound to this control bypasses the write
	if ( args.m_Veto )
	{
		return false;
	}

	return true;
}

bool Control::Write()
{
	if ( m_Widget )
	{
		return m_Widget->Write();
	}

	return true;
}

bool Control::WriteStringData(const std::string& str, bool preview)
{
	StringDataBinding* data = CastDataBinding<StringDataBinding, DataBindingTypes::String>( m_DataBinding );

	return WriteTypedData(str, data, preview);
}

bool Control::WriteAllStringData(const std::vector< std::string >& strs, bool preview)
{
	StringDataBinding* dataBinding = CastDataBinding<StringDataBinding, DataBindingTypes::String>( m_DataBinding );
	if (dataBinding)
	{
		std::vector< std::string > currentValues;
		dataBinding->GetAll( currentValues );

		if ( strs == currentValues )
		{
			return true;
		}

		AutoPtr< Reflect::Translator > translator( Reflect::AllocateTranslator< std::vector< std::string > >() );
		if ( !PreWrite( Reflect::Data( Reflect::Pointer( &currentValues ), translator.Ptr() ), preview ) )
		{
			Read();
			return false;
		}

		m_IsWriting = true;

		bool result = dataBinding->SetAll( strs );
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

void Control::PostWrite()
{
	SetDefaultAppearance( IsDefault() );

	// callback to our interpreter that we changed
	e_ControlChanged.Raise( this );

	// data validator could change our value, so re-read the value
	Read();
}

template<>
bool Control::GetProperty( const std::string& key, std::string& value ) const
{
	std::map< std::string, std::string >::const_iterator found = m_Properties.find( key ); 
	if ( found != m_Properties.end() )
	{
		value = found->second;
		return true;
	}

	return false;
}

template<>
void Control::SetProperty( const std::string& key, const std::string& value )
{
	m_Properties[key] = value;
}
