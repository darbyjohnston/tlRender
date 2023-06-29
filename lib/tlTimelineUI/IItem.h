// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IOManager.h>

#include <tlUI/IWidget.h>

#include <tlTimeline/TimeUnits.h>
#include <tlTimeline/Timeline.h>

namespace tl
{
    namespace timelineui
    {
        //! Item data.
        struct ItemData
        {
            std::string directory;
            timeline::Options options;
            std::shared_ptr<IOManager> ioManager;
            std::shared_ptr<timeline::ITimeUnitsModel> timeUnitsModel;
        };

        //! In/out points display options.
        enum class InOutDisplay
        {
            InsideRange,
            OutsideRange
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
            AudioGap,
            Transition
        };

        //! Item options.
        struct ItemOptions
        {
            ItemOptions();

            InOutDisplay inOutDisplay = InOutDisplay::InsideRange;
            CacheDisplay cacheDisplay = CacheDisplay::VideoAndAudio;
            std::map<ColorRole, imaging::Color4f> colors;
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

            std::string _durationLabel(const otime::RationalTime&);

            virtual void _timeUnitsUpdate();

            ItemData _data;
            double _scale = 500.0;
            ItemOptions _options;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
