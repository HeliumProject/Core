#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "Foundation/Math.h"

#include "Reflect/MetaStruct.h"

#include "Math/API.h"
#include "Math/Vector3.h"

namespace Helium
{
	struct Color4;

	struct HELIUM_MATH_API Color3 : Reflect::Struct
	{
	public:
		REFLECT_DECLARE_BASE_STRUCT( Color3 );
		static void PopulateMetaType( Reflect::MetaStruct& comp );

		uint8_t r, g, b;

		Color3            () : r(0), g(0), b(0) {}
		Color3            ( const Color3& c ) : r(c.r), g(c.g), b(c.b) {}
		Color3            ( const Vector3& v ) : r( (uint8_t)(v.x*255.0f) ), g( (uint8_t)(v.y*255.0f) ), b( (uint8_t)(v.z*255.0f) ) {}
		explicit Color3   ( uint8_t vr, uint8_t vg, uint8_t vb ) : r(vr), g(vg), b(vb) {}
		explicit Color3   ( uint8_t val ) : r( val ), g( val ), b( val ) {}

		Color3&           operator= (const Color4& v);
		Color3&           operator= (const Color3& v) { r = v.r; g = v.g; b = v.b; return *this; }
		Color3&           operator= (const Vector3& v) { r = (uint8_t)(v.x*255.0f); g = (uint8_t)(v.y*255.0f); b = (uint8_t)(v.z*255.0f); return *this; }
		Color3&           operator+= (const Color3& v) { r += v.r; g += v.g; b += v.b; return *this; }
		Color3&           operator-= (const Color3& v) { r -= v.r; g -= v.g; b -= v.b; return *this; }
		Color3&           operator*= (const Color3& v) { r *= v.r; g *= v.g; b *= v.b; return *this; }
		Color3&           operator*= (const uint8_t v) { r *= v; g *= v; b *= v; return *this; }
		Color3&           operator/= (const Color3& v) { r /= v.r; g /= v.g; b /= v.b; return *this; }
		Color3&           operator/= (const uint8_t v) { r /= v; g /= v; b /= v; return *this; }

		Color3            operator+ (const Color3& v) const { return Color3 (r + v.r, g + v.g, b + v.b); }
		Color3            operator- (const Color3& v) const { return Color3 (r - v.r, g - v.g, b - v.b); }
		Color3            operator* (const Color3& v) const { return Color3 (r * v.r, g * v.g, b * v.b); }
		Color3            operator* (const uint8_t v) const { return Color3 (r * v, g * v, b * v); }
		Color3            operator/ (const Color3& v) const { return Color3 (r / v.r, g / v.g, b / v.b); }
		Color3            operator/ (const uint8_t v) const { return Color3 (r / v, g / v, b / v); }

		uint8_t&          operator[] (const uint32_t i) {  HELIUM_ASSERT(i < 3); return (&r)[i]; }
		const uint8_t&    operator[] (const uint32_t i) const {  HELIUM_ASSERT(i < 3); return (&r)[i]; }

		bool              operator== (const Color3& v) const { return (r == v.r && g == v.g && b == v.b); }
		bool              operator!= (const Color3& v) const { return !(r == v.r && g == v.g && b == v.b); }

		operator Color4();

		void Set( uint8_t vr, uint8_t vg, uint8_t vb )
		{
			r = vr;
			g = vg;
			b = vb;
		}

		void Set( float32_t vr, float32_t vg, float32_t vb )
		{
			r = (uint8_t)( vr * 255.0f );
			g = (uint8_t)( vg * 255.0f );
			b = (uint8_t)( vb * 255.0f );
		}

		void Get( float32_t& vr, float32_t& vg, float32_t& vb ) const
		{
			vr = r / 255.0f;
			vg = g / 255.0f;
			vb = b / 255.0f;
		}

		void GetRGBA( uint32_t& out, uint8_t a = 0xFF ) const
		{
			out = ( ( r << 24 ) | ( g << 16 ) | ( b << 8 ) | a );
		}

		void Clamp ( const Color3 &min, const Color3 &mar );
	};

	inline void Color3::Clamp( const Color3 &min, const Color3 &mar )
	{
		r = r < min.r ? min.r : ( r > mar.r ) ? mar.r : r; 
		g = g < min.g ? min.g : ( g > mar.g ) ? mar.g : g; 
		b = b < min.b ? min.b : ( b > mar.b ) ? mar.b : b; 
	}
}