// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>

#include <feather-tk/ui/Menu.h>
#include <feather-tk/ui/MenuBar.h>

namespace tl
{
    namespace play
    {
        class App;
        class CompareActions;
        class FileActions;
        class WindowActions;
        class ViewActions;
        class PlaybackActions;

        //! File menu.
        class FileMenu : public feather_tk::Menu
        {
            FEATHER_TK_NON_COPYABLE(FileMenu);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<FileActions>&,
                const std::shared_ptr<IWidget>& parent);

            FileMenu() = default;

        public:
            ~FileMenu();

            static std::shared_ptr<FileMenu> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<FileActions>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            int _playerIndex = -1;
            std::vector<std::shared_ptr<feather_tk::Action> > _filesActions;
            std::shared_ptr<feather_tk::Menu> _filesMenu;
            std::vector<std::shared_ptr<feather_tk::Action> > _recentFilesActions;
            std::shared_ptr<feather_tk::Menu> _recentFilesMenu;
            std::shared_ptr<feather_tk::ListObserver<std::shared_ptr<timeline::Player> > > _playersObserver;
            std::shared_ptr<feather_tk::ValueObserver<int> > _playerIndexObserver;
            std::shared_ptr<feather_tk::ListObserver<std::filesystem::path> > _recentFilesObserver;
        };

        //! Compare menu.
        class CompareMenu : public feather_tk::Menu
        {
            FEATHER_TK_NON_COPYABLE(CompareMenu);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<CompareActions>&,
                const std::shared_ptr<IWidget>& parent);

            CompareMenu() = default;

        public:
            ~CompareMenu();

            static std::shared_ptr<CompareMenu> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<CompareActions>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            std::vector<std::shared_ptr<feather_tk::Action> > _bFileActions;
            std::shared_ptr<feather_tk::Menu> _bFileMenu;
            int _bPlayerIndex = -1;
            std::shared_ptr<feather_tk::ListObserver<std::shared_ptr<timeline::Player> > > _playersObserver;
            std::shared_ptr<feather_tk::ValueObserver<int> > _bPlayerIndexObserver;
        };

        //! Playback menu.
        class PlaybackMenu : public feather_tk::Menu
        {
            FEATHER_TK_NON_COPYABLE(PlaybackMenu);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<PlaybackActions>&,
                const std::shared_ptr<IWidget>& parent);

            PlaybackMenu() = default;

        public:
            ~PlaybackMenu();

            static std::shared_ptr<PlaybackMenu> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<PlaybackActions>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
        };

        //! View menu.
        class ViewMenu : public feather_tk::Menu
        {
            FEATHER_TK_NON_COPYABLE(ViewMenu);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<ViewActions>&,
                const std::shared_ptr<IWidget>& parent);

            ViewMenu() = default;

        public:
            ~ViewMenu();

            static std::shared_ptr<ViewMenu> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<ViewActions>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
        };

        //! Window menu.
        class WindowMenu : public feather_tk::Menu
        {
            FEATHER_TK_NON_COPYABLE(WindowMenu);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<WindowActions>&,
                const std::shared_ptr<IWidget>& parent);

            WindowMenu() = default;

        public:
            ~WindowMenu();

            static std::shared_ptr<WindowMenu> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<WindowActions>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
        };

        //! Menu bar.
        class MenuBar : public feather_tk::MenuBar
        {
            FEATHER_TK_NON_COPYABLE(MenuBar);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<FileActions>&,
                const std::shared_ptr<CompareActions>&,
                const std::shared_ptr<PlaybackActions>&,
                const std::shared_ptr<ViewActions>&,
                const std::shared_ptr<WindowActions>&,
                const std::shared_ptr<IWidget>& parent);

            MenuBar() = default;

        public:
            ~MenuBar();

            static std::shared_ptr<MenuBar> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<FileActions>&,
                const std::shared_ptr<CompareActions>&,
                const std::shared_ptr<PlaybackActions>&,
                const std::shared_ptr<ViewActions>&,
                const std::shared_ptr<WindowActions>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
        };
    }
}