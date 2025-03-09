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

            std::shared_ptr<dtk::ValueObserver<KeyShortcutsSettings> > keyShortcutsSettingsObserver;
        };

        void CompareActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["Next"] = dtk::Action::create(
                "Next",
                "Next",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->nextB();
                    }
                });

            p.actions["Prev"] = dtk::Action::create(
                "Previous",
                "Prev",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->prevB();
                    }
                });

            p.actions["A"] = dtk::Action::create(
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

            p.actions["B"] = dtk::Action::create(
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

            p.actions["Wipe"] = dtk::Action::create(
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

            p.actions["Overlay"] = dtk::Action::create(
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

            p.actions["Difference"] = dtk::Action::create(
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

            p.actions["Horizontal"] = dtk::Action::create(
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

            p.actions["Vertical"] = dtk::Action::create(
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

            p.actions["Tile"] = dtk::Action::create(
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

            p.actions["Relative"] = dtk::Action::create(
                "Relative",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->setCompareTime(timeline::CompareTime::Relative);
                    }
                });

            p.actions["Absolute"] = dtk::Action::create(
                "Absolute",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->setCompareTime(timeline::CompareTime::Absolute);
                    }
                });

            p.keyShortcutsSettingsObserver = dtk::ValueObserver<KeyShortcutsSettings>::create(
                app->getSettingsModel()->observeKeyShortcuts(),
                [this](const KeyShortcutsSettings& value)
                {
                    _keyShortcutsUpdate(value);
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

        const std::map<std::string, std::shared_ptr<dtk::Action> >& CompareActions::getActions() const
        {
            return _p->actions;
        }

        void CompareActions::_keyShortcutsUpdate(const KeyShortcutsSettings& value)
        {
            DTK_P();
            const std::map<std::string, std::string> tooltips =
            {
                {
                    "Next",
                    "Go to the next B file.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Prev",
                    "Go to the previous B file.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "A",
                    "Show the A file.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "B",
                    "Show the B file.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Wipe",
                    "Wipe between the A and B files.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Overlay",
                    "Overlay the A and B files.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Difference",
                    "Show the difference between the A and B files.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Horizontal",
                    "Show the A and B files side by side.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Vertical",
                    "Show the A and B files over and under.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Tile",
                    "Show the A and B files tiled.\n"
                    "\n"
                    "Shortcut: {0}"
                },
            };
            for (const auto& i : p.actions)
            {
                auto j = value.shortcuts.find(dtk::Format("Compare/{0}").arg(i.first));
                if (j != value.shortcuts.end())
                {
                    i.second->setShortcut(j->second.key);
                    i.second->setShortcutModifiers(j->second.modifiers);
                    const auto k = tooltips.find(i.first);
                    if (k != tooltips.end())
                    {
                        i.second->setTooltip(dtk::Format(k->second).
                            arg(dtk::getShortcutLabel(j->second.key, j->second.modifiers)));
                    }
                }
            }
        }
    }
}
