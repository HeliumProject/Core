#pragma once

#include "Inspect/API.h"
#include "Inspect/Control.h"

namespace Helium
{
    namespace Inspect
    {
        const static char LIST_ATTR_SORTED[]    = "sorted";

        namespace MoveDirections
        {
            enum MoveDirection
            {
                Up,
                Down
            };
        }
        typedef MoveDirections::MoveDirection MoveDirection;

        struct AddItemArgs
        {
            AddItemArgs()
            {
            }
        };
        typedef Helium::Signature< const AddItemArgs& > AddItemSignature;

        class HELIUM_INSPECT_API List : public Control
        {
        public:
            HELIUM_DECLARE_CLASS( List, Control );

            List();

        protected:
            virtual bool Process(const std::string& key, const std::string& value) override;
            void SetToDefault(const ContextMenuEventArgs& event);

        public:
            AddItemSignature::Event             e_AddItem;
            Attribute< bool >                   a_IsSorted;
            Attribute< std::set< size_t > >     a_SelectedItemIndices;
        };

        typedef Helium::StrongPtr<List> ListPtr;
    }
}
