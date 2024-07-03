// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlay/Viewport.h>

#include <tlUI/GridLayout.h>
#include <tlUI/Label.h>

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

            std::shared_ptr<ui::Label> fpsLabel;
            std::shared_ptr<ui::Label> colorPickerLabel;
            std::shared_ptr<ui::GridLayout> hudLayout;

            std::shared_ptr<observer::ValueObserver<double> > fpsObserver;
            std::shared_ptr<observer::ValueObserver<size_t> > droppedFramesObserver;
            std::shared_ptr<observer::ValueObserver<image::Color4f> > colorPickerObserver;
        };

        void Viewport::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            TimelineViewport::_init(context, parent);
            TLRENDER_P();

            p.hud = observer::Value<bool>::create(false);

            p.fpsLabel = ui::Label::create(context);
            p.fpsLabel->setMarginRole(ui::SizeRole::MarginInside);
            p.fpsLabel->setBackgroundRole(ui::ColorRole::Base);

            p.colorPickerLabel = ui::Label::create(context);

            p.hudLayout = ui::GridLayout::create(context, shared_from_this());
            p.hudLayout->setMarginRole(ui::SizeRole::MarginSmall);
            p.fpsLabel->setParent(p.hudLayout);
            p.hudLayout->setGridPos(p.fpsLabel, 0, 0);
            p.hudLayout->hide();

            p.fpsObserver = observer::ValueObserver<double>::create(
                observeFPS(),
                [this](double value)
                {
                    _p->fps = value;
                    _textUpdate();
                });
            p.droppedFramesObserver = observer::ValueObserver<size_t>::create(
                observeDroppedFrames(),
                [this](size_t value)
                {
                    _p->droppedFrames = value;
                    _textUpdate();
                });
            p.colorPickerObserver = observer::ValueObserver<image::Color4f>::create(
                observeColorPicker(),
                [this](const image::Color4f& value)
                {
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
        }

        void Viewport::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            TimelineViewport::sizeHintEvent(event);
            TLRENDER_P();
            _sizeHint = p.hudLayout->getSizeHint();
        }

        void Viewport::_textUpdate()
        {
            TLRENDER_P();
            p.fpsLabel->setText(
                string::Format("FPS: {0} ({1} dropped)").
                arg(p.fps, 2, 4).
                arg(p.droppedFrames));
        }
    }
}
