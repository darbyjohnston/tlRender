// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <feather-tk/core/Color.h>
#include <feather-tk/core/Size.h>
#include <feather-tk/core/Util.h>

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
        FEATHER_TK_ENUM(Background);

        //! Outline.
        struct Outline
        {
            bool         enabled = false;
            int          width = 2;
            feather_tk::Color4F color = feather_tk::Color4F(1.F, 0.F, 0.F);

            bool operator == (const Outline&) const;
            bool operator != (const Outline&) const;
        };

        //! Background options.
        struct BackgroundOptions
        {
            Background type = Background::Solid;

            feather_tk::Color4F solidColor = feather_tk::Color4F(0.F, 0.F, 0.F);

            std::pair<feather_tk::Color4F, feather_tk::Color4F> checkersColor =
            {
                feather_tk::Color4F(0.F, 0.F, 0.F),
                feather_tk::Color4F(1.F, 1.F, 1.F)
            };
            feather_tk::Size2I checkersSize = feather_tk::Size2I(100, 100);

            std::pair<feather_tk::Color4F, feather_tk::Color4F> gradientColor =
            {
                feather_tk::Color4F(0.F, 0.F, 0.F),
                feather_tk::Color4F(1.F, 1.F, 1.F)
            };

            Outline outline;

            bool operator == (const BackgroundOptions&) const;
            bool operator != (const BackgroundOptions&) const;
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const Outline&);
        void to_json(nlohmann::json&, const BackgroundOptions&);

        void from_json(const nlohmann::json&, Outline&);
        void from_json(const nlohmann::json&, BackgroundOptions&);

        ///@}
    }
}
