// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Menus/CompareMenu.h>

#include <tlPlayApp/Actions/CompareActions.h>
#include <tlPlayApp/App.h>

namespace tl
{
    namespace play
    {
        struct CompareMenu::Private
        {
            std::weak_ptr<App> app;

            std::vector<std::shared_ptr<dtk::Action> > bActions;
            std::map<std::string, std::shared_ptr<dtk::Menu> > menus;

            std::shared_ptr<dtk::ListObserver<std::shared_ptr<FilesModelItem> > > filesObserver;
            std::shared_ptr<dtk::ListObserver<int> > bIndexesObserver;
        };

        void CompareMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<CompareActions>& compareActions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            DTK_P();

            p.app = app;

            p.menus["B"] = addSubMenu("B");
            auto actions = compareActions->getActions();
            addItem(actions["Next"]);
            addItem(actions["Prev"]);
            addDivider();
            const auto compareLabels = timeline::getCompareLabels();
            for (const auto& label : compareLabels)
            {
                addItem(actions[label]);
            }
            addDivider();
            p.menus["Time"] = addSubMenu("Time");
            const auto timeLabels = timeline::getCompareTimeLabels();
            for (const auto& label : timeLabels)
            {
                p.menus["Time"]->addItem(actions[label]);
            }

            p.filesObserver = dtk::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                app->getFilesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
                {
                    _filesUpdate(value);
                });

            p.bIndexesObserver = dtk::ListObserver<int>::create(
                app->getFilesModel()->observeBIndexes(),
                [this](const std::vector<int>& value)
                {
                    _bUpdate(value);
                });
        }

        CompareMenu::CompareMenu() :
            _p(new Private)
        {}

        CompareMenu::~CompareMenu()
        {}

        std::shared_ptr<CompareMenu> CompareMenu::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<CompareActions>& compareActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<CompareMenu>(new CompareMenu);
            out->_init(context, app, compareActions, parent);
            return out;
        }

        void CompareMenu::close()
        {
            Menu::close();
            DTK_P();
            for (const auto& menu : p.menus)
            {
                menu.second->close();
            }
        }

        void CompareMenu::_filesUpdate(
            const std::vector<std::shared_ptr<FilesModelItem> >& value)
        {
            DTK_P();
            p.menus["B"]->clear();
            p.bActions.clear();
            if (auto app = p.app.lock())
            {
                const auto bIndexes = app->getFilesModel()->getBIndexes();
                for (size_t i = 0; i < value.size(); ++i)
                {
                    auto action = dtk::Action::create(
                        value[i]->path.get(-1, file::PathType::FileName),
                        [this, i]
                        {
                            close();
                            if (auto app = _p->app.lock())
                            {
                                app->getFilesModel()->toggleB(i);
                            }
                        });
                    const auto j = std::find(bIndexes.begin(), bIndexes.end(), i);
                    action->setChecked(j != bIndexes.end());
                    p.menus["B"]->addItem(action);
                    p.bActions.push_back(action);
                }
            }
        }

        void CompareMenu::_bUpdate(const std::vector<int>& value)
        {
            DTK_P();
            for (int i = 0; i < p.bActions.size(); ++i)
            {
                const auto j = std::find(value.begin(), value.end(), i);
                p.menus["B"]->setItemChecked(
                    p.bActions[i],
                    j != value.end());
            }
        }
    }
}
