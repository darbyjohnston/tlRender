// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/FileBrowser.h>

#include <tlUI/ButtonGroup.h>
#include <tlUI/ListButton.h>
#include <tlUI/LineEdit.h>
#include <tlUI/PushButton.h>
#include <tlUI/RowLayout.h>
#include <tlUI/Spacer.h>
#include <tlUI/ScrollWidget.h>
#include <tlUI/ToolButton.h>

#include <tlCore/FileInfo.h>

namespace tl
{
    namespace ui
    {
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

            void setPath(const std::string&);

            void setFileCallback(const std::function<void(const std::string&)>&);

            void setPathCallback(const std::function<void(const std::string&)>&);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            void _directoryUpdate();

            std::string _path;
            std::vector<file::FileInfo> _fileInfos;
            std::vector<std::shared_ptr<ListButton> > _buttons;
            std::shared_ptr<ButtonGroup> _buttonGroup;
            std::shared_ptr<VerticalLayout> _layout;
            std::function<void(const std::string&)> _fileCallback;
            std::function<void(const std::string&)> _pathCallback;
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

            void setFileCallback(const std::function<void(const std::string&)>&);

            void setCancelCallback(const std::function<void(void)>&);

            void setGeometry(const math::BBox2i&) override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;

        private:
            void _pathUpdate();

            file::Path _path;
            std::shared_ptr<LineEdit> _pathEdit;
            std::shared_ptr<ToolButton> _upButton;
            std::shared_ptr<ToolButton> _cwdButton;
            std::shared_ptr<DirectoryWidget> _directoryWidget;
            std::shared_ptr<ScrollWidget> _scrollWidget;
            std::shared_ptr<PushButton> _okButton;
            std::shared_ptr<PushButton> _cancelButton;
            std::shared_ptr<VerticalLayout> _layout;
            std::function<void(const std::string&)> _fileCallback;
            std::function<void(void)> _cancelCallback;
        };
    }
}
