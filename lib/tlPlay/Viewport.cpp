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
            std::shared_ptr<dtk::ObservableValue<dtk::Color4F> > colorPicker;
            std::shared_ptr<dtk::ObservableValue<bool> > hud;
            double fps = 0.0;
            size_t droppedFrames = 0;
            dtk::ImageType colorBuffer = dtk::ImageType::None;

            std::shared_ptr<dtk::Label> fpsLabel;
            std::shared_ptr<dtk::Label> colorBufferLabel;
            std::shared_ptr<dtk::GridLayout> hudLayout;

            enum class MouseMode
            {
                None,
                ColorPicker
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

            _setMouseHoverEnabled(true);
            _setMousePressEnabled(true);

            p.colorPicker = dtk::ObservableValue<dtk::Color4F>::create();
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
                    if (!value.empty())
                    {
                        _p->colorPicker->setIfChanged(value.front());
                    }
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

        const dtk::Color4F& Viewport::getColorPicker() const
        {
            return _p->colorPicker->get();
        }

        std::shared_ptr<dtk::IObservableValue<dtk::Color4F> > Viewport::observeColorPicker() const
        {
            return _p->colorPicker;
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
                setColorPickers({ event.pos });
                break;
            default: break;
            }
        }

        void Viewport::mousePressEvent(dtk::MouseClickEvent& event)
        {
            TimelineViewport::mousePressEvent(event);
            DTK_P();
            takeKeyFocus();
            if (Private::MouseMode::None == p.mouse.mode &&
                0 == event.button &&
                event.modifiers & static_cast<int>(dtk::KeyModifier::Shift))
            {
                p.mouse.mode = Private::MouseMode::ColorPicker;
                setColorPickers({ event.pos });
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
    }
}
