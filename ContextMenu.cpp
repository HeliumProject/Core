#include "Precompile.h"
#include "Inspect/ContextMenu.h"

#include "Inspect/Container.h"

#include <memory>

using namespace Helium::Inspect;

void ContextMenu::AddItem(const std::string& item, ContextMenuSignature::Delegate delegate)
{
	M_ContextMenuDelegate::iterator found = m_Delegates.find(item);

	if (found == m_Delegates.end())
	{
		m_Items.push_back(item);
		m_Delegates.insert(M_ContextMenuDelegate::value_type(item, delegate));
	}
	else
	{
		found->second = delegate;
	}
}

void ContextMenu::AddSeperator()
{
	m_Items.push_back( "-" );
}