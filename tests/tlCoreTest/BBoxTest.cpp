// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCoreTest/BBoxTest.h>

#include <tlCore/Assert.h>
#include <tlCore/BBox.h>

using namespace tl::math;

namespace tl
{
    namespace core_tests
    {
        BBoxTest::BBoxTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::BBoxTest", context)
        {}

        std::shared_ptr<BBoxTest> BBoxTest::create(const std::shared_ptr<system::Context>& context)
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
            _serialize();
        }

        void BBoxTest::_ctors()
        {
            {
                const BBox2i b;
                TLRENDER_ASSERT(0 == b.min.x);
                TLRENDER_ASSERT(0 == b.min.y);
                TLRENDER_ASSERT(0 == b.max.x);
                TLRENDER_ASSERT(0 == b.max.y);
            }
            {
                const BBox2f b;
                TLRENDER_ASSERT(0.F == b.min.x);
                TLRENDER_ASSERT(0.F == b.min.y);
                TLRENDER_ASSERT(0.F == b.max.x);
                TLRENDER_ASSERT(0.F == b.max.y);
            }
            {
                const BBox2i b(Vector2i(1, 2));
                TLRENDER_ASSERT(1 == b.min.x);
                TLRENDER_ASSERT(2 == b.min.y);
                TLRENDER_ASSERT(1 == b.max.x);
                TLRENDER_ASSERT(2 == b.max.y);
            }
            {
                const BBox2f b(Vector2f(1.F, 2.F));
                TLRENDER_ASSERT(1.F == b.min.x);
                TLRENDER_ASSERT(2.F == b.min.y);
                TLRENDER_ASSERT(1.F == b.max.x);
                TLRENDER_ASSERT(2.F == b.max.y);
            }
            {
                const BBox2i b(Vector2i(1, 2), Vector2i(3, 4));
                TLRENDER_ASSERT(1 == b.min.x);
                TLRENDER_ASSERT(2 == b.min.y);
                TLRENDER_ASSERT(3 == b.max.x);
                TLRENDER_ASSERT(4 == b.max.y);
            }
            {
                const BBox2f b(Vector2f(1.F, 2.F), Vector2f(3.F, 4.F));
                TLRENDER_ASSERT(1.F == b.min.x);
                TLRENDER_ASSERT(2.F == b.min.y);
                TLRENDER_ASSERT(3.F == b.max.x);
                TLRENDER_ASSERT(4.F == b.max.y);
            }
            {
                const BBox2i b(1, 2, 3, 4);
                TLRENDER_ASSERT(1 == b.min.x);
                TLRENDER_ASSERT(2 == b.min.y);
                TLRENDER_ASSERT(3 == b.max.x);
                TLRENDER_ASSERT(5 == b.max.y);
            }
            {
                const BBox2f b(1.F, 2.F, 3.F, 4.F);
                TLRENDER_ASSERT(1.F == b.min.x);
                TLRENDER_ASSERT(2.F == b.min.y);
                TLRENDER_ASSERT(4.F == b.max.x);
                TLRENDER_ASSERT(6.F == b.max.y);
            }
        }

        void BBoxTest::_components()
        {
            {
                const BBox2i b(1, 2, 3, 4);
                TLRENDER_ASSERT(1 == b.x());
                TLRENDER_ASSERT(2 == b.y());
                TLRENDER_ASSERT(3 == b.w());
                TLRENDER_ASSERT(4 == b.h());
            }
            {
                const BBox2f b(1.F, 2.F, 3.F, 4.F);
                TLRENDER_ASSERT(1.F == b.x());
                TLRENDER_ASSERT(2.F == b.y());
                TLRENDER_ASSERT(3.F == b.w());
                TLRENDER_ASSERT(4.F == b.h());
            }
            {
                TLRENDER_ASSERT(!BBox2i().isValid());
                TLRENDER_ASSERT(!BBox2f().isValid());
            }
            {
                BBox2i b(1, 2, 3, 4);
                b.zero();
                TLRENDER_ASSERT(0 == b.min.x);
                TLRENDER_ASSERT(0 == b.min.y);
                TLRENDER_ASSERT(0 == b.max.x);
                TLRENDER_ASSERT(0 == b.max.y);
            }
            {
                BBox2i b(1.F, 2.F, 3.F, 4.F);
                b.zero();
                TLRENDER_ASSERT(0.F == b.min.x);
                TLRENDER_ASSERT(0.F == b.min.y);
                TLRENDER_ASSERT(0.F == b.max.x);
                TLRENDER_ASSERT(0.F == b.max.y);
            }
        }

        void BBoxTest::_dimensions()
        {
            {
                BBox2i b(1, 2, 3, 4);
                TLRENDER_ASSERT(Vector2i(3, 4) == b.getSize());
                TLRENDER_ASSERT(Vector2i(2, 4) == b.getCenter());
                TLRENDER_ASSERT(12 == b.getArea());
                TLRENDER_ASSERT(3 / static_cast<float>(4) == b.getAspect());
            }
            {
                BBox2f b(1.F, 2.F, 3.F, 4.F);
                TLRENDER_ASSERT(Vector2f(3.F, 4.F) == b.getSize());
                const auto c = b.getCenter();
                TLRENDER_ASSERT(Vector2f(2.5F, 4.F) == c);
                TLRENDER_ASSERT(12.F == b.getArea());
                TLRENDER_ASSERT(3.F / 4.F == b.getAspect());
            }
        }

        void BBoxTest::_intersections()
        {
            {
                TLRENDER_ASSERT(BBox2i(0, 0, 1, 1).contains(BBox2i(0, 0, 1, 1)));
                TLRENDER_ASSERT(!BBox2i(0, 0, 1, 1).contains(BBox2i(1, 1, 1, 1)));
                TLRENDER_ASSERT(!BBox2i(0, 0, 1, 1).contains(BBox2i(-1, -1, 1, 1)));
            }
            {
                TLRENDER_ASSERT(BBox2f(0.F, 0.F, 1.F, 1.F).contains(BBox2f(0.F, 0.F, 1.F, 1.F)));
                TLRENDER_ASSERT(!BBox2f(0.F, 0.F, 1.F, 1.F).contains(BBox2f(1.F, 1.F, 1.F, 1.F)));
                TLRENDER_ASSERT(!BBox2f(0.F, 0.F, 1.F, 1.F).contains(BBox2f(-1.F, -1.F, 1.F, 1.F)));
            }
            {
                TLRENDER_ASSERT(BBox2i(0, 0, 1, 1).intersects(BBox2i(0, 0, 1, 1)));
                TLRENDER_ASSERT(!BBox2i(0, 0, 1, 1).intersects(BBox2i(2, 2, 1, 1)));
                TLRENDER_ASSERT(!BBox2i(0, 0, 1, 1).intersects(BBox2i(-2, -2, 1, 1)));
            }
            {
                TLRENDER_ASSERT(BBox2f(0.F, 0.F, 1.F, 1.F).intersects(BBox2f(0.F, 0.F, 1.F, 1.F)));
                TLRENDER_ASSERT(!BBox2f(0.F, 0.F, 1.F, 1.F).intersects(BBox2f(2.F, 2.F, 1.F, 1.F)));
                TLRENDER_ASSERT(!BBox2f(0.F, 0.F, 1.F, 1.F).intersects(BBox2f(-2.F, -2.F, 1.F, 1.F)));
            }
            {
                TLRENDER_ASSERT(BBox2i(0, 0, 1, 1).intersect(BBox2i(0, 0, 1, 1)) == BBox2i(0, 0, 1, 1));
                TLRENDER_ASSERT(BBox2i(0, 0, 1, 1).intersect(BBox2i(-1, -1, 2, 2)) == BBox2i(0, 0, 1, 1));
                TLRENDER_ASSERT(!BBox2i(BBox2i(0, 0, 1, 1).intersect(BBox2i(2, 2, 1, 1))).isValid());
                TLRENDER_ASSERT(!BBox2i(BBox2i(0, 0, 1, 1).intersect(BBox2i(-2, -2, 1, 1))).isValid());
            }
            {
                TLRENDER_ASSERT(BBox2f(0.F, 0.F, 1.F, 1.F).intersect(BBox2f(0.F, 0.F, 1.F, 1.F)) == BBox2f(0.F, 0.F, 1.F, 1.F));
                TLRENDER_ASSERT(BBox2f(0.F, 0.F, 1.F, 1.F).intersect(BBox2f(-1.F, -1.F, 2.F, 2.F)) == BBox2f(0.F, 0.F, 1.F, 1.F));
                TLRENDER_ASSERT(!BBox2f(BBox2f(0.F, 0.F, 1.F, 1.F).intersect(BBox2f(2.F, 2.F, 1.F, 1.F))).isValid());
                TLRENDER_ASSERT(!BBox2f(BBox2f(0.F, 0.F, 1.F, 1.F).intersect(BBox2f(-2.F, -2.F, 1.F, 1.F))).isValid());
            }
        }

        void BBoxTest::_expand()
        {
            {
                BBox2i b(0, 1, 2, 3);
                b.expand(BBox2i(4, 5, 6, 7));
                TLRENDER_ASSERT(BBox2i(0, 1, 10, 11) == b);
            }
            {
                BBox2f b(0.F, 1.F, 2.F, 3.F);
                b.expand(BBox2f(4.F, 5.F, 6.F, 7.F));
                TLRENDER_ASSERT(BBox2f(0.F, 1.F, 10.F, 11.F) == b);
            }
            {
                BBox2i b(0, 1, 2, 3);
                b.expand(Vector2i(6, 7));
                TLRENDER_ASSERT(BBox2i(0, 1, 7, 7) == b);
            }
            {
                BBox2f b(0.F, 1.F, 2.F, 3.F);
                b.expand(Vector2f(6.F, 7.F));
                TLRENDER_ASSERT(BBox2f(0.F, 1.F, 6.F, 6.F) == b);
            }
        }

        void BBoxTest::_margin()
        {
            {
                TLRENDER_ASSERT(BBox2i(0, 1, 2, 3).margin(Vector2i(1, 2)) == BBox2i(-1, -1, 4, 7));
                TLRENDER_ASSERT(BBox2f(0.F, 1.F, 2.F, 3.F).margin(Vector2f(1.F, 2.F)) == BBox2f(-1.F, -1.F, 4.F, 7.F));
            }
            {
                TLRENDER_ASSERT(BBox2i(0, 1, 2, 3).margin(1) == BBox2i(-1, 0, 4, 5));
                TLRENDER_ASSERT(BBox2f(0.F, 1.F, 2.F, 3.F).margin(1.F) == BBox2f(-1.F, 0.F, 4.F, 5.F));
            }
            {
                const auto b = BBox2i(0, 1, 2, 3).margin(1, 2, 3, 4);
                TLRENDER_ASSERT(BBox2i(0, 1, 2, 3).margin(1, 2, 3, 4) == BBox2i(-1, -1, 6, 9));
                const auto b2 = BBox2f(0.F, 1.F, 2.F, 3.F).margin(1.F, 2.F, 3.F, 4.F);
                TLRENDER_ASSERT(BBox2f(0.F, 1.F, 2.F, 3.F).margin(1.F, 2.F, 3.F, 4.F) == BBox2f(-1.F, -1.F, 6.F, 9.F));
            }
        }

        void BBoxTest::_operators()
        {
            {
                TLRENDER_ASSERT(BBox2i(0, 1, 2, 3) == BBox2i(0, 1, 2, 3));
                TLRENDER_ASSERT(BBox2i(0, 1, 2, 3) != BBox2i(3, 2, 1, 0));
                TLRENDER_ASSERT(BBox2f(0.F, 1.F, 2.F, 3.F) == BBox2f(0.F, 1.F, 2.F, 3.F));
                TLRENDER_ASSERT(BBox2f(0.F, 1.F, 2.F, 3.F) != BBox2f(3.F, 2.F, 1.F, 0.F));
            }
        }

        void BBoxTest::_serialize()
        {
            {
                const BBox2i b(1, 2, 3, 4);
                nlohmann::json json;
                to_json(json, b);
                BBox2i b2;
                from_json(json, b2);
                TLRENDER_ASSERT(b == b2);
            }
            {
                const BBox2f b(1.F, 2.F, 3.F, 4.F);
                nlohmann::json json;
                to_json(json, b);
                BBox2f b2;
                from_json(json, b2);
                TLRENDER_ASSERT(b == b2);
            }
            {
                const BBox2i b(1, 2, 3, 4);
                std::stringstream ss;
                ss << b;
                BBox2i b2;
                ss >> b2;
                TLRENDER_ASSERT(b == b2);
            }
            {
                const BBox2f b(1.F, 2.F, 3.F, 4.F);
                std::stringstream ss;
                ss << b;
                BBox2f b2;
                ss >> b2;
                TLRENDER_ASSERT(b == b2);
            }
            try
            {
                BBox2i b;
                std::stringstream ss("...");
                ss >> b;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {}
            try
            {
                BBox2f b;
                std::stringstream ss("...");
                ss >> b;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {}
        }
    }
}
