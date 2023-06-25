// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/FileBrowserPrivate.h>

#include <tlUI/Spacer.h>

#include <tlCore/File.h>
#include <tlCore/FileInfo.h>

namespace tl
{
    namespace ui
    {
        void PathsWidget::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::PathsWidget", context, parent);

            _drivesModel = DrivesModel::create(context);

            _buttonGroup = ButtonGroup::create(ButtonGroupType::Click, context);

            _layout = VerticalLayout::create(context, shared_from_this());
            _layout->setSpacingRole(SizeRole::None);

            _pathsUpdate();

            _buttonGroup->setClickedCallback(
                [this](int value)
                {
                    if (value >= 0 &&
                        value < _paths.size())
                    {
                        const std::string& path = _paths[value].first;
                        if (_callback)
                        {
                            _callback(path);
                        }
                    }
                });

            _drivesObserver = observer::ListObserver<std::string>::create(
                _drivesModel->observeDrives(),
                [this](const std::vector<std::string>& value)
                {
                    _drives = value;
                    _pathsUpdate();
                });
        }

        PathsWidget::PathsWidget()
        {}

        PathsWidget::~PathsWidget()
        {}

        std::shared_ptr<PathsWidget> PathsWidget::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PathsWidget>(new PathsWidget);
            out->_init(context, parent);
            return out;
        }

        void PathsWidget::setCallback(const std::function<void(const std::string&)>& value)
        {
            _callback = value;
        }

        void PathsWidget::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _layout->setGeometry(value);
        }

        void PathsWidget::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _layout->getSizeHint();
        }

        void PathsWidget::_pathsUpdate()
        {
            _paths.clear();
            for (const auto& i : _drives)
            {
                _paths.push_back(std::make_pair(i, i));
            }
            std::string path = file::getUserPath(file::UserPath::Home);
            _paths.push_back(std::make_pair(path, file::Path(path).getBaseName()));
            path = file::getUserPath(file::UserPath::Desktop);
            _paths.push_back(std::make_pair(path, file::Path(path).getBaseName()));
            path = file::getUserPath(file::UserPath::Documents);
            _paths.push_back(std::make_pair(path, file::Path(path).getBaseName()));
            path = file::getUserPath(file::UserPath::Downloads);
            _paths.push_back(std::make_pair(path, file::Path(path).getBaseName()));

            for (auto i : _buttons)
            {
                i->setParent(nullptr);
            }
            _buttons.clear();
            _buttonGroup->clearButtons();
            if (auto context = _context.lock())
            {
                for (auto i : _paths)
                {
                    auto button = ListButton::create(context);
                    button->setText(i.second);
                    button->setParent(_layout);
                    _buttons.push_back(button);
                    _buttonGroup->addButton(button);
                }
            }
        }

        void DirectoryWidget::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::DirectoryWidget", context, parent);

            _buttonGroup = ButtonGroup::create(ButtonGroupType::Click, context);

            _layout = VerticalLayout::create(context, shared_from_this());
            _layout->setSpacingRole(SizeRole::None);

            _buttonGroup->setClickedCallback(
                [this](int value)
                {
                    if (value >= 0 &&
                        value < _fileInfos.size())
                    {
                        const file::FileInfo& fileInfo = _fileInfos[value];
                        switch (fileInfo.getType())
                        {
                        case file::Type::File:
                            if (_fileCallback)
                            {
                                _fileCallback(fileInfo.getPath().get(-1, false));
                            }
                            break;
                        case file::Type::Directory:
                        {
                            _path = file::Path(_path, fileInfo.getPath().get(-1, false)).get();
                            _directoryUpdate();
                            if (_pathCallback)
                            {
                                _pathCallback(_path);
                            }
                            break;
                        }
                        default: break;
                        }
                    }
                });
        }

        DirectoryWidget::DirectoryWidget()
        {}

        DirectoryWidget::~DirectoryWidget()
        {}

        std::shared_ptr<DirectoryWidget> DirectoryWidget::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<DirectoryWidget>(new DirectoryWidget);
            out->_init(context, parent);
            return out;
        }

        void DirectoryWidget::setPath(const std::string& value)
        {
            if (value == _path)
                return;
            _path = value;
            _directoryUpdate();
        }

        void DirectoryWidget::setFileCallback(const std::function<void(const std::string&)>& value)
        {
            _fileCallback = value;
        }

        void DirectoryWidget::setPathCallback(const std::function<void(const std::string&)>& value)
        {
            _pathCallback = value;
        }

        void DirectoryWidget::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _layout->setGeometry(value);
        }

        void DirectoryWidget::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _layout->getSizeHint();
        }

        void DirectoryWidget::_directoryUpdate()
        {
            for (auto i : _buttons)
            {
                i->setParent(nullptr);
            }
            _buttons.clear();
            _buttonGroup->clearButtons();
            file::ListOptions listOptions;
            listOptions.sequence = false;
            _fileInfos = file::list(_path, listOptions);
            if (auto context = _context.lock())
            {
                for (auto i : _fileInfos)
                {
                    auto button = ListButton::create(context);
                    switch (i.getType())
                    {
                    case file::Type::File:
                        button->setIcon("File");
                        break;
                    case file::Type::Directory:
                        button->setIcon("Directory");
                        break;
                    }
                    button->setText(i.getPath().get(-1, false));
                    button->setParent(_layout);
                    _buttons.push_back(button);
                    _buttonGroup->addButton(button);
                }
            }
        }

        void FileBrowserWidget::_init(
            const std::string& path,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::FileBrowserWidget", context, parent);

            _path = file::Path(path);

            _pathEdit = LineEdit::create(context);
            _pathEdit->setHStretch(Stretch::Expanding);

            _upButton = ToolButton::create(context);
            _upButton->setText("Up");

            _cwdButton = ToolButton::create(context);
            _cwdButton->setText("Current");

            _pathsWidget = PathsWidget::create(context);
            _pathsScrollWidget = ScrollWidget::create(context);
            _pathsScrollWidget->setWidget(_pathsWidget);
            _pathsScrollWidget->setVStretch(Stretch::Expanding);

            _directoryWidget = DirectoryWidget::create(context);
            _directoryScrollWidget = ScrollWidget::create(context);
            _directoryScrollWidget->setWidget(_directoryWidget);
            _directoryScrollWidget->setVStretch(Stretch::Expanding);

            _okButton = PushButton::create(context);
            _okButton->setText("Ok");

            _cancelButton = PushButton::create(context);
            _cancelButton->setText("Cancel");

            _layout = VerticalLayout::create(context, shared_from_this());
            _layout->setSpacingRole(SizeRole::SpacingSmall);
            _layout->setMarginRole(SizeRole::MarginSmall);
            auto hLayout = HorizontalLayout::create(context, _layout);
            hLayout->setSpacingRole(SizeRole::SpacingSmall);
            _pathEdit->setParent(hLayout);
            _upButton->setParent(hLayout);
            _cwdButton->setParent(hLayout);
            _splitter = Splitter::create(Orientation::Horizontal, context, _layout);
            _pathsScrollWidget->setParent(_splitter);
            _directoryScrollWidget->setParent(_splitter);
            _splitter->setSplit(0.2);
            hLayout = HorizontalLayout::create(context, _layout);
            hLayout->setSpacingRole(SizeRole::SpacingSmall);
            auto spacer = Spacer::create(context, hLayout);
            spacer->setHStretch(Stretch::Expanding);
            _okButton->setParent(hLayout);
            _cancelButton->setParent(hLayout);

            _pathUpdate();

            _pathEdit->setTextCallback(
                [this](const std::string& value)
                {
                    if (file::exists(value))
                    {
                        _path = file::Path(value, _path.get(-1, false));
                    }
                    else
                    {
                        std::string path = value;
                        if (file::hasEndSeparator(path))
                        {
                            path.pop_back();
                        }
                        _path = file::Path(file::Path(path).getDirectory(), "");
                    }
                    _pathUpdate();
                });

            _upButton->setClickedCallback(
                [this]
                {
                    std::string path = _path.get();
                    if (file::hasEndSeparator(path))
                    {
                        path.pop_back();
                    }
                    else
                    {
                        path = _path.getDirectory();
                        if (file::hasEndSeparator(path))
                        {
                            path.pop_back();
                        }
                    }
                    _path = file::Path(file::Path(path).getDirectory(), "");
                    _pathUpdate();
                });

            _cwdButton->setClickedCallback(
                [this]
                {
                    _path = file::Path(file::getCWD());
                });

            _pathsWidget->setCallback(
                [this](const std::string& value)
                {
                    _path = file::Path(value, "");
                    _pathUpdate();
                });

            _directoryWidget->setFileCallback(
                [this](const std::string& value)
                {
                    _path = file::Path(_path.getDirectory(), value);
                    _pathUpdate();
                    if (_fileCallback)
                    {
                        _fileCallback(_path.get());
                    }
                });
            _directoryWidget->setPathCallback(
                [this](const std::string& value)
                {
                    _path = file::Path(value, "");
                    _pathUpdate();
                });

            _okButton->setClickedCallback(
                [this]
                {
                    if (_fileCallback)
                    {
                        _fileCallback(_path.get());
                    }
                });

            _cancelButton->setClickedCallback(
                [this]
                {
                    if (_cancelCallback)
                    {
                        _cancelCallback();
                    }
                });
        }

        FileBrowserWidget::FileBrowserWidget()
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

        void FileBrowserWidget::setFileCallback(const std::function<void(const std::string&)>& value)
        {
            _fileCallback = value;
        }

        void FileBrowserWidget::setCancelCallback(const std::function<void(void)>& value)
        {
            _cancelCallback = value;
        }

        void FileBrowserWidget::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _layout->setGeometry(value);
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
            _pathEdit->setText(_path.get());
            _directoryWidget->setPath(_path.getDirectory());
            _directoryScrollWidget->setScrollPos(math::Vector2i(0, 0));
        }

        struct FileBrowser::Private
        {
            std::shared_ptr<FileBrowserWidget> widget;
        };

        void FileBrowser::_init(
            const std::string& path,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IDialog::_init("tl::ui::FileBrowser", context, parent);
            TLRENDER_P();

            p.widget = FileBrowserWidget::create(path, context, shared_from_this());

            p.widget->setCancelCallback(
                [this]
                {
                    close();
                });
        }

        FileBrowser::FileBrowser() :
            _p(new Private)
        {}

        FileBrowser::~FileBrowser()
        {}

        std::shared_ptr<FileBrowser> FileBrowser::create(
            const std::string& path,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileBrowser>(new FileBrowser);
            out->_init(path, context, parent);
            return out;
        }

        void FileBrowser::setFileCallback(const std::function<void(const std::string&)>& value)
        {
            _p->widget->setFileCallback(value);
        }
    }
}
