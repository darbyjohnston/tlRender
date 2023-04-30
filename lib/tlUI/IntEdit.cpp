// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IntEdit.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        struct IntEdit::Private
        {
            std::shared_ptr<IntModel> model;
            int digits = 3;

            std::shared_ptr<observer::ValueObserver<int> > valueObserver;
            std::shared_ptr<observer::ValueObserver<math::IntRange> > rangeObserver;
        };

        void IntEdit::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            LineEdit::_init(context, parent);
            _name = "tl::ui::IntEdit";
            TLRENDER_P();

            setModel(IntModel::create(context));

            _intUpdate();
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
                        _intUpdate();
                    });
                p.rangeObserver = observer::ValueObserver<math::IntRange>::create(
                    p.model->observeRange(),
                    [this](const math::IntRange&)
                    {
                        _intUpdate();
                    });
            }
            _intUpdate();
        }

        void IntEdit::setDigits(int value)
        {
            TLRENDER_P();
            if (value == p.digits)
                return;
            p.digits = value;
            _intUpdate();
        }

        void IntEdit::keyPressEvent(KeyEvent& event)
        {
            LineEdit::keyPressEvent(event);
            TLRENDER_P();
            if (!event.accept)
            {
                switch (event.key)
                {
                case Key::Down:
                    event.accept = true;
                    p.model->subtractStep();
                    break;
                case Key::Up:
                    event.accept = true;
                    p.model->addStep();
                    break;
                case Key::PageUp:
                    event.accept = true;
                    p.model->addLargeStep();
                    break;
                case Key::PageDown:
                    event.accept = true;
                    p.model->subtractLargeStep();
                    break;
                }
            }
        }

        void IntEdit::keyReleaseEvent(KeyEvent& event)
        {
            LineEdit::keyPressEvent(event);
            event.accept = true;
        }

        void IntEdit::_intUpdate()
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
            setText(text);
            setFormat(format);
        }
    }
}
