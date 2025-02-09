// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlay/ViewportPrivate.h>

#include <dtk/ui/GridLayout.h>
#include <dtk/ui/Label.h>
#include <dtk/ui/Spacer.h>
#include <dtk/core/Format.h>

namespace tl
{
    namespace play
    {
        struct Viewport::Private
        {
            std::shared_ptr<dtk::ObservableValue<bool> > hud;
            double fps = 0.0;
            size_t droppedFrames = 0;
            dtk::ImageType colorBuffer = dtk::ImageType::None;

            std::shared_ptr<dtk::Label> fpsLabel;
            std::shared_ptr<dtk::Label> colorBufferLabel;
            std::shared_ptr<dtk::GridLayout> hudLayout;
            struct ColorPicker
            {
                dtk::Color4F color;
                dtk::V2I pos;
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
                dtk::V2I offset;
            };
            MouseData mouse;

            std::shared_ptr<dtk::ValueObserver<double> > fpsObserver;
            std::shared_ptr<dtk::ValueObserver<size_t> > droppedFramesObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::ImageType> > colorBufferObserver;
            std::shared_ptr<dtk::ListObserver<dtk::Color4F> > colorPickersObserver;
        };

        void Viewport::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            TimelineViewport::_init(context, parent);
            DTK_P();

            p.hud = dtk::ObservableValue<bool>::create(false);

            p.fpsLabel = dtk::Label::create(context);
            p.fpsLabel->setFontRole(dtk::FontRole::Mono);
            p.fpsLabel->setMarginRole(dtk::SizeRole::MarginInside);
            p.fpsLabel->setBackgroundRole(dtk::ColorRole::Base);

            p.colorBufferLabel = dtk::Label::create(context);
            p.colorBufferLabel->setFontRole(dtk::FontRole::Mono);
            p.colorBufferLabel->setMarginRole(dtk::SizeRole::MarginInside);
            p.colorBufferLabel->setBackgroundRole(dtk::ColorRole::Base);

            p.hudLayout = dtk::GridLayout::create(context, shared_from_this());
            p.hudLayout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.hudLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.fpsLabel->setParent(p.hudLayout);
            p.hudLayout->setGridPos(p.fpsLabel, 0, 0);
            p.colorBufferLabel->setParent(p.hudLayout);
            p.hudLayout->setGridPos(p.colorBufferLabel, 0, 2);
            auto spacer = dtk::Spacer::create(context, dtk::Orientation::Horizontal, p.hudLayout);
            spacer->setStretch(dtk::Stretch::Expanding, dtk::Stretch::Expanding);
            p.hudLayout->setGridPos(spacer, 1, 1);
            p.hudLayout->hide();

            p.fpsObserver = dtk::ValueObserver<double>::create(
                observeFPS(),
                [this](double value)
                {
                    _p->fps = value;
                    _hudUpdate();
                });
            p.droppedFramesObserver = dtk::ValueObserver<size_t>::create(
                observeDroppedFrames(),
                [this](size_t value)
                {
                    _p->droppedFrames = value;
                    _hudUpdate();
                });

            p.colorBufferObserver = dtk::ValueObserver<dtk::ImageType>::create(
                observeColorBuffer(),
                [this](dtk::ImageType value)
                {
                    _p->colorBuffer = value;
                    _hudUpdate();
                });

            p.colorPickersObserver = dtk::ListObserver<dtk::Color4F>::create(
                observeColorPickers(),
                [this](const std::vector<dtk::Color4F>& value)
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
            const std::shared_ptr<dtk::Context>& context,
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

        std::shared_ptr<dtk::IObservableValue<bool> > Viewport::observeHUD() const
        {
            return _p->hud;
        }

        void Viewport::setHUD(bool value)
        {
            DTK_P();
            if (p.hud->setIfChanged(value))
            {
                p.hudLayout->setVisible(value);
            }
        }

        void Viewport::setGeometry(const dtk::Box2I& value)
        {
            TimelineViewport::setGeometry(value);
            DTK_P();
            p.hudLayout->setGeometry(value);
            for (const auto& colorPicker : p.colorPickers)
            {
                dtk::Size2I sizeHint = colorPicker.widget->getSizeHint();
                colorPicker.widget->setGeometry(dtk::Box2I(
                    colorPicker.pos.x,
                    colorPicker.pos.y,
                    sizeHint.w,
                    sizeHint.h));
            }
        }

        void Viewport::childRemoveEvent(const dtk::ChildRemoveEvent& event)
        {
            TimelineViewport::childRemoveEvent(event);
            DTK_P();
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

        void Viewport::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            TimelineViewport::sizeHintEvent(event);
            DTK_P();
            _setSizeHint(p.hudLayout->getSizeHint());
        }

        void Viewport::mouseMoveEvent(dtk::MouseMoveEvent& event)
        {
            TimelineViewport::mouseMoveEvent(event);
            DTK_P();
            switch (p.mouse.mode)
            {
            case Private::MouseMode::ColorPicker:
                if (!p.colorPickers.empty())
                {
                    p.colorPickers.back().pos = event.pos;
                    _colorPickersUpdate();
                    _setSizeUpdate();
                    _setDrawUpdate();
                }
                break;
            case Private::MouseMode::DragWidget:
                if (p.mouse.index < p.colorPickers.size())
                {
                    p.colorPickers[p.mouse.index].pos = event.pos - p.mouse.offset;
                    _colorPickersUpdate();
                    _setSizeUpdate();
                    _setDrawUpdate();
                }
                break;
            default: break;
            }
        }

        void Viewport::mousePressEvent(dtk::MouseClickEvent& event)
        {
            TimelineViewport::mousePressEvent(event);
            DTK_P();
            takeKeyFocus();
            if (0 == event.button)
            {
                for (size_t i = 0; i < p.colorPickers.size(); ++i)
                {
                    if (dtk::contains(p.colorPickers[i].widget->getGeometry(), event.pos))
                    {
                        p.mouse.mode = Private::MouseMode::DragWidget;
                        p.mouse.index = i;
                        p.mouse.offset = event.pos - p.colorPickers[i].widget->getGeometry().min;
                    }
                }
            }
            if (Private::MouseMode::None == p.mouse.mode &&
                0 == event.button &&
                event.modifiers & static_cast<int>(dtk::KeyModifier::Shift))
            {
                if (auto context = getContext())
                {
                    p.mouse.mode = Private::MouseMode::ColorPicker;
                    p.colorPickers.push_back({
                        dtk::Color4F(),
                        event.pos,
                        ViewportColorWidget::create(context, shared_from_this()) });
                    _colorPickersUpdate();
                    _colorWidgetsUpdate();
                    _setSizeUpdate();
                    _setDrawUpdate();
                }
            }
        }

        void Viewport::mouseReleaseEvent(dtk::MouseClickEvent& event)
        {
            TimelineViewport::mouseReleaseEvent(event);
            DTK_P();
            p.mouse = Private::MouseData();
        }

        void Viewport::_hudUpdate()
        {
            DTK_P();
            p.fpsLabel->setText(
                dtk::Format("FPS: {0} ({1} dropped)").
                arg(p.fps, 2, 4).
                arg(p.droppedFrames));
            p.colorBufferLabel->setText(
                dtk::Format("Color buffer: {0}").
                arg(p.colorBuffer));
        }

        void Viewport::_colorPickersUpdate()
        {
            DTK_P();
            std::vector<dtk::V2I> colorPickers;
            for (const auto& colorPicker : p.colorPickers)
            {
                colorPickers.push_back(colorPicker.pos);
            }
            setColorPickers(colorPickers);
        }

        void Viewport::_colorWidgetsUpdate()
        {
            DTK_P();
            for (const auto& colorPicker : p.colorPickers)
            {
                colorPicker.widget->setColor(colorPicker.color);
            }
        }
    }
}
