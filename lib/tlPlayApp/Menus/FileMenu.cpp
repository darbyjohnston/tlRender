// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Menus/FileMenu.h>

#include <tlPlayApp/Actions/FileActions.h>
#include <tlPlayApp/Models/RecentFilesModel.h>
#include <tlPlayApp/App.h>

#include <dtk/ui/FileBrowser.h>

#include <tlIO/System.h>

namespace tl
{
    namespace play
    {
        struct FileMenu::Private
        {
            std::weak_ptr<App> app;
            std::vector<std::string> extensions;
            std::shared_ptr<dtk::RecentFilesModel> recentFilesModel;

            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
            std::vector<std::shared_ptr<dtk::Action> > currentItems;
            std::vector<std::shared_ptr<dtk::Action> > layersItems;
            std::map<std::string, std::shared_ptr<dtk::Menu> > menus;

            std::shared_ptr<dtk::ListObserver<std::shared_ptr<FilesModelItem> > > filesObserver;
            std::shared_ptr<dtk::ValueObserver<std::shared_ptr<FilesModelItem> > > aObserver;
            std::shared_ptr<dtk::ValueObserver<int> > aIndexObserver;
            std::shared_ptr<dtk::ListObserver<int> > layersObserver;
            std::shared_ptr<dtk::ListObserver<std::filesystem::path> > recentObserver;
        };

        void FileMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<FileActions>& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            DTK_P();

            p.app = app;

            auto ioSystem = context->getSystem<io::System>();
            for (const auto& extension : ioSystem->getExtensions())
            {
                p.extensions.push_back(extension);
            }
            p.extensions.push_back(".otio");
            p.extensions.push_back(".otioz");

            p.recentFilesModel = app->getRecentFilesModel();

            p.actions = actions->getActions();

            addItem(p.actions["Open"]);
            addItem(p.actions["OpenSeparateAudio"]);
            addItem(p.actions["Close"]);
            addItem(p.actions["CloseAll"]);
            addItem(p.actions["Reload"]);
            p.menus["Recent"] = addSubMenu("Recent");
            addDivider();
            p.menus["Current"] = addSubMenu("Current");
            addItem(p.actions["Next"]);
            addItem(p.actions["Prev"]);
            addDivider();
            p.menus["Layers"] = addSubMenu("Layers");
            addItem(p.actions["NextLayer"]);
            addItem(p.actions["PrevLayer"]);
            addDivider();
            addItem(p.actions["Exit"]);

            p.filesObserver = dtk::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                app->getFilesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
                {
                    _filesUpdate(value);
                });

            p.aObserver = dtk::ValueObserver<std::shared_ptr<FilesModelItem> >::create(
                app->getFilesModel()->observeA(),
                [this](const std::shared_ptr<FilesModelItem>& value)
                {
                    _aUpdate(value);
                });

            p.aIndexObserver = dtk::ValueObserver<int>::create(
                app->getFilesModel()->observeAIndex(),
                [this](int value)
                {
                    _aIndexUpdate(value);
                });

            p.layersObserver = dtk::ListObserver<int>::create(
                app->getFilesModel()->observeLayers(),
                [this](const std::vector<int>& value)
                {
                    _layersUpdate(value);
                });

            if (p.recentFilesModel)
            {
                p.recentObserver = dtk::ListObserver<std::filesystem::path>::create(
                    p.recentFilesModel->observeRecent(),
                    [this](const std::vector<std::filesystem::path>& value)
                    {
                        _recentUpdate(value);
                    });
            }
        }

        FileMenu::FileMenu() :
            _p(new Private)
        {}

        FileMenu::~FileMenu()
        {}

        std::shared_ptr<FileMenu> FileMenu::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<FileActions>& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileMenu>(new FileMenu);
            out->_init(context, app, actions, parent);
            return out;
        }

        void FileMenu::close()
        {
            Menu::close();
            DTK_P();
            for (const auto& menu : p.menus)
            {
                menu.second->close();
            }
        }

        void FileMenu::_filesUpdate(
            const std::vector<std::shared_ptr<FilesModelItem> >& value)
        {
            DTK_P();

            setItemEnabled(p.actions["Close"], !value.empty());
            setItemEnabled(p.actions["CloseAll"], !value.empty());
            setItemEnabled(p.actions["Reload"], !value.empty());
            setItemEnabled(p.actions["Next"], value.size() > 1);
            setItemEnabled(p.actions["Prev"], value.size() > 1);

            p.menus["Current"]->clear();
            p.currentItems.clear();
            for (size_t i = 0; i < value.size(); ++i)
            {
                auto item = dtk::Action::create(
                    value[i]->path.get(-1, file::PathType::FileName),
                    [this, i]
                    {
                        close();
                        if (auto app = _p->app.lock())
                        {
                            app->getFilesModel()->setA(i);
                        }
                    });
                p.menus["Current"]->addItem(item);
                p.currentItems.push_back(item);
            }
        }

        void FileMenu::_aUpdate(const std::shared_ptr<FilesModelItem>& value)
        {
            DTK_P();

            p.menus["Layers"]->clear();
            p.layersItems.clear();
            if (value)
            {
                for (size_t i = 0; i < value->videoLayers.size(); ++i)
                {
                    auto item = dtk::Action::create(
                        value->videoLayers[i],
                        [this, value, i]
                        {
                            close();
                            if (auto app = _p->app.lock())
                            {
                                app->getFilesModel()->setLayer(value, i);
                            }
                        });
                    item->setChecked(i == value->videoLayer);
                    p.menus["Layers"]->addItem(item);
                    p.layersItems.push_back(item);
                }
            }

            setItemEnabled(p.actions["NextLayer"], value ? value->videoLayers.size() > 1 : false);
            setItemEnabled(p.actions["PrevLayer"], value ? value->videoLayers.size() > 1 : false);
        }

        void FileMenu::_aIndexUpdate(int value)
        {
            DTK_P();
            for (int i = 0; i < p.currentItems.size(); ++i)
            {
                p.menus["Current"]->setItemChecked(p.currentItems[i], i == value);
            }
        }

        void FileMenu::_layersUpdate(const std::vector<int>& value)
        {
            DTK_P();
            if (auto app = p.app.lock())
            {
                auto a = app->getFilesModel()->getA();
                for (size_t i = 0; i < p.layersItems.size(); ++i)
                {
                    p.menus["Layers"]->setItemChecked(p.layersItems[i], i == a->videoLayer);
                }
            }
        }

        void FileMenu::_recentUpdate(const std::vector<std::filesystem::path>& value)
        {
            DTK_P();
            p.menus["Recent"]->clear();
            if (!value.empty())
            {
                for (auto i = value.rbegin(); i != value.rend(); ++i)
                {
                    const auto path = *i;
                    auto weak = std::weak_ptr<FileMenu>(std::dynamic_pointer_cast<FileMenu>(shared_from_this()));
                    auto item = dtk::Action::create(
                        path.u8string(),
                        [weak, path]
                        {
                            if (auto widget = weak.lock())
                            {
                                if (auto app = widget->_p->app.lock())
                                {
                                    app->open(file::Path(path.u8string()));
                                }
                                widget->close();
                            }
                        });
                    p.menus["Recent"]->addItem(item);
                }
            }
        }
    }
}
