// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <opentimelineio/composition.h>

namespace tlr
{
    namespace timeline
    {
        template<typename T>
        inline const T* getParent(const otio::Item* value)
        {
            const T* out = nullptr;
            while (value)
            {
                if (auto t = dynamic_cast<const T*>(value))
                {
                    out = t;
                    break;
                }
                value = value->parent();
            }
            return out;
        }
    }
}
