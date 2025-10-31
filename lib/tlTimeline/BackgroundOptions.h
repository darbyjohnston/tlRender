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
