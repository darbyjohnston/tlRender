// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include "IExampleWidget.h"

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            //! Scroll areas widget.
            class ScrollAreasWidget : public ui::IWidget
            {
            protected:
                void _init(
                    const dtk::V2I& cellCount,
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent);

                ScrollAreasWidget();

            public:
                ~ScrollAreasWidget();

                static std::shared_ptr<ScrollAreasWidget> create(
                    const dtk::V2I& cellCount,
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void sizeHintEvent(const ui::SizeHintEvent&) override;
                void clipEvent(const dtk::Box2I&, bool) override;
                void drawEvent(const dtk::Box2I&, const ui::DrawEvent&) override;

            private:
                DTK_PRIVATE();
            };

            //! Scroll areas.
            class ScrollAreas : public IExampleWidget
            {
                DTK_NON_COPYABLE(ScrollAreas);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent);

                ScrollAreas();

            public:
                ~ScrollAreas();

                static std::shared_ptr<ScrollAreas> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setGeometry(const dtk::Box2I&) override;
                void sizeHintEvent(const ui::SizeHintEvent&) override;

            private:
                DTK_PRIVATE();
            };
        }
    }
}
