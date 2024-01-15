// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/FileEdit.h>

#include <tlUI/LineEdit.h>
#include <tlUI/FileBrowser.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

#include <tlCore/File.h>

namespace tl
{
    namespace ui
    {
        struct FileEdit::Private
        {
            file::Path path;
            std::shared_ptr<LineEdit> lineEdit;
            std::shared_ptr<ToolButton> browseButton;
            std::shared_ptr<ToolButton> clearButton;
            std::shared_ptr<HorizontalLayout> layout;
            std::function<void(const file::Path&)> callback;
        };

        void FileEdit::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::FileEdit", context, parent);
            TLRENDER_P();

            setHStretch(Stretch::Expanding);

            p.lineEdit = LineEdit::create(context);
            p.lineEdit->setHStretch(Stretch::Expanding);

            p.browseButton = ToolButton::create(context);
            p.browseButton->setIcon("FileBrowser");
            p.browseButton->setToolTip("Show the file browser");

            p.clearButton = ToolButton::create(context);
            p.clearButton->setIcon("Reset");
            p.clearButton->setToolTip("Reset the file name");

            p.layout = HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::SpacingTool);
            p.lineEdit->setParent(p.layout);
            p.browseButton->setParent(p.layout);
            p.clearButton->setParent(p.layout);

            p.lineEdit->setTextCallback(
                [this](const std::string& value)
                {
                    _p->path = file::Path(value);
                    if (_p->callback)
                    {
                        _p->callback(_p->path);
                    }
                });

            p.browseButton->setClickedCallback(
                [this]
                {
                    _openDialog();
                });

            p.clearButton->setClickedCallback(
                [this]
                {
                    _p->lineEdit->clearText();
                    _p->path = file::Path();
                    if (_p->callback)
                    {
                        _p->callback(_p->path);
                    }
                });
        }

        FileEdit::FileEdit() :
            _p(new Private)
        {}

        FileEdit::~FileEdit()
        {}

        std::shared_ptr<FileEdit> FileEdit::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileEdit>(new FileEdit);
            out->_init(context, parent);
            return out;
        }

        void FileEdit::setPath(const file::Path& value)
        {
            TLRENDER_P();
            if (value == p.path)
                return;
            p.path = value;
            p.lineEdit->setText(value.get());
        }

        const file::Path& FileEdit::getPath() const
        {
            return _p->path;
        }

        void FileEdit::setCallback(const std::function<void(const file::Path&)>& value)
        {
            _p->callback = value;
        }

        void FileEdit::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FileEdit::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        void FileEdit::_openDialog()
        {
            TLRENDER_P();
            if (auto context = _context.lock())
            {
                if (auto fileBrowserSystem = context->getSystem<FileBrowserSystem>())
                {
                    fileBrowserSystem->open(
                        getWindow(),
                        [this](const file::FileInfo& value)
                        {
                            _p->path = value.getPath();
                            _p->lineEdit->setText(_p->path.get());
                            if (_p->callback)
                            {
                                _p->callback(_p->path);
                            }
                        });
                }
            }
        }
    }
}
