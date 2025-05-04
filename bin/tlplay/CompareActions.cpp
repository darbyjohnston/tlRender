// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "CompareActions.h"

#include "App.h"
#include "FilesModel.h"

namespace tl
{
    namespace play
    {
        void CompareActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            auto appWeak = std::weak_ptr<App>(app);
            auto labels = timeline::getCompareLabels();
            const std::array<std::string, static_cast<size_t>(timeline::Compare::Count)> icons =
            {
                "CompareA",
                "CompareB",
                "CompareWipe",
                "CompareOverlay",
                "CompareDifference",
                "CompareHorizontal",
                "CompareVertical",
                "CompareTile",
            };
            const std::array<dtk::Key, static_cast<size_t>(timeline::Compare::Count)> shortcuts =
            {
                dtk::Key::A,
                dtk::Key::B,
                dtk::Key::W,
                dtk::Key::Unknown,
                dtk::Key::Unknown,
                dtk::Key::Unknown,
                dtk::Key::Unknown
            };
            const std::array<std::string, static_cast<size_t>(timeline::Compare::Count)> tooltips =
            {
                "Show the A file.",
                "Show the B file.",
                "Wipe between the A and B file.",
                "Overlay the A and B file.",
                "Show the difference between the A and B file.",
                "Show the A and B file side by side.",
                "Show the A and B file over and under.",
            };
            for (size_t i = 0; i < labels.size(); ++i)
            {
                _actions[labels[i]] = dtk::Action::create(
                    labels[i],
                    icons[i],
                    shortcuts[i],
                    static_cast<int>(dtk::commandKeyModifier),
                    [appWeak, i]
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->getFilesModel()->setCompare(static_cast<timeline::Compare>(i));
                        }
                    });
                _actions[labels[i]]->setTooltip(tooltips[i]);
            }

            _playersObserver = dtk::ListObserver<std::shared_ptr<timeline::Player> >::create(
                app->getFilesModel()->observePlayers(),
                [this](const std::vector<std::shared_ptr<timeline::Player> >& value)
                {
                    for (const auto& label : timeline::getCompareLabels())
                    {
                        _actions[label]->setEnabled(value.size() > 0);
                    }
                });

            _compareObserver = dtk::ValueObserver<timeline::Compare>::create(
                app->getFilesModel()->observeCompare(),
                [this](timeline::Compare value)
                {
                    for (auto compare : timeline::getCompareEnums())
                    {
                        _actions[timeline::getLabel(compare)]->setChecked(value == compare);
                    }
                });
        }

        CompareActions::~CompareActions()
        {
        }

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
            return _actions;
        }
    }
}