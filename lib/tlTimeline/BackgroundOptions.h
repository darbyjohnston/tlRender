// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Size.h>
#include <tlCore/Util.h>

#include <dtk/core/Color.h>

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
        TLRENDER_ENUM(Background);
        TLRENDER_ENUM_SERIALIZE(Background);

        //! Background options.
        struct BackgroundOptions
        {
            Background   type         = Background::Solid;
            dtk::Color4F color0       = dtk::Color4F(0.F, 0.F, 0.F);
            dtk::Color4F color1       = dtk::Color4F(0.F, 0.F, 0.F);
            math::Size2i checkersSize = math::Size2i(100, 100);

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
