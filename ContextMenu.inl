Helium::Inspect::ContextMenuEventArgs::ContextMenuEventArgs(Control* control, const std::string& item)
	: m_Control (control)
	, m_Item (item)
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
