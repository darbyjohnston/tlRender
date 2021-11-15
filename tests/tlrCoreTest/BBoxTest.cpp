// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/BBoxTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/BBox.h>

using namespace tlr::math;

namespace tlr
{
    namespace CoreTest
    {
        BBoxTest::BBoxTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::BBoxTest", context)
        {}

        std::shared_ptr<BBoxTest> BBoxTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<BBoxTest>(new BBoxTest(context));
        }

        void BBoxTest::run()
        {
            _ctors();
            _components();
            _dimensions();
            _intersections();
            _expand();
            _margin();
            _operators();
        }

        void BBoxTest::_ctors()
        {
            {
                const BBox2i b;
                TLR_ASSERT(0 == b.min.x);
                TLR_ASSERT(0 == b.min.y);
                TLR_ASSERT(0 == b.max.x);
                TLR_ASSERT(0 == b.max.y);
            }
            {
                const BBox2f b;
                TLR_ASSERT(0.F == b.min.x);
                TLR_ASSERT(0.F == b.min.y);
                TLR_ASSERT(0.F == b.max.x);
                TLR_ASSERT(0.F == b.max.y);
            }
            {
                const BBox2i b(glm::ivec2(1, 2));
                TLR_ASSERT(1 == b.min.x);
                TLR_ASSERT(2 == b.min.y);
                TLR_ASSERT(1 == b.max.x);
                TLR_ASSERT(2 == b.max.y);
            }
            {
                const BBox2f b(glm::vec2(1.F, 2.F));
                TLR_ASSERT(1.F == b.min.x);
                TLR_ASSERT(2.F == b.min.y);
                TLR_ASSERT(1.F == b.max.x);
                TLR_ASSERT(2.F == b.max.y);
            }
            {
                const BBox2i b(glm::ivec2(1, 2), glm::ivec2(3, 4));
                TLR_ASSERT(1 == b.min.x);
                TLR_ASSERT(2 == b.min.y);
                TLR_ASSERT(3 == b.max.x);
                TLR_ASSERT(4 == b.max.y);
            }
            {
                const BBox2f b(glm::vec2(1.F, 2.F), glm::vec2(3.F, 4.F));
                TLR_ASSERT(1.F == b.min.x);
                TLR_ASSERT(2.F == b.min.y);
                TLR_ASSERT(3.F == b.max.x);
                TLR_ASSERT(4.F == b.max.y);
            }
            {
                const BBox2i b(1, 2, 3, 4);
                TLR_ASSERT(1 == b.min.x);
                TLR_ASSERT(2 == b.min.y);
                TLR_ASSERT(3 == b.max.x);
                TLR_ASSERT(5 == b.max.y);
            }
            {
                const BBox2f b(1.F, 2.F, 3.F, 4.F);
                TLR_ASSERT(1.F == b.min.x);
                TLR_ASSERT(2.F == b.min.y);
                TLR_ASSERT(4.F == b.max.x);
                TLR_ASSERT(6.F == b.max.y);
            }
        }

        void BBoxTest::_components()
        {
            {
                const BBox2i b(1, 2, 3, 4);
                TLR_ASSERT(1 == b.x());
                TLR_ASSERT(2 == b.y());
                TLR_ASSERT(3 == b.w());
                TLR_ASSERT(4 == b.h());
            }
            {
                const BBox2f b(1.F, 2.F, 3.F, 4.F);
                TLR_ASSERT(1.F == b.x());
                TLR_ASSERT(2.F == b.y());
                TLR_ASSERT(3.F == b.w());
                TLR_ASSERT(4.F == b.h());
            }
            {
                TLR_ASSERT(!BBox2i().isValid());
                TLR_ASSERT(!BBox2f().isValid());
            }
            {
                BBox2i b(1, 2, 3, 4);
                b.zero();
                TLR_ASSERT(0 == b.min.x);
                TLR_ASSERT(0 == b.min.y);
                TLR_ASSERT(0 == b.max.x);
                TLR_ASSERT(0 == b.max.y);
            }
            {
                BBox2i b(1.F, 2.F, 3.F, 4.F);
                b.zero();
                TLR_ASSERT(0.F == b.min.x);
                TLR_ASSERT(0.F == b.min.y);
                TLR_ASSERT(0.F == b.max.x);
                TLR_ASSERT(0.F == b.max.y);
            }
        }

        void BBoxTest::_dimensions()
        {
            {
                BBox2i b(1, 2, 3, 4);
                TLR_ASSERT(glm::ivec2(3, 4) == b.getSize());
                TLR_ASSERT(glm::ivec2(2, 4) == b.getCenter());
                TLR_ASSERT(12 == b.getArea());
                TLR_ASSERT(3 / static_cast<float>(4) == b.getAspect());
            }
            {
                BBox2f b(1.F, 2.F, 3.F, 4.F);
                TLR_ASSERT(glm::vec2(3.F, 4.F) == b.getSize());
                const auto c = b.getCenter();
                TLR_ASSERT(glm::vec2(2.5F, 4.F) == c);
                TLR_ASSERT(12.F == b.getArea());
                TLR_ASSERT(3.F / 4.F == b.getAspect());
            }
        }

        void BBoxTest::_intersections()
        {
            {
                TLR_ASSERT(BBox2i(0, 0, 1, 1).contains(BBox2i(0, 0, 1, 1)));
                TLR_ASSERT(!BBox2i(0, 0, 1, 1).contains(BBox2i(1, 1, 1, 1)));
                TLR_ASSERT(!BBox2i(0, 0, 1, 1).contains(BBox2i(-1, -1, 1, 1)));
            }
            {
                TLR_ASSERT(BBox2f(0.F, 0.F, 1.F, 1.F).contains(BBox2f(0.F, 0.F, 1.F, 1.F)));
                TLR_ASSERT(!BBox2f(0.F, 0.F, 1.F, 1.F).contains(BBox2f(1.F, 1.F, 1.F, 1.F)));
                TLR_ASSERT(!BBox2f(0.F, 0.F, 1.F, 1.F).contains(BBox2f(-1.F, -1.F, 1.F, 1.F)));
            }
            {
                TLR_ASSERT(BBox2i(0, 0, 1, 1).intersects(BBox2i(0, 0, 1, 1)));
                TLR_ASSERT(!BBox2i(0, 0, 1, 1).intersects(BBox2i(2, 2, 1, 1)));
                TLR_ASSERT(!BBox2i(0, 0, 1, 1).intersects(BBox2i(-2, -2, 1, 1)));
            }
            {
                TLR_ASSERT(BBox2f(0.F, 0.F, 1.F, 1.F).intersects(BBox2f(0.F, 0.F, 1.F, 1.F)));
                TLR_ASSERT(!BBox2f(0.F, 0.F, 1.F, 1.F).intersects(BBox2f(2.F, 2.F, 1.F, 1.F)));
                TLR_ASSERT(!BBox2f(0.F, 0.F, 1.F, 1.F).intersects(BBox2f(-2.F, -2.F, 1.F, 1.F)));
            }
            {
                TLR_ASSERT(BBox2i(0, 0, 1, 1).intersect(BBox2i(0, 0, 1, 1)) == BBox2i(0, 0, 1, 1));
                TLR_ASSERT(BBox2i(0, 0, 1, 1).intersect(BBox2i(-1, -1, 2, 2)) == BBox2i(0, 0, 1, 1));
                TLR_ASSERT(!BBox2i(BBox2i(0, 0, 1, 1).intersect(BBox2i(2, 2, 1, 1))).isValid());
                TLR_ASSERT(!BBox2i(BBox2i(0, 0, 1, 1).intersect(BBox2i(-2, -2, 1, 1))).isValid());
            }
            {
                TLR_ASSERT(BBox2f(0.F, 0.F, 1.F, 1.F).intersect(BBox2f(0.F, 0.F, 1.F, 1.F)) == BBox2f(0.F, 0.F, 1.F, 1.F));
                TLR_ASSERT(BBox2f(0.F, 0.F, 1.F, 1.F).intersect(BBox2f(-1.F, -1.F, 2.F, 2.F)) == BBox2f(0.F, 0.F, 1.F, 1.F));
                TLR_ASSERT(!BBox2f(BBox2f(0.F, 0.F, 1.F, 1.F).intersect(BBox2f(2.F, 2.F, 1.F, 1.F))).isValid());
                TLR_ASSERT(!BBox2f(BBox2f(0.F, 0.F, 1.F, 1.F).intersect(BBox2f(-2.F, -2.F, 1.F, 1.F))).isValid());
            }
        }

        void BBoxTest::_expand()
        {
            {
                BBox2i b(0, 1, 2, 3);
                b.expand(BBox2i(4, 5, 6, 7));
                TLR_ASSERT(BBox2i(0, 1, 10, 11) == b);
            }
            {
                BBox2f b(0.F, 1.F, 2.F, 3.F);
                b.expand(BBox2f(4.F, 5.F, 6.F, 7.F));
                TLR_ASSERT(BBox2f(0.F, 1.F, 10.F, 11.F) == b);
            }
            {
                BBox2i b(0, 1, 2, 3);
                b.expand(glm::ivec2(6, 7));
                TLR_ASSERT(BBox2i(0, 1, 7, 7) == b);
            }
            {
                BBox2f b(0.F, 1.F, 2.F, 3.F);
                b.expand(glm::vec2(6.F, 7.F));
                TLR_ASSERT(BBox2f(0.F, 1.F, 6.F, 6.F) == b);
            }
        }

        void BBoxTest::_margin()
        {
            {
                TLR_ASSERT(BBox2i(0, 1, 2, 3).margin(glm::ivec2(1, 2)) == BBox2i(-1, -1, 4, 7));
                TLR_ASSERT(BBox2f(0.F, 1.F, 2.F, 3.F).margin(glm::vec2(1.F, 2.F)) == BBox2f(-1.F, -1.F, 4.F, 7.F));
            }
            {
                TLR_ASSERT(BBox2i(0, 1, 2, 3).margin(1) == BBox2i(-1, 0, 4, 5));
                TLR_ASSERT(BBox2f(0.F, 1.F, 2.F, 3.F).margin(1.F) == BBox2f(-1.F, 0.F, 4.F, 5.F));
            }
            {
                const auto b = BBox2i(0, 1, 2, 3).margin(1, 2, 3, 4);
                TLR_ASSERT(BBox2i(0, 1, 2, 3).margin(1, 2, 3, 4) == BBox2i(-1, -1, 6, 9));
                const auto b2 = BBox2f(0.F, 1.F, 2.F, 3.F).margin(1.F, 2.F, 3.F, 4.F);
                TLR_ASSERT(BBox2f(0.F, 1.F, 2.F, 3.F).margin(1.F, 2.F, 3.F, 4.F) == BBox2f(-1.F, -1.F, 6.F, 9.F));
            }
        }

        void BBoxTest::_operators()
        {
            {
                TLR_ASSERT(BBox2i(0, 1, 2, 3) == BBox2i(0, 1, 2, 3));
                TLR_ASSERT(BBox2i(0, 1, 2, 3) != BBox2i(3, 2, 1, 0));
                TLR_ASSERT(BBox2f(0.F, 1.F, 2.F, 3.F) == BBox2f(0.F, 1.F, 2.F, 3.F));
                TLR_ASSERT(BBox2f(0.F, 1.F, 2.F, 3.F) != BBox2f(3.F, 2.F, 1.F, 0.F));
            }
        }
    }
}
