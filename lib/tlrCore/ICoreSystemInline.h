// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace core
    {
        inline const std::weak_ptr<Context>& ICoreSystem::getContext() const
        {
            return _context;
        }

        inline const std::string& ICoreSystem::getName() const
        {
            return _name;
        }
    }
}
