Helium::Inspect::CanvasShowArgs::CanvasShowArgs(bool show)
	: m_Show(show)
{
}

int Helium::Inspect::Canvas::GetDefaultSize(Axis axis)
{
	return m_DefaultSize[axis];
}

int Helium::Inspect::Canvas::GetBorder()
{
	return m_Border;
}

int Helium::Inspect::Canvas::GetPad()
{
	return m_Pad;
}
