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

			void Interpret(const std::vector<Reflect::Object*>& instances, int32_t includeFlags = 0xFFFFFFFF, int32_t excludeFlags = 0x0, bool expandPanel = true);
			void InterpretType(const std::vector<Reflect::Object*>& instances, Container* parent, int32_t includeFlags = 0xFFFFFFFF, int32_t excludeFlags = 0x0, bool expandPanel = true);

		private:
			void InterpretValueField(const Reflect::Field* field, const std::vector<Reflect::Object*>& instances, Container* parent);
			void InterpretBitfieldField( const Reflect::Field* field, const std::vector<Reflect::Object*>& instances, Container* parent );
			void InterpretColorField( const Reflect::Field* field, const std::vector<Reflect::Object*>& instances, Container* parent );

			void InterpretFilePathField(const Reflect::Field* field, const std::vector<Reflect::Object*>& instances, Container* parent);
			void FilePathDataChanging( const DataChangingArgs& args );
			void FilePathEdit( const ButtonClickedArgs& args );
			FileDialogSignature::Delegate d_FindMissingFile;
			std::string m_FileFilter;

			void InterpretSequenceField(const Reflect::Field* field, const std::vector<Reflect::Object*>& instances, Container* parent);
			ButtonPtr SequenceAddAddButton( List* list );
			ButtonPtr SequenceAddRemoveButton( List* list );
			ButtonPtr SequenceAddMoveUpButton( List* list );
			ButtonPtr SequenceAddMoveDownButton( List* list );
			void SequenceOnAdd( const ButtonClickedArgs& args );
			void SequenceOnRemove( const ButtonClickedArgs& args );
			void SequenceOnMoveUp( const ButtonClickedArgs& args );
			void SequenceOnMoveDown( const ButtonClickedArgs& args );

			void InterpretSetField( const Reflect::Field* field, const std::vector<Reflect::Object*>& instances, Container* parent );
			void SetOnAdd( const ButtonClickedArgs& args );
			void SetOnRemove( const ButtonClickedArgs& args );
		};

		typedef Helium::SmartPtr<ReflectInterpreter> ReflectInterpreterPtr;
	}
}