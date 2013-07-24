#include "MathPch.h"
#include "Math/Color3.h"
#include "Math/Color4.h"
#include "Reflect/TranslatorDeduction.h"

REFLECT_DEFINE_BASE_STRUCT( Helium::Color3 );

using namespace Helium;

void Color3::PopulateMetaType( Reflect::MetaStruct& comp )
{
	comp.AddField( &Color3::r, "r" );
	comp.AddField( &Color3::g, "g" );
	comp.AddField( &Color3::b, "b" );
}

Color3& Color3::operator=( const Color4& v )
{
	r = v.r;
	g = v.g;
	b = v.b;
	return *this;
}

Color3::operator Color4()
{
	return Color4( r, g, b, 255 );
}