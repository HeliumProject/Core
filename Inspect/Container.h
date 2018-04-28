#pragma once

#include "Inspect/API.h"
#include "Inspect/Control.h"

namespace Helium
{
	namespace Inspect
	{
		const static char CONTAINER_ATTR_NAME[] = "name";
		const static char CONTAINER_ATTR_ICON[] = "icon";

		namespace UIHint
		{
			enum UIHints
			{
				Advanced = 1 << 0,
				Popup = 1 << 1,
			};

			const uint32_t Default = 0;
		}
		typedef uint32_t UIHints;

		//
		// Contains other controls and distributes layout logic
		//

		class HELIUM_INSPECT_API Container : public Control
		{
		public:
			HELIUM_DECLARE_CLASS( Container, Control );

			Container();
			~Container();

			inline const std::vector< ControlPtr >& GetChildren() const;
			inline std::vector< ControlPtr > ReleaseChildren();

			virtual void AddChild( Control* control );
			virtual void InsertChild( int index, Control* control );
			virtual void RemoveChild( Control* control );
			virtual void Clear();

			inline const std::string& GetPath() const;
			inline void BuildPath(std::string& path) const;

			inline UIHints GetUIHints() const;
			inline void SetUIHints( const UIHints hints );

			// recusively binds contained controls to data
			virtual void Bind(const DataBindingPtr& data) override;

			// process properties coming from script
			virtual bool Process(const std::string& key, const std::string& value) override;

			// populate
			virtual void Populate() override;

			// refreshes the UI state from data
			virtual void Read() override;

			// updates the data based on the state of the UI
			virtual bool Write() override;

			Attribute< std::string > a_Name;
			Attribute< std::string > a_Icon;

			mutable ControlSignature::Event     e_ControlAdded;
			mutable ControlSignature::Event     e_ControlRemoved;

		private:
			void IsEnabledChanged( const Attribute<bool>::ChangeArgs& args );
			void IsReadOnlyChanged( const Attribute<bool>::ChangeArgs& args );
			void IsFrozenChanged( const Attribute<bool>::ChangeArgs& args );
			void IsHiddenChanged( const Attribute<bool>::ChangeArgs& args );

		protected:
			// the children controls
			std::vector< ControlPtr > m_Children;

			// the path of the container
			mutable std::string m_Path;

			UIHints m_UIHints;
		};

		typedef Helium::StrongPtr<Container> ContainerPtr;
	}
}

#include "Inspect/Container.inl"