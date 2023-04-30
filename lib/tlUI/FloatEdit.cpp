// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/FloatEdit.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        struct FloatEdit::Private
        {
            std::shared_ptr<FloatModel> model;
            int digits = 3;
            int precision = 2;

            std::shared_ptr<observer::ValueObserver<float> > valueObserver;
            std::shared_ptr<observer::ValueObserver<math::FloatRange> > rangeObserver;
        };

        void FloatEdit::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            LineEdit::_init(context, parent);
            _name = "tl::ui::FloatEdit";
            TLRENDER_P();

            setModel(FloatModel::create(context));

            _floatUpdate();
        }

        FloatEdit::FloatEdit() :
            _p(new Private)
        {}

        FloatEdit::~FloatEdit()
        {}

        std::shared_ptr<FloatEdit> FloatEdit::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FloatEdit>(new FloatEdit);
            out->_init(context, parent);
            return out;
        }

        const std::shared_ptr<FloatModel>& FloatEdit::getModel() const
        {
            return _p->model;
        }

        void FloatEdit::setModel(const std::shared_ptr<FloatModel>& value)
        {
            TLRENDER_P();
            p.valueObserver.reset();
            p.rangeObserver.reset();
            p.model = value;
            if (p.model)
            {
                p.valueObserver = observer::ValueObserver<float>::create(
                    p.model->observeValue(),
                    [this](float)
                    {
                        _floatUpdate();
                    });
                p.rangeObserver = observer::ValueObserver<math::FloatRange>::create(
                    p.model->observeRange(),
                    [this](const math::FloatRange&)
                    {
                        _floatUpdate();
                    });
            }
            _floatUpdate();
        }

        void FloatEdit::setDigits(int value)
        {
            TLRENDER_P();
            if (value == p.digits)
                return;
            p.digits = value;
            _floatUpdate();
        }

        void FloatEdit::setPrecision(int value)
        {
            TLRENDER_P();
            if (value == p.precision)
                return;
            p.precision = value;
            _floatUpdate();
        }

        void FloatEdit::keyPressEvent(KeyEvent& event)
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

        void FloatEdit::keyReleaseEvent(KeyEvent& event)
        {
            LineEdit::keyPressEvent(event);
            event.accept = true;
        }

        void FloatEdit::_floatUpdate()
        {
            TLRENDER_P();
            std::string text;
            std::string format;
            if (p.model)
            {
                text = string::Format("{0}").arg(p.model->getValue(), p.precision);
                const auto& range = p.model->getRange();
                format = string::Format("{0}{1}").
                    arg(range.getMin() < 0 ? "-" : "").
                    arg(0.F, p.precision, p.precision + 1 + p.digits);
            }
            setText(text);
            setFormat(format);
        }
    }
}
