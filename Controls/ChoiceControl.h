#pragma once

#include "Inspect/API.h"
#include "Inspect/Control.h"

namespace Helium
{
    namespace Inspect
    {
        const static tchar_t CHOICE_ATTR_ENUM[]      = TXT( "enum" );
        const static tchar_t CHOICE_ATTR_SORTED[]    = TXT( "sorted" );
        const static tchar_t CHOICE_ATTR_DROPDOWN[]  = TXT( "dropdown" );
        const static tchar_t CHOICE_ATTR_PREFIX[]    = TXT( "prefix" );

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
            ChoiceEnumerateArgs(Choice* choice, const tstring& enumeration)
                : ChoiceArgs (choice)
                , m_Enumeration (enumeration)
            {

            }

            tstring m_Enumeration;
        };
        typedef Helium::Signature< const ChoiceEnumerateArgs&> ChoiceEnumerateSignature;

        struct ChoiceItem
        {
            ChoiceItem(const tstring& key = TXT(""), const tstring& data = TXT(""))
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

            tstring m_Key;
            tstring m_Data;
        };

        class HELIUM_INSPECT_API Choice : public Control
        {
        public:
            REFLECT_DECLARE_OBJECT( Choice, Control );

            Choice();

            virtual bool Process(const tstring& key, const tstring& value) HELIUM_OVERRIDE;
            virtual void SetDefaultAppearance(bool def) HELIUM_OVERRIDE;
            void SetToDefault(const ContextMenuEventArgs& event);

            const tstring& GetPrefix()
            {
                return m_Prefix;
            }

            bool Contains(const tstring& data);
            void Clear();

            virtual void Populate() HELIUM_OVERRIDE;

            Attribute< bool >                       a_Highlight;
            Attribute< bool >                       a_IsSorted;
            Attribute< bool >                       a_IsDropDown;
            Attribute< bool >                       a_EnableAdds;
            Attribute< std::vector< ChoiceItem > >  a_Items;

            ChoiceSignature::Event                  e_Populate;
            ChoiceEnumerateSignature::Event         e_Enumerate;

        private:
            tstring             m_Enum;
            tstring             m_Prefix;
        };

        typedef Helium::StrongPtr<Choice> ChoicePtr;
    }
}
