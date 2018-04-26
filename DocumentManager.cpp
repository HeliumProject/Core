#include "Precompile.h"
#include "DocumentManager.h"

#include "Platform/Assert.h"
#include "Foundation/Flags.h"
#include "Foundation/Log.h"
#include "Application/RCS.h"

#include <algorithm>
#include <cctype>
#include <sstream>

using namespace Helium;

Document::Document( const std::string& path )
	: m_Path( path )
	, m_DocumentStatus( DocumentStatus::Default )
	, m_AllowUnsavableChanges( false )
	, m_Revision( -1 )
{
	UpdateRCSFileInfo();
}

Document::~Document()
{
}

///////////////////////////////////////////////////////////////////////////////
bool Document::Save( std::string& error )
{
	SetFlag<uint32_t>( m_DocumentStatus, DocumentStatus::Saving, true );

	bool result = false;

	DocumentEventArgs savingArgs( this );
	e_Saving.Raise( savingArgs );
	if ( !savingArgs.m_Veto )
	{
		DocumentEventArgs saveArgs( this, &error );
		d_Save.Invoke( saveArgs );
		if ( saveArgs.m_Result )
		{
			SetFlag<uint32_t>( m_DocumentStatus, DocumentStatus::Saving, false );

			e_Saved.Raise( DocumentEventArgs( this ) );

			HasChanged( false );

			result = true;
		}
	}

	SetFlag<uint32_t>( m_DocumentStatus, DocumentStatus::Saving, false );

	return result;
}

///////////////////////////////////////////////////////////////////////////////
void Document::Close()
{
	e_Closing.Raise( DocumentEventArgs( this ) );

	d_Close.Invoke( DocumentEventArgs( this ) );

	e_Closed.Raise( DocumentEventArgs( this ) );
}

///////////////////////////////////////////////////////////////////////////////
void Document::Checkout() const
{
	e_CheckedOut.Raise( DocumentEventArgs( this ) );
}

///////////////////////////////////////////////////////////////////////////////
// Sets the path to this file.  The name of the file is also updated.  Notifies
// any interested listeners about this event.
// 
void Document::SetPath( const Helium::FilePath& newPath )
{
	Helium::FilePath oldPath( m_Path.Get() );

	m_Path = newPath;
	UpdateRCSFileInfo();

	e_PathChanged.Raise( DocumentPathChangedArgs( this, oldPath ) );
}

///////////////////////////////////////////////////////////////////////////////
uint32_t Document::GetStatus() const
{
	return m_DocumentStatus;
}

///////////////////////////////////////////////////////////////////////////////
bool Document::HasChanged() const
{
	return HasFlags<uint32_t>( m_DocumentStatus, DocumentStatus::Changed );
}

///////////////////////////////////////////////////////////////////////////////
// Sets the internal flag indicating the the file has been modified (thus it
// should probably be saved before closing).
// 
void Document::HasChanged( bool changed )
{
	if ( HasFlags<uint32_t>( m_DocumentStatus, DocumentStatus::Changed ) != changed )
	{
		SetFlag<uint32_t>( m_DocumentStatus, DocumentStatus::Changed, changed );

		e_Changed.Raise( DocumentEventArgs( this ) );
	}
}

void Document::OnObjectChanged( const DocumentObjectChangedArgs& args )
{
	HasChanged( args.m_HasChanged );
}

///////////////////////////////////////////////////////////////////////////////
// Returns true if the file is currently checked out by this user.
// 
bool Document::IsCheckedOut() const
{
	std::string path ( GetPath().Get() );
	if ( !path.empty() && RCS::PathIsManaged( path ) )
	{
		RCS::File rcsFile( path );

		try
		{
			rcsFile.GetInfo();
		}
		catch ( Helium::Exception& ex )
		{
			std::stringstream str;
			str << "Unable to get info for '" << path << "': " << ex.What();
			Log::Error( "%s\n", str.str().c_str() );
			// TODO: Should trigger RCS error status event
		}

		return rcsFile.IsCheckedOutByMe();
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Checks to see if the local revision of this file is the same as the head
// revision, and returns true if so.
// 
bool Document::IsUpToDate() const
{
	std::string path ( GetPath().Get() );
	if ( !path.empty() )
	{
		if ( RCS::PathIsManaged( path ) )
		{
			RCS::File rcsFile( path );

			try
			{
				rcsFile.GetInfo();
			}
			catch ( Helium::Exception& ex )
			{
				std::stringstream str;
				str << "Unable to get info for '" << path << "': " << ex.What();
				Log::Error( "%s\n", str.str().c_str() );
				// TODO: Should trigger RCS error status event
			}

			if ( rcsFile.ExistsInDepot() )
			{
				return rcsFile.IsUpToDate() && ( rcsFile.m_LocalRevision == GetRevision() );
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Returns true if the user has specified that they want to make changes to
// this file even if it is not checked out by them.
// 
bool Document::AllowUnsavableChanges() const
{
	return m_AllowUnsavableChanges;
}

///////////////////////////////////////////////////////////////////////////////
// Sets whether to allow changes regardless of file check out state.
// 
void Document::AllowUnsavableChanges( bool allowUnsavableChanges )
{
	m_AllowUnsavableChanges = allowUnsavableChanges;
}

///////////////////////////////////////////////////////////////////////////////
void Document::UpdateRCSFileInfo()
{
	m_Revision = -1;

	if ( !m_Path.Get().empty() )
	{
		if ( RCS::PathIsManaged( m_Path.Get() ) )
		{
			RCS::File rcsFile( m_Path.Get() );

			try
			{
				rcsFile.GetInfo();
			}
			catch ( Helium::Exception& ex )
			{
				std::stringstream str;
				str << "Unable to get info for '" << m_Path.Get() << "': " << ex.What();
				Log::Warning( "%s\n", str.str().c_str() );
			}

			m_Revision = rcsFile.m_LocalRevision;
		}
	}
}

DocumentManager::DocumentManager( MessageSignature::Delegate message, FileDialogSignature::Delegate fileDialog )
	: m_Message( message )
	, m_FileDialog( fileDialog )
{
}

///////////////////////////////////////////////////////////////////////////////
// Returns the first document found with the specified path.
// 
Document* DocumentManager::FindDocument( const Helium::FilePath& path ) const
{
	OS_DocumentSmartPtr::Iterator docItr = m_Documents.Begin();
	OS_DocumentSmartPtr::Iterator docEnd = m_Documents.End();
	for ( ; docItr != docEnd; ++docItr )
	{
		Document* document = *docItr;
		if ( document->GetPath() == path )
		{
			return document;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Returns true if it successfully opens a document with the specified path.
// 
bool DocumentManager::OpenDocument( const DocumentPtr& document, std::string& error )
{
	if ( !document->GetPath().Empty() )
	{
		if ( FindDocument( document->GetPath() ) )
		{
			error = "The specified file (" + document->GetPath().Filename().Get() + ") is already open.";
			return false;
		}

		if ( !document->IsUpToDate() )
		{
			error = "The version of '" + document->GetPath().Filename().Get() + "' on your computer is out of date.  You will not be able to check it out.";
			return false;
		}
	}

	AddDocument( document );
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Adds a document to the list managed by this object.  Returns false if the 
// document was already in the list belonging to this manager.
// 
bool DocumentManager::AddDocument( const DocumentPtr& document )
{
	if ( m_Documents.Append( document ) )
	{
		document->e_Closed.AddMethod( this, &DocumentManager::OnDocumentClosed );

		e_DocumentOpened.Raise( DocumentEventArgs( document ) );

		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
// Iterates over all the documents, calling save on each one.
// 
bool DocumentManager::SaveAll( std::string& error )
{
	bool savedAll = true;
	bool prompt = true;
	bool dirtyDocuments = true;

	// here, we loop until we couldn't save any more documents (or there was a failure to save a document),
	// because saving a document can cause other documents to need saving again
	while ( dirtyDocuments && savedAll )
	{
		dirtyDocuments = false;
		OS_DocumentSmartPtr::Iterator docItr = m_Documents.Begin();
		OS_DocumentSmartPtr::Iterator docEnd = m_Documents.End();
		for ( ; docItr != docEnd; ++docItr )
		{
			Document* document = *docItr;

			bool abort = false;
			bool save = true;
			if ( prompt )
			{
				switch ( QuerySave( document ) )
				{
				case SaveActions::SaveAll:
					save = true;
					prompt = false;
					break;

				case SaveActions::Save:
					save = true;
					prompt = true;
					break;

				case SaveActions::Skip:
					save = false;
					prompt = true;
					break;

				case SaveActions::SkipAll:
					save = false;
				case SaveActions::Abort:
				default:
					abort = true;
					break; 
				}
			}

			if ( abort )
			{
				break;
			}

			if ( save )
			{
				dirtyDocuments = true;

				std::string msg;
				if ( !SaveDocument( document, msg ) )
				{
					savedAll = false;
					if ( !error.empty() )
					{
						error += "\n";
					}
					error += msg;
				}
			}
		}
	}

	return savedAll;
}

///////////////////////////////////////////////////////////////////////////////
// Saves the specified document and returns true if successful.
//
// Derived classes should override this function to actually perform saving 
// data to disk as appropriate.  The base implementation fires the appropriate
// events.  A derived class may want to call this implementation if the save
// is successful.
//
bool DocumentManager::SaveDocument( DocumentPtr document, std::string& error )
{
	// Check for "save as"
	if ( document->GetPath().Empty() || !document->GetPath().IsAbsolute() )
	{
		std::string filters = document->GetPath().Extension() + "|*." + document->GetPath().Extension() + "|All Files|*";
		FileDialogArgs args ( FileDialogTypes::SaveFile, "Save As...", filters, FilePath( document->GetPath().Directory() ), FilePath( document->GetPath().Filename() ) );
		m_FileDialog.Invoke( args );
		if ( !args.m_Result.Empty() )
		{
			document->SetPath( args.m_Result );
		}
		else
		{
			error = "Cancelled saving of document.";
			return false;
		}
	}

	if ( document->Save( error ) )
	{
		return true;
	}

	if ( error.empty() )
	{
		error = "Failed to save " + document->GetPath().Filename().Get();
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
// Closes all currently open documents.
// 
bool DocumentManager::CloseAll()
{
	return CloseDocuments( m_Documents );
}

///////////////////////////////////////////////////////////////////////////////
// Iterates over all the documents, closing each one.  The user will be prompted
// to save any modified documents before closing them.  Returns true if all 
// documents were successfully closed.
// 
bool DocumentManager::CloseDocuments( OS_DocumentSmartPtr documents )
{
	// NOTE: We purposefully make a copy of the files to iterate over
	// because the original list may be changing as we close files,
	// which will invalidate our iterator.

	if ( documents.Empty() )
	{
		return true;
	}

	bool prompt = true;
	bool save = true;
	bool abort = false;

	OS_DocumentSmartPtr::ReverseIterator docItr = documents.ReverseBegin();
	OS_DocumentSmartPtr::ReverseIterator docEnd = documents.ReverseEnd();
	for ( ; docItr != docEnd && !abort; ++docItr )
	{
		const DocumentPtr& document = *docItr;

		if ( prompt )
		{
			SaveActions::SaveAction action = SaveActions::Abort;
			if ( documents.Size() == 1 )
			{
				action = QueryClose( document );
			}
			else
			{
				action = QueryCloseAll( document );
			}

			switch ( action )
			{
			case SaveActions::SaveAll:
				prompt = false;
				save = true;
				break;

			case SaveActions::Save:
				prompt = true;
				save = true;
				break;

			case SaveActions::SkipAll:
				prompt = false;
				save = false;
				break;

			case SaveActions::Skip:
				prompt = true;
				save = false;
				break;

			case SaveActions::Abort:
				abort = true;
				break;
			}
		}

		if ( abort )
		{
			return false;
		}

		if ( save )
		{
			if ( RCS::PathIsManaged( document->GetPath().Get() ) )
			{
				if ( !CheckOut( document ) )
				{
					return false;
				}
			}

			std::string error;
			if ( !SaveDocument( document, error ) )
			{
				error += "\nAborting operation.";
				m_Message.Invoke( MessageArgs( "Error", error, MessagePriorities::Error, MessageAppearances::Ok ) );
				return false;
			}
		}

		if ( !CloseDocument( document, false ) )
		{
			std::string error;
			error = "Failed to close '" + document->GetPath().Filename().Get() + "'.  Aborting operation.";
			m_Message.Invoke( MessageArgs( "Error", error, MessagePriorities::Error, MessageAppearances::Ok ) );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Closes the specified file.  Determines if the file needs to be saved, and
// handles any other revision control interactions.  Notifies any interested 
// listeners if the file is successfully closed.  If prompt is set to true, 
// the user will be prompted to save their file before it is closed.
// 
// Fires an event to notify listeners that this document is now closed.  Any
// objects that are holding pointers to this document should release them.
//
bool DocumentManager::CloseDocument( DocumentPtr document, bool prompt )
{
	HELIUM_ASSERT( document.ReferencesObject() );

	bool shouldClose = !prompt;
	bool wasClosed = false;

	if ( prompt )
	{
		std::string unused;
		switch ( QueryClose( document ) )
		{
		case SaveActions::Save:
			shouldClose = SaveDocument( document, unused );
			break;

		case SaveActions::Skip:
			shouldClose = true;
			break;

		case SaveActions::Abort:
			shouldClose = false;
			break;

		case SaveActions::SaveAll:
		case SaveActions::SkipAll:
			break;
		}
	}

	if ( shouldClose )
	{
		// This will raise the e_Closed event when the document is finished closing and 
		// callback to OnDocumentClosed, which will remove the document from m_Documents
		document->Close();

		wasClosed = true;
	}

	return wasClosed;
}

///////////////////////////////////////////////////////////////////////////////
// Removes the document from the list managed by this object.
// NOTE: The document may be deleted if no one is holding a smart pointer to it
// after this function is called.
// 
bool DocumentManager::RemoveDocument( const DocumentPtr& document )
{
	document->e_Closed.RemoveMethod( this, &DocumentManager::OnDocumentClosed );

	if ( m_Documents.Remove( document ) )
	{
		e_DocumenClosed.Raise( DocumentEventArgs( document ) );
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
// Callback for when a document is closed.  Removes the document from the list
// managed by this class.
// 
void DocumentManager::OnDocumentClosed( const DocumentEventArgs& args )
{
	RemoveDocument( args.m_Document );
}

///////////////////////////////////////////////////////////////////////////////
// Call this function if the user fails to checkout a file, or attempts to edit
// a file without checking it out.  The user is prompted as to whether they want
// to edit the file (even though they won't be able to save).  Returns true if the
// user wants to allow changes, false otherwise.
// 
bool DocumentManager::QueryAllowChanges( Document* document ) const
{
	if ( !document->AllowUnsavableChanges() && !document->IsCheckedOut() )
	{
		QueryCheckOut( document );
		if ( !document->IsCheckedOut() )
		{
			MessageArgs args ( "Edit anyway?",
				"Would you like to edit this file anyway?\n(NOTE: You may not be able to save your changes)",
				MessagePriorities::Question,
				MessageAppearances::YesNo );

			m_Message.Invoke( args );

			if ( MessageResults::Yes == args.m_Result )
			{
				document->AllowUnsavableChanges( true );
			}
		}
	}

	return document->AllowUnsavableChanges() || document->IsCheckedOut();
}

///////////////////////////////////////////////////////////////////////////////
// Returns true if this file is allowed to be changed.  The file is allowed to 
// be changed if the user has it checked out, or if they chose to edit the file
// anyway when their attempt to check out the file failed.  See QueryAllowChanges.
// 
bool DocumentManager::AllowChanges( Document* document ) const
{
	if ( document->IsCheckedOut() || document->AllowUnsavableChanges() )
	{
		return true;
	}

	return QueryAllowChanges( document );
}

///////////////////////////////////////////////////////////////////////////////
// Asks the user if they want to check out the file.  Returns true if the user
// opts not to check out the file.  Also returns true if the user chooses to 
// check out the file, and it is successfully checked out.  Use this function
// when opening a file to see if it should be opened for edit.
// 
bool DocumentManager::QueryCheckOut( Document* document ) const
{
	if ( !document->IsUpToDate() )
	{
		std::ostringstream str;
		str << "The version of " << document->GetPath().Filename().Get() << " on your computer is out of date.  You will not be able to check it out.";
		m_Message.Invoke( MessageArgs( "Warning", str.str(), MessagePriorities::Warning, MessageAppearances::Ok ) );
	}
	else
	{
		if ( !document->IsCheckedOut() )
		{
			std::ostringstream str;
			str << "Do you wish to check out " << document->GetPath().Filename().Get() << "?";

			MessageArgs args ( "Check Out?", str.str(), MessagePriorities::Question, MessageAppearances::YesNo );
			m_Message.Invoke( args );
			if ( MessageResults::Yes == args.m_Result )
			{
				return CheckOut( document );
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Checks a file out from revision control.  Returns false if unable to check
// out the file.
// 
bool DocumentManager::CheckOut( Document* document ) const
{
	std::string path ( document->GetPath().Get() );

	if ( !RCS::PathIsManaged( path ) )
	{
		return true;
	}

	RCS::File rcsFile( path );

	try
	{
		rcsFile.GetInfo();
	}
	catch ( Helium::Exception& ex )
	{
		std::stringstream str;
		str << "Unable to get info for '" << document->GetPath().Filename().Get() << "': " << ex.What();
		m_Message.Invoke( MessageArgs( "Error", str.str(), MessagePriorities::Error, MessageAppearances::Ok ) );
		return false;
	}

	if ( rcsFile.ExistsInDepot() && !rcsFile.IsUpToDate() )
	{
		std::ostringstream str;
		str << "The version of " << document->GetPath().Filename().Get() << " on your computer is out of date.  You will not be able to check it out.";
		m_Message.Invoke( MessageArgs( "Warning", str.str(), MessagePriorities::Warning, MessageAppearances::Ok ) );
		return false;
	}

	if ( rcsFile.IsCheckedOutByMe() )
	{
		return true;
	}
	else if ( rcsFile.IsCheckedOutBySomeoneElse() )
	{
		std::string usernames;
		rcsFile.GetOpenedByUsers( usernames );

		std::ostringstream str;
		str << "Unable to check out " << document->GetPath().Filename().Get() << ", it's currently checked out by " << usernames << ".";
		m_Message.Invoke( MessageArgs( "Error", str.str(), MessagePriorities::Error, MessageAppearances::Ok ) );
		return false;
	}

	try
	{
		rcsFile.Open();
	}
	catch ( Helium::Exception& ex )
	{
		std::stringstream str;
		str << "Unable to open '" << document->GetPath().Filename().Get() << "': " << ex.What();
		m_Message.Invoke( MessageArgs( "Error", str.str(), MessagePriorities::Error, MessageAppearances::Ok ) );
		return false;
	}

	document->Checkout();

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Returns a flag indicating whether or not to continue the open process 
// (not the checkout status).
// 
bool DocumentManager::QueryOpen( Document* document ) const
{
	std::string path ( document->GetPath().Get() );
	if ( RCS::PathIsManaged( path ) )
	{
		RCS::File rcsFile( path );
		try
		{
			rcsFile.GetInfo();
		}
		catch ( Helium::Exception& ex )
		{
			std::stringstream str;
			str << "Unable to get info for '" << document->GetPath().Filename().Get() << "': " << ex.What();
			m_Message.Invoke( MessageArgs( "Error", str.str(), MessagePriorities::Error, MessageAppearances::Ok ) );
		}

		// Is the file already managed?
		if ( rcsFile.ExistsInDepot() )
		{
			if ( rcsFile.IsCheckedOut() && !rcsFile.IsCheckedOutByMe() )
			{
				std::string usernames;
				rcsFile.GetOpenedByUsers( usernames );

				std::ostringstream str;
				std::string capitalized = document->GetPath().Filename().Get();
				capitalized[0] = toupper( capitalized[0] );
				str << capitalized << " is already checked out by \"" << usernames << "\"\nDo you still wish to open the file?";

				MessageArgs args ( "Checked Out by Another User", str.str(), MessagePriorities::Question, MessageAppearances::YesNo );
				m_Message.Invoke( args );
				if ( MessageResults::Yes == args.m_Result )
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return QueryCheckOut( document );
			}
		}
		// Should the file be added to a pending changelist?
		else
		{
			if ( !QueryAdd( document ) )
			{
				std::ostringstream str;
				str << "Unable to add " << document->GetPath().Filename().Get() << " to revision control.  Would you like to continue opening the file?";

				MessageArgs args ( "Continue Opening?", str.str(), MessagePriorities::Question, MessageAppearances::YesNo );
				m_Message.Invoke( args );
				if ( MessageResults::Yes == args.m_Result )
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			return true;
		}
	}

	// File is not in source control, and shouldn't be (perhaps it's on the user's desktop
	// or something, so just let it be opened).
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Checks to see if this file should be in revision control.  If the file should
// be in revision control, but isn't, the user is prompted to add the file.
// Returns false if the user attempts to add a file, but it fails for some reason.
// 
bool DocumentManager::QueryAdd( Document* document ) const
{
	bool isOk = true;
	std::string path ( document->GetPath().Get() );
	if ( RCS::PathIsManaged( path ) )
	{
		// Is the file already managed?
		RCS::File rcsFile( path );
		rcsFile.GetInfo();

		if ( !rcsFile.ExistsInDepot() )
		{
			std::ostringstream msg;
			msg << "Would you like to add \"" << document->GetPath().Filename().Get() << "\" to revision control?";

			MessageArgs args ( "Add to Revision Control?", msg.str(), MessagePriorities::Question, MessageAppearances::YesNo );
			m_Message.Invoke( args );
			if ( MessageResults::Yes == args.m_Result )
			{
				try
				{
					rcsFile.Open();
				}
				catch ( Helium::Exception& ex )
				{
					std::stringstream str;
					str << "Unable to open '" << document->GetPath().Filename().Get() << "': " << ex.What();
					m_Message.Invoke( MessageArgs( "Error", str.str(), MessagePriorities::Error, MessageAppearances::Ok ) );
					isOk = false;
				}
			}
		}
	}
	return isOk;
}

///////////////////////////////////////////////////////////////////////////////
// Opens a dialog prompting the user whether they want to save the current file
// or not.  The user has the option to specifiy that they want the same action
// taken for all subsequent files.
// 
SaveAction DocumentManager::QueryCloseAll( Document* document ) const
{
	if ( document->HasChanged() )
	{
		bool attemptCheckOut = false;
		SaveActions::SaveAction action = SaveActions::Save;

		std::ostringstream msg;
		msg << "You are attempting to close file " << document->GetPath().Filename().Get() << " which has changed. Would you like to save your changes before closing?";       

		MessageArgs args ( "Save Changes?", msg.str(), MessagePriorities::Question, MessageAppearances::YesNoCancelToAll );
		m_Message.Invoke( args );
		switch ( args.m_Result )
		{
		case MessageResults::Yes:
			action = SaveActions::Save;
			attemptCheckOut = true;
			break;

		case MessageResults::YesToAll:
			action = SaveActions::SaveAll;
			attemptCheckOut = true;
			break;

		case MessageResults::No:
			action = SaveActions::Skip;
			break;

		case MessageResults::NoToAll:
			action = SaveActions::SkipAll;
			break;

		case MessageResults::Cancel:
			action = SaveActions::Abort;
			break;
		}

		if ( attemptCheckOut )
		{
			if ( RCS::PathIsManaged( document->GetPath().Get() ) )
			{
				if ( !CheckOut( document ) )
				{
					action = SaveActions::Abort;
				}
			}
		}

		return action;
	}

	return QueryClose( document );
}

///////////////////////////////////////////////////////////////////////////////
// Prompts for whether or not to save the file before closing.
// 
SaveAction DocumentManager::QueryClose( Document* document ) const
{
	if ( !document->HasChanged() )
	{
		return SaveActions::Skip;
	}

	if ( document->IsCheckedOut() || !RCS::PathIsManaged( document->GetPath().Get() ) )
	{
		std::string msg( "Would you like to save changes to " );
		msg += "'" + document->GetPath().Filename().Get() + "' before closing?";

		MessageArgs args ( "Save Changes?", msg.c_str(), MessagePriorities::Question, MessageAppearances::YesNoCancel );
		m_Message.Invoke( args );
		switch ( args.m_Result )
		{
		case MessageResults::Yes:
			return SaveActions::Save;
			break;

		case MessageResults::No:
			return SaveActions::Skip;
			break;

		case MessageResults::Cancel:
		default:
			return SaveActions::Abort;
		}
	}

	return QuerySave( document );
}

///////////////////////////////////////////////////////////////////////////////
// Returns a value indicating whether the save operation should continue or not
// (not the checkout status).
// 
SaveAction DocumentManager::QuerySave( Document* document ) const
{
	if ( !document->HasChanged() )
	{
		return SaveActions::Skip;
	}

	if ( !RCS::PathIsManaged( document->GetPath().Get() ) )
	{
		return SaveActions::Save;
	}

	if ( !document->IsCheckedOut() )
	{
		if ( document->HasChanged() )
		{
			std::string msg;

			if ( !document->IsUpToDate() )
			{
				msg = "Unfortunately, the file '" + document->GetPath().Filename().Get() + "' has been modified in revsion control since you opened it.\n\nYou cannot save the changes you have made.\n\nTo fix this:\n1) Close the file\n2) Get updated assets\n3) Make your changes again\n\nSorry for the inconvenience.";
				m_Message.Invoke( MessageArgs( "Cannot save", msg, MessagePriorities::Error, MessageAppearances::Ok ) );
				return SaveActions::Skip;
			}

			msg = "File '" + document->GetPath().Filename().Get() + "' has been changed, but is not checked out.  Would you like to check out and save this file?";

			MessageArgs args ( "Check out and save?", msg.c_str(), MessagePriorities::Question, MessageAppearances::YesNo );
			m_Message.Invoke( args );
			if ( MessageResults::No == args.m_Result )
			{
				return SaveActions::Skip;
			}
		}

		if ( !CheckOut( document ) )
		{
			return SaveActions::Abort;
		}
	}

	// File was already checked out, or was successfully checked out
	return SaveActions::Save;
}

