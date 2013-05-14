#include "InspectPch.h"
#include "ReflectInterpreter.h"
#include "Inspect/Interpreters/Reflect/ReflectValueInterpreter.h"

#include "Inspect/Script.h"
#include "Inspect/Controls/ButtonControl.h"
#include "Inspect/Controls/ValueControl.h"
#include "Inspect/Controls/ListControl.h"
#include "Inspect/Container.h"

using namespace Helium;
using namespace Helium::Reflect;
using namespace Helium::Inspect;

ReflectFieldInterpreterFactory::M_Creator ReflectFieldInterpreterFactory::m_Map;

ReflectInterpreter::ReflectInterpreter (Container* container)
: Interpreter (container)
{

}

void ReflectInterpreter::Interpret(const std::vector<Reflect::Object*>& instances, int32_t includeFlags, int32_t excludeFlags, bool expandPanel)
{
    m_Instances = instances;

    InterpretType(instances, m_Container, includeFlags, excludeFlags, expandPanel);
}

void ReflectInterpreter::InterpretType(const std::vector<Reflect::Object*>& instances, Container* parent, int32_t includeFlags, int32_t excludeFlags, bool expandPanel)
{
    const Composite* composite = instances[0]->GetClass();

    // create a container
    ContainerPtr container = CreateControl<Container>();

    // parse
    ContainerPtr scriptOutput = CreateControl<Container>();

    tstring typeInfoUI;
    composite->GetProperty( TXT( "UIScript" ), typeInfoUI );
    bool result = Script::Parse(typeInfoUI, this, parent->GetCanvas(), scriptOutput);

    // compute container label
    tstring labelText;
    if (result)
    {
        V_Control::const_iterator itr = scriptOutput->GetChildren().begin();
        V_Control::const_iterator end = scriptOutput->GetChildren().end();
        for( ; itr != end; ++itr )
        {
            Label* label = Reflect::SafeCast<Label>( *itr );
            if (label)
            {
                label->ReadStringData( labelText );

                if ( !labelText.empty() )
                {
                    break;
                }
            }
        }
    }

    if (labelText.empty())
    {
        composite->GetProperty( TXT( "UIName" ), labelText );
    }

    if ( labelText.empty() )
    {
        labelText = *composite->m_Name;
    }

    container->a_Name.Set( labelText );

    std::map< tstring, ContainerPtr > containersMap;
    containersMap.insert( std::make_pair( TXT( "" ), container) );

    std::stack< const Composite* > bases;
    for ( const Composite* current = composite; current != NULL; current = current->m_Base )
    {
        bases.push( current );
    }

    while ( !bases.empty() )
    {
        const Composite* current = bases.top();
        bases.pop();

        // for each field in the type
        DynamicArray< Field >::ConstIterator itr = current->m_Fields.Begin();
        DynamicArray< Field >::ConstIterator end = current->m_Fields.End();
        for ( ; itr != end; ++itr )
        {
            const Field* field = &*itr;

            bool noFlags = ( field->m_Flags == 0 && includeFlags == 0xFFFFFFFF );
            bool doInclude = ( field->m_Flags & includeFlags ) != 0;
            bool dontExclude = ( excludeFlags == 0 ) || !(field->m_Flags & excludeFlags );
            bool hidden = (field->m_Flags & Reflect::FieldFlags::Hide) != 0; 

            // if we don't have flags (or we are included, and we aren't excluded) then make UI
            if ( ( noFlags || doInclude ) && ( dontExclude ) )
            {
                tstring fieldUIGroup;
                field->GetProperty( TXT( "UIGroup" ), fieldUIGroup );
                if ( !fieldUIGroup.empty() )
                {
                    std::map< tstring, ContainerPtr >::iterator itr = containersMap.find( fieldUIGroup );
                    if ( itr == containersMap.end() )
                    {
                        // This container isn't in our list so make a new one
                        ContainerPtr newContainer = CreateControl<Container>();
                        containersMap.insert( std::make_pair(fieldUIGroup, newContainer) );

                        ContainerPtr parent;
                        tstring groupName;
                        size_t idx = fieldUIGroup.find_last_of( TXT( "/" ) );
                        if ( idx != tstring::npos )
                        {
                            tstring parentName = fieldUIGroup.substr( 0, idx );
                            groupName = fieldUIGroup.substr( idx+1 );
                            if ( containersMap.find( parentName ) == containersMap.end() )
                            {          
                                parent = CreateControl<Container>();

                                // create the parent hierarchy since it hasn't already been made
                                tstring currentParent = parentName;
                                for (;;)
                                {
                                    idx = currentParent.find_last_of( TXT( "/" ) );
                                    if ( idx == tstring::npos )
                                    {
                                        // no more parents so we add it to the root
                                        containersMap.insert( std::make_pair(currentParent, parent) );
                                        parent->a_Name.Set( currentParent );
                                        containersMap[ TXT( "" ) ]->AddChild( parent );
                                        break;
                                    }
                                    else
                                    {
                                        parent->a_Name.Set( currentParent.substr( idx+1 ) );

                                        if ( containersMap.find( currentParent ) != containersMap.end() )
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            ContainerPtr grandParent = CreateControl<Container>();
                                            grandParent->AddChild( parent );
                                            containersMap.insert( std::make_pair(currentParent, parent) );

                                            parent = grandParent;
                                        }
                                        currentParent = currentParent.substr( 0, idx );
                                    }
                                }
                                containersMap.insert( std::make_pair(parentName, parent) );
                            }
                            parent = containersMap[parentName];
                        }
                        else
                        {
                            parent = containersMap[ TXT( "" )];
                            groupName = fieldUIGroup;
                        }
                        newContainer->a_Name.Set( groupName );
                        parent->AddChild( newContainer );
                    }

                    container = containersMap[fieldUIGroup];
                }
                else
                {
                    container = containersMap[ TXT( "" )];
                }


                //
                // Pointer support
                //

                if (field->m_DataClass == Reflect::GetClass<Reflect::PointerData>())
                {
                    if (hidden)
                    {
                        continue; 
                    }        

                    std::vector<Reflect::Object*> fieldInstances;

                    std::vector<Reflect::Object*>::const_iterator elementItr = instances.begin();
                    std::vector<Reflect::Object*>::const_iterator elementEnd = instances.end();
                    for ( ; elementItr != elementEnd; ++elementItr )
                    {
                        uintptr_t fieldAddress = (uintptr_t)(*elementItr) + itr->m_Offset;

                        Object* element = *((ObjectPtr*)(fieldAddress));

                        if ( element )
                        {
                            fieldInstances.push_back( element );
                        }
                    }

                    if ( !fieldInstances.empty() && fieldInstances.size() == instances.size() )
                    {
                        InterpretType(fieldInstances, container);
                    }

                    continue;
                }


                //
                // Attempt to find a handler via the factory
                //

                ReflectFieldInterpreterPtr fieldInterpreter;

                for ( const Reflect::Class* type = field->m_DataClass;
                    type != Reflect::GetClass<Reflect::Object>() && !fieldInterpreter;
                    type = Reflect::ReflectionCast< const Class >( type->m_Base ) )
                {
                    fieldInterpreter = ReflectFieldInterpreterFactory::Create( type, field->m_Flags, m_Container );
                }

                if ( fieldInterpreter.ReferencesObject() )
                {
                    Interpreter::ConnectInterpreterEvents( this, fieldInterpreter );
                    fieldInterpreter->InterpretField( field, instances, container );
                    m_Interpreters.push_back( fieldInterpreter );
                    continue;
                }


                //
                // ElementArray support
                //

#pragma TODO("Move this out to an interpreter")
                if (field->m_DataClass == Reflect::GetClass<ObjectStlVectorData>())
                {
                    if (hidden)
                    {
                        continue;
                    }

                    if ( instances.size() == 1 )
                    {
                        uintptr_t fieldAddress = (uintptr_t)(instances.front()) + itr->m_Offset;

                        std::vector< ObjectPtr >* elements = (std::vector< ObjectPtr >*)fieldAddress;

                        if ( elements->size() > 0 )
                        {
                            ContainerPtr childContainer = CreateControl<Container>();

                            tstring temp;
                            field->GetProperty( TXT( "UIName" ), temp );
                            if ( temp.empty() )
                            {
                                bool converted = Helium::ConvertString( field->m_Name, temp );
                                HELIUM_ASSERT( converted );
                            }

                            childContainer->a_Name.Set( temp );

                            std::vector< ObjectPtr >::const_iterator elementItr = elements->begin();
                            std::vector< ObjectPtr >::const_iterator elementEnd = elements->end();
                            for ( ; elementItr != elementEnd; ++elementItr )
                            {
                                std::vector<Reflect::Object*> childInstances;
                                childInstances.push_back(*elementItr);
                                InterpretType(childInstances, childContainer);
                            }

                            container->AddChild( childContainer );
                        }
                    }

                    continue;
                }


                //
                // Lastly fall back to the value interpreter
                //

                const Reflect::Class* type = field->m_DataClass;
                if ( !type->IsType( Reflect::GetClass<Reflect::ContainerData>() ) )
                {
                    fieldInterpreter = CreateInterpreter< ReflectValueInterpreter >( m_Container );
                    fieldInterpreter->InterpretField( field, instances, container );
                    m_Interpreters.push_back( fieldInterpreter );
                    continue;
                }
            }
        }
    }

    // Make sure we have the base container
    container = containersMap[TXT( "" )];

    if ( !container->GetChildren().empty() )
    {
        parent->AddChild(container);
    }
}


void ReflectFieldInterpreterFactory::Register(const Reflect::Class* type, uint32_t mask, Creator creator)
{
    m_Map[ type ].push_back( std::make_pair(mask, creator) );
}

void ReflectFieldInterpreterFactory::Unregister(const Reflect::Class* type, uint32_t mask, Creator creator)
{
    M_Creator::iterator found = m_Map.find( type );
    if ( found != m_Map.end() )
    {
        std::remove( found->second.begin(), found->second.end(), std::make_pair( mask, creator ) );
    }
}

ReflectFieldInterpreterPtr ReflectFieldInterpreterFactory::Create(const Reflect::Class* type, uint32_t flags, Container* container)
{
    Creator creator = NULL;

    M_Creator::const_iterator found = m_Map.find( type );
    if ( found != m_Map.end() )
    {
        std::map<uint32_t, Creator> results;

        V_Creator::const_iterator itr = found->second.begin();
        V_Creator::const_iterator end = found->second.end();
        for ( ; itr != end; ++itr )
        {
            if ( flags == itr->first ) // exact match
            {
                creator = itr->second;
            }
            else if ( flags && (flags & itr->first) != 0 ) // it has flags, and at least on flag is in the mask
            {
                uint32_t value = flags & itr->first;

                // count the number of bits set
                uint32_t bits = 0;
                while (value)
                {
                    value = value & (value - 1);
                    bits++;
                }

                // rank it by the number of matching flags
                results[ bits ] = itr->second;
            }
            else if ( itr->first == 0 ) // fall back to folks that don't care about flags
            {
                // rank it at the bottom
                results[ 0x0 ] = itr->second;
            }
        }

        if ( !results.empty() )
        {
            creator = results.rbegin()->second;
        }
    }

    if ( creator )
    {
        return creator( container );
    }
    else
    {
        return NULL;
    }
}

void ReflectFieldInterpreterFactory::Clear()
{
    m_Map.clear();
}