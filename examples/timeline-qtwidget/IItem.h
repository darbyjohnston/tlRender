// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

#include <tlTimeline/Timeline.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Base class for timeline items.
            class IItem : public ui::IWidget
            {
            protected:
                void _init(
                    const std::string& name,
                    const std::shared_ptr<timeline::Timeline>&,
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

                static std::string _durationLabel(const otime::RationalTime&);
                static std::string _secondsLabel(const otime::RationalTime&);
                static std::string _frameLabel(const otime::RationalTime&);

                std::shared_ptr<timeline::Timeline> _timeline;
                float _scale = 100.F;
                int _thumbnailHeight = 100;
                math::BBox2i _viewport;
            };
        }
    }
}
