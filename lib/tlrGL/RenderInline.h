// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace gl
    {
        inline bool ColorConfig::operator == (const ColorConfig& other) const
        {
            return config == other.config &&
                input == other.input &&
                display == other.display &&
                view == other.view;
        }

        inline bool ColorConfig::operator != (const ColorConfig& other) const
        {
            return !(*this == other);
        }

        inline bool ImageColor::operator == (const ImageColor& other) const
        {
            return
                brightness == other.brightness &&
                contrast == other.contrast &&
                saturation == other.saturation &&
                invert == other.invert;
        }

        inline bool ImageColor::operator != (const ImageColor& other) const
        {
            return !(*this == other);
        }

        inline bool ImageLevels::operator == (const ImageLevels& other) const
        {
            return
                inLow == other.inLow &&
                inHigh == other.inHigh &&
                gamma == other.gamma &&
                outLow == other.outLow &&
                outHigh == other.outHigh;
        }

        inline bool ImageLevels::operator != (const ImageLevels& other) const
        {
            return !(*this == other);
        }

        inline bool ImageExposure::operator == (const ImageExposure& other) const
        {
            return
                exposure == other.exposure &&
                defog == other.defog &&
                kneeLow == other.kneeLow &&
                kneeHigh == other.kneeHigh;
        }

        inline bool ImageExposure::operator != (const ImageExposure& other) const
        {
            return !(*this == other);
        }

        inline bool ImageOptions::operator == (const ImageOptions& other) const
        {
            return
                channelsDisplay == other.channelsDisplay &&
                alphaBlend == other.alphaBlend &&
                mirror == other.mirror &&
                color == other.color &&
                levelsEnabled == other.levelsEnabled &&
                levels == other.levels &&
                exposureEnabled == other.exposureEnabled &&
                exposure == other.exposure &&
                softClipEnabled == other.softClipEnabled &&
                softClip == other.softClip;
        }

        inline bool ImageOptions::operator != (const ImageOptions& other) const
        {
            return !(*this == other);
        }
    }
}
