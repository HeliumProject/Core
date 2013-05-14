#include "InspectPch.h"
#include "Script.h"

#include "Inspect/Container.h"
#include "Inspect/Controls/LabelControl.h"
#include "Inspect/Controls/CheckBoxControl.h"
#include "Inspect/Controls/ColorPickerControl.h"
#include "Inspect/Controls/ListControl.h"
#include "Inspect/Controls/SliderControl.h"
#include "Inspect/Controls/ChoiceControl.h"
#include "Inspect/Controls/ValueControl.h"

#include "Foundation/Log.h"
#include "Foundation/Regex.h"

#include <regex> 

using namespace Helium;
using namespace Helium::Inspect;

// #define INSPECT_DEBUG_SCRIPT_COMPILE

// L strings
#define LS_REGEX_DELIM_BEGIN TXT( "UI\\[\\.\\[" )
#define LS_REGEX_DELIM_END TXT( "\\]\\.\\]" )
#define LS_WHITESPACE TXT( " \t\n" )

// L chars
#define LC_COMMENT TXT( "#" )


//
// Control Registry
//

typedef std::map<tstring, const Reflect::Class*> M_ControlType;

M_ControlType g_ControlTypeMap;


//
// Script Parsing
//

void Script::Initialize()
{
  if (g_ControlTypeMap.size() > 0)
  {
    return;
  }

  struct ControlEntry
  {
    const tchar_t*          name;
    const Reflect::Class*   type;
  };

  const ControlEntry controls[] =
  {
    { TXT( "label" ),       Reflect::GetClass<Label>()},
    { TXT( "value" ),       Reflect::GetClass<Value>()},
    { TXT( "slider" ),      Reflect::GetClass<Slider>()},
    { TXT( "check" ),       Reflect::GetClass<CheckBox>()},
    { TXT( "color" ),       Reflect::GetClass<ColorPicker>()},
    { TXT( "choice" ),      Reflect::GetClass<Choice>()},
    { TXT( "combo" ),       Reflect::GetClass<Choice>()},
    { TXT( "list" ),        Reflect::GetClass<List>()},
  };

  int size = sizeof(controls)/sizeof(controls[0]);
  for (int i=0; i<size; i++)
  {
    g_ControlTypeMap[ controls[i].name ] = controls[i].type;
  }
}

void Script::Cleanup()
{
  g_ControlTypeMap.clear();
}

bool Script::PreProcess(tstring& script)
{
  INSPECT_SCOPE_TIMER( ("Attributes Script Pre-Processing") );

  //
  // Debug
  //

#ifdef INSPECT_DEBUG_SCRIPT_COMPILE
  Log::Debug(TXT("Pre-preprocessed:\n\n%s\n\n"), script.c_str());
#endif

  //
  // Check for L code
  //

  const tregex cfPattern ( TXT( ".*" ) LS_REGEX_DELIM_BEGIN TXT( ".*" ) LS_REGEX_DELIM_END TXT( ".*" ) );

  tsmatch matchResult; 
  if(!std::tr1::regex_search(script, matchResult, cfPattern))
  {
    return false; 
  }

  //
  // Cull outside of our begin/end descriptors
  //

  // the .* at the end of this secretly culls the rest of the string for you
  // including comments and additional UI[.[ (.*) ].] 
  // 
  const tregex cfStartEndPattern ( TXT( ".*" ) LS_REGEX_DELIM_BEGIN TXT( "(.*)" ) LS_REGEX_DELIM_END TXT( ".*" ) ); 
  script = std::tr1::regex_replace(script, cfStartEndPattern, tstring (TXT( "$1" )) ); 

  //
  // Cull comments
  //
  
  const tregex cfCommentPattern ( LC_COMMENT TXT( ".*\n" ) ); 
  script = std::tr1::regex_replace(script, cfCommentPattern, tstring (TXT( "\n" )) ); 

  //
  // Debug
  //

#ifdef INSPECT_DEBUG_SCRIPT_COMPILE
  Log::Debug(TXT("Post-preprocessed:\n\n%s\n\n\n\n"), script.c_str());
#endif

  return true;
}

void Script::ParseAttributes(tstring& attributes, Control* control)
{
  INSPECT_SCOPE_TIMER( ("Attributes Script Attribute Processing") );
  
  size_t pos = 0;
  size_t end = tstring::npos;

  while (pos < attributes.length() && pos != tstring::npos)
  {
    // eat ws
    pos = attributes.find_first_not_of(LS_WHITESPACE, pos);

    // the rest is WS, abort
    if (pos == tstring::npos)
      break;

    // search for end of keyword
    end = attributes.find_first_of(LS_WHITESPACE TXT( "=" ), pos);

    // we have no symbol term, just abort
    if (end == tstring::npos)
      break;

    // copy just our symbol into a string
    tstring key (attributes.data() + pos, end - pos);

    // next tchar_t
    pos = end+1;

    // eat ws
    pos = attributes.find_first_not_of(LS_WHITESPACE, pos);

    // the rest is WS, abort
    if (pos == tstring::npos)
      break;

    // see if the value is directly quoted
    size_t startQuote = attributes.find_first_of( TXT( "\"" ), pos);
    size_t endQuote = attributes.find_first_of( TXT( "\"" ), startQuote+1);

    // search for end of keyword
    end = attributes.find_first_of( TXT( ";" ), pos);

    // if the semi is in the quote
    if (startQuote != tstring::npos && endQuote != tstring::npos && startQuote < end && end < endQuote)
    {
      // search for end of value not quoted
      end = attributes.find_first_of( TXT( ";" ), endQuote);
    }

    // we have no symbol term, just abort
    if (end == tstring::npos)
    {
      end = attributes.length();
    }

    // copy just our symbol into a string
    tstring value (attributes.data() + pos, end - pos);

    // next tchar_t
    pos = end+1;

    // trim quoted values
    {
      size_t start = value.find_first_of('\"');
      size_t finish = value.find_last_of('\"');

      if (start != tstring::npos)
      {
        if (start == finish)
        {
          value.erase(start, 1);
        }
        else if (start < finish)
        {
          value = value.substr(start + 1, finish - start - 1);
        }
      }
    }

    // insert
    control->Process(key, value);
  }
}

bool Script::Parse(const tstring& script, Interpreter* interpreter, Canvas* canvas, Container* output, uint32_t fieldFlags)
{
  INSPECT_SCOPE_TIMER( ("Attributes Script Parsing") );

  // make working copy
  tstring str = script;

  // remove delimiters and exterior string data
  if (!PreProcess(str))
    return false;

  size_t pos = 0;
  size_t end = tstring::npos;

  while (pos < str.length() && pos != tstring::npos)
  {
    // eat ws
    pos = str.find_first_not_of(LS_WHITESPACE, pos);

    // the rest is WS, abort
    if (pos == tstring::npos)
      break;

    // search for end of keyword
    end = str.find_first_of(LS_WHITESPACE TXT( "{" ), pos);

    // we have no symbol term, just abort
    if (end == tstring::npos)
      break;

    // this shouldn't happen
    if (pos > end)
      break;
    
    // copy just our symbol into a string
    tstring symbol (str.data() + pos, end - pos);


    //
    // Lookup type
    //

    M_ControlType::iterator i = g_ControlTypeMap.find(symbol);

    if (i == g_ControlTypeMap.end())
    {
      Log::Warning( TXT( "Undefined script symbol \"%s\"\n" ), symbol.c_str());
      return false;
    }


    //
    // Create control
    //

    ControlPtr control = Reflect::SafeCast<Inspect::Control> ( Reflect::Registry::GetInstance()->CreateInstance( i->second ) );
    if (!control.ReferencesObject())
    {
      Log::Warning( TXT( "Unable to construct control \"%s\"\n" ), symbol.c_str());
      return false;
    }

    bool readOnly = ( fieldFlags & Reflect::FieldFlags::ReadOnly ) == Reflect::FieldFlags::ReadOnly;
    control->a_IsReadOnly.Set( readOnly );


    //
    // Process attributes
    //

    pos = str.find_first_of('{', pos);
    end = str.find_first_of('}', pos);


    if (pos != tstring::npos && 
        end != tstring::npos &&
        pos < end && pos+1 != end)
    {
      ParseAttributes(tstring (str.data() + pos + 1, end - pos - 1), control);
    }


    //
    // Add control
    //

    output->AddChild(control);

    // keep going
    pos = end+1;
  }

  return true;
}
