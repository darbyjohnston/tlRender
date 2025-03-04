// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/CompareActions.h>

#include <tlPlayApp/Models/FilesModel.h>
#include <tlPlayApp/App.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace play
    {
        struct CompareActions::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
        };

        void CompareActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["Next"] = std::make_shared<dtk::Action>(
                "Next",
                "Next",
                dtk::Key::PageDown,
                static_cast<int>(dtk::KeyModifier::Shift),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->nextB();
                    }
                });

            p.actions["Prev"] = std::make_shared<dtk::Action>(
                "Previous",
                "Prev",
                dtk::Key::PageUp,
                static_cast<int>(dtk::KeyModifier::Shift),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->prevB();
                    }
                });

            const std::array<std::string, static_cast<size_t>(timeline::Compare::Count)> compareIcons =
            {
                "CompareA",
                "CompareB",
                "CompareWipe",
                "CompareOverlay",
                "CompareDifference",
                "CompareHorizontal",
                "CompareVertical",
                "CompareTile"
            };
            const std::array<std::pair<dtk::Key, int>, static_cast<size_t>(timeline::Compare::Count)> compareShortcuts =
            {
                std::make_pair(dtk::Key::A, static_cast<int>(dtk::KeyModifier::Control)),
                std::make_pair(dtk::Key::B, static_cast<int>(dtk::KeyModifier::Control)),
                std::make_pair(dtk::Key::W, static_cast<int>(dtk::KeyModifier::Control)),
                std::make_pair(dtk::Key::Unknown, 0),
                std::make_pair(dtk::Key::Unknown, 0),
                std::make_pair(dtk::Key::Unknown, 0),
                std::make_pair(dtk::Key::Unknown, 0),
                std::make_pair(dtk::Key::T, static_cast<int>(dtk::KeyModifier::Control))
            };
            const std::array<std::string, static_cast<size_t>(timeline::Compare::Count)> compareToolTips =
            {
                dtk::Format(
                    "Show the A file\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(
                        compareShortcuts[static_cast<size_t>(timeline::Compare::A)].first,
                        compareShortcuts[static_cast<size_t>(timeline::Compare::A)].second)),
                dtk::Format(
                    "Show the B file\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(
                        compareShortcuts[static_cast<size_t>(timeline::Compare::B)].first,
                        compareShortcuts[static_cast<size_t>(timeline::Compare::B)].second)),
                dtk::Format(
                    "Wipe between the A and B files\n"
                    "\n"
                    "Use the Alt key + left mouse button to move the wipe\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(
                        compareShortcuts[static_cast<size_t>(timeline::Compare::Wipe)].first,
                        compareShortcuts[static_cast<size_t>(timeline::Compare::Wipe)].second)),
                "Show the A file over the B file with transparency",
                "Show the difference between the A and B files",
                "Show the A and B files side by side",
                "Show the A file above the B file",
                dtk::Format(
                    "Tile the A and B files\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(
                        compareShortcuts[static_cast<size_t>(timeline::Compare::Tile)].first,
                        compareShortcuts[static_cast<size_t>(timeline::Compare::Tile)].second))
            };
            const auto compareEnums = timeline::getCompareEnums();
            const auto comapreLabels = timeline::getCompareLabels();
            for (size_t i = 0; i < compareEnums.size(); ++i)
            {
                const auto compare = compareEnums[i];
                p.actions[comapreLabels[i]] = std::make_shared<dtk::Action>(
                    timeline::getLabel(compare),
                    compareIcons[i],
                    compareShortcuts[i].first,
                    compareShortcuts[i].second,
                    [appWeak, compare]
                    {
                        if (auto app = appWeak.lock())
                        {
                            auto options = app->getFilesModel()->getCompareOptions();
                            options.compare = compare;
                            app->getFilesModel()->setCompareOptions(options);
                        }
                    });
                p.actions[comapreLabels[i]]->toolTip = compareToolTips[i];
            }

            const auto compareTimeEnums = timeline::getCompareTimeEnums();
            const auto comapreTimeLabels = timeline::getCompareTimeLabels();
            for (size_t i = 0; i < compareTimeEnums.size(); ++i)
            {
                const auto compareTime = compareTimeEnums[i];
                p.actions[comapreTimeLabels[i]] = std::make_shared<dtk::Action>(
                    comapreTimeLabels[i],
                    [appWeak, compareTime]
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->getFilesModel()->setCompareTime(compareTime);
                        }
                    });
            }
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

        const std::map<std::string, std::shared_ptr<dtk::Action> >& CompareActions::getActions() const
        {
            return _p->actions;
        }
    }
}
