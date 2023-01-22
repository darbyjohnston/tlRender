// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

#include <cstddef>
#include <iostream>

namespace tl
{
    namespace math
    {
        //! Number range.
        template<typename T>
        class Range
        {
        public:
            constexpr Range() noexcept;
            explicit constexpr Range(T minMax) noexcept;
            constexpr Range(T min, T max);
            ~Range();

            //! Get the minimum value.
            constexpr T getMin() const noexcept;

            //! Get the maximum value.
            constexpr T getMax() const noexcept;

            //! Set the range minimum and maximum to zero.
            void zero() noexcept;

            //! Does the range contain the given number?
            constexpr bool contains(T) const noexcept;

            //! Does the range interset the given range?
            constexpr bool intersects(const Range<T>&) const noexcept;

            //! Expand the range to include the given number.
            void expand(T);

            //! Expand the range to include the given range.
            void expand(const Range<T>&);

            constexpr bool operator == (const Range<T>&) const;
            constexpr bool operator != (const Range<T>&) const;
            constexpr bool operator  < (const Range<T>&) const;

        private:
            T _min, _max;
        };

        //! This typedef provides an integer range.
        typedef Range<int> IntRange;

        //! This typedef provides a size_t range.
        typedef Range<std::size_t> SizeTRange;

        //! This typedef provides a floating point range.
        typedef Range<float> FloatRange;

        void to_json(nlohmann::json&, const IntRange&);
        void to_json(nlohmann::json&, const SizeTRange&);
        void to_json(nlohmann::json&, const FloatRange&);

        void from_json(const nlohmann::json&, IntRange&);
        void from_json(const nlohmann::json&, SizeTRange&);
        void from_json(const nlohmann::json&, FloatRange&);

        std::ostream& operator << (std::ostream&, const IntRange&);
        std::ostream& operator << (std::ostream&, const SizeTRange&);
        std::ostream& operator << (std::ostream&, const FloatRange&);

        std::istream& operator >> (std::istream&, IntRange&);
        std::istream& operator >> (std::istream&, SizeTRange&);
        std::istream& operator >> (std::istream&, FloatRange&);
    }
}

#include <tlCore/RangeInline.h>
