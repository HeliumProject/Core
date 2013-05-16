#include "InspectPch.h"
#include "InspectReflectInit.h"

#include "Application/InitializerStack.h"
#include "Inspect/Inspect.h"
#include "Inspect/Container.h"

#include "Inspect/Interpreters/ReflectInterpreter.h"
#include "Inspect/Interpreters/ReflectBitfieldInterpreter.h"
#include "Inspect/Interpreters/ReflectColorInterpreter.h"
#include "Inspect/Interpreters/ReflectSequenceInterpreter.h"
#include "Inspect/Interpreters/ReflectSetInterpreter.h"

#include "Inspect/Interpreters/ReflectPathInterpreter.h"

REFLECT_DEFINE_ABSTRACT( Helium::Inspect::ClientDataFilter );

using namespace Helium;
using namespace Helium::Inspect;

static Helium::InitializerStack g_InspectReflectInitStack;

void InspectReflect::Initialize()
{
    if ( g_InspectReflectInitStack.Increment() == 1 )
    {
        g_InspectReflectInitStack.Push( Inspect::Initialize, Inspect::Cleanup );

#if REFLECT_REFACTOR
        // scalars
        ReflectFieldInterpreterFactory::Register<ReflectBitfieldInterpreter>( Reflect::GetClass<Reflect::BitfieldData>() );
        ReflectFieldInterpreterFactory::Register<ReflectVectorInterpreter>( Reflect::GetClass<Reflect::Vector2Data>() );
        ReflectFieldInterpreterFactory::Register<ReflectVectorInterpreter>( Reflect::GetClass<Reflect::Vector3Data>() );
        ReflectFieldInterpreterFactory::Register<ReflectVectorInterpreter>( Reflect::GetClass<Reflect::Vector4Data>() );
        ReflectFieldInterpreterFactory::Register<ReflectColorInterpreter>( Reflect::GetClass<Reflect::Color3Data>() );
        ReflectFieldInterpreterFactory::Register<ReflectColorInterpreter>( Reflect::GetClass<Reflect::HDRColor3Data>() );
        ReflectFieldInterpreterFactory::Register<ReflectColorInterpreter>( Reflect::GetClass<Reflect::Color4Data>() );
        ReflectFieldInterpreterFactory::Register<ReflectColorInterpreter>( Reflect::GetClass<Reflect::HDRColor4Data>() );

        // containers
        ReflectFieldInterpreterFactory::Register<ReflectSequenceInterpreter>( Reflect::GetClass<Reflect::StlVectorData>() );
        ReflectFieldInterpreterFactory::Register<ReflectSetInterpreter>( Reflect::GetClass<Reflect::StlSetData>() );

        // paths
        ReflectFieldInterpreterFactory::Register<PathInterpreter>( Reflect::GetClass<Reflect::PathData>() );
#endif
    }
}

void InspectReflect::Cleanup()
{
    if ( g_InspectReflectInitStack.Decrement() == 0 )
    {
        ReflectFieldInterpreterFactory::Clear();
    }
}
