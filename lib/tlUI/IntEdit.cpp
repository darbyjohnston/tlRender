// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IntEdit.h>

#include <tlUI/IncButtons.h>
#include <tlUI/LineEdit.h>
#include <tlUI/RowLayout.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        struct IntEdit::Private
        {
            std::shared_ptr<IntModel> model;
            int digits = 3;
            std::shared_ptr<LineEdit> lineEdit;
            std::shared_ptr<IntIncButtons> incButtons;
            std::shared_ptr<HorizontalLayout> layout;

            std::function<void(int)> callback;

            std::shared_ptr<observer::ValueObserver<int> > valueObserver;
            std::shared_ptr<observer::ValueObserver<math::IntRange> > rangeObserver;
        };

        void IntEdit::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IntModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::IntEdit", context, parent);
            TLRENDER_P();

            p.model = model;
            if (!p.model)
            {
                p.model = IntModel::create(context);
            }

            p.lineEdit = LineEdit::create(context, shared_from_this());
            p.lineEdit->setFontRole(FontRole::Mono);

            p.incButtons = IntIncButtons::create(p.model, context);

            p.layout = HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::SpacingTool);
            p.lineEdit->setParent(p.layout);
            p.incButtons->setParent(p.layout);

            p.lineEdit->setTextCallback(
                [this](const std::string& value)
                {
                    _p->model->setValue(std::atoi(value.c_str()));
                    _textUpdate();
                });
            p.lineEdit->setFocusCallback(
                [this](bool value)
                {
                    if (!value)
                    {
                        _textUpdate();
                    }
                });

            p.valueObserver = observer::ValueObserver<int>::create(
                p.model->observeValue(),
                [this](int value)
                {
                    _textUpdate();
                    if (_p->callback)
                    {
                        _p->callback(value);
                    }
                });

            p.rangeObserver = observer::ValueObserver<math::IntRange>::create(
                p.model->observeRange(),
                [this](const math::IntRange&)
                {
                    _textUpdate();
                });

            _textUpdate();
        }

        IntEdit::IntEdit() :
            _p(new Private)
        {}

        IntEdit::~IntEdit()
        {}

        std::shared_ptr<IntEdit> IntEdit::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IntModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<IntEdit>(new IntEdit);
            out->_init(context, model, parent);
            return out;
        }

        int IntEdit::getValue() const
        {
            return _p->model->getValue();
        }

        void IntEdit::setValue(int value)
        {
            _p->model->setValue(value);
        }

        void IntEdit::setCallback(const std::function<void(int)>& value)
        {
            _p->callback = value;
        }

        const math::IntRange& IntEdit::getRange() const
        {
            return _p->model->getRange();
        }

        void IntEdit::setRange(const math::IntRange& value)
        {
            _p->model->setRange(value);
        }

        void IntEdit::setStep(int value)
        {
            _p->model->setStep(value);
        }

        void IntEdit::setLargeStep(int value)
        {
            _p->model->setLargeStep(value);
        }

        const std::shared_ptr<IntModel>& IntEdit::getModel() const
        {
            return _p->model;
        }

        void IntEdit::setDigits(int value)
        {
            TLRENDER_P();
            if (value == p.digits)
                return;
            p.digits = value;
            _textUpdate();
        }

        void IntEdit::setFontRole(FontRole value)
        {
            _p->lineEdit->setFontRole(value);
        }

        void IntEdit::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void IntEdit::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        void IntEdit::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
            if (isEnabled() && p.model)
            {
                switch (event.key)
                {
                case Key::Down:
                    event.accept = true;
                    p.model->decrementStep();
                    break;
                case Key::Up:
                    event.accept = true;
                    p.model->incrementStep();
                    break;
                case Key::PageUp:
                    event.accept = true;
                    p.model->incrementLargeStep();
                    break;
                case Key::PageDown:
                    event.accept = true;
                    p.model->decrementLargeStep();
                    break;
                default: break;
                }
            }
        }

        void IntEdit::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }

        void IntEdit::_textUpdate()
        {
            TLRENDER_P();
            std::string text;
            std::string format;
            if (p.model)
            {
                text = string::Format("{0}").arg(p.model->getValue());
                format = string::Format("{0}").arg(0, p.digits);
            }
            p.lineEdit->setText(text);
            p.lineEdit->setFormat(format);
        }
    }
}
