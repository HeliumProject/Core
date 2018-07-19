#pragma once

#include "Inspect/API.h"
#include "Inspect/ButtonControl.h"

namespace Helium
{
    namespace Inspect
    {
        const static char BUTTON_FILEDIALOG_ATTR_FILTER[] = "filter";
        const static char BUTTON_FILEDIALOG_ATTR_TITLE[] = "caption";

        namespace FileDialogTypes
        {
            enum FileDialogType
            {
                OpenFile,
                SaveFile
            };
        }
        typedef FileDialogTypes::FileDialogType FileDialogType;

        class FileDialogButton;

        struct FileDialogButtonClickedArgs
        {
            FileDialogButtonClickedArgs( FileDialogButton* control, const FileDialogType& type, const std::string& caption, const FilePath& startPath, const std::string& filter )
                : m_Control( control )
                , m_Type( type )
                , m_Caption( caption )
                , m_StartPath( startPath )
                , m_Filter( filter )
            {
            }

            FileDialogButton* m_Control;
            FileDialogType    m_Type;
            std::string           m_Caption;
            FilePath              m_StartPath;
            std::string           m_Filter;
            mutable FilePath      m_Result;
        };
        typedef Helium::Signature< const FileDialogButtonClickedArgs& > FileDialogButtonClickedSignature;

        ///////////////////////////////////////////////////////////////////////////
        // Button control that opens a file browser dialog.
        // 
        class HELIUM_INSPECT_API FileDialogButton : public Inspect::Button
        {
        public:
            HELIUM_DECLARE_CLASS( FileDialogButton, Inspect::Button );

            FileDialogButton( const FileDialogType& type = FileDialogTypes::OpenFile, const std::string& caption = "Open", const std::string& filter = "All files (*.*)|*.*" )
            {
                a_Type.Set( type );
                a_Caption.Set( caption );
                a_Filter.Set( filter );
            }

            virtual bool Write() override
            {
                std::string path;
                ReadStringData( path );
                FilePath startPath( path );

                FileDialogButtonClickedArgs args ( this, a_Type.Get(), a_Caption.Get(), startPath, a_Filter.Get() );
                d_Clicked.Invoke( args );
                WriteStringData( args.m_Result.Get() );
                return true;
            }

            FileDialogButtonClickedSignature::Delegate d_Clicked;

            Attribute< FileDialogType >  a_Type;
            Attribute< std::string >         a_Caption;
            Attribute< std::string >         a_Filter;

        protected:
            virtual bool  Process( const std::string& key, const std::string& value ) override;
        };

        typedef Helium::StrongPtr< FileDialogButton > FileDialogButtonPtr;
    }
}