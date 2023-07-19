// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/FileBrowser.h>

#include <tlUI/IButton.h>
#include <tlUI/RecentFilesModel.h>

#include <tlCore/FileInfo.h>

namespace tl
{
    namespace ui
    {
        class PathsWidget : public IWidget
        {
            TLRENDER_NON_COPYABLE(PathsWidget);

        protected:
            void _init(
                const std::shared_ptr<RecentFilesModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            PathsWidget();

        public:
            ~PathsWidget() override;

            static std::shared_ptr<PathsWidget> create(
                const std::shared_ptr<RecentFilesModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setCallback(const std::function<void(const std::string&)>&);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            void _createLabel(
                const std::string&,
                const std::shared_ptr<system::Context>&);
            void _createButton(
                const std::string&,
                const std::shared_ptr<system::Context>&);

            void _pathsUpdate();

            TLRENDER_PRIVATE();
        };

        class Button : public IButton
        {
            TLRENDER_NON_COPYABLE(Button);

        protected:
            void _init(
                const file::FileInfo&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            Button();

        public:
            ~Button() override;

            static std::shared_ptr<Button> create(
                const file::FileInfo&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            const std::vector<int>& getTextWidths() const;

            void setColumns(const std::vector<int>&);

            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(
                const math::BBox2i&,
                bool,
                const ClipEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const DrawEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };

        class DirectoryWidget : public IWidget
        {
            TLRENDER_NON_COPYABLE(DirectoryWidget);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            DirectoryWidget();

        public:
            ~DirectoryWidget() override;

            static std::shared_ptr<DirectoryWidget> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setPath(
                const std::string&,
                const file::ListOptions&);

            void setFileCallback(const std::function<void(const std::string&)>&);

            void setPathCallback(const std::function<void(const std::string&)>&);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            void _directoryUpdate();

            TLRENDER_PRIVATE();
        };

        class FileBrowserWidget : public IWidget
        {
            TLRENDER_NON_COPYABLE(FileBrowserWidget);

        protected:
            void _init(
                const std::string&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            FileBrowserWidget();

        public:
            ~FileBrowserWidget() override;

            static std::shared_ptr<FileBrowserWidget> create(
                const std::string&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setFileCallback(const std::function<void(const file::Path&)>&);

            void setCancelCallback(const std::function<void(void)>&);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;

        private:
            void _pathUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
