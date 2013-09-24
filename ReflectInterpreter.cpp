#include "InspectPch.h"
#include "ReflectInterpreter.h"

#include "Inspect/Inspect.h"
#include "Inspect/DataBinding.h"
#include "Inspect/Script.h"
#include "Inspect/Container.h"
#include "Inspect/Controls/ButtonControl.h"
#include "Inspect/Controls/ValueControl.h"
#include "Inspect/Controls/ListControl.h"
#include "Inspect/Controls/CheckBoxControl.h"
#include "Inspect/Controls/ChoiceControl.h"
#include "Inspect/Controls/ValueControl.h"
#include "Inspect/Controls/ListControl.h"

#include "Reflect/Object.h"
#include "Reflect/MetaEnum.h"

using namespace Helium;
using namespace Helium::Reflect;
using namespace Helium::Inspect;

ReflectInterpreter::ReflectInterpreter (Container* container)
	: Interpreter (container)
{

}

void ReflectInterpreter::Interpret(const std::vector<Reflect::Object*>& instances, int32_t includeFlags, int32_t excludeFlags, bool expandPanel)
{
	InterpretType(instances, m_Container, includeFlags, excludeFlags, expandPanel);
}

void ReflectInterpreter::InterpretType(const std::vector<Reflect::Object*>& instances, Container* parent, int32_t includeFlags, int32_t excludeFlags, bool expandPanel)
{
	const Reflect::Object *pObject = instances[0];
	const MetaStruct* composite = pObject->GetMetaClass();

	// create a container
	ContainerPtr container = CreateControl<Container>();

	// parse
	ContainerPtr scriptOutput = CreateControl<Container>();

	std::string typeInfoUI;
	composite->GetProperty( TXT( "UIScript" ), typeInfoUI );
	bool result = Script::Parse(typeInfoUI, this, parent->GetCanvas(), scriptOutput);

	// compute container label
	std::string labelText;
	if (result)
	{
		V_Control::const_iterator itr = scriptOutput->GetChildren().begin();
		V_Control::const_iterator end = scriptOutput->GetChildren().end();
		for( ; itr != end; ++itr )
		{
			Label* label = Reflect::SafeCast<Label>( *itr );
			if (label)
			{
				label->ReadStringData( labelText );

				if ( !labelText.empty() )
				{
					break;
				}
			}
		}
	}

	if (labelText.empty())
	{
		composite->GetProperty( TXT( "UIName" ), labelText );
	}

	if ( labelText.empty() )
	{
		labelText = *composite->m_Name;
	}

	container->a_Name.Set( labelText );

	std::map< std::string, ContainerPtr > containersMap;
	containersMap.insert( std::make_pair( TXT( "" ), container) );

	std::stack< const MetaStruct* > bases;
	for ( const MetaStruct* current = composite; current != NULL; current = current->m_Base )
	{
		bases.push( current );
	}

	while ( !bases.empty() )
	{
		const MetaStruct* current = bases.top();
		bases.pop();

		// for each field in the type
		DynamicArray< Field >::ConstIterator itr = current->m_Fields.Begin();
		DynamicArray< Field >::ConstIterator end = current->m_Fields.End();
		for ( ; itr != end; ++itr )
		{
			const Field* field = &*itr;

			bool noFlags = ( field->m_Flags == 0 && includeFlags == 0xFFFFFFFF );
			bool doInclude = ( field->m_Flags & includeFlags ) != 0;
			bool dontExclude = ( excludeFlags == 0 ) || !(field->m_Flags & excludeFlags );
			bool hidden = (field->m_Flags & Reflect::FieldFlags::Hide) != 0; 

			// if we don't have flags (or we are included, and we aren't excluded) then make UI
			if ( ( noFlags || doInclude ) && ( dontExclude ) )
			{
				std::string fieldUIGroup;
				field->GetProperty( TXT( "UIGroup" ), fieldUIGroup );
				if ( !fieldUIGroup.empty() )
				{
					std::map< std::string, ContainerPtr >::iterator itr = containersMap.find( fieldUIGroup );
					if ( itr == containersMap.end() )
					{
						// This container isn't in our list so make a new one
						ContainerPtr newContainer = CreateControl<Container>();
						containersMap.insert( std::make_pair(fieldUIGroup, newContainer) );

						ContainerPtr parent;
						std::string groupName;
						size_t idx = fieldUIGroup.find_last_of( TXT( "/" ) );
						if ( idx != std::string::npos )
						{
							std::string parentName = fieldUIGroup.substr( 0, idx );
							groupName = fieldUIGroup.substr( idx+1 );
							if ( containersMap.find( parentName ) == containersMap.end() )
							{          
								parent = CreateControl<Container>();

								// create the parent hierarchy since it hasn't already been made
								std::string currentParent = parentName;
								for (;;)
								{
									idx = currentParent.find_last_of( TXT( "/" ) );
									if ( idx == std::string::npos )
									{
										// no more parents so we add it to the root
										containersMap.insert( std::make_pair(currentParent, parent) );
										parent->a_Name.Set( currentParent );
										containersMap[ TXT( "" ) ]->AddChild( parent );
										break;
									}
									else
									{
										parent->a_Name.Set( currentParent.substr( idx+1 ) );

										if ( containersMap.find( currentParent ) != containersMap.end() )
										{
											break;
										}
										else
										{
											ContainerPtr grandParent = CreateControl<Container>();
											grandParent->AddChild( parent );
											containersMap.insert( std::make_pair(currentParent, parent) );

											parent = grandParent;
										}
										currentParent = currentParent.substr( 0, idx );
									}
								}
								containersMap.insert( std::make_pair(parentName, parent) );
							}
							parent = containersMap[parentName];
						}
						else
						{
							parent = containersMap[ TXT( "" )];
							groupName = fieldUIGroup;
						}
						newContainer->a_Name.Set( groupName );
						parent->AddChild( newContainer );
					}

					container = containersMap[fieldUIGroup];
				}
				else
				{
					container = containersMap[ TXT( "" )];
				}


				//
				// Pointer support
				//

#if REFLECT_REFACTOR
				if (field->m_DataClass == Reflect::GetMetaClass<Reflect::PointerData>())
				{
					if (field->m_Translator->)
					{
						if (hidden)
						{
							continue; 
						}        

						std::vector<Reflect::Object*> fieldInstances;

						std::vector<Reflect::Object*>::const_iterator elementItr = instances.begin();
						std::vector<Reflect::Object*>::const_iterator elementEnd = instances.end();
						for ( ; elementItr != elementEnd; ++elementItr )
						{
							uintptr_t fieldAddress = (uintptr_t)(*elementItr) + itr->m_Offset;

							Object* element = *((ObjectPtr*)(fieldAddress));

							if ( element )
							{
								fieldInstances.push_back( element );
							}
						}

						if ( !fieldInstances.empty() && fieldInstances.size() == instances.size() )
						{
							InterpretType(fieldInstances, container);
						}

						continue;
					}
				}

				//
				// ElementArray support
				//

				if (field->m_DataClass == Reflect::GetMetaClass<ObjectStlVectorData>())
				{
					if (hidden)
					{
						continue;
					}

					if ( instances.size() == 1 )
					{
						uintptr_t fieldAddress = (uintptr_t)(instances.front()) + itr->m_Offset;

						std::vector< ObjectPtr >* elements = (std::vector< ObjectPtr >*)fieldAddress;

						if ( elements->size() > 0 )
						{
							ContainerPtr childContainer = CreateControl<Container>();

							std::string temp;
							field->GetProperty( TXT( "UIName" ), temp );
							if ( temp.empty() )
							{
								bool converted = Helium::ConvertString( field->m_Name, temp );
								HELIUM_ASSERT( converted );
							}

							childContainer->a_Name.Set( temp );

							std::vector< ObjectPtr >::const_iterator elementItr = elements->begin();
							std::vector< ObjectPtr >::const_iterator elementEnd = elements->end();
							for ( ; elementItr != elementEnd; ++elementItr )
							{
								std::vector<Reflect::Object*> childInstances;
								childInstances.push_back(*elementItr);
								InterpretType(childInstances, childContainer);
							}

							container->AddChild( childContainer );
						}
					}

					continue;
				}
#endif

				//
				// Lastly fall back to the value interpreter
				//

				HELIUM_ASSERT( field->m_Translator );
				if ( field->m_Translator->IsA( MetaIds::SimpleTranslator ) )
				{
					InterpretValueField( field, instances, container );
					continue;
				}
			}
		}
	}

	// Make sure we have the base container
	container = containersMap[TXT( "" )];

	if ( !container->GetChildren().empty() )
	{
		parent->AddChild(container);
	}
}

void ReflectInterpreter::InterpretValueField(const Field* field, const std::vector<Reflect::Object*>& instances, Container* parent)
{
	if (field->m_Flags & FieldFlags::Hide)
	{
		return;
	}

	//
	// Create the ui we are generating
	//

	ContainerPtr container = CreateControl<Container>();

	bool readOnly = ( field->m_Flags & FieldFlags::ReadOnly ) == FieldFlags::ReadOnly;

	//
	// Parse
	//

	std::string fieldUI;
	field->GetProperty( TXT( "UIScript" ), fieldUI );
	bool result = Script::Parse(fieldUI, this, parent->GetCanvas(), container, field->m_Flags);

	if (!result)
	{
		if ( field->m_ValueType && field->m_ValueType->IsA( MetaIds::MetaEnum ) )
		{
			const Reflect::MetaEnum* enumeration = Reflect::ReflectionCast< MetaEnum >( field->m_ValueType );

			ChoicePtr choice = CreateControl<Choice>();

			std::vector< ChoiceItem > items;
			items.resize( enumeration->m_Elements.GetSize() );

			DynamicArray< MetaEnum::Element >::ConstIterator itr = enumeration->m_Elements.Begin();
			DynamicArray< MetaEnum::Element >::ConstIterator end = enumeration->m_Elements.End();
			for ( size_t index=0; itr != end; ++itr, ++index )
			{
				ChoiceItem& item = items[index];

				item.m_Key = itr->m_Name;
				item.m_Data = itr->m_Name;
			}

			choice->a_HelpText.Set( field->GetProperty( TXT( "HelpText" ) ) );
			choice->a_Items.Set( items );
			choice->a_IsDropDown.Set( true );
			choice->a_IsReadOnly.Set( readOnly );

			container->AddChild(choice);
		}
		else
		{
			ScalarTranslator* scalar = ReflectionCast< ScalarTranslator >( field->m_Translator );
			if ( scalar && scalar->m_Type == ScalarTypes::Boolean )
			{
				CheckBoxPtr checkBox = CreateControl<CheckBox>();
				checkBox->a_IsReadOnly.Set( readOnly );
				checkBox->a_HelpText.Set( field->GetProperty( TXT( "HelpText" ) ) );
				container->AddChild( checkBox );
			}
			else
			{
				ValuePtr value = CreateControl<Value>();
				value->a_IsReadOnly.Set( readOnly );
				value->a_HelpText.Set( field->GetProperty( TXT( "HelpText" ) ) );
				container->AddChild( value );
			}
		}
	}

	//
	// Setup label
	//

	LabelPtr label = NULL;

	{
		V_Control::const_iterator itr = container->GetChildren().begin();
		V_Control::const_iterator end = container->GetChildren().end();
		for( ; itr != end; ++itr )
		{
			Label* label = Reflect::SafeCast<Label>( *itr );
			if (label)
			{
				break;
			}
		}
	}

	if (!label.ReferencesObject())
	{
		label = CreateControl<Label>();

		std::string temp;
		field->GetProperty( TXT( "UIName" ), temp );
		if ( temp.empty() )
		{
			bool converted = Helium::ConvertString( field->m_Name, temp );
			HELIUM_ASSERT( converted );
		}

		label->BindText( temp );
		label->a_HelpText.Set( field->GetProperty( TXT( "HelpText" ) ) );

		container->InsertChild(0, label);
	}

	//
	// Bind data
	//

	std::vector<Data*> ser;

	{
		std::vector<Reflect::Object*>::const_iterator itr = instances.begin();
		std::vector<Reflect::Object*>::const_iterator end = instances.end();
		for ( ; itr != end; ++itr )
		{
			ser.push_back( new Data ( Pointer( field, *itr ), field->m_Translator ) );
		}
	}

	Helium::SmartPtr< MultiStringFormatter<Data> > data = new MultiStringFormatter<Data>( ser, true );

	container->Bind( data );

	//
	// Set default
	//

	String defaultStr;
	Pointer defaultPtr ( field, field->m_Structure->m_Default, NULL );
	ScalarTranslator* scalar = Reflect::ReflectionCast< ScalarTranslator >( field->m_Translator );
	if ( scalar )
	{
		scalar->Print( defaultPtr, defaultStr );
	}

	container->a_Default.Set( defaultStr.GetData() );

	//
	// Close
	//

	parent->AddChild(container);
}

class MultiBitfieldStringFormatter : public MultiStringFormatter<Data>
{
public:
	MultiBitfieldStringFormatter( const Reflect::MetaEnum::Element* element, const std::vector<Data*>& data )
		: MultiStringFormatter<Data>( data, false )
		, m_EnumerationElement( element )
	{

	}

	virtual bool Set(const std::string& s, const DataChangedSignature::Delegate& emitter = NULL)
	{
		// get the full string set
		std::string bitSet;
		MultiStringFormatter<Data>::Get( bitSet );

		if ( s == TXT("1") )
		{
			if ( !bitSet.find_first_of( m_EnumerationElement->m_Name ) )
			{
				if ( bitSet.empty() )
				{
					bitSet = m_EnumerationElement->m_Name;
				}
				else
				{
					bitSet += TXT("|") + m_EnumerationElement->m_Name;
				}
			}
		}
		else if ( s == TXT("0") )
		{
			if ( bitSet == m_EnumerationElement->m_Name )
			{
				bitSet.clear();
			}
			else
			{
				size_t pos = bitSet.find_first_of( m_EnumerationElement->m_Name );
				if ( pos != std::string::npos )
				{
					// remove the bit from the bitfield value
					bitSet.erase( pos, m_EnumerationElement->m_Name.length() );

					// cleanup delimiter
					bitSet.erase( pos, 1 );
				}
			}
		}
		else if ( s == MULTI_VALUE_STRING || s == UNDEF_VALUE_STRING )
		{
			bitSet = s;
		}

		return MultiStringFormatter<Data>::Set( bitSet, emitter );
	}

	virtual void Get(std::string& s) const HELIUM_OVERRIDE
	{
		MultiStringFormatter<Data>::Get( s );

		if ( s.find_first_of( m_EnumerationElement->m_Name ) != std::string::npos )
		{
			s = TXT("1");
		}
		else
		{
			s = TXT("0");
		}
	}

private:
	const Reflect::MetaEnum::Element* m_EnumerationElement;
};

void ReflectInterpreter::InterpretBitfieldField(const Field* field, const std::vector<Reflect::Object*>& instances, Container* parent)
{
	bool isEnumeration = field->m_Translator->IsA( Reflect::MetaIds::EnumerationTranslator );

	// If you hit this, you are misusing this interpreter
	HELIUM_ASSERT( isEnumeration );
	if ( !isEnumeration )
	{
		return;
	}

	if ( field->m_Flags & FieldFlags::Hide )
	{
		return;
	}

	// create the container
	ContainerPtr container = CreateControl<Container>();

	std::string temp;
	field->GetProperty( TXT( "UIName" ), temp );
	if ( temp.empty() )
	{
		bool converted = Helium::ConvertString( field->m_Name, temp );
		HELIUM_ASSERT( converted );
	}

	container->a_Name.Set( temp );

	parent->AddChild(container);

	// create the data objects
	std::vector< Data* > datas;
	std::vector<Reflect::Object*>::const_iterator itr = instances.begin();
	std::vector<Reflect::Object*>::const_iterator end = instances.end();
	for ( ; itr != end; ++itr )
	{
		datas.push_back( new Data( Reflect::Pointer( field, *itr ), field->m_Translator ) );
	}

	String defaultStr;
	Pointer defaultPtr ( field, field->m_Structure->m_Default, NULL );
	ScalarTranslator* scalar = Reflect::ReflectionCast< ScalarTranslator >( field->m_Translator );
	if ( scalar )
	{
		scalar->Print( defaultPtr, defaultStr );
	}

	const Reflect::MetaEnum* enumeration = Reflect::ReflectionCast< MetaEnum >( field->m_ValueType );

	// build the child gui elements
	bool readOnly = ( field->m_Flags & FieldFlags::ReadOnly ) == FieldFlags::ReadOnly;
	DynamicArray< Reflect::MetaEnum::Element >::ConstIterator enumItr = enumeration->m_Elements.Begin();
	DynamicArray< Reflect::MetaEnum::Element >::ConstIterator enumEnd = enumeration->m_Elements.End();
	for ( ; enumItr != enumEnd; ++enumItr )
	{
		ContainerPtr row = CreateControl< Container >();
		container->AddChild( row );

		LabelPtr label = CreateControl< Label >();
		label->a_HelpText.Set( enumItr->m_HelpText );
		label->BindText( enumItr->m_Name );
		row->AddChild( label );

		CheckBoxPtr checkbox = CreateControl< CheckBox >();
		checkbox->a_IsReadOnly.Set( readOnly );
		checkbox->a_HelpText.Set( enumItr->m_HelpText );
#pragma TODO("Compute correct default value")
		checkbox->a_Default.Set( defaultStr.GetData() );
		checkbox->Bind( new MultiBitfieldStringFormatter ( &*enumItr, datas ) );
		row->AddChild( checkbox );
	}
}

void ReflectInterpreter::InterpretColorField( const Field* field, const std::vector<Reflect::Object*>& instances, Container* parent )
{
#if REFLECT_REFACTOR
    ContainerPtr container = CreateControl< Container >();
    parent->AddChild( container );

    LabelPtr label = CreateControl< Label >();

    std::string temp;
    field->GetProperty( TXT( "UIName" ), temp );
    if ( temp.empty() )
    {
        bool converted = Helium::ConvertString( field->m_Name, temp );
        HELIUM_ASSERT( converted );
    }

    label->BindText( temp );
    label->a_HelpText.Set( field->GetProperty( TXT( "HelpText" ) ) );

    container->AddChild( label );

    bool color3 = field->m_ValueType == Reflect::GetMetaStruct<Color3>() || field->m_ValueType == Reflect::GetMetaStruct<HDRColor3>();
    bool color4 = field->m_ValueType == Reflect::GetMetaStruct<Color4>() || field->m_ValueType == Reflect::GetMetaStruct<HDRColor4>();
    HELIUM_ASSERT( !(color3 && color4) ); // we shouldn't ever have both!

    if ( color3 || color4 )
    {
        std::vector<Data*> ser;
        std::vector<Reflect::Object*>::const_iterator itr = instances.begin();
        std::vector<Reflect::Object*>::const_iterator end = instances.end();
        for ( ; itr != end; ++itr )
        {
            Data s;

            if ( color3 )
            {
                s = new Color3Data();
            }

            if ( color4 )
            {
                s = new Color4Data();
            }

            if (s.ReferencesObject())
            {
                s->ConnectField( *itr, field );
                ser.push_back( s );
                m_Datas.push_back( s );
            }
        }

        if ( !m_Datas.empty() )
        {
            ColorPickerPtr colorPicker = CreateControl<ColorPicker>();
            container->AddChild( colorPicker );

            colorPicker->a_HelpText.Set( field->GetProperty( TXT( "HelpText" ) ) );

            bool readOnly = ( field->m_Flags & FieldFlags::ReadOnly ) == FieldFlags::ReadOnly;
            colorPicker->a_IsReadOnly.Set( readOnly );

            DataBindingPtr data = new MultiStringFormatter<Data>( ser );
            colorPicker->Bind( data );

            if ( color3 )
            {
                colorPicker->a_Alpha.Set( false );
            }

            if ( color4 )
            {
                colorPicker->a_Alpha.Set( true );

                SliderPtr slider = CreateControl<Slider>();
                container->AddChild( slider );
                slider->a_Min.Set( 0.0 );
                slider->a_Max.Set( 255.0f );
                slider->a_IsReadOnly.Set( readOnly );
                slider->a_HelpText.Set( TXT( "Sets the alpha value for the color." ) );

                ValuePtr value = CreateControl<Value>();
                container->AddChild( value );
                value->a_IsReadOnly.Set( readOnly );
                value->a_HelpText.Set( TXT( "Sets the alpha value for the color." ) );

                std::vector<Data*> alphaSer;
                std::vector<Reflect::Object*>::const_iterator itr = instances.begin();
                std::vector<Reflect::Object*>::const_iterator end = instances.end();
                for ( ; itr != end; ++itr )
                {
                    Data s = new UInt8Data ();

                    uintptr_t fieldAddress = (uintptr_t)(*itr) + field->m_Offset;

                    Color4* col = (Color4*)fieldAddress;
                    intptr_t offsetInField = (intptr_t)( &col->a ) - fieldAddress;
                    s->ConnectField( *itr, field, offsetInField );

                    alphaSer.push_back( s );

                    m_Datas.push_back( s );
                }

                data = new MultiStringFormatter<Data>( alphaSer );
                slider->Bind( data );
                value->Bind( data );
            }

            if ( field->m_DataClass == Reflect::GetMetaClass<HDRColor3Data>() || field->m_DataClass == Reflect::GetMetaClass<HDRColor4Data>() )
            {
                SliderPtr slider = CreateControl<Slider>();
                container->AddChild( slider );
                slider->a_Min.Set( 0.0 );
                slider->a_Max.Set( 8.0 );
                slider->a_IsReadOnly.Set( readOnly );
                slider->a_HelpText.Set( TXT( "Adjusts the HDR value of the color." ) );

                ValuePtr value = CreateControl<Value>();
                container->AddChild( value );
                value->a_IsReadOnly.Set( readOnly );
                value->a_HelpText.Set( TXT( "Adjusts the HDR value of the color." ) );

                std::vector<Data*> intensitySer;
                std::vector<Reflect::Object*>::const_iterator itr = instances.begin();
                std::vector<Reflect::Object*>::const_iterator end = instances.end();
                for ( ; itr != end; ++itr )
                {
                    Data s = new Float32Data();

                    uintptr_t fieldAddress = (uintptr_t)(*itr) + field->m_Offset;

                    if ( color3 )
                    {
                        HDRColor3* col = (HDRColor3*)fieldAddress;
                        intptr_t offsetInField = (intptr_t)( &col->s ) - fieldAddress;
                        s->ConnectField( *itr, field, offsetInField );
                    }

                    if ( color4 )
                    {
                        HDRColor4* col = (HDRColor4*)fieldAddress;
                        intptr_t offsetInField = (intptr_t)( &col->s ) - fieldAddress;
                        s->ConnectField( *itr, field, offsetInField );
                    }

                    intensitySer.push_back( s );

                    m_Datas.push_back( s );
                }

                data = new MultiStringFormatter<Data>( intensitySer );
                slider->Bind( data );
                value->Bind( data );
            }
        }
    }
#endif
}

void ReflectInterpreter::InterpretFilePathField(const Field* field, const std::vector<Reflect::Object*>& instances, Container* parent)
{
#if REFLECT_REFACTOR
	if (field->m_Flags & FieldFlags::Hide)
	{
		return;
	}

	//
	// Create the ui we are generating
	//

	std::vector< ContainerPtr > groups;

	ContainerPtr container = CreateControl<Container>();
	groups.push_back( container );

	bool pathField = field->m_DataClass == Reflect::GetMetaClass< PathData >();
	bool readOnly = ( field->m_Flags & FieldFlags::ReadOnly ) == FieldFlags::ReadOnly;

	DataChangingSignature::Delegate changingDelegate;

	FileDialogButtonPtr fileDialogButton;

	//
	// Parse
	//
	std::string fieldUI;
	field->GetProperty( TXT( "UIScript" ), fieldUI );
	bool result = Script::Parse(fieldUI, this, parent->GetCanvas(), container, field->m_Flags);

	if (!result)
	{
		if ( pathField || field->m_DataClass == Reflect::GetMetaClass<StlStringData>() )
		{
			ContainerPtr valueContainer = CreateControl<Container>();
			ValuePtr value = CreateControl< Value >();
			value->a_Justification.Set( Justifications::Right );
			value->a_IsReadOnly.Set( readOnly );
			value->a_HelpText.Set( field->GetProperty( TXT( "HelpText" ) ) );
			valueContainer->AddChild( value );
			groups.push_back( valueContainer );

			if ( !readOnly )
			{
				changingDelegate = DataChangingSignature::Delegate(this, &PathInterpreter::DataChanging);

				// File dialog button
				fileDialogButton = CreateControl< FileDialogButton >();
				fileDialogButton->a_HelpText.Set( TXT( "Open a file dialog to choose a new file." ) );

				field->GetProperty( TXT( "FileFilter" ), m_FileFilter );

				if ( !m_FileFilter.empty() )
				{
					fileDialogButton->a_Filter.Set( m_FileFilter );
				}
				container->AddChild( fileDialogButton );

				value->SetProperty( TXT( "FileFilter" ), m_FileFilter );
			}

			if ( instances.size() == 1 )
			{
				// File edit button
				ButtonPtr editButton = CreateControl< Button >();
				editButton->ButtonClickedEvent().Add( ButtonClickedSignature::Delegate ( this, &PathInterpreter::Edit ) );
				editButton->a_Label.Set( TXT( "Edit" ) );
				editButton->a_HelpText.Set( TXT( "Attempt to edit the file using its associated default application." ) );
				container->AddChild( editButton );
			}
		}
	}
	else
	{
		ValuePtr value = CreateControl< Value >();
		value->a_IsReadOnly.Set( readOnly );
		value->a_HelpText.Set( field->GetProperty( TXT( "HelpText" ) ) );
		container->AddChild( value );
	}


	//
	// Setup label
	//

	LabelPtr label = NULL;

	{
		V_Control::const_iterator itr = container->GetChildren().begin();
		V_Control::const_iterator end = container->GetChildren().end();
		for( ; itr != end; ++itr )
		{
			Label* label = Reflect::SafeCast<Label>( *itr );
			if (label)
			{
				break;
			}
		}
	}

	if (!label.ReferencesObject())
	{
		label = CreateControl< Label >();

		std::string temp;
		field->GetProperty( TXT( "UIName" ), temp );
		if ( temp.empty() )
		{
			bool converted = Helium::ConvertString( field->m_Name, temp );
			HELIUM_ASSERT( converted );
		}

		label->BindText( temp );
		label->a_HelpText.Set( field->GetProperty( TXT( "HelpText" ) ) );

		container->InsertChild(0, label);
	}

	//
	// Create type m_FinderSpecific data bound to this and additional instances
	//

	std::vector<Data*> ser;

	{
		std::vector<Reflect::Object*>::const_iterator itr = instances.begin();
		std::vector<Reflect::Object*>::const_iterator end = instances.end();
		for ( ; itr != end; ++itr )
		{
			Data s = field->CreateData();

			if (s->IsA(Reflect::GetMetaClass<ContainerData>()))
			{
				return;
			}

			s->ConnectField(*itr, field);

			ser.push_back(s);

			m_Datas.push_back(s);
		}
	}

	//
	// Create data and bind
	//

	Helium::SmartPtr< MultiStringFormatter<Data> > data = new MultiStringFormatter<Data>( ser );

	if (changingDelegate.Valid())
	{
		data->AddChangingListener( changingDelegate );
	}

	{
		std::vector<ContainerPtr>::const_iterator itr = groups.begin();
		std::vector<ContainerPtr>::const_iterator end = groups.end();
		for ( ; itr != end; ++itr )
		{
			(*itr)->Bind( data );
		}
	}

	//
	// Set default
	//

	Data defaultData = field->CreateDefaultData();
	if (defaultData.ReferencesObject())
	{
		std::stringstream defaultStream;
		*defaultData >> defaultStream;
		container->a_Default.Set( defaultStream.str() );
	}

	//
	// Close
	//

	{
		std::vector<ContainerPtr>::const_iterator itr = groups.begin();
		std::vector<ContainerPtr>::const_iterator end = groups.end();
		for ( ; itr != end; ++itr )
		{
			parent->AddChild(*itr);
		}
	}
#endif
}

void ReflectInterpreter::FilePathDataChanging( const DataChangingArgs& args )
{
#if REFLECT_REFACTOR
	std::string text;
	Reflect::Data::GetValue( args.m_NewValue, text );

	if ( !text.empty() )
	{
		Helium::FilePath path( text );

		if ( path.IsFile() )
		{
			return;
		}

		path.TrimToExisting();

		FileDialogArgs fileDialogArgs( Helium::FileDialogTypes::OpenFile, TXT( "FilePath Does Not Exist" ), m_FileFilter, path );
		d_FindMissingFile.Invoke( fileDialogArgs );
		Reflect::Data::SetValue< std::string >( args.m_NewValue, fileDialogArgs.m_Result.Get() );
	}
#endif
}

void ReflectInterpreter::FilePathEdit( const ButtonClickedArgs& args )
{
	std::string str;
	args.m_Control->ReadStringData( str );

	if ( !str.empty() )
	{
		g_EditFilePath.Raise( EditFilePathArgs( str ) );
	}
}

void ReflectInterpreter::InterpretSequenceField(const Field* field, const std::vector<Reflect::Object*>& instances, Container* parent)
{
#if REFLECT_REFACTOR
    if (field->m_Flags & FieldFlags::Hide)
    {
        return;
    }

    // create the label
    ContainerPtr labelContainer = CreateControl<Container>();
    parent->AddChild( labelContainer );
    LabelPtr label = CreateControl< Label >();
    label->a_HelpText.Set( field->GetProperty( TXT( "HelpText" ) ) );
    labelContainer->AddChild( label );

    std::string temp;
    field->GetProperty( TXT( "UIName" ), temp );
    if ( temp.empty() )
    {
        bool converted = Helium::ConvertString( field->m_Name, temp );
        HELIUM_ASSERT( converted );
    }

    label->BindText( temp );

    // create the list view
    ContainerPtr listContainer = CreateControl<Container>();
    parent->AddChild( listContainer );
    ListPtr list = CreateControl<List>();
    list->a_HelpText.Set( field->GetProperty( TXT( "HelpText" ) ) );
    listContainer->AddChild( list );

    // create the buttons
    ButtonPtr addButton;
    ButtonPtr removeButton;
    ButtonPtr upButton;
    ButtonPtr downButton;
    if ( !(field->m_Flags & FieldFlags::ReadOnly) )
    {
        addButton = AddAddButton( list );
        removeButton = AddRemoveButton( list );
        upButton = AddMoveUpButton( list );
        downButton = AddMoveDownButton( list );
    }

    // add the buttons to the container
    ContainerPtr buttonContainer = CreateControl<Container>();
    parent->AddChild( buttonContainer );
    if ( addButton )
    {
        buttonContainer->AddChild( addButton );
    }
    if ( removeButton )
    {
        buttonContainer->AddChild( removeButton );
    }
    if ( upButton )
    {
        buttonContainer->AddChild( upButton );
    }
    if ( downButton )
    {
        buttonContainer->AddChild( downButton );
    }

    // create the data objects
    std::vector<Reflect::Object*>::const_iterator itr = instances.begin();
    std::vector<Reflect::Object*>::const_iterator end = instances.end();
    for ( ; itr != end; ++itr )
    {
        Data s = field->CreateData();

        OnCreateFieldData( s );

        s->ConnectField(*itr, field);

        m_Datas.push_back(s);
    }

    // bind the ui to the data objects
    Helium::SmartPtr< MultiStringFormatter<Data> > data = new MultiStringFormatter<Reflect::Data>( (std::vector<Reflect::Data*>&)m_Datas );
    list->Bind( data );

    // setup the default value
    Data defaultData = field->CreateDefaultData();
    if (defaultData)
    {
        std::stringstream defaultStream;
        *defaultData >> defaultStream;
        list->a_Default.Set( defaultStream.str() );
    }
#endif
}

ButtonPtr ReflectInterpreter::SequenceAddAddButton( List* list )
{
    ButtonPtr addButton =CreateControl< Button >();
    addButton->ButtonClickedEvent().Add( ButtonClickedSignature::Delegate ( this, &ReflectInterpreter::SequenceOnAdd ) );
    addButton->SetClientData( new ClientData( list ) );
    addButton->a_Label.Set( TXT( "Add" ) );
    addButton->a_HelpText.Set( TXT( "Add an item to the list." ) );

    return addButton;
}

ButtonPtr ReflectInterpreter::SequenceAddRemoveButton( List* list )
{
    ButtonPtr removeButton = CreateControl< Button >();
    removeButton->a_Label.Set( TXT( "Remove" ) );
    removeButton->ButtonClickedEvent().Add( ButtonClickedSignature::Delegate ( this, &ReflectInterpreter::SequenceOnRemove ) );
    removeButton->SetClientData( new ClientData( list ) );
    removeButton->a_HelpText.Set( TXT( "Remove the selected item(s) from the list." ) );

    return removeButton;
}

ButtonPtr ReflectInterpreter::SequenceAddMoveUpButton( List* list )
{
    ButtonPtr upButton = CreateControl< Button >();
    upButton->a_Icon.Set( TXT( "actions/go-up" ) );
    upButton->ButtonClickedEvent().Add( ButtonClickedSignature::Delegate ( this, &ReflectInterpreter::SequenceOnMoveUp ) );
    upButton->SetClientData( new ClientData( list ) );
    upButton->a_HelpText.Set( TXT( "Move the selected item(s) up the list." ) );

    return upButton;
}

ButtonPtr ReflectInterpreter::SequenceAddMoveDownButton( List* list )
{
    ButtonPtr downButton = CreateControl< Button >();
    downButton->a_Icon.Set( TXT( "actions/go-down" ) );
    downButton->ButtonClickedEvent().Add( ButtonClickedSignature::Delegate ( this, &ReflectInterpreter::SequenceOnMoveDown ) );
    downButton->SetClientData( new ClientData( list ) );
    downButton->a_HelpText.Set( TXT( "Move the selected item(s) down the list." ) );

    return downButton;
}

void ReflectInterpreter::SequenceOnAdd( const ButtonClickedArgs& args )
{
    Reflect::ObjectPtr clientData = args.m_Control->GetClientData();
    if ( clientData.ReferencesObject() && clientData->IsA( Reflect::GetMetaClass<ClientData>() ) )
    {
        ClientData* data = static_cast< ClientData* >( clientData.Ptr() );
        List* list = static_cast< List* >( data->GetControl() );
        list->e_AddItem.Raise( AddItemArgs() );
        args.m_Control->GetCanvas()->Read();
    }
}

void ReflectInterpreter::SequenceOnRemove( const ButtonClickedArgs& args )
{
#if REFLECT_REFACTOR
    Reflect::ObjectPtr clientData = args.m_Control->GetClientData();
    if ( clientData.ReferencesObject() && clientData->IsA( Reflect::GetMetaClass<ClientData>() ) )
    {
        ClientData* data = static_cast< ClientData* >( clientData.Ptr() );
        List* list = static_cast< List* >( data->GetControl() );
        const std::set< size_t >& selectedItemIndices = list->a_SelectedItemIndices.Get();
        if ( !selectedItemIndices.empty() )
        {
            // for each item in the array to remove (by index)
            std::set< size_t >::const_reverse_iterator itr = selectedItemIndices.rbegin();
            std::set< size_t >::const_reverse_iterator end = selectedItemIndices.rend();
            for ( ; itr != end; ++itr )
            {
                // for each array in the selection set (the objects the array data is connected to)
                std::vector< Data >::const_iterator serItr = m_Datas.begin();
                std::vector< Data >::const_iterator serEnd = m_Datas.end();
                for ( ; serItr != serEnd; ++serItr )
                {
                    Reflect::StlVectorData* arrayData = Reflect::AssertCast<Reflect::StlVectorData>(*serItr);

                    arrayData->Remove( *itr );
                }
            }

            list->a_SelectedItemIndices.Set( std::set< size_t > () );

            args.m_Control->GetCanvas()->Read();
        }
    }
#endif
}

void ReflectInterpreter::SequenceOnMoveUp( const ButtonClickedArgs& args )
{
#if REFLECT_REFACTOR
    Reflect::ObjectPtr clientData = args.m_Control->GetClientData();
    if ( clientData.ReferencesObject() && clientData->IsA( Reflect::GetMetaClass<ClientData>() ) )
    {
        ClientData* data = static_cast< ClientData* >( clientData.Ptr() );
        List* list = static_cast< List* >( data->GetControl() );
        std::set< size_t > selectedItemIndices = list->a_SelectedItemIndices.Get();
        if ( !selectedItemIndices.empty() )
        {
            // for each array in the selection set (the objects the array data is connected to)
            std::vector< Data >::const_iterator serItr = m_Datas.begin();
            std::vector< Data >::const_iterator serEnd = m_Datas.end();
            for ( ; serItr != serEnd; ++serItr )
            {
                Reflect::StlVectorData* arrayData = Reflect::AssertCast<Reflect::StlVectorData>(*serItr);

                arrayData->MoveUp( selectedItemIndices );
            }

            list->a_SelectedItemIndices.Set( selectedItemIndices );

            args.m_Control->GetCanvas()->Read();
        }
    }
#endif
}

void ReflectInterpreter::SequenceOnMoveDown( const ButtonClickedArgs& args )
{
#if REFLECT_REFACTOR
    Reflect::ObjectPtr clientData = args.m_Control->GetClientData();
    if ( clientData.ReferencesObject() && clientData->IsA( Reflect::GetMetaClass<ClientData>() ) )
    {
        ClientData* data = static_cast< ClientData* >( clientData.Ptr() );
        List* list = static_cast< List* >( data->GetControl() );
        std::set< size_t > selectedItemIndices = list->a_SelectedItemIndices.Get();
        if ( !selectedItemIndices.empty() )
        {
            // for each array in the selection set (the objects the array data is connected to)
            std::vector< Data >::const_iterator serItr = m_Datas.begin();
            std::vector< Data >::const_iterator serEnd = m_Datas.end();
            for ( ; serItr != serEnd; ++serItr )
            {
                Reflect::StlVectorData* arrayData = Reflect::AssertCast<Reflect::StlVectorData>(*serItr);

                arrayData->MoveDown( selectedItemIndices );
            }

            list->a_SelectedItemIndices.Set( selectedItemIndices );

            args.m_Control->GetCanvas()->Read();
        }
    }
#endif
}

void ReflectInterpreter::InterpretSetField( const Reflect::Field* field, const std::vector<Reflect::Object*>& instances, Container* parent )
{
    if ( field->m_Flags & Reflect::FieldFlags::Hide )
    {
        return;
    }

    // create the container
    ContainerPtr container = CreateControl< Container >();
    parent->AddChild( container );

    std::string temp;
    field->GetProperty( TXT( "UIName" ), temp );
    if ( temp.empty() )
    {
        bool converted = Helium::ConvertString( field->m_Name, temp );
        HELIUM_ASSERT( converted );
    }

    container->a_Name.Set( temp );

    // create the data objects
	std::vector<Data*> ser;
    std::vector< Reflect::Object* >::const_iterator itr = instances.begin();
    std::vector< Reflect::Object* >::const_iterator end = instances.end();
    for ( ; itr != end; ++itr )
    {
		ser.push_back( new Data ( Pointer( field, *itr ), field->m_Translator ) );
    }

    // create the list
    ListPtr list = CreateControl< List >();
    list->a_HelpText.Set( field->GetProperty( TXT( "HelpText" ) ) );
    container->AddChild( list );

    // bind the ui to the serialiers
    list->Bind( new MultiStringFormatter< Reflect::Data >( ser, true ) );

    // create the buttons if we are not read only
    if ( !( field->m_Flags & Reflect::FieldFlags::ReadOnly ) )
    {
        ContainerPtr buttonContainer = CreateControl< Container >();
        container->AddChild( buttonContainer );

        ButtonPtr buttonAdd = CreateControl< Button >();
        buttonContainer->AddChild( buttonAdd );
        buttonAdd->a_Label.Set( TXT( "Add" ) );
        buttonAdd->a_HelpText.Set( TXT( "Add an item to the list." ) );
        buttonAdd->ButtonClickedEvent().Add( ButtonClickedSignature::Delegate ( this, &ReflectInterpreter::SetOnAdd ) );
        buttonAdd->SetClientData( new ClientData( list ) );

        ButtonPtr buttonRemove = CreateControl< Button >();
        buttonContainer->AddChild( buttonRemove );
        buttonRemove->a_Label.Set( TXT( "Remove" ) );
        buttonRemove->a_HelpText.Set( TXT( "Remove the selected item(s) from the list." ) );
        buttonRemove->ButtonClickedEvent().Add( ButtonClickedSignature::Delegate ( this, &ReflectInterpreter::SetOnRemove ) );
        buttonRemove->SetClientData( new ClientData( list ) );
    }

    // for now let's just disable this container if there is more than one item selected. I'm not sure if it will behave properly in this case.
    if ( instances.size() > 1 )
    {
        container->a_IsEnabled.Set( false );
    }
}

///////////////////////////////////////////////////////////////////////////////
// Callback for when the add button is pressed.  Displays a dialog that lets
// you enter a new key-value pair.  If you enter a key that already exists in
// the list, you will be asked if you want to replace it or not.
// 
void ReflectInterpreter::SetOnAdd( const ButtonClickedArgs& args )
{
    Reflect::ObjectPtr clientData = args.m_Control->GetClientData();
    if ( clientData.ReferencesObject() && clientData->IsA( Reflect::GetMetaClass<ClientData>() ) )
    {
        ClientData* data = static_cast< ClientData* >( clientData.Ptr() );
        List* list = static_cast< List* >( data->GetControl() );
        list->e_AddItem.Raise( AddItemArgs() );
        args.m_Control->GetCanvas()->Read();
    }
}

///////////////////////////////////////////////////////////////////////////////
// Callback for when the remove button is pressed.  If there are any items 
// selected in the list control, they will be removed from the list.
// 
void ReflectInterpreter::SetOnRemove( const ButtonClickedArgs& args )
{
    Reflect::ObjectPtr clientData = args.m_Control->GetClientData();
    if ( clientData.ReferencesObject() && clientData->IsA( Reflect::GetMetaClass<ClientData>() ) )
    {
        ClientData* data = static_cast< ClientData* >( clientData.Ptr() );
        List* list = static_cast< List* >( data->GetControl() );
        const std::set< size_t >& selectedItemIndices = list->a_SelectedItemIndices.Get();
        if ( !selectedItemIndices.empty() )
        {
            // for each item in the array to remove (by index)
            std::set< size_t >::const_reverse_iterator itr = selectedItemIndices.rbegin();
            std::set< size_t >::const_reverse_iterator end = selectedItemIndices.rend();
            for ( ; itr != end; ++itr )
            {
#if REFLECT_REFACTOR
                // for each array in the selection set (the objects the array data is connected to)
                std::vector< Data >::const_iterator serItr = m_Datas.begin();
                std::vector< Data >::const_iterator serEnd = m_Datas.end();
                for ( ; serItr != serEnd; ++serItr )
                {
                    Reflect::StlSetData* setData = Reflect::AssertCast<Reflect::StlSetData>(*serItr);
                    std::vector< Data > items;
                    setData->GetItems( items );
                    setData->RemoveItem( items[ *itr ] );
                }
#endif
            }

            list->a_SelectedItemIndices.Set( std::set< size_t > () );

            args.m_Control->GetCanvas()->Read();
        }
    }
}
