// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Models/ViewportModel.h>

#include <dtk/ui/Settings.h>

namespace tl
{
    namespace play
    {
        struct ViewportModel::Private
        {
            std::weak_ptr<dtk::Context> context;
            std::shared_ptr<dtk::Settings> settings;
            std::shared_ptr<dtk::ObservableValue<dtk::Color4F> > colorPicker;
            std::shared_ptr<dtk::ObservableValue<dtk::ImageOptions> > imageOptions;
            std::shared_ptr<dtk::ObservableValue<timeline::DisplayOptions> > displayOptions;
            std::shared_ptr<dtk::ObservableValue<timeline::BackgroundOptions> > backgroundOptions;
            std::shared_ptr<dtk::ObservableValue<timeline::ForegroundOptions> > foregroundOptions;
            std::shared_ptr<dtk::ObservableValue<dtk::ImageType> > colorBuffer;
        };

        void ViewportModel::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<dtk::Settings>& settings)
        {
            DTK_P();

            p.context = context;
            p.settings = settings;

            p.colorPicker = dtk::ObservableValue<dtk::Color4F>::create();

            dtk::ImageOptions imageOptions;
            p.settings->getT("Viewport/Image", imageOptions);
            p.imageOptions = dtk::ObservableValue<dtk::ImageOptions>::create(imageOptions);

            timeline::DisplayOptions displayOptions;
            p.settings->getT("Viewport/Display", displayOptions);
            p.displayOptions = dtk::ObservableValue<timeline::DisplayOptions>::create(displayOptions);

            timeline::BackgroundOptions backgroundOptions;
            p.settings->getT("Viewport/Background", backgroundOptions);
            p.backgroundOptions = dtk::ObservableValue<timeline::BackgroundOptions>::create(
                backgroundOptions);

            timeline::ForegroundOptions foregroundOptions;
            p.settings->getT("Viewport/Foreground", foregroundOptions);
            p.foregroundOptions = dtk::ObservableValue<timeline::ForegroundOptions>::create(
                foregroundOptions);

            dtk::ImageType colorBuffer = dtk::ImageType::RGBA_U8;
            std::string s = dtk::to_string(colorBuffer);
            p.settings->get("Viewport/ColorBuffer", s);
            dtk::from_string(s, colorBuffer);
            p.colorBuffer = dtk::ObservableValue<dtk::ImageType>::create(colorBuffer);
        }

        ViewportModel::ViewportModel() :
            _p(new Private)
        {}

        ViewportModel::~ViewportModel()
        {
            DTK_P();
            p.settings->setT("Viewport/Image", p.imageOptions->get());
            p.settings->setT("Viewport/Display", p.displayOptions->get());
            p.settings->setT("Viewport/Background", p.backgroundOptions->get());
            p.settings->setT("Viewport/Foreground", p.foregroundOptions->get());
            p.settings->set("Viewport/ColorBuffer", dtk::to_string(p.colorBuffer->get()));
        }

        std::shared_ptr<ViewportModel> ViewportModel::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<dtk::Settings>& settings)
        {
            auto out = std::shared_ptr<ViewportModel>(new ViewportModel);
            out->_init(context, settings);
            return out;
        }

        const dtk::Color4F& ViewportModel::getColorPicker() const
        {
            return _p->colorPicker->get();
        }

        std::shared_ptr<dtk::IObservableValue<dtk::Color4F> > ViewportModel::observeColorPicker() const
        {
            return _p->colorPicker;
        }

        void ViewportModel::setColorPicker(const dtk::Color4F& value)
        {
            _p->colorPicker->setIfChanged(value);
        }

        const dtk::ImageOptions& ViewportModel::getImageOptions() const
        {
            return _p->imageOptions->get();
        }

        std::shared_ptr<dtk::IObservableValue<dtk::ImageOptions> > ViewportModel::observeImageOptions() const
        {
            return _p->imageOptions;
        }

        void ViewportModel::setImageOptions(const dtk::ImageOptions& value)
        {
            _p->imageOptions->setIfChanged(value);
        }

        const timeline::DisplayOptions& ViewportModel::getDisplayOptions() const
        {
            return _p->displayOptions->get();
        }

        std::shared_ptr<dtk::IObservableValue<timeline::DisplayOptions> > ViewportModel::observeDisplayOptions() const
        {
            return _p->displayOptions;
        }

        void ViewportModel::setDisplayOptions(const timeline::DisplayOptions& value)
        {
            _p->displayOptions->setIfChanged(value);
        }

        const timeline::BackgroundOptions& ViewportModel::getBackgroundOptions() const
        {
            return _p->backgroundOptions->get();
        }

        std::shared_ptr<dtk::IObservableValue<timeline::BackgroundOptions> > ViewportModel::observeBackgroundOptions() const
        {
            return _p->backgroundOptions;
        }

        void ViewportModel::setBackgroundOptions(const timeline::BackgroundOptions& value)
        {
            _p->settings->setT("Viewport/Background", value);
            _p->backgroundOptions->setIfChanged(value);
        }

        const timeline::ForegroundOptions& ViewportModel::getForegroundOptions() const
        {
            return _p->foregroundOptions->get();
        }

        std::shared_ptr<dtk::IObservableValue<timeline::ForegroundOptions> > ViewportModel::observeForegroundOptions() const
        {
            return _p->foregroundOptions;
        }

        void ViewportModel::setForegroundOptions(const timeline::ForegroundOptions& value)
        {
            _p->settings->setT("Viewport/Foreground", value);
            _p->foregroundOptions->setIfChanged(value);
        }

        dtk::ImageType ViewportModel::getColorBuffer() const
        {
            return _p->colorBuffer->get();
        }

        std::shared_ptr<dtk::IObservableValue<dtk::ImageType> > ViewportModel::observeColorBuffer() const
        {
            return _p->colorBuffer;
        }

        void ViewportModel::setColorBuffer(dtk::ImageType value)
        {
            _p->colorBuffer->setIfChanged(value);
        }
    }
}
