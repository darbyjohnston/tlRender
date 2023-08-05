// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Vector.h>

namespace tl
{
    namespace math
    {
        //! Two-dimensional axis aligned box.
        template<typename T>
        class Box2
        {
        public:
            Box2() noexcept;
            explicit Box2(const Vector2<T>&) noexcept;
            Box2(const Vector2<T>& min, const Vector2<T>& max) noexcept;
            Box2(T x, T y, T w, T h) noexcept;

            Vector2<T> min;
            Vector2<T> max;

            //! \name Components
            ///@{

            T x() const noexcept;
            T y() const noexcept;
            T w() const noexcept;
            T h() const noexcept;

            constexpr bool isValid() const noexcept;

            void zero() noexcept;

            ///@}

            //! \name Dimensions
            ///@{

            Vector2<T> getSize() const noexcept;
            Vector2<T> getCenter() const noexcept;
            T getArea() const noexcept;
            float getAspect() const noexcept;

            ///@}

            //! \name Intersections
            ///@{

            bool contains(const Box2<T>&) const noexcept;
            bool contains(const Vector2<T>&) const noexcept;

            bool intersects(const Box2<T>&) const noexcept;
            Box2<T> intersect(const Box2<T>&) const;

            ///@}

            //! \name Expand
            ///@{

            void expand(const Box2<T>&);
            void expand(const Vector2<T>&);

            ///@}

            //! \name Margin
            ///@{

            constexpr Box2<T> margin(const Vector2<T>&) const noexcept;
            constexpr Box2<T> margin(T) const noexcept;
            constexpr Box2<T> margin(T x0, T y0, T x1, T y1) const noexcept;

            ///@}

            constexpr bool operator == (const Box2<T>&) const noexcept;
            constexpr bool operator != (const Box2<T>&) const noexcept;
        };

        //! Two-dimensional integer box.
        typedef Box2<int> Box2i;

        //! Two-dimensional floating point box.
        typedef Box2<float> Box2f;

        //! \name Operators
        ///@{

        Box2i operator * (const Box2i&, float);
        Box2f operator * (const Box2f&, float);

        ///@}

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const Box2i&);
        void to_json(nlohmann::json&, const Box2f&);

        void from_json(const nlohmann::json&, Box2i&);
        void from_json(const nlohmann::json&, Box2f&);

        std::ostream& operator << (std::ostream&, const Box2i&);
        std::ostream& operator << (std::ostream&, const Box2f&);

        std::istream& operator >> (std::istream&, Box2i&);
        std::istream& operator >> (std::istream&, Box2f&);

        ///@}
    }
}

#include <tlCore/BoxInline.h>
