#pragma once

#include "Math/API.h"
#include "Math/Vector3.h"

namespace Helium
{
    class Matrix4;

    class HELIUM_MATH_API AlignedBox
    {
    public:
        Vector3 minimum;
        Vector3 maximum;
        bool seeded;

        const static AlignedBox Unit;
        const static AlignedBox Singular;

        AlignedBox      ()
        {
            Reset();
        }

        AlignedBox      (const Vector3& min, const Vector3& max)
            : minimum (min)
            , maximum (max)
            , seeded (false)
        {

        }

        void            Reset ()
        {
            minimum = Vector3::Zero;
            maximum = Vector3::Zero;
            seeded = false;
        }

        bool            IsSingular () const
        {
            return (minimum == Vector3::Zero) && (maximum == Vector3::Zero);
        }

        float32_t             Width () const
        {
            return maximum.x - minimum.x;
        }

        float32_t             Length () const
        {
            return maximum.y - minimum.y;
        }

        float32_t             Height () const
        {
            return maximum.z - minimum.z;
        }

        Vector3         Center () const
        {
            return (minimum + maximum) * 0.5f;
        }

        Vector3         ClosestCorner( const Vector3& v ) const;

        Vector3 Test(Vector3 vertex);

        void Merge(const AlignedBox& box);
        void Merge(const Vector3& position);

        void Transform(const Matrix4& matrix);
        void GetVertices(V_Vector3& vertices) const;

        static void GetWireframe(const V_Vector3& vertices, V_Vector3& lineList, bool clear = true);
        static void GetTriangulated(const V_Vector3& vertices, V_Vector3& triangleList, bool clear = true);

        bool IntersectsSphere( const Vector3& pos, const float32_t radius ) const;
        bool IntersectsBox( const AlignedBox& box ) const;
    };
}