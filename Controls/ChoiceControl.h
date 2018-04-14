#pragma once

#include "Inspect/API.h"
#include "Inspect/Control.h"

namespace Helium
{
    namespace Inspect
    {
        const static char CHOICE_ATTR_ENUM[]      = "enum";
        const static char CHOICE_ATTR_SORTED[]    = "sorted";
        const static char CHOICE_ATTR_DROPDOWN[]  = "dropdown";
        const static char CHOICE_ATTR_PREFIX[]    = "prefix";

        class Choice;

        struct ChoiceArgs
        {
            ChoiceArgs(Choice* choice)
                : m_Choice (choice)
            {

            }

            Choice* m_Choice;
        };
        typedef Helium::Signature< const ChoiceArgs&> ChoiceSignature;

        struct ChoiceEnumerateArgs : public ChoiceArgs
        {
            ChoiceEnumerateArgs(Choice* choice, const std::string& enumeration)
                : ChoiceArgs (choice)
                , m_Enumeration (enumeration)
            {

            }

            std::string m_Enumeration;
        };
        typedef Helium::Signature< const ChoiceEnumerateArgs&> ChoiceEnumerateSignature;

        struct ChoiceItem
        {
            ChoiceItem(const std::string& key = "", const std::string& data = "")
                : m_Key( key )
                , m_Data( data )
            {

            }

            bool operator==( const ChoiceItem& rhs ) const
            {
                return rhs.m_Key == m_Key && rhs.m_Data == m_Data;
            }

            bool operator!=( const ChoiceItem& rhs ) const
            {
                return !operator==( rhs );
            }

            std::string m_Key;
            std::string m_Data;
        };

        class HELIUM_INSPECT_API Choice : public Control
        {
        public:
            HELIUM_DECLARE_CLASS( Choice, Control );

            Choice();

            virtual bool Process(const std::string& key, const std::string& value) override;
            virtual void SetDefaultAppearance(bool def) override;
            void SetToDefault(const ContextMenuEventArgs& event);

            const std::string& GetPrefix()
            {
                return m_Prefix;
            }

            bool Contains(const std::string& data);
            void Clear();

            virtual void Populate() override;

            Attribute< bool >                       a_Highlight;
            Attribute< bool >                       a_IsSorted;
            Attribute< bool >                       a_IsDropDown;
            Attribute< bool >                       a_EnableAdds;
            Attribute< std::vector< ChoiceItem > >  a_Items;

            ChoiceSignature::Event                  e_Populate;
            ChoiceEnumerateSignature::Event         e_Enumerate;

        private:
            std::string             m_Enum;
            std::string             m_Prefix;
        };

        typedef Helium::StrongPtr<Choice> ChoicePtr;
    }
}
