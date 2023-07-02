// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/CompareMenu.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct CompareMenu::Private
        {
            std::weak_ptr<App> app;

            timeline::CompareOptions compareOptions;

            std::map<timeline::CompareMode, std::shared_ptr<ui::MenuItem> > compareItems;
            std::shared_ptr<Menu> currentMenu;
            std::vector<std::shared_ptr<ui::MenuItem> > currentItems;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ListObserver<int> > filesBObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
        };

        void CompareMenu::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            Menu::_init(context);
            TLRENDER_P();

            p.app = app;

            const std::array<std::string, static_cast<size_t>(timeline::CompareMode::Count)> icons =
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
            const std::array<ui::Key, static_cast<size_t>(timeline::CompareMode::Count)> shortcuts =
            {
                ui::Key::A,
                ui::Key::B,
                ui::Key::W,
                ui::Key::Unknown,
                ui::Key::Unknown,
                ui::Key::Unknown,
                ui::Key::Unknown,
                ui::Key::T
            };
            const auto enums = timeline::getCompareModeEnums();
            for (size_t i = 0; i < enums.size(); ++i)
            {
                const auto mode = enums[i];
                p.compareItems[mode] = std::make_shared<ui::MenuItem>(
                    timeline::getLabel(mode),
                    icons[i],
                    shortcuts[i],
                    static_cast<int>(ui::KeyModifier::Control),
                    [this, mode]
                    {
                        close();
                        if (auto app = _p->app.lock())
                        {
                            auto tmp = _p->compareOptions;
                            tmp.mode = mode;
                            app->getFilesModel()->setCompareOptions(tmp);
                        }
                    });
                addItem(p.compareItems[mode]);
            }

            addDivider();

            p.currentMenu = addSubMenu("Current");

            auto item = std::make_shared<ui::MenuItem>(
                "Next",
                "Next",
                ui::Key::PageDown,
                static_cast<int>(ui::KeyModifier::Shift),
                [this]
                {
                    close();
                    if (auto app = _p->app.lock())
                    {
                        app->getFilesModel()->nextB();
                    }
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Previous",
                "Prev",
                ui::Key::PageUp,
                static_cast<int>(ui::KeyModifier::Shift),
                [this]
                {
                    close();
                    if (auto app = _p->app.lock())
                    {
                        app->getFilesModel()->prevB();
                    }
                });
            addItem(item);
            setItemEnabled(item, false);

            p.filesObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->getFilesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _currentUpdate(value);
                });

            p.filesBObserver = observer::ListObserver<int>::create(
                app->getFilesModel()->observeBIndexes(),
                [this](const std::vector<int>& value)
                {
                    _currentCheckedUpdate(value);
                });

            p.compareOptionsObserver = observer::ValueObserver<timeline::CompareOptions>::create(
                app->getFilesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _compareUpdate(value);
                });
        }

        CompareMenu::CompareMenu() :
            _p(new Private)
        {}

        CompareMenu::~CompareMenu()
        {}

        std::shared_ptr<CompareMenu> CompareMenu::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<CompareMenu>(new CompareMenu);
            out->_init(app, context);
            return out;
        }

        void CompareMenu::close()
        {
            Menu::close();
            TLRENDER_P();
            p.currentMenu->close();
        }

        void CompareMenu::_currentUpdate(
            const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
        {
            TLRENDER_P();
            p.currentMenu->clear();
            p.currentItems.clear();
            if (auto app = p.app.lock())
            {
                const auto bIndexes = app->getFilesModel()->getBIndexes();
                for (size_t i = 0; i < value.size(); ++i)
                {
                    auto item = std::make_shared<ui::MenuItem>(
                        value[i]->path.get(-1, false),
                        [this, i]
                        {
                            close();
                        if (auto app = _p->app.lock())
                        {
                            app->getFilesModel()->toggleB(i);
                        }
                        });
                    const auto j = std::find(bIndexes.begin(), bIndexes.end(), i);
                    item->checked = j != bIndexes.end();
                    p.currentMenu->addItem(item);
                    p.currentItems.push_back(item);
                }
            }
        }

        void CompareMenu::_currentCheckedUpdate(const std::vector<int>& value)
        {
            TLRENDER_P();
            for (int i = 0; i < p.currentItems.size(); ++i)
            {
                const auto j = std::find(value.begin(), value.end(), i);
                p.currentMenu->setItemChecked(
                    p.currentItems[i],
                    j != value.end());
            }
        }

        void CompareMenu::_compareUpdate(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            p.compareOptions = value;
            for (const auto& item : p.compareItems)
            {
                setItemChecked(item.second, item.first == value.mode);
            }
        }
    }
}
