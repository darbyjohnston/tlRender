// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/Viewport.h>

namespace tl
{
    namespace play
    {
        class ViewportColorWidget : public dtk::IWidget
        {
            DTK_NON_COPYABLE(ViewportColorWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            ViewportColorWidget();

        public:
            virtual ~ViewportColorWidget();

            static std::shared_ptr<ViewportColorWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setColor(const dtk::Color4F&);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;
            void drawEvent(const dtk::Box2I&, const dtk::DrawEvent&) override;

        private:
            void _colorUpdate();

            DTK_PRIVATE();
        };
    }
}

