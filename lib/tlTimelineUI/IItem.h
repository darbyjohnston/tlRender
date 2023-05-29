// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IOManager.h>

#include <tlUI/IWidget.h>

#include <tlTimeline/TimeUnits.h>

namespace tl
{
    namespace timelineui
    {
        //! Item data.
        struct ItemData
        {
            std::string directory;
            file::PathOptions pathOptions;
            std::shared_ptr<IOManager> ioManager;
        };

        //! Cache display options.
        enum class CacheDisplay
        {
            VideoAndAudio,
            VideoOnly
        };

        //! Timeline color roles.
        enum class ColorRole
        {
            InOut,
            VideoCache,
            AudioCache,
            VideoClip,
            VideoGap,
            AudioClip,
            AudioGap
        };

        //! Item options.
        struct ItemOptions
        {
            timeline::TimeUnits timeUnits = timeline::TimeUnits::Timecode;
            CacheDisplay cacheDisplay = CacheDisplay::VideoAndAudio;
            std::map<ColorRole, imaging::Color4f> colors =
            {
                { ColorRole::InOut, imaging::Color4f(1.F, .7F, .2F, .1F) },
                { ColorRole::VideoCache, imaging::Color4f(.2F, .4F, .4F) },
                { ColorRole::AudioCache, imaging::Color4f(.3F, .25F, .4F) },
                { ColorRole::VideoClip, imaging::Color4f(.2F, .4F, .4F) },
                { ColorRole::VideoGap, imaging::Color4f(.25F, .31F, .31F) },
                { ColorRole::AudioClip, imaging::Color4f(.3F, .25F, .4F) },
                { ColorRole::AudioGap, imaging::Color4f(.25F, .24F, .3F) }
            };
            float clipRectScale = 2.F;
            bool thumbnails = true;
            int thumbnailHeight = 100;
            int waveformHeight = 50;
            float thumbnailFade = .5F;

            bool operator == (const ItemOptions&) const;
            bool operator != (const ItemOptions&) const;
        };

        //! Base class for items.
        class IItem : public ui::IWidget
        {
        protected:
            void _init(
                const std::string& name,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IItem();

        public:
            ~IItem() override;

            virtual void setScale(double);
            virtual void setOptions(const ItemOptions&);

        protected:
            static math::BBox2i _getClipRect(
                const math::BBox2i&,
                double scale);

            static std::string _durationLabel(
                const otime::RationalTime&,
                timeline::TimeUnits);
            static std::string _timeLabel(
                const otime::RationalTime&,
                timeline::TimeUnits);

            ItemData _data;
            double _scale = 500.0;
            ItemOptions _options;
        };
    }
}
