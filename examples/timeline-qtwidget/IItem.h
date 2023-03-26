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
                Frames
            };

            //! Item data.
            struct ItemData
            {
                std::string directory;
                io::Options ioOptions;
                file::PathOptions pathOptions;
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

                virtual void setScale(float);

                virtual void setThumbnailHeight(int);

                virtual void setViewport(const math::BBox2i&);

            protected:
                void _setScale(float, const std::shared_ptr<IItem>&);
                void _setThumbnailHeight(int, const std::shared_ptr<IItem>&);
                void _setViewport(const math::BBox2i&, const std::shared_ptr<IItem>&);

                static std::string _durationLabel(const otime::RationalTime&, TimeUnits);
                static std::string _timeLabel(const otime::RationalTime&, TimeUnits);

                ItemData _itemData;
                float _scale = 100.F;
                int _thumbnailHeight = 100;
                TimeUnits _timeUnits = TimeUnits::Seconds;
                math::BBox2i _viewport;
            };
        }
    }
}
