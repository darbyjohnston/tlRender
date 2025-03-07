// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp//Widgets/Viewport.h>

#include <tlPlayApp/Models/ColorModel.h>
#include <tlPlayApp/Models/FilesModel.h>
#include <tlPlayApp/Models/ViewportModel.h>
#include <tlPlayApp/App.h>

#include <tlTimeline/Util.h>

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
            std::weak_ptr<App> app;
            std::shared_ptr<dtk::ObservableValue<bool> > hud;
            double fps = 0.0;
            size_t droppedFrames = 0;

            std::shared_ptr<dtk::Label> fpsLabel;
            std::shared_ptr<dtk::Label> colorBufferLabel;
            std::shared_ptr<dtk::GridLayout> hudLayout;

            enum class MouseMode
            {
                None,
                Shuttle,
                ColorPicker
            };
            struct MouseData
            {
                MouseMode mode = MouseMode::None;
                OTIO_NS::RationalTime shuttleStart = time::invalidTime;
            };
            MouseData mouse;

            std::shared_ptr<dtk::ValueObserver<double> > fpsObserver;
            std::shared_ptr<dtk::ValueObserver<size_t> > droppedFramesObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::OCIOOptions> > ocioOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::LUTOptions> > lutOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::ImageOptions> > imageOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::DisplayOptions> > displayOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::BackgroundOptions> > bgOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::ForegroundOptions> > fgOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::ImageType> > colorBufferObserver;
        };

        void Viewport::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            timelineui::Viewport::_init(context, parent);
            DTK_P();

            _setMouseHoverEnabled(true);
            _setMousePressEnabled(true);

            p.app = app;
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

            p.compareOptionsObserver = dtk::ValueObserver<timeline::CompareOptions>::create(
                app->getFilesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    setCompareOptions(value);
                });

            p.ocioOptionsObserver = dtk::ValueObserver<timeline::OCIOOptions>::create(
                app->getColorModel()->observeOCIOOptions(),
                [this](const timeline::OCIOOptions& value)
                {
                   setOCIOOptions(value);
                });

            p.lutOptionsObserver = dtk::ValueObserver<timeline::LUTOptions>::create(
                app->getColorModel()->observeLUTOptions(),
                [this](const timeline::LUTOptions& value)
                {
                   setLUTOptions(value);
                });

            p.imageOptionsObserver = dtk::ValueObserver<dtk::ImageOptions>::create(
                app->getViewportModel()->observeImageOptions(),
                [this](const dtk::ImageOptions& value)
                {
                   setImageOptions({ value });
                });

            p.displayOptionsObserver = dtk::ValueObserver<timeline::DisplayOptions>::create(
                app->getViewportModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                   setDisplayOptions({ value });
                });

            p.bgOptionsObserver = dtk::ValueObserver<timeline::BackgroundOptions>::create(
                app->getViewportModel()->observeBackgroundOptions(),
                [this](const timeline::BackgroundOptions& value)
                {
                    setBackgroundOptions(value);
                });

            p.fgOptionsObserver = dtk::ValueObserver<timeline::ForegroundOptions>::create(
                app->getViewportModel()->observeForegroundOptions(),
                [this](const timeline::ForegroundOptions& value)
                {
                    setForegroundOptions(value);
                });

            p.colorBufferObserver = dtk::ValueObserver<dtk::ImageType>::create(
                app->getViewportModel()->observeColorBuffer(),
                [this](dtk::ImageType value)
                {
                    setColorBuffer(value);
                    _hudUpdate();
                });
        }

        Viewport::Viewport() :
            _p(new Private)
        {}

        Viewport::~Viewport()
        {}

        std::shared_ptr<Viewport> Viewport::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Viewport>(new Viewport);
            out->_init(context, app, parent);
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
            timelineui::Viewport::setGeometry(value);
            DTK_P();
            p.hudLayout->setGeometry(value);
        }

        void Viewport::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            timelineui::Viewport::sizeHintEvent(event);
            DTK_P();
            _setSizeHint(p.hudLayout->getSizeHint());
        }

        void Viewport::mouseMoveEvent(dtk::MouseMoveEvent& event)
        {
            timelineui::Viewport::mouseMoveEvent(event);
            DTK_P();
            switch (p.mouse.mode)
            {
            case Private::MouseMode::Shuttle:
                if (auto player = getPlayer())
                {
                    const OTIO_NS::RationalTime offset = OTIO_NS::RationalTime(
                        (event.pos.x - _getMousePressPos().x) * .05F,
                        p.mouse.shuttleStart.rate()).round();
                    const OTIO_NS::TimeRange& timeRange = player->getTimeRange();
                    OTIO_NS::RationalTime t = p.mouse.shuttleStart + offset;
                    if (t < timeRange.start_time())
                    {
                        t = timeRange.end_time_exclusive() - (timeRange.start_time() - t);
                    }
                    else if (t > timeRange.end_time_exclusive())
                    {
                        t = timeRange.start_time() + (t - timeRange.end_time_exclusive());
                    }
                    player->seek(t);
                }
                break;
            case Private::MouseMode::ColorPicker:
                if (auto app = p.app.lock())
                {
                    const dtk::Color4F color = getColorSample(event.pos);
                    app->getViewportModel()->setColorPicker(color);
                }
                break;
            default: break;
            }
        }

        void Viewport::mousePressEvent(dtk::MouseClickEvent& event)
        {
            timelineui::Viewport::mousePressEvent(event);
            DTK_P();
            takeKeyFocus();
            if (0 == event.button &&
                event.modifiers & static_cast<int>(dtk::KeyModifier::Shift))
            {
                p.mouse.mode = Private::MouseMode::Shuttle;
                if (auto player = getPlayer())
                {
                    player->stop();
                    p.mouse.shuttleStart = player->getCurrentTime();
                }
            }
            else if (0 == event.button)
            {
                p.mouse.mode = Private::MouseMode::ColorPicker;
                if (auto app = p.app.lock())
                {
                    const dtk::Color4F color = getColorSample(event.pos);
                    app->getViewportModel()->setColorPicker(color);
                }
            }
        }

        void Viewport::mouseReleaseEvent(dtk::MouseClickEvent& event)
        {
            timelineui::Viewport::mouseReleaseEvent(event);
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
                arg(getColorBuffer()));
        }
    }
}
