// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/core/Color.h>
#include <dtk/core/Size.h>
#include <dtk/core/Util.h>

namespace tl
{
    namespace timeline
    {
        //! Grid.
        struct Grid
        {
            bool         enabled = false;
            dtk::Size2I  size = dtk::Size2I(100, 100);
            dtk::Color4F color = dtk::Color4F(0.F, 0.F, 0.F);

            bool operator == (const Grid&) const;
            bool operator != (const Grid&) const;
        };

        //! Foreground options.
        struct ForegroundOptions
        {
            Grid grid;

            bool operator == (const ForegroundOptions&) const;
            bool operator != (const ForegroundOptions&) const;
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const Grid&);
        void to_json(nlohmann::json&, const ForegroundOptions&);

        void from_json(const nlohmann::json&, Grid&);
        void from_json(const nlohmann::json&, ForegroundOptions&);

        ///@}
    }
}
