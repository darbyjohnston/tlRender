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
        FTK_ENUM(Background);

        //! Outline.
        struct Outline
        {
            bool         enabled = false;
            int          width = 2;
            ftk::Color4F color = ftk::Color4F(1.F, 0.F, 0.F);

            bool operator == (const Outline&) const;
            bool operator != (const Outline&) const;
        };

        //! Background options.
        struct BackgroundOptions
        {
            Background type = Background::Solid;

            ftk::Color4F solidColor = ftk::Color4F(0.F, 0.F, 0.F);

            std::pair<ftk::Color4F, ftk::Color4F> checkersColor =
            {
                ftk::Color4F(0.F, 0.F, 0.F),
                ftk::Color4F(1.F, 1.F, 1.F)
            };
            ftk::Size2I checkersSize = ftk::Size2I(100, 100);

            std::pair<ftk::Color4F, ftk::Color4F> gradientColor =
            {
                ftk::Color4F(0.F, 0.F, 0.F),
                ftk::Color4F(1.F, 1.F, 1.F)
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
