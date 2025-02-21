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
        //! Background type.
        enum class Background
        {
            Solid,
            Checkers,
            Gradient,

            Count,
            First = Solid
        };
        DTK_ENUM(Background);

        //! Background options.
        struct BackgroundOptions
        {
            Background type = Background::Solid;

            dtk::Color4F solidColor = dtk::Color4F(0.F, 0.F, 0.F);

            std::pair<dtk::Color4F, dtk::Color4F> checkersColor =
            {
                dtk::Color4F(0.F, 0.F, 0.F),
                dtk::Color4F(1.F, 1.F, 1.F)
            };
            dtk::Size2I checkersSize = dtk::Size2I(100, 100);

            std::pair<dtk::Color4F, dtk::Color4F> gradientColor =
            {
                dtk::Color4F(0.F, 0.F, 0.F),
                dtk::Color4F(1.F, 1.F, 1.F)
            };

            bool operator == (const BackgroundOptions&) const;
            bool operator != (const BackgroundOptions&) const;
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const BackgroundOptions&);

        void from_json(const nlohmann::json&, BackgroundOptions&);

        ///@}
    }
}

#include <tlTimeline/BackgroundOptionsInline.h>
