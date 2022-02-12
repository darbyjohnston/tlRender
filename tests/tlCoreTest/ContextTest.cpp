// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/ContextTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Context.h>
#include <tlCore/ISystem.h>

using namespace tl::core;

namespace tl
{
    namespace CoreTest
    {
        ContextTest::ContextTest(const std::shared_ptr<Context>& context) :
            ITest("CoreTest::ContextTest", context)
        {}

        std::shared_ptr<ContextTest> ContextTest::create(const std::shared_ptr<Context>& context)
        {
            return std::shared_ptr<ContextTest>(new ContextTest(context));
        }
        
        namespace
        {
            class TestSystem : public ISystem
            {
                TLRENDER_NON_COPYABLE(TestSystem);

            protected:
                void _init(const std::shared_ptr<Context>& context)
                {
                    ISystem::_init("TestSystem", context);
                    _log("Hello world!");
                    _log("Hello world!", LogType::Warning);
                    _log("Hello world!", LogType::Error);
                }

                TestSystem()
                {}

            public:
                static std::shared_ptr<TestSystem> create(const std::shared_ptr<Context>& context)
                {
                    auto out = std::shared_ptr<TestSystem>(new TestSystem);
                    out->_init(context);
                    return out;
                }
            };
        }

        void ContextTest::run()
        {
            {
                auto testSystem = TestSystem::create(_context);
                TLRENDER_ASSERT(!_context->getSystem<TestSystem>());
                _context->addSystem(testSystem);
                TLRENDER_ASSERT(testSystem == _context->getSystem<TestSystem>());
            }
        }
    }
}
