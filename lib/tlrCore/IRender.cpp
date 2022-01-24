// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrCore/IRender.h>

#include <tlrCore/Error.h>
#include <tlrCore/String.h>

#include <array>

namespace tlr
{
    namespace render
    {
        TLR_ENUM_IMPL(
            YUVRange,
            "FromFile",
            "Full",
            "Video");
        TLR_ENUM_SERIALIZE_IMPL(YUVRange);

        TLR_ENUM_IMPL(
            ImageChannelsDisplay,
            "Color",
            "Red",
            "Green",
            "Blue",
            "Alpha");
        TLR_ENUM_SERIALIZE_IMPL(ImageChannelsDisplay);

        TLR_ENUM_IMPL(
            AlphaBlend,
            "None",
            "Straight",
            "Premultiplied");
        TLR_ENUM_SERIALIZE_IMPL(AlphaBlend);

        TLR_ENUM_IMPL(
            CompareMode,
            "A",
            "B",
            "Wipe",
            "Tiles");
        TLR_ENUM_SERIALIZE_IMPL(CompareMode);

        void IRender::_init(const std::shared_ptr<core::Context>& context)
        {
            _context = context;
        }

        IRender::IRender()
        {}

        IRender::~IRender()
        {}
    }
}
