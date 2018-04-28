#pragma once

#include "Math/Point.h"
#include "Reflect/Object.h"
#include "Foundation/Attribute.h"

#include "Inspect/API.h"
#include "Inspect/DataBinding.h"
#include "Inspect/ContextMenu.h"

namespace Helium
{
	namespace Inspect
	{
		class Control;
		class Container;
		class Canvas;

		const static char ATTR_VALUE_TRUE[]    = "true";
		const static char ATTR_VALUE_FALSE[]   = "false";
		const static char ATTR_HELPTEXT[]      = "helptext";

		//
		// Event Args and Signatures
		//

		typedef Helium::Signature< Control* > ControlSignature;

		struct ControlChangingArgs
		{
			inline ControlChangingArgs( class Control* control, Reflect::Data newValue, bool preview );

			Control*      m_Control;
			Reflect::Data m_NewValue;
			bool          m_Preview;
			mutable bool  m_Veto;
		};
		typedef Helium::Signature<const ControlChangingArgs&> ControlChangingSignature;

		struct ControlChangedArgs
		{
			inline ControlChangedArgs(class Control* control);

			Control* m_Control;
		};
		typedef Helium::Signature< const ControlChangedArgs&> ControlChangedSignature;

		struct PopulateItem
		{
			inline PopulateItem(const std::string& key, const std::string& data);

			std::string m_Key;
			std::string m_Data;
		};

		typedef std::vector<PopulateItem> V_PopulateItem;

		struct PopulateLinkArgs
		{
			inline PopulateLinkArgs(uint32_t type);

			uint32_t        m_Type;
			V_PopulateItem  m_Items;
		};
		typedef Helium::Signature< PopulateLinkArgs&> PopulateLinkSignature;

		struct SelectLinkArgs
		{
			inline SelectLinkArgs(const std::string& id);

			const std::string& m_ID;
		};
		typedef Helium::Signature< const SelectLinkArgs&> SelectLinkSignature;

		struct PickLinkArgs
		{
			inline PickLinkArgs(const DataBindingPtr& data);

			const DataBindingPtr& m_DataBinding;
		};
		typedef Helium::Signature< const PickLinkArgs&> PickLinkSignature;

		//
		// ClientData, this could be toolkit OR interpreter client data, there are two pointer in Control
		//

		class HELIUM_INSPECT_API ClientData : public Reflect::Object
		{
		public:
			HELIUM_DECLARE_ABSTRACT( ClientData, Reflect::Object );

			inline ClientData( Control* control = NULL );

			inline Control* GetControl() const;
			inline void SetControl( Control* control );

		protected:
			Control* m_Control;
		};
		typedef Helium::StrongPtr<ClientData> ClientDataPtr;

		//
		// Widget, a base class for a GUI system implementation-specific Widget classes
		//

		class HELIUM_INSPECT_API Widget : public Reflect::Object
		{
		public:
			HELIUM_DECLARE_ABSTRACT( Widget, Reflect::Object );

			inline Widget();

			inline Control* GetControl() const;
			inline void SetControl( Control* control );

			virtual void Read() = 0;
			virtual bool Write() = 0;

		protected:
			Control* m_Control;
		};
		typedef Helium::StrongPtr<Widget> WidgetPtr;

		//
		// Control, a class that is binadable to data and controls the state and appearance of a widget
		//  Controls own a Widget via a references counter smart pointer
		//  Controls can be created and modified without causing GUI widgets being created)
		//  Widgets are allocated to a Control when Realized, and deleted when Unrealized
		//

		class HELIUM_INSPECT_API Control : public Reflect::Object
		{
		public:
			HELIUM_DECLARE_ABSTRACT( Control, Reflect::Object );
			Control();
			virtual ~Control();

			int GetDepth();

			// every control knows what canvas its on
			inline Canvas* GetCanvas() const;
			void SetCanvas(Canvas* canvas);

			// every control has a parent (which may be the canvas)
			inline Container* GetParent() const;
			void SetParent( Container* parent );

			// the widget is the interface to the actual ui
			inline Widget* GetWidget() const;
			inline void SetWidget( Widget* widget );

			// client data is metadata about this control
			inline ClientData* GetClientData() const;
			inline void SetClientData( ClientData* clientData );

			// get a specified property as a string
			inline const std::string& GetProperty( const std::string& key ) const;

			// access to properties as typed data
			template<class T> bool GetProperty( const std::string& key, T& value ) const;
			template<class T> void SetProperty( const std::string& key, const T& value );

			// data binding governs the data state of the ui
			inline bool IsBound() const;
			inline const DataBinding* GetBinding() const;
			virtual void Bind(const DataBindingPtr& data);

			// queries if value is at default
			virtual bool IsDefault() const;

			// sets data back to default
			virtual bool SetDefault();

			// updates control appearance to appear to be at default value
			virtual void SetDefaultAppearance(bool def) {}

			// Context Menu
			virtual const ContextMenuPtr& GetContextMenu();
			virtual void SetContextMenu(const ContextMenuPtr& contextMenu);

			// process individual attribute key
			virtual bool Process(const std::string& key, const std::string& value);

			// Checks for initialization status
			virtual bool IsRealized();

			// Creates the canvas control, called during layout
			virtual void Realize(Canvas* canvas);

			// Unrealizes the control (delete toolkit object)
			virtual void Unrealize();

			// populated cachend UI state (drop down lists, etc)
			virtual void Populate();

			//
			// Read
			//

			// refreshes the UI state from data
			virtual void Read();

			// helper read call for string based controls
			bool ReadStringData(std::string& str) const;

			// helper read call to get values of all bound data
			bool ReadAllStringData(std::vector< std::string >& strs) const;

			// helper write function for all other types of data
			template<class T>
			bool ReadTypedData(const typename DataBindingTemplate<T>::Ptr& data, T& val);

			// callback when data changed, implements DataReference
			void DataChanged(const DataChangedArgs& args);

			//
			// Write
			//

			// fires callback
			bool PreWrite( Reflect::Data newValue, bool preview );

			// updates the data based on the state of the UI
			virtual bool Write();

			// helper write call for string based controls
			bool WriteStringData(const std::string& str, bool preview = false);

			// helper to write values to each bound data member separately
			bool WriteAllStringData(const std::vector< std::string >& strs, bool preview = false);

			// helper write function for all other types of data
			template<class T>
			bool WriteTypedData(const T& val, const typename DataBindingTemplate<T>::Ptr& data, bool preview = false);

			// fires callback
			void PostWrite();

		public:
			Attribute< bool >                       a_IsEnabled;              // are we enabled?
			Attribute< bool >                       a_IsReadOnly;             // are we writable?
			Attribute< bool >                       a_IsFrozen;               // is updating (polling, sorting, etc) disabled?
			Attribute< bool >                       a_IsHidden;               // is rendering disabled?
			Attribute< uint32_t >                   a_ForegroundColor;        // our colors for appearange
			Attribute< uint32_t >                   a_BackgroundColor;
			Attribute< bool >                       a_IsFixedWidth;           // are we fixed along an axis?
			Attribute< bool >                       a_IsFixedHeight;
			Attribute< float32_t >                  a_ProportionalWidth;      // are we proportional along an axis?
			Attribute< float32_t >                  a_ProportionalHeight;
			Attribute< std::string >                a_Default;                // the default value
			Attribute< std::string >                a_HelpText;               // the help text for this control

			mutable ControlSignature::Event         e_Realized;               // upon realization of the control
			mutable ControlSignature::Event         e_Unrealized;

			mutable ControlChangingSignature::Event e_ControlChanging;        // these mean the *data state* of the control, not the appearance metrics
			mutable ControlChangedSignature::Event  e_ControlChanged;
			mutable PopulateLinkSignature::Event    e_PopulateLink;
			mutable SelectLinkSignature::Event      e_SelectLink;
			mutable PickLinkSignature::Event        e_PickLink;

		protected:
			// our context menu, if any
			ContextMenuPtr      m_ContextMenu;

			// the canvas that implements us
			Canvas*             m_Canvas;

			// the parent
			Container*          m_Parent;

			// writing flag (for re-entrancy checking)
			bool                m_IsWriting;

			// have we really fully realized?
			bool                m_IsRealized;

			// the data we manipulate
			DataBindingPtr      m_DataBinding;

			// GUI toolkit object
			WidgetPtr           m_Widget;

			// client-configurable data
			ClientDataPtr       m_ClientData;

			// Properties System
			mutable std::map< std::string, std::string > m_Properties;
		};

		typedef Helium::StrongPtr<Control> ControlPtr;

		// access to properties as string data
		template<> bool Control::GetProperty( const std::string& key, std::string& value ) const;
		template<> void Control::SetProperty( const std::string& key, const std::string& value );

#if HELIUM_PROFILE_ENABLE
		HELIUM_INSPECT_API extern Profile::Sink g_RealizeSink;
		HELIUM_INSPECT_API extern Profile::Sink g_UnrealizeSink;
#endif
	}
}

#include "Inspect/Control.inl"