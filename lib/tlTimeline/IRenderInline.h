// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool Color::operator == (const Color& other) const
        {
            return
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

        inline bool Exposure::operator == (const Exposure& other) const
        {
            return
                exposure == other.exposure &&
                defog == other.defog &&
                kneeLow == other.kneeLow &&
                kneeHigh == other.kneeHigh;
        }

        inline bool Exposure::operator != (const Exposure& other) const
        {
            return !(*this == other);
        }

        inline bool ImageOptions::operator == (const ImageOptions& other) const
        {
            return
                yuvRange == other.yuvRange &&
                alphaBlend == other.alphaBlend;
        }

        inline bool ImageOptions::operator != (const ImageOptions& other) const
        {
            return !(*this == other);
        }

        inline bool DisplayOptions::operator == (const DisplayOptions& other) const
        {
            return
                channels == other.channels &&
                mirror == other.mirror &&
                colorEnabled == other.colorEnabled &&
                color == other.color &&
                levelsEnabled == other.levelsEnabled &&
                levels == other.levels &&
                exposureEnabled == other.exposureEnabled &&
                exposure == other.exposure &&
                softClipEnabled == other.softClipEnabled &&
                softClip == other.softClip;
        }

        inline bool DisplayOptions::operator != (const DisplayOptions& other) const
        {
            return !(*this == other);
        }

        inline bool CompareOptions::operator == (const CompareOptions& other) const
        {
            return
                mode == other.mode &&
                wipeCenter == other.wipeCenter &&
                wipeRotation == other.wipeRotation &&
                overlay == other.overlay;
        }

        inline bool CompareOptions::operator != (const CompareOptions& other) const
        {
            return !(*this == other);
        }
    }
}
