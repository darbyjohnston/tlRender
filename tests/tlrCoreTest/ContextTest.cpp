// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/ContextTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Context.h>
#include <tlrCore/ISystem.h>

using namespace tlr::core;

namespace tlr
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
                TLR_NON_COPYABLE(TestSystem);

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
                TLR_ASSERT(!_context->getSystem<TestSystem>());
                _context->addSystem(testSystem);
                TLR_ASSERT(testSystem == _context->getSystem<TestSystem>());
            }
        }
    }
}
