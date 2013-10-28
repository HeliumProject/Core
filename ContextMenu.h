#pragma once

#include <map>

#include "Platform/Types.h"
#include "Foundation/Event.h"
#include "Inspect/API.h"

namespace Helium
{
	namespace Inspect
	{
		class HELIUM_INSPECT_API Control;


		//
		// Static context menu support
		//

		struct ContextMenuEventArgs
		{
			inline ContextMenuEventArgs(Control* control, const std::string& item);

			Control*           m_Control;
			const std::string& m_Item;
		};

		// the delegate for a per-item callback to be called upon activation
		typedef Helium::Signature< const ContextMenuEventArgs&> ContextMenuSignature;

		// container for each delegate of each context menu item
		typedef std::map<std::string, ContextMenuSignature::Delegate> M_ContextMenuDelegate;


		//
		// Dynamic context menu support
		//

		class HELIUM_INSPECT_API ContextMenu;
		typedef Helium::SmartPtr<ContextMenu> ContextMenuPtr;

		// popup-time context menu setup delegate
		typedef Helium::Signature< ContextMenuPtr> SetupContextMenuSignature;


		//
		// The menu class
		//

		class HELIUM_INSPECT_API ContextMenu : public Helium::RefCountBase<ContextMenu>
		{
		public:
			inline ContextMenu(Control* control);
			inline ~ContextMenu();

			inline const std::vector< std::string >& GetItems();
			void AddItem(const std::string& item, ContextMenuSignature::Delegate delegate);
			void AddSeperator();

			inline const M_ContextMenuDelegate& GetDelegates();

		private:
			Control*                   m_Control;
			M_ContextMenuDelegate      m_Delegates;
			std::vector< std::string > m_Items;
		};

		typedef Helium::SmartPtr<ContextMenu> ContextMenuPtr;
	}
}

#include "Inspect/ContextMenu.inl"