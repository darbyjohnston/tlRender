// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <sstream>

#include <ftk/Core/Assert.h>

namespace tl
{
    namespace tests
    {
        template<typename T>
        inline void ITest::_enum(
            const std::string& name,
            const std::function<std::vector<T>(void)>& value)
        {
            for (auto i : value())
            {
                {
                    std::stringstream ss;
                    ss << name << ": " << i;
                    _print(ss.str());
                }
                {
                    const T v = T::First;
                    const std::string s = to_string(v);
                    T v2 = T::First;
                    from_string(s, v2);
                    FTK_ASSERT(v == v2);
                }
                {
                    std::stringstream ss;
                    ss << i;
                }
            }
        }
    }
}
