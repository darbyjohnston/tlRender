// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlay/ViewportModel.h>

#include <tlPlay/Settings.h>

namespace tl
{
    namespace play
    {
        struct ViewportModel::Private
        {
            std::weak_ptr<system::Context> context;
            std::shared_ptr<Settings> settings;
            std::shared_ptr<dtk::ObservableValue<timeline::BackgroundOptions> > backgroundOptions;
            std::shared_ptr<dtk::ObservableValue<timeline::DisplayOptions> > displayOptions;
        };

        void ViewportModel::_init(
            const std::shared_ptr<Settings>& settings,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.context = context;
            p.settings = settings;

            p.settings->setDefaultValue("Viewport/Background", timeline::BackgroundOptions());
            p.backgroundOptions = dtk::ObservableValue<timeline::BackgroundOptions>::create(
                p.settings->getValue< timeline::BackgroundOptions>("Viewport/Background"));
            p.displayOptions = dtk::ObservableValue<timeline::DisplayOptions>::create();
        }

        ViewportModel::ViewportModel() :
            _p(new Private)
        {}

        ViewportModel::~ViewportModel()
        {}

        std::shared_ptr<ViewportModel> ViewportModel::create(
            const std::shared_ptr<Settings>& settings,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<ViewportModel>(new ViewportModel);
            out->_init(settings, context);
            return out;
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
            _p->settings->setValue("Viewport/Background", value);
            _p->backgroundOptions->setIfChanged(value);
        }
    }
}
