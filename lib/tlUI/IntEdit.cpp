// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IntEdit.h>

#include <tlUI/IncButton.h>
#include <tlUI/LineEdit.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        struct IntEdit::Private
        {
            std::shared_ptr<IntModel> model;
            std::shared_ptr<LineEdit> lineEdit;
            std::shared_ptr<IncButton> incrementButton;
            std::shared_ptr<IncButton> decrementButton;
            int digits = 3;

            struct SizeData
            {
                int margin = 0;
            };
            SizeData size;

            std::shared_ptr<observer::ValueObserver<int> > valueObserver;
            std::shared_ptr<observer::ValueObserver<math::IntRange> > rangeObserver;
        };

        void IntEdit::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::IntEdit", context, parent);
            TLRENDER_P();

            p.lineEdit = LineEdit::create(context, shared_from_this());

            p.incrementButton = IncButton::create(context, shared_from_this());
            p.incrementButton->setIcon("Increment");
            p.decrementButton = IncButton::create(context, shared_from_this());
            p.decrementButton->setIcon("Decrement");

            setModel(IntModel::create(context));

            _valueUpdate();
            _textUpdate();

            p.incrementButton->setClickedCallback(
                [this]
                {
                    if (_p->model)
                    {
                        _p->model->incrementStep();
                    }
                });
            p.decrementButton->setClickedCallback(
                [this]
                {
                    if (_p->model)
                    {
                        _p->model->decrementStep();
                    }
                });
        }

        IntEdit::IntEdit() :
            _p(new Private)
        {}

        IntEdit::~IntEdit()
        {}

        std::shared_ptr<IntEdit> IntEdit::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<IntEdit>(new IntEdit);
            out->_init(context, parent);
            return out;
        }

        const std::shared_ptr<IntModel>& IntEdit::getModel() const
        {
            return _p->model;
        }

        void IntEdit::setModel(const std::shared_ptr<IntModel>& value)
        {
            TLRENDER_P();
            p.valueObserver.reset();
            p.rangeObserver.reset();
            p.model = value;
            if (p.model)
            {
                p.valueObserver = observer::ValueObserver<int>::create(
                    p.model->observeValue(),
                    [this](int)
                    {
                        _valueUpdate();
                        _textUpdate();
                    });
                p.rangeObserver = observer::ValueObserver<math::IntRange>::create(
                    p.model->observeRange(),
                    [this](const math::IntRange&)
                    {
                        _valueUpdate();
                        _textUpdate();
                    });
            }
            _valueUpdate();
            _textUpdate();
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

        void IntEdit::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            math::BBox2i g = value;
            const int buttonsWidth = std::max(
                p.incrementButton->getSizeHint().x,
                p.decrementButton->getSizeHint().x);
            g.max.x -= p.size.margin + buttonsWidth;
            p.lineEdit->setGeometry(g);
            g = value;
            g.min.x = g.max.x - buttonsWidth;
            g.max.y = g.min.y + g.h() / 2;
            p.incrementButton->setGeometry(g);
            g.min.y = g.max.y;
            g.max.y = value.max.y;
            p.decrementButton->setGeometry(g);
        }

        void IntEdit::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(SizeRole::MarginInside, event.displayScale);

            _sizeHint = p.lineEdit->getSizeHint();
            const int buttonsWidth = std::max(
                p.incrementButton->getSizeHint().x,
                p.decrementButton->getSizeHint().x);
            _sizeHint.x += p.size.margin + buttonsWidth;
        }

        void IntEdit::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
            if (_enabled && p.model)
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
                }
            }
        }

        void IntEdit::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }

        void IntEdit::_valueUpdate()
        {
            TLRENDER_P();
            bool incrementEnabled = false;
            bool decrementEnabled = false;
            if (p.model)
            {
                const int value = p.model->getValue();
                const math::IntRange& range = p.model->getRange();
                incrementEnabled = value < range.getMax();
                decrementEnabled = value > range.getMin();
            }
            p.incrementButton->setEnabled(incrementEnabled);
            p.decrementButton->setEnabled(decrementEnabled);
        }

        void IntEdit::_textUpdate()
        {
            TLRENDER_P();
            std::string text;
            std::string format;
            if (p.model)
            {
                text = string::Format("{0}").arg(p.model->getValue());
                const auto& range = p.model->getRange();
                format = string::Format("{0}{1}").
                    arg(range.getMin() < 0 ? "-" : "").
                    arg(0, p.digits);
            }
            p.lineEdit->setText(text);
            p.lineEdit->setFormat(format);
        }
    }
}
