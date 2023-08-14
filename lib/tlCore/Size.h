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
        //! Two-dimensional size.
        template<typename T>
        class Size2
        {
        public:
            constexpr Size2() noexcept;
            constexpr Size2(T w, T h) noexcept;

            T w;
            T h;

            //! Is the size valid?
            constexpr bool isValid() const noexcept;

            //! Get the area.
            constexpr float getArea() const noexcept;

            //! Get the aspect ratio.
            constexpr float getAspect() const noexcept;

            constexpr bool operator == (const Size2&) const noexcept;
            constexpr bool operator != (const Size2&) const noexcept;
            bool operator < (const Size2&) const noexcept;
        };

        //! Two-dimensional integer size.
        typedef Size2<int> Size2i;

        //! Two-dimensional floating point size.
        typedef Size2<float> Size2f;

        //! \name Operators
        ///@{

        template<typename T>
        inline Size2<T> operator + (const Size2<T>&, const Size2<T>&);

        template<typename T>
        inline Size2<T> operator + (const Size2<T>&, T);

        template<typename T>
        inline Size2<T> operator - (const Size2<T>&, const Size2<T>&);

        template<typename T>
        inline Size2<T> operator - (const Size2<T>&, T);

        Size2i operator * (const Size2i&, float);

        Size2i operator / (const Size2i&, float);

        ///@}

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const Size2i&);
        void to_json(nlohmann::json&, const Size2f&);

        void from_json(const nlohmann::json&, Size2i&);
        void from_json(const nlohmann::json&, Size2f&);

        std::ostream& operator << (std::ostream&, const Size2i&);
        std::ostream& operator << (std::ostream&, const Size2f&);

        std::istream& operator >> (std::istream&, Size2i&);
        std::istream& operator >> (std::istream&, Size2f&);

        ///@}
    }
}

#include <tlCore/SizeInline.h>

