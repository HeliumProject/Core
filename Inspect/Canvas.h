#pragma once

#include "Inspect/API.h"
#include "Inspect/Container.h"

namespace Helium
{
	namespace Inspect
	{
		struct CanvasShowArgs
		{
			inline CanvasShowArgs(bool show);

			bool m_Show;
		};
		typedef Helium::Signature< const CanvasShowArgs&> CanvasShowSignature;

		class HELIUM_INSPECT_API Canvas : public Container
		{
		public:
			HELIUM_DECLARE_ABSTRACT( Canvas, Container );

			Canvas();
			~Canvas();

			virtual void RealizeControl(Control* control) = 0;
			virtual void UnrealizeControl(Control* control) = 0;

			inline int GetDefaultSize(Axis axis);
			inline int GetBorder();
			inline int GetPad();

			CanvasShowSignature::Event e_Show;

		protected:
			Point m_DefaultSize;  // standard control size
			int   m_Border;       // standard border width
			int   m_Pad;          // standard pad b/t controls
		};
	}
}

#include "Inspect/Canvas.inl"