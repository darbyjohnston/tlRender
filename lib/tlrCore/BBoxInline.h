// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <algorithm>

namespace tlr
{
    namespace math
    {
        template<>
        inline BBox2<int>::BBox2() noexcept :
            min(0, 0),
            max(0, 0)
        {}

        template<>
        inline BBox2<float>::BBox2() noexcept :
            min(0.F, 0.F),
            max(0.F, 0.F)
        {}

        template<typename T>
        inline BBox2<T>::BBox2(const Vector2<T>& value) noexcept :
            min(value),
            max(value)
        {}

        template<typename T>
        inline BBox2<T>::BBox2(const Vector2<T>& min, const Vector2<T>& max) noexcept :
            min(min),
            max(max)
        {}

        template<>
        inline BBox2<int>::BBox2(int x, int y, int w, int h) noexcept :
            min(x, y),
            max(x + w - 1, y + h - 1)
        {}

        template<>
        inline BBox2<float>::BBox2(float x, float y, float w, float h) noexcept :
            min(x, y),
            max(x + w, y + h)
        {}

        template<typename T>
        inline T BBox2<T>::x() const noexcept { return min.x; }

        template<typename T>
        inline T BBox2<T>::y() const noexcept { return min.y; }

        template<>
        inline float BBox2<float>::w() const noexcept { return max.x - min.x; }
        template<>
        inline int BBox2<int>::w() const noexcept { return max.x - min.x + 1; }

        template<>
        inline float BBox2<float>::h() const noexcept { return max.y - min.y; }
        template<>
        inline int BBox2<int>::h() const noexcept { return max.y - min.y + 1; }

        template<typename T>
        constexpr bool BBox2<T>::isValid() const noexcept
        {
            return
                min.x < max.x &&
                min.y < max.y;
        }

        template<>
        inline void BBox2<int>::zero() noexcept
        {
            min.x = min.y = max.x = max.y = 0;
        }

        template<>
        inline void BBox2<float>::zero() noexcept
        {
            min.x = min.y = max.x = max.y = 0.F;
        }

        template<>
        inline Vector2<int> BBox2<int>::getSize() const noexcept
        {
            return Vector2<int>(max.x - min.x + 1, max.y - min.y + 1);
        }

        template<>
        inline Vector2<float> BBox2<float>::getSize() const noexcept
        {
            return Vector2<float>(max.x - min.x, max.y - min.y);
        }

        template<>
        inline Vector2<int> BBox2<int>::getCenter() const noexcept
        {
            return Vector2<int>(
                min.x + (max.x - min.x + 1) / 2.F,
                min.y + (max.y - min.y + 1) / 2.F);
        }

        template<>
        inline Vector2<float> BBox2<float>::getCenter() const noexcept
        {
            return Vector2<float>(
                min.x + (max.x - min.x) / 2.F,
                min.y + (max.y - min.y) / 2.F);
        }

        template<>
        inline int BBox2<int>::getArea() const noexcept
        {
            return w() * h();
        }

        template<>
        inline float BBox2<float>::getArea() const noexcept
        {
            return w() * h();
        }

        template<>
        inline float BBox2<int>::getAspect() const noexcept
        {
            const int h = this->h();
            return h != 0 ? w() / static_cast<float>(h) : 0.F;
        }

        template<>
        inline float BBox2<float>::getAspect() const noexcept
        {
            const float h = this->h();
            return h != 0 ? w() / h : 0.F;
        }

        template<>
        inline bool BBox2<int>::contains(const BBox2<int>& value) const noexcept
        {
            return
                value.min.x >= min.x && value.max.x <= max.x &&
                value.min.y >= min.y && value.max.y <= max.y;
        }

        template<>
        inline bool BBox2<float>::contains(const BBox2<float>& value) const noexcept
        {
            return
                value.min.x >= min.x && value.max.x <= max.x &&
                value.min.y >= min.y && value.max.y <= max.y;
        }

        template<>
        inline bool BBox2<int>::contains(const Vector2<int>& value) const noexcept
        {
            return
                value.x >= min.x && value.x < max.x &&
                value.y >= min.y && value.y < max.y;
        }

        template<>
        inline bool BBox2<float>::contains(const Vector2<float>& value) const noexcept
        {
            return
                value.x >= min.x && value.x <= max.x &&
                value.y >= min.y && value.y <= max.y;
        }

        template<>
        inline bool BBox2<int>::intersects(const BBox2<int>& value) const noexcept
        {
            return !(
                value.max.x < min.x ||
                value.min.x > max.x ||
                value.max.y < min.y ||
                value.min.y > max.y);
        }

        template<>
        inline bool BBox2<float>::intersects(const BBox2<float>& value) const noexcept
        {
            return !(
                value.max.x < min.x ||
                value.min.x > max.x ||
                value.max.y < min.y ||
                value.min.y > max.y);
        }

        template<typename T>
        inline BBox2<T> BBox2<T>::intersect(const BBox2<T>& value) const
        {
            BBox2<T> out;
            out.min.x = std::max(min.x, value.min.x);
            out.min.y = std::max(min.y, value.min.y);
            out.max.x = std::min(max.x, value.max.x);
            out.max.y = std::min(max.y, value.max.y);
            return out;
        }

        template<typename T>
        inline void BBox2<T>::expand(const BBox2<T>& value)
        {
            min.x = std::min(min.x, value.min.x);
            min.y = std::min(min.y, value.min.y);
            max.x = std::max(max.x, value.max.x);
            max.y = std::max(max.y, value.max.y);
        }

        template<typename T>
        inline void BBox2<T>::expand(const Vector2<T>& value)
        {
            min.x = std::min(min.x, value.x);
            min.y = std::min(min.y, value.y);
            max.x = std::max(max.x, value.x);
            max.y = std::max(max.y, value.y);
        }

        template<typename T>
        constexpr BBox2<T> BBox2<T>::margin(const Vector2<T>& value) const noexcept
        {
            return BBox2<T>(
                Vector2<T>(min.x - value.x, min.y - value.y),
                Vector2<T>(max.x + value.x, max.y + value.y));
        }

        template<typename T>
        constexpr BBox2<T> BBox2<T>::margin(T value) const noexcept
        {
            return BBox2<T>(
                Vector2<T>(min.x - value, min.y - value),
                Vector2<T>(max.x + value, max.y + value));
        }

        template<typename T>
        constexpr BBox2<T> BBox2<T>::margin(T x0, T y0, T x1, T y1) const noexcept
        {
            return BBox2<T>(
                Vector2<T>(min.x - x0, min.y - y0),
                Vector2<T>(max.x + x1, max.y + y1));
        }

        template<typename T>
        constexpr bool BBox2<T>::operator == (const BBox2<T>& value) const noexcept
        {
            return min == value.min && max == value.max;
        }

        template<typename T>
        constexpr bool BBox2<T>::operator != (const BBox2<T>& value) const noexcept
        {
            return !(*this == value);
        }
    }
}
