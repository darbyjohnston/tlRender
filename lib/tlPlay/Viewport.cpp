// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlay/ViewportPrivate.h>

#include <tlUI/GridLayout.h>
#include <tlUI/Label.h>
#include <tlUI/Spacer.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play
    {
        struct Viewport::Private
        {
            std::shared_ptr<observer::Value<bool> > hud;
            double fps = 0.0;
            size_t droppedFrames = 0;
            image::PixelType colorBuffer = image::PixelType::None;

            std::shared_ptr<ui::Label> fpsLabel;
            std::shared_ptr<ui::Label> colorBufferLabel;
            std::shared_ptr<ui::GridLayout> hudLayout;
            struct ColorPicker
            {
                image::Color4f color;
                math::Vector2i pos;
                std::shared_ptr<ViewportColorWidget> widget;
            };
            std::vector<ColorPicker> colorPickers;

            enum class MouseMode
            {
                None,
                ColorPicker,
                DragWidget
            };
            struct MouseData
            {
                MouseMode mode = MouseMode::None;
                size_t index = 0;
                math::Vector2i offset;
            };
            MouseData mouse;

            std::shared_ptr<observer::ValueObserver<double> > fpsObserver;
            std::shared_ptr<observer::ValueObserver<size_t> > droppedFramesObserver;
            std::shared_ptr<observer::ValueObserver<image::PixelType> > colorBufferObserver;
            std::shared_ptr<observer::ListObserver<image::Color4f> > colorPickersObserver;
        };

        void Viewport::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            TimelineViewport::_init(context, parent);
            TLRENDER_P();

            p.hud = observer::Value<bool>::create(false);

            p.fpsLabel = ui::Label::create(context);
            p.fpsLabel->setFontRole(ui::FontRole::Mono);
            p.fpsLabel->setMarginRole(ui::SizeRole::MarginInside);
            p.fpsLabel->setBackgroundRole(ui::ColorRole::Base);

            p.colorBufferLabel = ui::Label::create(context);
            p.colorBufferLabel->setFontRole(ui::FontRole::Mono);
            p.colorBufferLabel->setMarginRole(ui::SizeRole::MarginInside);
            p.colorBufferLabel->setBackgroundRole(ui::ColorRole::Base);

            p.hudLayout = ui::GridLayout::create(context, shared_from_this());
            p.hudLayout->setMarginRole(ui::SizeRole::MarginSmall);
            p.hudLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
            p.fpsLabel->setParent(p.hudLayout);
            p.hudLayout->setGridPos(p.fpsLabel, 0, 0);
            p.colorBufferLabel->setParent(p.hudLayout);
            p.hudLayout->setGridPos(p.colorBufferLabel, 0, 2);
            auto spacer = ui::Spacer::create(ui::Orientation::Horizontal, context, p.hudLayout);
            spacer->setStretch(ui::Stretch::Expanding, ui::Stretch::Expanding);
            p.hudLayout->setGridPos(spacer, 1, 1);
            p.hudLayout->hide();

            p.fpsObserver = observer::ValueObserver<double>::create(
                observeFPS(),
                [this](double value)
                {
                    _p->fps = value;
                    _hudUpdate();
                });
            p.droppedFramesObserver = observer::ValueObserver<size_t>::create(
                observeDroppedFrames(),
                [this](size_t value)
                {
                    _p->droppedFrames = value;
                    _hudUpdate();
                });

            p.colorBufferObserver = observer::ValueObserver<image::PixelType>::create(
                observeColorBuffer(),
                [this](image::PixelType value)
                {
                    _p->colorBuffer = value;
                    _hudUpdate();
                });

            p.colorPickersObserver = observer::ListObserver<image::Color4f>::create(
                observeColorPickers(),
                [this](const std::vector<image::Color4f>& value)
                {
                    for (size_t i = 0; i < value.size() && i < _p->colorPickers.size(); ++i)
                    {
                        _p->colorPickers[i].color = value[i];
                    }
                    _colorWidgetsUpdate();
                });
        }

        Viewport::Viewport() :
            _p(new Private)
        {}

        Viewport::~Viewport()
        {}

        std::shared_ptr<Viewport> Viewport::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Viewport>(new Viewport);
            out->_init(context, parent);
            return out;
        }

        bool Viewport::hasHUD() const
        {
            return _p->hud->get();
        }

        std::shared_ptr<observer::IValue<bool> > Viewport::observeHUD() const
        {
            return _p->hud;
        }

        void Viewport::setHUD(bool value)
        {
            TLRENDER_P();
            if (p.hud->setIfChanged(value))
            {
                p.hudLayout->setVisible(value);
            }
        }

        void Viewport::setGeometry(const math::Box2i& value)
        {
            TimelineViewport::setGeometry(value);
            TLRENDER_P();
            p.hudLayout->setGeometry(value);
            for (const auto& colorPicker : p.colorPickers)
            {
                math::Size2i sizeHint = colorPicker.widget->getSizeHint();
                colorPicker.widget->setGeometry(math::Box2i(
                    colorPicker.pos.x,
                    colorPicker.pos.y,
                    sizeHint.w,
                    sizeHint.h));
            }
        }

        void Viewport::childRemovedEvent(const ui::ChildEvent& event)
        {
            TimelineViewport::childRemovedEvent(event);
            TLRENDER_P();
            const auto i = std::find_if(
                p.colorPickers.begin(),
                p.colorPickers.end(),
                [event](const Private::ColorPicker& value)
                {
                    return event.child == value.widget;
                });
            if (i != p.colorPickers.end())
            {
                p.colorPickers.erase(i);
                _colorPickersUpdate();
            }
        }

        void Viewport::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            TimelineViewport::sizeHintEvent(event);
            TLRENDER_P();
            _sizeHint = p.hudLayout->getSizeHint();
        }

        void Viewport::mouseMoveEvent(ui::MouseMoveEvent& event)
        {
            TimelineViewport::mouseMoveEvent(event);
            TLRENDER_P();
            switch (p.mouse.mode)
            {
            case Private::MouseMode::ColorPicker:
                if (!p.colorPickers.empty())
                {
                    p.colorPickers.back().pos = event.pos;
                    _colorPickersUpdate();
                    _updates |= ui::Update::Size;
                    _updates |= ui::Update::Draw;
                }
                break;
            case Private::MouseMode::DragWidget:
                if (p.mouse.index < p.colorPickers.size())
                {
                    p.colorPickers[p.mouse.index].pos = event.pos - p.mouse.offset;
                    _colorPickersUpdate();
                    _updates |= ui::Update::Size;
                    _updates |= ui::Update::Draw;
                }
                break;
            default: break;
            }
        }

        void Viewport::mousePressEvent(ui::MouseClickEvent& event)
        {
            TimelineViewport::mousePressEvent(event);
            TLRENDER_P();
            takeKeyFocus();
            if (0 == event.button)
            {
                for (size_t i = 0; i < p.colorPickers.size(); ++i)
                {
                    if (p.colorPickers[i].widget->getGeometry().contains(event.pos))
                    {
                        p.mouse.mode = Private::MouseMode::DragWidget;
                        p.mouse.index = i;
                        p.mouse.offset = event.pos - p.colorPickers[i].widget->getGeometry().min;
                    }
                }
            }
            if (Private::MouseMode::None == p.mouse.mode &&
                0 == event.button &&
                event.modifiers & static_cast<int>(ui::KeyModifier::Shift))
            {
                if (auto context = _context.lock())
                {
                    p.mouse.mode = Private::MouseMode::ColorPicker;
                    p.colorPickers.push_back({
                        image::Color4f(),
                        event.pos,
                        ViewportColorWidget::create(context, shared_from_this()) });
                    _colorPickersUpdate();
                    _colorWidgetsUpdate();
                    _updates |= ui::Update::Size;
                    _updates |= ui::Update::Draw;
                }
            }
        }

        void Viewport::mouseReleaseEvent(ui::MouseClickEvent& event)
        {
            TimelineViewport::mouseReleaseEvent(event);
            TLRENDER_P();
            p.mouse = Private::MouseData();
        }

        void Viewport::_hudUpdate()
        {
            TLRENDER_P();
            p.fpsLabel->setText(
                string::Format("FPS: {0} ({1} dropped)").
                arg(p.fps, 2, 4).
                arg(p.droppedFrames));
            p.colorBufferLabel->setText(
                string::Format("Color buffer: {0}").
                arg(p.colorBuffer));
        }

        void Viewport::_colorPickersUpdate()
        {
            TLRENDER_P();
            std::vector<math::Vector2i> colorPickers;
            for (const auto& colorPicker : p.colorPickers)
            {
                colorPickers.push_back(colorPicker.pos);
            }
            setColorPickers(colorPickers);
        }

        void Viewport::_colorWidgetsUpdate()
        {
            TLRENDER_P();
            for (const auto& colorPicker : p.colorPickers)
            {
                colorPicker.widget->setColor(colorPicker.color);
            }
        }
    }
}
