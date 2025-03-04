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

            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
            std::vector<std::shared_ptr<dtk::Action> > bActions;
            std::map<std::string, std::shared_ptr<dtk::Menu> > menus;

            std::shared_ptr<dtk::ListObserver<std::shared_ptr<FilesModelItem> > > filesObserver;
            std::shared_ptr<dtk::ListObserver<int> > bIndexesObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::CompareTime> > compareTimeObserver;
        };

        void CompareMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<CompareActions>& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            DTK_P();

            p.app = app;

            p.actions = actions->getActions();

            p.menus["B"] = addSubMenu("B");
            addItem(p.actions["Next"]);
            addItem(p.actions["Prev"]);
            addDivider();
            const auto compareLabels = timeline::getCompareLabels();
            for (const auto& label : compareLabels)
            {
                addItem(p.actions[label]);
            }
            addDivider();
            p.menus["Time"] = addSubMenu("Time");
            const auto timeLabels = timeline::getCompareTimeLabels();
            for (const auto& label : timeLabels)
            {
                p.menus["Time"]->addItem(p.actions[label]);
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

            p.compareOptionsObserver = dtk::ValueObserver<timeline::CompareOptions>::create(
                app->getFilesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _compareUpdate(value);
                });

            p.compareTimeObserver = dtk::ValueObserver<timeline::CompareTime>::create(
                app->getFilesModel()->observeCompareTime(),
                [this](timeline::CompareTime value)
                {
                    _compareTimeUpdate(value);
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
            const std::shared_ptr<CompareActions>& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<CompareMenu>(new CompareMenu);
            out->_init(context, app, actions, parent);
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

            setItemEnabled(p.actions["Next"], value.size() > 1);
            setItemEnabled(p.actions["Prev"], value.size() > 1);

            p.menus["B"]->clear();
            p.bActions.clear();
            if (auto app = p.app.lock())
            {
                const auto bIndexes = app->getFilesModel()->getBIndexes();
                for (size_t i = 0; i < value.size(); ++i)
                {
                    auto action = std::make_shared<dtk::Action>(
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
                    action->checked = j != bIndexes.end();
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

        void CompareMenu::_compareUpdate(const timeline::CompareOptions& value)
        {
            DTK_P();
            const auto enums = timeline::getCompareEnums();
            const auto labels = timeline::getCompareLabels();
            for (size_t i = 0; i < enums.size(); ++i)
            {
                setItemChecked(p.actions[labels[i]], enums[i] == value.compare);
            }
        }

        void CompareMenu::_compareTimeUpdate(timeline::CompareTime value)
        {
            DTK_P();
            const auto enums = timeline::getCompareTimeEnums();
            const auto labels = timeline::getCompareTimeLabels();
            for (size_t i = 0; i < enums.size(); ++i)
            {
                p.menus["Time"]->setItemChecked(p.actions[labels[i]], enums[i] == value);
            }
        }
    }
}
