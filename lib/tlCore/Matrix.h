// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

#include <iostream>

namespace tl
{
    namespace math
    {
        //! 3x3 matrix.
        template<typename T>
        class Matrix3x3
        {
        public:
            Matrix3x3() noexcept;
            Matrix3x3(
                T e0, T e1, T e2,
                T e3, T e4, T e5,
                T e6, T e7, T e8) noexcept;

            T e[9];

            bool operator == (const Matrix3x3<T>&) const;
            bool operator != (const Matrix3x3<T>&) const;

            Matrix3x3<T> operator * (const Matrix3x3<T>&) const;
        };

        //! 4x4 matrix.
        template<typename T>
        class Matrix4x4
        {
        public:
            Matrix4x4() noexcept;
            Matrix4x4(
                T e0, T e1, T e2, T e3,
                T e4, T e5, T e6, T e7,
                T e8, T e9, T e10, T e11,
                T e12, T e13, T e14, T e15) noexcept;

            T e[16];

            bool operator == (const Matrix4x4<T>&) const;
            bool operator != (const Matrix4x4<T>&) const;

            Matrix4x4<T> operator * (const Matrix4x4<T>&) const;
        };

        //! 3x3 floating point matrix.
        typedef Matrix3x3<float> Matrix3x3f;

        //! 4x4 floating point matrix.
        typedef Matrix4x4<float> Matrix4x4f;

        void to_json(nlohmann::json&, const Matrix3x3f&);
        void to_json(nlohmann::json&, const Matrix4x4f&);

        void from_json(const nlohmann::json&, Matrix3x3f&);
        void from_json(const nlohmann::json&, Matrix4x4f&);

        std::ostream& operator << (std::ostream&, const Matrix3x3f&);
        std::ostream& operator << (std::ostream&, const Matrix4x4f&);

        std::istream& operator >> (std::istream&, Matrix3x3f&);
        std::istream& operator >> (std::istream&, Matrix4x4f&);
    }
}

#include <tlCore/MatrixInline.h>
