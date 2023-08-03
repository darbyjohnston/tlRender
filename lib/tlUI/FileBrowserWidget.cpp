// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/FileBrowserPrivate.h>

#include <tlUI/CheckBox.h>
#include <tlUI/ComboBox.h>
#include <tlUI/Divider.h>
#include <tlUI/Label.h>
#include <tlUI/LineEdit.h>
#include <tlUI/PushButton.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>
#include <tlUI/Spacer.h>
#include <tlUI/Splitter.h>
#include <tlUI/ToolButton.h>

#include <tlIO/IOSystem.h>

#include <tlCore/File.h>
#include <tlCore/StringFormat.h>

#include <list>

namespace tl
{
    namespace ui
    {
        struct FileBrowserWidget::Private
        {
            file::Path path;
            std::shared_ptr<RecentFilesModel> recentModel;
            std::vector<std::string> extensions;

            std::shared_ptr<ToolButton> upButton;
            std::shared_ptr<ToolButton> cwdButton;
            std::shared_ptr<LineEdit> pathEdit;
            std::shared_ptr<PathsWidget> pathsWidget;
            std::shared_ptr<ScrollWidget> pathsScrollWidget;
            std::shared_ptr<DirectoryWidget> directoryWidget;
            std::shared_ptr<ScrollWidget> directoryScrollWidget;
            std::shared_ptr<LineEdit> filterEdit;
            std::shared_ptr<ToolButton> filterClearButton;
            std::shared_ptr<ComboBox> extensionsComboBox;
            std::shared_ptr<ComboBox> sortComboBox;
            std::shared_ptr<CheckBox> reverseSortCheckBox;
            std::shared_ptr<PushButton> okButton;
            std::shared_ptr<PushButton> cancelButton;
            std::shared_ptr<Splitter> splitter;
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

            setHStretch(Stretch::Expanding);
            setVStretch(Stretch::Expanding);
            setMouseHover(true);

            p.path = file::Path(path);

            p.recentModel = RecentFilesModel::create(context);

            std::vector<std::string> extensionsLabels;
            if (auto context = _context.lock())
            {
                auto ioSystem = context->getSystem<io::System>();
                for (const auto& extension : ioSystem->getExtensions())
                {
                    p.extensions.push_back(extension);
                    extensionsLabels.push_back(string::Format("*{0}").arg(extension));
                }
            }
            p.extensions.push_back(std::string());
            extensionsLabels.push_back("*.*");

            p.upButton = ToolButton::create(context);
            p.upButton->setIcon("DirectoryUp");
            p.upButton->setToolTip("Go up a directory");

            p.pathEdit = LineEdit::create(context);
            p.pathEdit->setHStretch(Stretch::Expanding);
            p.pathEdit->setToolTip("The current directory");

            p.pathsWidget = PathsWidget::create(p.recentModel, context);
            p.pathsScrollWidget = ScrollWidget::create(context);
            p.pathsScrollWidget->setWidget(p.pathsWidget);
            p.pathsScrollWidget->setVStretch(Stretch::Expanding);

            p.directoryWidget = DirectoryWidget::create(context);
            p.directoryScrollWidget = ScrollWidget::create(context);
            p.directoryScrollWidget->setWidget(p.directoryWidget);
            p.directoryScrollWidget->setVStretch(Stretch::Expanding);

            p.filterEdit = LineEdit::create(context);
            p.filterEdit->setToolTip("Filter the contents of the directory");
            
            p.filterClearButton = ToolButton::create(context);
            p.filterClearButton->setIcon("Clear");
            p.filterClearButton->setToolTip("Clear the filter");

            p.extensionsComboBox = ComboBox::create(extensionsLabels, context);
            if (!extensionsLabels.empty())
            {
                p.extensionsComboBox->setCurrentIndex(extensionsLabels.size() - 1);
            }
            p.extensionsComboBox->setToolTip(
                "Filter the contents of the directory by file extension");

            p.sortComboBox = ComboBox::create(file::getListSortLabels(), context);
            p.sortComboBox->setToolTip("Set the sort mode");

            p.reverseSortCheckBox = CheckBox::create("Reverse sort", context);
            p.reverseSortCheckBox->setToolTip("Reverse the sort");

            p.okButton = PushButton::create(context);
            p.okButton->setText("Ok");

            p.cancelButton = PushButton::create(context);
            p.cancelButton->setText("Cancel");

            p.layout = VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::None);
            auto label = Label::create("File Browser", context, p.layout);
            label->setBackgroundRole(ColorRole::Button);
            label->setMarginRole(SizeRole::MarginSmall);
            Divider::create(Orientation::Vertical, context, p.layout);
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
            p.pathsScrollWidget->setParent(p.splitter);
            p.directoryScrollWidget->setParent(p.splitter);
            hLayout = HorizontalLayout::create(context, vLayout);
            hLayout->setSpacingRole(SizeRole::SpacingSmall);
            label = Label::create("Filter:", context, hLayout);
            label->setMarginRole(SizeRole::MarginInside);
            auto hLayout2 = HorizontalLayout::create(context, hLayout);
            hLayout2->setSpacingRole(SizeRole::SpacingTool);
            p.filterEdit->setParent(hLayout2);
            p.filterClearButton->setParent(hLayout2);
            label = Label::create("Extensions:", context, hLayout);
            label->setMarginRole(SizeRole::MarginInside);
            p.extensionsComboBox->setParent(hLayout);
            label = Label::create("Sort:", context, hLayout);
            label->setMarginRole(SizeRole::MarginInside);
            p.sortComboBox->setParent(hLayout);
            p.reverseSortCheckBox->setParent(hLayout);
            auto spacer = Spacer::create(Orientation::Horizontal, context, hLayout);
            spacer->setSpacingRole(SizeRole::None);
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
                [this](const file::Path& value)
                {
                    TLRENDER_P();
                    if (p.fileCallback)
                    {
                        p.fileCallback(value);
                    }
                });
            p.directoryWidget->setPathCallback(
                [this](const file::Path& value)
                {
                    _p->path = value;
                    _pathUpdate();
                });

            p.filterEdit->setTextChangedCallback(
                [this](const std::string& value)
                {
                    FileBrowserOptions options = _p->directoryWidget->getOptions();
                    options.filter = value;
                    _p->directoryWidget->setOptions(options);
                });

            p.filterClearButton->setClickedCallback(
                [this]
                {
                    FileBrowserOptions options = _p->directoryWidget->getOptions();
                    options.filter = std::string();
                    _p->directoryWidget->setOptions(options);
                    _p->filterEdit->clearText();
                });

            p.extensionsComboBox->setIndexCallback(
                [this](int value)
                {
                    if (value >= 0 && value < _p->extensions.size())
                    {
                        FileBrowserOptions options = _p->directoryWidget->getOptions();
                        options.extension = _p->extensions[value];
                        _p->directoryWidget->setOptions(options);
                    }
                });

            p.sortComboBox->setIndexCallback(
                [this](int value)
                {
                    FileBrowserOptions options = _p->directoryWidget->getOptions();
                    options.list.sort = static_cast<file::ListSort>(value);
                    _p->directoryWidget->setOptions(options);
                });

            p.reverseSortCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    FileBrowserOptions options = _p->directoryWidget->getOptions();
                    options.list.reverseSort = value;
                    _p->directoryWidget->setOptions(options);
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

        const file::Path& FileBrowserWidget::getPath() const
        {
            return _p->path;
        }
        
        const FileBrowserOptions& FileBrowserWidget::getOptions() const
        {
            return _p->directoryWidget->getOptions();
        }

        void FileBrowserWidget::setOptions(const FileBrowserOptions& value)
        {
            TLRENDER_P();
            p.directoryWidget->setOptions(value);
            p.filterEdit->setText(value.filter);
            const auto i = std::find(p.extensions.begin(), p.extensions.end(), value.extension);
            if (i != p.extensions.end())
            {
                p.extensionsComboBox->setCurrentIndex(i - p.extensions.begin());
            }
            p.sortComboBox->setCurrentIndex(static_cast<int>(value.list.sort));
            p.reverseSortCheckBox->setChecked(value.list.reverseSort);
        }

        void FileBrowserWidget::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FileBrowserWidget::sizeHintEvent(const SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _sizeHint = _p->layout->getSizeHint();
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
            p.directoryWidget->setPath(p.path);
            p.directoryScrollWidget->setScrollPos(math::Vector2i(0, 0));
        }
    }
}
