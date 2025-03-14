// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/CompareActions.h>

#include <tlPlayApp/Models/FilesModel.h>
#include <tlPlayApp/App.h>

namespace tl
{
    namespace play
    {
        struct CompareActions::Private
        {
            std::shared_ptr<dtk::ListObserver<std::shared_ptr<FilesModelItem> > > filesObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::CompareOptions> > optionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::CompareTime> > timeObserver;
        };

        void CompareActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            IActions::_init(context, app, "Compare");
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            _actions["Next"] = dtk::Action::create(
                "Next",
                "Next",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->nextB();
                    }
                });

            _actions["Prev"] = dtk::Action::create(
                "Previous",
                "Prev",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->prevB();
                    }
                });

            _actions["A"] = dtk::Action::create(
                "A",
                "CompareA",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getFilesModel()->getCompareOptions();
                        options.compare = timeline::Compare::A;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                });

            _actions["B"] = dtk::Action::create(
                "B",
                "CompareB",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getFilesModel()->getCompareOptions();
                        options.compare = timeline::Compare::B;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                });

            _actions["Wipe"] = dtk::Action::create(
                "Wipe",
                "CompareWipe",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getFilesModel()->getCompareOptions();
                        options.compare = timeline::Compare::Wipe;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                });

            _actions["Overlay"] = dtk::Action::create(
                "Overlay",
                "CompareOverlay",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getFilesModel()->getCompareOptions();
                        options.compare = timeline::Compare::Overlay;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                });

            _actions["Difference"] = dtk::Action::create(
                "Difference",
                "CompareDifference",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getFilesModel()->getCompareOptions();
                        options.compare = timeline::Compare::Difference;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                });

            _actions["Horizontal"] = dtk::Action::create(
                "Horizontal",
                "CompareHorizontal",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getFilesModel()->getCompareOptions();
                        options.compare = timeline::Compare::Horizontal;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                });

            _actions["Vertical"] = dtk::Action::create(
                "Vertical",
                "CompareVertical",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getFilesModel()->getCompareOptions();
                        options.compare = timeline::Compare::Vertical;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                });

            _actions["Tile"] = dtk::Action::create(
                "Tile",
                "CompareTile",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getFilesModel()->getCompareOptions();
                        options.compare = timeline::Compare::Tile;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                });

            _actions["Relative"] = dtk::Action::create(
                "Relative",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->setCompareTime(timeline::CompareTime::Relative);
                    }
                });

            _actions["Absolute"] = dtk::Action::create(
                "Absolute",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->setCompareTime(timeline::CompareTime::Absolute);
                    }
                });

            _tooltips =
            {
                { "Next", "Go to the next B file." },
                { "Prev", "Go to the previous B file." },
                { "A", "Show the A file." },
                { "B", "Show the B file." },
                { "Wipe", "Wipe between the A and B files." },
                { "Overlay", "Overlay the A and B files." },
                { "Difference", "Show the difference between the A and B files." },
                { "Horizontal", "Show the A and B files side by side." },
                { "Vertical", "Show the A and B files over and under." },
                { "Tile", "Show the A and B files tiled." },
            };

            _shortcutsUpdate(app->getSettingsModel()->getShortcuts());

            p.filesObserver = dtk::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                app->getFilesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
                {
                    DTK_P();
                    _actions["Next"]->setEnabled(value.size() > 1);
                    _actions["Prev"]->setEnabled(value.size() > 1);
                });

            p.optionsObserver = dtk::ValueObserver<timeline::CompareOptions>::create(
                app->getFilesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    DTK_P();
                    const auto enums = timeline::getCompareEnums();
                    const auto labels = timeline::getCompareLabels();
                    for (size_t i = 0; i < enums.size(); ++i)
                    {
                        _actions[labels[i]]->setChecked(enums[i] == value.compare);
                    }
                });

            p.timeObserver = dtk::ValueObserver<timeline::CompareTime>::create(
                app->getFilesModel()->observeCompareTime(),
                [this](timeline::CompareTime value)
                {
                    const auto enums = timeline::getCompareTimeEnums();
                    const auto labels = timeline::getCompareTimeLabels();
                    for (size_t i = 0; i < enums.size(); ++i)
                    {
                        _actions[labels[i]]->setChecked(enums[i] == value);
                    }
                });
        }

        CompareActions::CompareActions() :
            _p(new Private)
        {}

        CompareActions::~CompareActions()
        {}

        std::shared_ptr<CompareActions> CompareActions::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            auto out = std::shared_ptr<CompareActions>(new CompareActions);
            out->_init(context, app);
            return out;
        }
    }
}
