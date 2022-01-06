// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <iostream>

namespace tlr
{
    namespace math
    {
        //! Two-dimensional bounding box.
        template<typename T, glm::precision P = glm::defaultp>
        class BBox2
        {
        public:
            BBox2() noexcept;
            explicit BBox2(const glm::tvec2<T, P>&) noexcept;
            BBox2(const glm::tvec2<T, P>& min, const glm::tvec2<T, P>& max) noexcept;
            BBox2(T x, T y, T w, T h) noexcept;

            glm::tvec2<T, P> min, max;

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

            glm::tvec2<T, P> getSize() const noexcept;
            glm::tvec2<T, P> getCenter() const noexcept;
            T getArea() const noexcept;
            float getAspect() const noexcept;

            ///@}

            //! \name Intersections
            ///@{

            bool contains(const BBox2<T, P>&) const noexcept;
            bool contains(const glm::tvec2<T, P>&) const noexcept;

            bool intersects(const BBox2<T, P>&) const noexcept;
            BBox2<T, P> intersect(const BBox2<T, P>&) const;

            ///@}

            //! \name Expand
            ///@{

            void expand(const BBox2<T, P>&);
            void expand(const glm::tvec2<T, P>&);

            ///@}

            //! \name Margin
            ///@{

            constexpr BBox2<T, P> margin(const glm::tvec2<T, P>&) const noexcept;
            constexpr BBox2<T, P> margin(T) const noexcept;
            constexpr BBox2<T, P> margin(T x0, T y0, T x1, T y1) const noexcept;

            ///@}

            constexpr bool operator == (const BBox2<T, P>&) const noexcept;
            constexpr bool operator != (const BBox2<T, P>&) const noexcept;
        };

        //! Two-dimensional integer bounding box.
        typedef BBox2<int> BBox2i;

        //! Two-dimensional floating point bounding box.
        typedef BBox2<float> BBox2f;

        std::ostream& operator << (std::ostream&, const BBox2i&);
        std::ostream& operator << (std::ostream&, const BBox2f&);

        std::istream& operator >> (std::istream&, BBox2i&);
        std::istream& operator >> (std::istream&, BBox2f&);
    }
}

#include <tlrCore/BBoxInline.h>
