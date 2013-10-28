#pragma once

#include "Inspect/Canvas.h"
#include "Inspect/Container.h"
#include "Inspect/Interpreter.h"

#include "Application/FileDialog.h"

namespace Helium
{
	namespace Inspect
	{
		class Button;
		class List;
		typedef Helium::StrongPtr<Button> ButtonPtr;

		struct EditFilePathArgs
		{
			std::string m_File;

			EditFilePathArgs( const std::string& file )
				: m_File( file )
			{

			}
		};
		typedef Helium::Signature< const EditFilePathArgs&> EditFilePathSignature;

		extern HELIUM_INSPECT_API EditFilePathSignature::Event g_EditFilePath;

		class HELIUM_INSPECT_API ReflectInterpreter : public Interpreter
		{
		public:
			ReflectInterpreter (Container* container);

			void Interpret(
				const std::vector< Reflect::Object* >& objects,
				const Reflect::MetaClass* commonType,
				Container* parent = NULL
				);

			void Interpret(
				const std::vector< void* >& instances,
				const Reflect::MetaStruct* commonType,
				const std::vector< Reflect::Object* >& objects,
				Container* parent = NULL
				);

		private:
			void InterpretField( const std::vector< Reflect::Pointer >& pointers, Reflect::Translator* translator, const Reflect::Field* field, Container* parent );

			void InterpretValueField( const std::vector< Reflect::Pointer >& pointers, Reflect::Translator* translator, const Reflect::Field* field, Container* parent );
			void InterpretBitfieldField( const std::vector< Reflect::Pointer >& pointers, Reflect::Translator* translator, const Reflect::Field* field, Container* parent );
			void InterpretColorField( const std::vector< Reflect::Pointer >& pointers, Reflect::Translator* translator, const Reflect::Field* field, Container* parent );

			void InterpretFilePathField( const std::vector< Reflect::Pointer >& pointers, Reflect::Translator* translator, const Reflect::Field* field, Container* parent );
			void FilePathDataChanging( const DataChangingArgs& args );
			void FilePathEdit( const ButtonClickedArgs& args );
			FileDialogSignature::Delegate d_FindMissingFile;
			std::string m_FileFilter;

			void InterpretSequenceField( const std::vector< Reflect::Pointer >& pointers, Reflect::Translator* translator, const Reflect::Field* field, Container* parent );
			ButtonPtr SequenceAddAddButton( List* list );
			ButtonPtr SequenceAddRemoveButton( List* list );
			ButtonPtr SequenceAddMoveUpButton( List* list );
			ButtonPtr SequenceAddMoveDownButton( List* list );
			void SequenceOnAdd( const ButtonClickedArgs& args );
			void SequenceOnRemove( const ButtonClickedArgs& args );
			void SequenceOnMoveUp( const ButtonClickedArgs& args );
			void SequenceOnMoveDown( const ButtonClickedArgs& args );

			void InterpretSetField( const std::vector< Reflect::Pointer >& pointers, Reflect::Translator* translator, const Reflect::Field* field, Container* parent );
			void SetOnAdd( const ButtonClickedArgs& args );
			void SetOnRemove( const ButtonClickedArgs& args );
		};

		typedef Helium::SmartPtr<ReflectInterpreter> ReflectInterpreterPtr;
	}
}