// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Color.h>
#include <tlCore/Size.h>
#include <tlCore/Util.h>

namespace tl
{
    namespace timeline
    {
        //! Background type.
        enum class Background
        {
            Solid,
            Checkers,

            Count,
            First = Solid
        };
        TLRENDER_ENUM(Background);
        TLRENDER_ENUM_SERIALIZE(Background);

        //! Background options.
        struct BackgroundOptions
        {
            Background     type           = Background::Solid;
            image::Color4f solidColor     = image::Color4f(0.F, 0.F, 0.F);
            image::Color4f checkersColor0 = image::Color4f(1.F, 1.F, 1.F);
            image::Color4f checkersColor1 = image::Color4f(0.F, 0.F, 0.F);
            math::Size2i   checkersSize   = math::Size2i(100, 100);

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
