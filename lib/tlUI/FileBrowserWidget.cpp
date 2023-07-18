// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/FileBrowserPrivate.h>

#include <tlUI/ComboBox.h>
#include <tlUI/Label.h>
#include <tlUI/LineEdit.h>
#include <tlUI/PushButton.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>
#include <tlUI/Spacer.h>
#include <tlUI/Splitter.h>
#include <tlUI/ToolButton.h>

#include <tlCore/File.h>

#include <list>

namespace tl
{
    namespace ui
    {
        struct FileBrowserWidget::Private
        {
            file::Path path;
            file::ListOptions listOptions;
            std::shared_ptr<RecentFilesModel> recentModel;

            std::shared_ptr<ToolButton> upButton;
            std::shared_ptr<ToolButton> cwdButton;
            std::shared_ptr<LineEdit> pathEdit;
            std::shared_ptr<PathsWidget> pathsWidget;
            std::shared_ptr<ScrollWidget> pathsScrollWidget;
            std::shared_ptr<DirectoryWidget> directoryWidget;
            std::shared_ptr<ScrollWidget> directoryScrollWidget;
            std::shared_ptr<Splitter> splitter;
            std::shared_ptr<ComboBox> sortComboBox;
            std::shared_ptr<PushButton> okButton;
            std::shared_ptr<PushButton> cancelButton;
            std::shared_ptr<VerticalLayout> layout;

            std::function<void(const file::Path&)> fileCallback;
            std::function<void(void)> cancelCallback;
        };

        void FileBrowserWidget::_init(
            const std::string& path,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::FileBrowserWidget", context, parent);
            TLRENDER_P();

            p.path = file::Path(path);

            p.recentModel = RecentFilesModel::create(context);

            p.upButton = ToolButton::create(context);
            p.upButton->setIcon("DirectoryUp");

            p.pathEdit = LineEdit::create(context);
            p.pathEdit->setHStretch(Stretch::Expanding);

            p.pathsWidget = PathsWidget::create(p.recentModel, context);
            p.pathsScrollWidget = ScrollWidget::create(context);
            p.pathsScrollWidget->setWidget(p.pathsWidget);
            p.pathsScrollWidget->setVStretch(Stretch::Expanding);

            p.directoryWidget = DirectoryWidget::create(context);
            p.directoryScrollWidget = ScrollWidget::create(context);
            p.directoryScrollWidget->setWidget(p.directoryWidget);
            p.directoryScrollWidget->setVStretch(Stretch::Expanding);

            p.sortComboBox = ComboBox::create(context);
            p.sortComboBox->setItems(file::getListSortLabels());

            p.okButton = PushButton::create(context);
            p.okButton->setText("Ok");

            p.cancelButton = PushButton::create(context);
            p.cancelButton->setText("Cancel");

            p.layout = VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::None);
            auto label = Label::create("File Browser", context, p.layout);
            label->setBackgroundRole(ColorRole::Button);
            label->setMarginRole(SizeRole::MarginSmall);
            auto vLayout = VerticalLayout::create(context, p.layout);
            vLayout->setSpacingRole(SizeRole::SpacingSmall);
            vLayout->setMarginRole(SizeRole::MarginSmall);
            vLayout->setVStretch(Stretch::Expanding);
            auto hLayout = HorizontalLayout::create(context, vLayout);
            hLayout->setSpacingRole(SizeRole::SpacingSmall);
            p.upButton->setParent(hLayout);
            p.pathEdit->setParent(hLayout);
            p.splitter = Splitter::create(Orientation::Horizontal, context, vLayout);
            p.splitter->setSplit(0.2);
            p.splitter->setSpacingRole(SizeRole::SpacingSmall);
            p.pathsScrollWidget->setParent(p.splitter);
            p.directoryScrollWidget->setParent(p.splitter);
            hLayout = HorizontalLayout::create(context, vLayout);
            hLayout->setSpacingRole(SizeRole::SpacingSmall);
            label = Label::create("Sort:", context, hLayout);
            p.sortComboBox->setParent(hLayout);
            auto spacer = Spacer::create(context, hLayout);
            spacer->setHStretch(Stretch::Expanding);
            p.okButton->setParent(hLayout);
            p.cancelButton->setParent(hLayout);

            _pathUpdate();

            p.upButton->setClickedCallback(
                [this]
                {
                    TLRENDER_P();
                    std::string path = p.path.get();
                    if (file::hasEndSeparator(path))
                    {
                        path.pop_back();
                    }
                    else
                    {
                        path = p.path.getDirectory();
                        if (file::hasEndSeparator(path))
                        {
                            path.pop_back();
                        }
                    }
                    p.path = file::Path(file::Path(path).getDirectory(), "");
                    _pathUpdate();
                });

            p.pathEdit->setTextCallback(
                [this](const std::string& value)
                {
                    TLRENDER_P();
                    if (file::exists(value))
                    {
                        p.path = file::Path(value, p.path.get(-1, false));
                    }
                    else
                    {
                        std::string path = value;
                        if (file::hasEndSeparator(path))
                        {
                            path.pop_back();
                        }
                        p.path = file::Path(file::Path(path).getDirectory(), "");
                    }
                    _pathUpdate();
                });

            p.pathsWidget->setCallback(
                [this](const std::string& value)
                {
                    _p->path = file::Path(value, "");
                    _pathUpdate();
                });

            p.directoryWidget->setFileCallback(
                [this](const std::string& value)
                {
                    TLRENDER_P();
                    p.path = file::Path(p.path.getDirectory(), value);
                    _pathUpdate();
                    if (p.fileCallback)
                    {
                        p.fileCallback(p.path);
                    }
                });
            p.directoryWidget->setPathCallback(
                [this](const std::string& value)
                {
                    _p->path = file::Path(value, "");
                    _pathUpdate();
                });

            p.sortComboBox->setIndexCallback(
                [this](int value)
                {
                    _p->listOptions.sort = static_cast<file::ListSort>(value);
                    _pathUpdate();
                });

            p.okButton->setClickedCallback(
                [this]
                {
                    TLRENDER_P();
                    if (p.fileCallback)
                    {
                        p.fileCallback(p.path);
                    }
                });

            p.cancelButton->setClickedCallback(
                [this]
                {
                    TLRENDER_P();
                    if (p.cancelCallback)
                    {
                        p.cancelCallback();
                    }
                });
        }

        FileBrowserWidget::FileBrowserWidget() :
            _p(new Private)
        {}

        FileBrowserWidget::~FileBrowserWidget()
        {}

        std::shared_ptr<FileBrowserWidget> FileBrowserWidget::create(
            const std::string& path,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileBrowserWidget>(new FileBrowserWidget);
            out->_init(path, context, parent);
            return out;
        }

        void FileBrowserWidget::setFileCallback(const std::function<void(const file::Path&)>& value)
        {
            _p->fileCallback = value;
        }

        void FileBrowserWidget::setCancelCallback(const std::function<void(void)>& value)
        {
            _p->cancelCallback = value;
        }

        void FileBrowserWidget::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FileBrowserWidget::mouseMoveEvent(MouseMoveEvent& event)
        {
            event.accept = true;
        }

        void FileBrowserWidget::mousePressEvent(MouseClickEvent& event)
        {
            event.accept = true;
        }

        void FileBrowserWidget::mouseReleaseEvent(MouseClickEvent& event)
        {
            event.accept = true;
        }

        void FileBrowserWidget::_pathUpdate()
        {
            TLRENDER_P();
            p.pathEdit->setText(p.path.get());
            p.directoryWidget->setPath(p.path.getDirectory(), p.listOptions);
            p.directoryScrollWidget->setScrollPos(math::Vector2i(0, 0));
        }
    }
}
