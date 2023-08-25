// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Color widget.
        class ColorWidget : public IWidget
        {
            TLRENDER_NON_COPYABLE(ColorWidget);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            ColorWidget();

        public:
            virtual ~ColorWidget();

            //! Create a new widget.
            static std::shared_ptr<ColorWidget> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the color.
            const image::Color4f& getColor() const;

            //! Set the color.
            void setColor(const image::Color4f&);

            //! Set the color callback.
            void setCallback(const std::function<void(const image::Color4f&)>&);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            void _colorUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
