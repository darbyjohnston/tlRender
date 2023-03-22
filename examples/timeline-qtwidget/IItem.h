// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

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
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                IItem();

            public:
                ~IItem() override;

                virtual void setScale(float);

                virtual void setThumbnailHeight(int);

            protected:
                static std::string _durationLabel(const otime::RationalTime&);
                static std::string _timeLabel(const otime::RationalTime&);

                float _scale = 100.F;
                int _thumbnailHeight = 100;
            };
        }
    }
}
