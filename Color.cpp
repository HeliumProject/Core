#include "MathPch.h"
#include "Math/Color3.h"
#include "Math/Color4.h"
#include "Reflect/Data/DataDeduction.h"

REFLECT_DEFINE_BASE_STRUCTURE( Helium::Color4 );

void Helium::Color4::PopulateComposite( Reflect::Composite& comp )
{
	comp.AddField( &Color4::r,       TXT( "r" ) );
	comp.AddField( &Color4::g,       TXT( "g" ) );
	comp.AddField( &Color4::b,       TXT( "b" ) );
	comp.AddField( &Color4::a,       TXT( "a" ) );
}

Helium::Color3& Helium::Color3::operator=( const Helium::Color4& v )
{
    r = v.r;
    g = v.g;
    b = v.b;
    return *this;
}

Helium::Color3::operator Helium::Color4()
{
    return Helium::Color4( r, g, b, 255 );
}