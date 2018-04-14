#pragma once

#include "Platform/Thread.h"
#include "Platform/Locks.h"

#include "Inspect/API.h"
#include "Inspect/DataBinding.h"
#include "Inspect/Controls.h"

#include <stack>

namespace Helium
{
	namespace Inspect
	{
		class HELIUM_INSPECT_API ContainerStackPointer : public ThreadLocalPointer
		{
		public:
			ContainerStackPointer();
			~ContainerStackPointer();

			std::stack< ContainerPtr >& Get();

		private:
			static std::multimap< uint32_t, std::stack< ContainerPtr >* > s_Stacks;
		};

		class HELIUM_INSPECT_API Interpreter : public RefCountBase< Interpreter >
		{
		public:
			Interpreter(Container* container);
			~Interpreter();

			//
			// These helpers provide a pinch point for connecting nested interpreter events into this object's event emitters
			//  It essentially keeps all the events emitted in nested interpreters emitting events in the parent interpreter
			//

			template <class T>
			Helium::StrongPtr<T> CreateControl();

			inline static void ConnectControlEvents( Interpreter* interpreter, Control* control );

			//
			// Panel/container state management
			//

			void Add(Control* control);
			void Push(Container* container);

			Container* GetContainer();
			Container* PushContainer( const std::string& name = "" );
			Container* Pop( bool setParent = true );
			Container* Top();

			//
			// Add controls to the container
			//

			Label* AddLabel(const std::string& name);
			Button* AddButton( const ButtonClickedSignature::Delegate& listener );

			template <class T>
			CheckBox* AddCheckBox( const Helium::SmartPtr< Helium::Property<T> >& property );

			template <class T>
			Value* AddValue( const Helium::SmartPtr< Helium::Property<T> >& property );

			template <class T>
			Choice* AddChoice( const Helium::SmartPtr< Helium::Property<T> >& property );

			template <class T>
			Choice* AddChoice( const Reflect::MetaEnum* enumInfo, const Helium::SmartPtr< Helium::Property<T> >& property );

			template <class T>
			List* AddList( const Helium::SmartPtr< Helium::Property<T> >& property );

			template <class T>
			Slider* AddSlider( const Helium::SmartPtr< Helium::Property<T> >& property );

			template <class T>
			ColorPicker* AddColorPicker( const Helium::SmartPtr< Helium::Property<T> >& property );

		public:
			inline ControlChangingSignature::Event& PropertyChanging() const;
			inline ControlChangedSignature::Event& PropertyChanged() const;
			inline PopulateLinkSignature::Event& PopulateLink() const;
			inline SelectLinkSignature::Event& SelectLink() const;
			inline PickLinkSignature::Event& PickLink() const;

		protected:
			// the container to inject into (these are not long lived hard references)
			//  this only stores pointers to GUI when generating UI (before they are added
			//  to the canvas, which is where the controls live permanently)
			Container* m_Container;

			// context for push/pop api
			ContainerStackPointer m_ContainerStack;

			// the changing event, emitted from Changing()
			mutable ControlChangingSignature::Event m_PropertyChanging;

			// the changed event, emitted from Changed()
			mutable ControlChangedSignature::Event m_PropertyChanged;

			// the find event, handlers should seek and select the contents
			mutable PopulateLinkSignature::Event m_PopulateLink;

			// the select event, handlers should seek and select the item linked by the data
			mutable SelectLinkSignature::Event m_SelectLink;

			// the pick event, handlers should stash data and write descriptor when selection occurs
			mutable PickLinkSignature::Event m_PickLink;
		};

		typedef Helium::SmartPtr<Interpreter> InterpreterPtr;
	}
}

#include "Inspect/Interpreter.inl"