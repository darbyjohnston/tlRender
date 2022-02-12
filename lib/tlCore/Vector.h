// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <iostream>

namespace tl
{
    namespace math
    {
        //! Two-dimensional vector.
        template<typename T>
        class Vector2
        {
        public:
            Vector2() noexcept;
            Vector2(T x, T y) noexcept;

            T x;
            T y;

            bool operator == (const Vector2<T>&) const;
            bool operator != (const Vector2<T>&) const;
        };

        //! Three-dimensional vector.
        template<typename T>
        class Vector3
        {
        public:
            Vector3() noexcept;
            Vector3(T x, T y, T z) noexcept;

            T x;
            T y;
            T z;

            bool operator == (const Vector3<T>&) const;
            bool operator != (const Vector3<T>&) const;
        };

        //! Four-dimensional vector.
        template<typename T>
        class Vector4
        {
        public:
            Vector4() noexcept;
            Vector4(T x, T y, T z, T w) noexcept;

            T x;
            T y;
            T z;
            T w;

            bool operator == (const Vector4<T>&) const;
            bool operator != (const Vector4<T>&) const;
        };

        //! Two-dimensional integer vector.
        typedef Vector2<int> Vector2i;

        //! Two-dimensional floating point vector.
        typedef Vector2<float> Vector2f;

        //! Three-dimensional floating point vector.
        typedef Vector3<float> Vector3f;

        //! Four-dimensional floating point vector.
        typedef Vector4<float> Vector4f;

        std::ostream& operator << (std::ostream&, const Vector2i&);
        std::ostream& operator << (std::ostream&, const Vector2f&);
        std::ostream& operator << (std::ostream&, const Vector3f&);
        std::ostream& operator << (std::ostream&, const Vector4f&);

        std::istream& operator >> (std::istream&, Vector2i&);
        std::istream& operator >> (std::istream&, Vector2f&);
        std::istream& operator >> (std::istream&, Vector3f&);
        std::istream& operator >> (std::istream&, Vector4f&);
    }
}

#include <tlCore/VectorInline.h>
