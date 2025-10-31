// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <ftk/Core/Color.h>
#include <ftk/Core/Size.h>
#include <ftk/Core/Util.h>

namespace tl
{
    namespace timeline
    {
        //! Grid.
        struct Grid
        {
            bool         enabled   = false;
            int          size      = 100;
            int          lineWidth = 2;
            ftk::Color4F color     = ftk::Color4F(0.F, 0.F, 0.F);

            bool operator == (const Grid&) const;
            bool operator != (const Grid&) const;
        };

        //! Outline.
        struct Outline
        {
            bool         enabled = false;
            int          width   = 2;
            ftk::Color4F color   = ftk::Color4F(1.F, 0.F, 0.F);

            bool operator == (const Outline&) const;
            bool operator != (const Outline&) const;
        };

        //! Foreground options.
        struct ForegroundOptions
        {
            Grid grid;
            Outline outline;

            bool operator == (const ForegroundOptions&) const;
            bool operator != (const ForegroundOptions&) const;
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const Grid&);
        void to_json(nlohmann::json&, const Outline&);
        void to_json(nlohmann::json&, const ForegroundOptions&);

        void from_json(const nlohmann::json&, Grid&);
        void from_json(const nlohmann::json&, Outline&);
        void from_json(const nlohmann::json&, ForegroundOptions&);

        ///@}
    }
}
