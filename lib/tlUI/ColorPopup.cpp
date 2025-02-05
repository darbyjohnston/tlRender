// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/ColorPopup.h>

#include <tlUI/ColorWidget.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace ui
    {
        struct ColorPopup::Private
        {
            std::shared_ptr<ColorWidget> widget;
            std::shared_ptr<VerticalLayout> layout;
            std::function<void(const dtk::Color4F&)> callback;
        };

        void ColorPopup::_init(
            const dtk::Color4F& color,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidgetPopup::_init("tl::ui::ColorPopup", context, parent);
            DTK_P();

            p.widget = ColorWidget::create(context);
            p.widget->setColor(color);

            p.layout = VerticalLayout::create(context);
            p.layout->setMarginRole(SizeRole::MarginInside);
            p.widget->setParent(p.layout);
            setWidget(p.layout);

            p.widget->setCallback(
                [this](const dtk::Color4F& value)
                {
                    if (_p->callback)
                    {
                        _p->callback(value);
                    }
                });
        }

        ColorPopup::ColorPopup() :
            _p(new Private)
        {}

        ColorPopup::~ColorPopup()
        {}

        std::shared_ptr<ColorPopup> ColorPopup::create(
            const dtk::Color4F& color,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ColorPopup>(new ColorPopup);
            out->_init(color, context, parent);
            return out;
        }

        void ColorPopup::setCallback(const std::function<void(const dtk::Color4F&)>& value)
        {
            _p->callback = value;
        }
    }
}
