// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/Viewport.h>

#include <ftk/UI/Action.h>

namespace tl
{
    namespace play
    {
        class App;

        //! View actions.
        class ViewActions : public std::enable_shared_from_this<ViewActions>
        {
            FTK_NON_COPYABLE(ViewActions);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<timelineui::Viewport>&);

            ViewActions() = default;

        public:
            ~ViewActions();

            static std::shared_ptr<ViewActions> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<timelineui::Viewport>&);

            const std::map<std::string, std::shared_ptr<ftk::Action> >& getActions() const;

        private:
            std::map<std::string, std::shared_ptr<ftk::Action> > _actions;
            std::shared_ptr<ftk::ValueObserver<bool> > _frameObserver;
        };
    }
}