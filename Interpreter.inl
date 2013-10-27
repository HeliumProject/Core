template <class T>
Helium::StrongPtr<T> Helium::Inspect::Interpreter::CreateControl()
{
	Helium::StrongPtr<T> control = new T ();
	ConnectControlEvents( this, control );
	return control;
}

void Helium::Inspect::Interpreter::ConnectControlEvents( Interpreter* interpreter, Control* control )
{
	control->e_ControlChanging.AddMethod( &interpreter->PropertyChanging(), &ControlChangingSignature::Event::Raise );
	control->e_ControlChanged.AddMethod( &interpreter->PropertyChanged(), &ControlChangedSignature::Event::Raise );
	control->e_PopulateLink.AddMethod( &interpreter->PopulateLink(), &PopulateLinkSignature::Event::Raise );
	control->e_SelectLink.AddMethod( &interpreter->SelectLink(), &SelectLinkSignature::Event::Raise );
	control->e_PickLink.AddMethod( &interpreter->PickLink(), &PickLinkSignature::Event::Raise );
}

template <class T>
Helium::Inspect::CheckBox* Helium::Inspect::Interpreter::AddCheckBox( const Helium::SmartPtr< Helium::Property<T> >& property )
{
	CheckBoxPtr control = CreateControl<CheckBox>();
	control->Bind( new PropertyStringFormatter<T> ( property ) );
	m_ContainerStack.Get().top()->AddChild(control);
	return control;
}

template <class T>
Helium::Inspect::Value* Helium::Inspect::Interpreter::AddValue( const Helium::SmartPtr< Helium::Property<T> >& property )
{
	ValuePtr control = CreateControl<Value>();
	control->Bind( new PropertyStringFormatter<T> ( property ) );
	m_ContainerStack.Get().top()->AddChild(control);
	return control;
}

template <class T>
Helium::Inspect::Choice* Helium::Inspect::Interpreter::AddChoice( const Helium::SmartPtr< Helium::Property<T> >& property )
{
	ChoicePtr control = CreateControl<Choice>();
	control->Bind( new PropertyStringFormatter<T> ( property ) );
	m_ContainerStack.Get().top()->AddChild(control);
	return control;
}

template <class T>
Helium::Inspect::Choice* Helium::Inspect::Interpreter::AddChoice( const Reflect::MetaEnum* enumInfo, const Helium::SmartPtr< Helium::Property<T> >& property )
{
	Choice* control = AddChoice<T>( property );

	std::vector< ChoiceItem > items;
	DynamicArray< Reflect::MetaEnum::Element >::ConstIterator itr = enumInfo->m_Elements.Begin();
	DynamicArray< Reflect::MetaEnum::Element >::ConstIterator end = enumInfo->m_Elements.End();
	for ( ; itr != end; ++itr )
	{
		std::ostringstream str;
		str << itr->m_Value;
		items.push_back( ChoiceItem ( itr->m_Name, str.str() ) );
	}
	control->a_Items.Set(items);
	control->a_IsDropDown.Set(true);

	return control;        
}

template <class T>
Helium::Inspect::List* Helium::Inspect::Interpreter::AddList( const Helium::SmartPtr< Helium::Property<T> >& property )
{
	ListPtr control = CreateControl<List>();
	control->Bind( new PropertyStringFormatter<T> ( property ) );
	m_ContainerStack.Get().top()->AddChild(control);
	return control;
}

template <class T>
Helium::Inspect::Slider* Helium::Inspect::Interpreter::AddSlider( const Helium::SmartPtr< Helium::Property<T> >& property )
{
	SliderPtr control = CreateControl<Slider>();
	control->Bind( new PropertyStringFormatter<T> ( property ) );
	m_ContainerStack.Get().top()->AddChild(control);
	return control;
}

template <class T>
Helium::Inspect::ColorPicker* Helium::Inspect::Interpreter::AddColorPicker( const Helium::SmartPtr< Helium::Property<T> >& property )
{
	ColorPickerPtr control = CreateControl<ColorPicker>();
	control->Bind( new PropertyStringFormatter<T> ( property ) );
	m_ContainerStack.Get().top()->AddChild(control);
	return control;
}

Helium::Inspect::ControlChangingSignature::Event& Helium::Inspect::Interpreter::PropertyChanging() const
{
	return m_PropertyChanging;
}

Helium::Inspect::ControlChangedSignature::Event& Helium::Inspect::Interpreter::PropertyChanged() const
{
	return m_PropertyChanged;
}

Helium::Inspect::PopulateLinkSignature::Event& Helium::Inspect::Interpreter::PopulateLink() const
{
	return m_PopulateLink;
}

Helium::Inspect::SelectLinkSignature::Event& Helium::Inspect::Interpreter::SelectLink() const
{
	return m_SelectLink;
}

Helium::Inspect::PickLinkSignature::Event& Helium::Inspect::Interpreter::PickLink() const
{
	return m_PickLink;
}