// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlCoreTest/URLTest.h>

#include <tlCore/URL.h>

#include <ftk/Core/Format.h>

#include <sstream>

using namespace tl::url;

namespace tl
{
    namespace core_tests
    {
        URLTest::URLTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "core_tests::URLTest")
        {}

        std::shared_ptr<URLTest> URLTest::create(const std::shared_ptr<ftk::Context>& context)
        {
            return std::shared_ptr<URLTest>(new URLTest(context));
        }

        void URLTest::run()
        {
            _util();
            _encode();
        }
                
        void URLTest::_util()
        {
            {
                const std::string scheme = url::scheme("file://path");
                FTK_ASSERT("file://" == scheme);
            }
            {
                const std::string scheme = url::scheme("path");
                FTK_ASSERT(scheme.empty());
            }
        }
        
        void URLTest::_encode()
        {
            struct Data
            {
                std::string encoded;
                std::string decoded;
            };
            const std::vector<Data> data =
            {
                { "NoSpaces", "NoSpaces" },
                { "With%20Spaces", "With Spaces" },
                { "%20With%20Spaces", " With Spaces" },
                { "With%20Spaces%20", "With Spaces " },
                { "%20With%20Spaces%20", " With Spaces " }
            };
            for (const auto& i : data)
            {
                const std::string decoded = decode(i.encoded);
                FTK_ASSERT(decoded == i.decoded);
                const std::string encoded = encode(decoded);
                FTK_ASSERT(encoded == i.encoded);
            }
        }
    }
}
