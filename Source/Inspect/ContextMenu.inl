Helium::Inspect::ContextMenuEventArgs::ContextMenuEventArgs(Control* control, const std::string& item)
	: m_Control (control)
	, m_Item (item)
{

}

Helium::Inspect::ContextMenu::ContextMenu(Control* control)
	: m_Control (control)
{
}

Helium::Inspect::ContextMenu::~ContextMenu()
{
}

const std::vector< std::string >& Helium::Inspect::ContextMenu::GetItems()
{
	return m_Items;
}

const Helium::Inspect::M_ContextMenuDelegate& Helium::Inspect::ContextMenu::GetDelegates()
{
	return m_Delegates;
}
