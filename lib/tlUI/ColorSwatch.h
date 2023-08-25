// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Color swatch widget.
        class ColorSwatch : public IWidget
        {
            TLRENDER_NON_COPYABLE(ColorSwatch);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            ColorSwatch();

        public:
            virtual ~ColorSwatch();

            //! Create a new widget.
            static std::shared_ptr<ColorSwatch> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the color.
            const image::Color4f& getColor() const;

            //! Set the color.
            void setColor(const image::Color4f&);

            //! Set whether the color is editable.
            void setEditable(bool);

            //! Set the color callback.
            void setCallback(const std::function<void(const image::Color4f&)>&);

            //! Set the size role.
            void setSizeRole(SizeRole);

            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(
                const math::Box2i&,
                const DrawEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;

        private:
            void _showPopup();

            TLRENDER_PRIVATE();
        };
    }
}
