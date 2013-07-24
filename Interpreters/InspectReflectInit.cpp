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
        ReflectFieldInterpreterFactory::Register<ReflectBitfieldInterpreter>( Reflect::GetMetaClass<Reflect::BitfieldData>() );
        ReflectFieldInterpreterFactory::Register<ReflectVectorInterpreter>( Reflect::GetMetaClass<Reflect::Vector2Data>() );
        ReflectFieldInterpreterFactory::Register<ReflectVectorInterpreter>( Reflect::GetMetaClass<Reflect::Vector3Data>() );
        ReflectFieldInterpreterFactory::Register<ReflectVectorInterpreter>( Reflect::GetMetaClass<Reflect::Vector4Data>() );
        ReflectFieldInterpreterFactory::Register<ReflectColorInterpreter>( Reflect::GetMetaClass<Reflect::Color3Data>() );
        ReflectFieldInterpreterFactory::Register<ReflectColorInterpreter>( Reflect::GetMetaClass<Reflect::HDRColor3Data>() );
        ReflectFieldInterpreterFactory::Register<ReflectColorInterpreter>( Reflect::GetMetaClass<Reflect::Color4Data>() );
        ReflectFieldInterpreterFactory::Register<ReflectColorInterpreter>( Reflect::GetMetaClass<Reflect::HDRColor4Data>() );

        // containers
        ReflectFieldInterpreterFactory::Register<ReflectSequenceInterpreter>( Reflect::GetMetaClass<Reflect::StlVectorData>() );
        ReflectFieldInterpreterFactory::Register<ReflectSetInterpreter>( Reflect::GetMetaClass<Reflect::StlSetData>() );

        // paths
        ReflectFieldInterpreterFactory::Register<PathInterpreter>( Reflect::GetMetaClass<Reflect::PathData>() );
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
