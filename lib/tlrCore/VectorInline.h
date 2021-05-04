// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

namespace tlr
{
    namespace math
    {
        template<typename T>
        inline Vector2<T>::Vector2() :
            x(static_cast<T>(0)),
            y(static_cast<T>(0))
        {}

        template<typename T>
        inline Vector2<T>::Vector2(T x, T y) :
            x(x),
            y(y)
        {}

        template<typename T>
        inline Vector3<T>::Vector3() :
            x(static_cast<T>(0)),
            y(static_cast<T>(0)),
            z(static_cast<T>(0))
        {}

        template<typename T>
        inline Vector3<T>::Vector3(T x, T y, T z) :
            x(x),
            y(y),
            z(z)
        {}

        template<typename T>
        inline Vector4<T>::Vector4() :
            x(static_cast<T>(0)),
            y(static_cast<T>(0)),
            z(static_cast<T>(0)),
            w(static_cast<T>(0))
        {}

        template<typename T>
        inline Vector4<T>::Vector4(T x, T y, T z, T w) :
            x(x),
            y(y),
            z(z),
            w(w)
        {}

        template<typename T>
        bool Vector2<T>::operator == (const Vector2<T>& other) const
        {
            return this->x == other.x && this->y == other.y;
        }

        template<typename T>
        bool Vector2<T>::operator != (const Vector2<T>& other) const
        {
            return !(*this == other);
        }

        template<typename T>
        bool Vector3<T>::operator == (const Vector3<T>& other) const
        {
            return this->x == other.x && this->y == other.y && this->z == other.z;
        }

        template<typename T>
        bool Vector3<T>::operator != (const Vector3<T>& other) const
        {
            return !(*this == other);
        }

        template<typename T>
        bool Vector4<T>::operator == (const Vector4<T>& other) const
        {
            return this->x == other.x && this->y == other.y && this->z == other.z && this->w == other.w;
        }

        template<typename T>
        bool Vector4<T>::operator != (const Vector4<T>& other) const
        {
            return !(*this == other);
        }

        template<typename T>
        inline Vector2<T> operator + (const Vector2<T>& a, T b)
        {
            return Vector2<T>(a.x + b, a.y + b);
        }

        template<typename T>
        inline Vector3<T> operator + (const Vector3<T>& a, T b)
        {
            return Vector3<T>(a.x + b, a.y + b, a.z + b);
        }

        template<typename T>
        inline Vector4<T> operator + (const Vector4<T>& a, T b)
        {
            return Vector4<T>(a.x + b, a.y + b, a.z + b, a.w + b);
        }
    }
}
