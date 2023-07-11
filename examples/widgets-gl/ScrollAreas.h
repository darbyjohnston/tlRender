// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "IExampleWidget.h"

namespace tl
{
    namespace examples
    {
        namespace widgets_gl
        {
            //! Scroll areas widget.
            class ScrollAreasWidget : public ui::IWidget
            {
            protected:
                void _init(
                    const math::Vector2i& cellCount,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent);

                ScrollAreasWidget();

            public:
                ~ScrollAreasWidget();

                static std::shared_ptr<ScrollAreasWidget> create(
                    const math::Vector2i& cellCount,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void sizeHintEvent(const ui::SizeHintEvent&) override;
                void clipEvent(
                    const math::BBox2i&,
                    bool,
                    const ui::ClipEvent&) override;
                void drawEvent(
                    const math::BBox2i&,
                    const ui::DrawEvent&) override;

            private:
                TLRENDER_PRIVATE();
            };

            //! Scroll areas.
            class ScrollAreas : public IExampleWidget
            {
                TLRENDER_NON_COPYABLE(ScrollAreas);

            protected:
                void _init(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent);

                ScrollAreas();

            public:
                ~ScrollAreas();

                static std::shared_ptr<ScrollAreas> create(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setGeometry(const math::BBox2i&) override;
                void sizeHintEvent(const ui::SizeHintEvent&) override;

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
