// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool Color::operator == (const Color& other) const
        {
            return
                enabled == other.enabled &&
                add == other.add &&
                brightness == other.brightness &&
                contrast == other.contrast &&
                saturation == other.saturation &&
                tint == other.tint &&
                invert == other.invert;
        }

        inline bool Color::operator != (const Color& other) const
        {
            return !(*this == other);
        }

        inline bool Levels::operator == (const Levels& other) const
        {
            return
                enabled == other.enabled &&
                inLow == other.inLow &&
                inHigh == other.inHigh &&
                gamma == other.gamma &&
                outLow == other.outLow &&
                outHigh == other.outHigh;
        }

        inline bool Levels::operator != (const Levels& other) const
        {
            return !(*this == other);
        }

        inline bool EXRDisplay::operator == (const EXRDisplay& other) const
        {
            return
                enabled == other.enabled &&
                exposure == other.exposure &&
                defog == other.defog &&
                kneeLow == other.kneeLow &&
                kneeHigh == other.kneeHigh;
        }

        inline bool EXRDisplay::operator != (const EXRDisplay& other) const
        {
            return !(*this == other);
        }

        inline bool SoftClip::operator == (const SoftClip& other) const
        {
            return
                enabled == other.enabled &&
                value == other.value;
        }

        inline bool SoftClip::operator != (const SoftClip& other) const
        {
            return !(*this == other);
        }

        inline bool Outline::operator == (const Outline& other) const
        {
            return
                enabled == other.enabled &&
                width == other.width &&
                color == other.color;
        }

        inline bool Outline::operator != (const Outline& other) const
        {
            return !(*this == other);
        }

        inline bool Grid::operator == (const Grid& other) const
        {
            return
                enabled == other.enabled &&
                size == other.size &&
                color == other.color;
        }

        inline bool Grid::operator != (const Grid& other) const
        {
            return !(*this == other);
        }

        inline bool DisplayOptions::operator == (const DisplayOptions& other) const
        {
            return
                channels == other.channels &&
                mirror == other.mirror &&
                color == other.color &&
                levels == other.levels &&
                exrDisplay == other.exrDisplay &&
                softClip == other.softClip &&
                imageFilters == other.imageFilters &&
                videoLevels == other.videoLevels &&
                outline == other.outline &&
                grid == other.grid;
        }

        inline bool DisplayOptions::operator != (const DisplayOptions& other) const
        {
            return !(*this == other);
        }
    }
}
