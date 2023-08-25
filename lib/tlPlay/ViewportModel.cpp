// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlay/ViewportModel.h>

namespace tl
{
    namespace play
    {
        struct ViewportModel::Private
        {
            std::weak_ptr<system::Context> context;
            std::shared_ptr<observer::Value<timelineui::ViewportBackgroundOptions> > backgroundOptions;
        };

        void ViewportModel::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.context = context;

            p.backgroundOptions = observer::Value<timelineui::ViewportBackgroundOptions>::create();
        }

        ViewportModel::ViewportModel() :
            _p(new Private)
        {}

        ViewportModel::~ViewportModel()
        {}

        std::shared_ptr<ViewportModel> ViewportModel::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<ViewportModel>(new ViewportModel);
            out->_init(context);
            return out;
        }

        const timelineui::ViewportBackgroundOptions& ViewportModel::getBackgroundOptions() const
        {
            return _p->backgroundOptions->get();
        }

        std::shared_ptr<observer::IValue<timelineui::ViewportBackgroundOptions> > ViewportModel::observeBackgroundOptions() const
        {
            return _p->backgroundOptions;
        }

        void ViewportModel::setBackgroundOptions(const timelineui::ViewportBackgroundOptions& value)
        {
            _p->backgroundOptions->setIfChanged(value);
        }
    }
}
