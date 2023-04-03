// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

#include <tlIO/IO.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Time units.
            enum class TimeUnits
            {
                Seconds,
                Frames,
                Timecode,

                Count,
                First = Seconds
            };
            TLRENDER_ENUM(TimeUnits);
            TLRENDER_ENUM_SERIALIZE(TimeUnits);

            //! Item data.
            struct ItemData
            {
                std::string directory;
                io::Options ioOptions;
                file::PathOptions pathOptions;
            };

            //! Item options.
            struct ItemOptions
            {
                TimeUnits timeUnits = TimeUnits::Seconds;
                float scale = 100.F;
                int thumbnailHeight = 100;
                int waveformHeight = 50;

                bool operator == (const ItemOptions&) const;
                bool operator != (const ItemOptions&) const;
            };

            //! Base class for timeline items.
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

                virtual void setOptions(const ItemOptions&);

                virtual void setViewport(const math::BBox2i&);

            protected:
                bool _insideViewport() const;

                static std::string _durationLabel(const otime::RationalTime&, TimeUnits);
                static std::string _timeLabel(const otime::RationalTime&, TimeUnits);

                ItemData _data;
                ItemOptions _options;
                math::BBox2i _viewport;
            };
        }
    }
}
