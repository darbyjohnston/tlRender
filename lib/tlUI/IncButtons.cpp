// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IncButtons.h>

#include <tlUI/DoubleModel.h>
#include <tlUI/FloatModel.h>
#include <tlUI/IntModel.h>

namespace tl
{
    namespace ui
    {
        struct IncButton::Private
        {
            struct SizeData
            {
                int margin = 0;
            };
            SizeData size;
        };

        void IncButton::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IButton::_init("tl::ui::IncButton", context, parent);
            setButtonRole(ColorRole::None);
            setRepeatClick(true);
        }

        IncButton::IncButton() :
            _p(new Private)
        {}

        IncButton::~IncButton()
        {}

        std::shared_ptr<IncButton> IncButton::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<IncButton>(new IncButton);
            out->_init(context, parent);
            return out;
        }

        void IncButton::sizeHintEvent(const SizeHintEvent& event)
        {
            IButton::sizeHintEvent(event);
            TLRENDER_P();

            //p.size.margin = event.style->getSizeRole(SizeRole::MarginInside, event.displayScale);

            _sizeHint = math::Vector2i();
            if (_iconImage)
            {
                _sizeHint.x = _iconImage->getWidth();
                _sizeHint.y = _iconImage->getHeight();
            }
            _sizeHint.x += p.size.margin * 2;
            _sizeHint.y += p.size.margin * 2;
        }

        void IncButton::drawEvent(
            const math::Box2i& drawRect,
            const DrawEvent& event)
        {
            IButton::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::Box2i& g = _geometry;

            const ColorRole colorRole = _checked ?
                ColorRole::Checked :
                _buttonRole;
            if (colorRole != ColorRole::None)
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(colorRole));
            }

            if (_mouse.press && _geometry.contains(_mouse.pos))
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (_mouse.inside)
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ColorRole::Hover));
            }

            int x = g.x() + p.size.margin;
            if (_iconImage)
            {
                const auto iconSize = _iconImage->getSize();
                event.render->drawImage(
                  _iconImage,
                  math::Box2i(
                      x,
                      g.y() + g.h() / 2 - iconSize.h / 2,
                      iconSize.w,
                      iconSize.h));
            }
        }

        void IncButtons::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::IncButtons", context, parent);
            _incButton = IncButton::create(context, shared_from_this());
            _incButton->setIcon("Increment");
            _incButton->setVAlign(VAlign::Top);
            _decButton = IncButton::create(context, shared_from_this());
            _decButton->setIcon("Decrement");
            _decButton->setVAlign(VAlign::Bottom);
        }

        IncButtons::IncButtons()
        {}

        IncButtons::~IncButtons()
        {}

        std::shared_ptr<IncButtons> IncButtons::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<IncButtons>(new IncButtons);
            out->_init(context, parent);
            return out;
        }

        void IncButtons::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _incButton->setGeometry(math::Box2i(
                value.min.x,
                value.min.y,
                value.w(),
                value.h() / 2));
            _decButton->setGeometry(math::Box2i(
                value.min.x,
                value.max.y - value.h() / 2,
                value.w(),
                value.h() / 2));
        }

        void IncButtons::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            const math::Vector2i incSizeHint = _incButton->getSizeHint();
            const math::Vector2i decSizeHint = _decButton->getSizeHint();
            _sizeHint.x = std::max(incSizeHint.x, decSizeHint.x);
            _sizeHint.y = incSizeHint.y + decSizeHint.y;
        }

        void IncButtons::setIncCallback(const std::function<void(void)>& value)
        {
            _incButton->setClickedCallback(value);
        }

        void IncButtons::setDecCallback(const std::function<void(void)>& value)
        {
            _decButton->setClickedCallback(value);
        }

        struct IntIncButtons::Private
        {
            std::shared_ptr<IntModel> model;
            std::shared_ptr<observer::ValueObserver<int> > valueObserver;
            std::shared_ptr<observer::ValueObserver<math::IntRange> > rangeObserver;
        };

        void IntIncButtons::_init(
            const std::shared_ptr<IntModel>& model,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IncButtons::_init(context, parent);
            setName("tl::ui::IntIncButtons");
            TLRENDER_P();

            p.model = model;

            _modelUpdate();

            _incButton->setClickedCallback(
                [this]
                {
                    _p->model->incrementStep();
                });

            _decButton->setClickedCallback(
                [this]
                {
                    _p->model->decrementStep();
                });

            p.valueObserver = observer::ValueObserver<int>::create(
                p.model->observeValue(),
                [this](int)
                {
                    _modelUpdate();
                });

            p.rangeObserver = observer::ValueObserver<math::IntRange>::create(
                p.model->observeRange(),
                [this](const math::IntRange&)
                {
                    _modelUpdate();
                });
        }

        IntIncButtons::IntIncButtons() :
            _p(new Private)
        {}

        IntIncButtons::~IntIncButtons()
        {}

        std::shared_ptr<IntIncButtons> IntIncButtons::create(
            const std::shared_ptr<IntModel>& model,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<IntIncButtons>(new IntIncButtons);
            out->_init(model, context, parent);
            return out;
        }

        const std::shared_ptr<IntModel>& IntIncButtons::getModel() const
        {
            return _p->model;
        }

        void IntIncButtons::_modelUpdate()
        {
            TLRENDER_P();
            const int value = p.model->getValue();
            const math::IntRange& range = p.model->getRange();
            _incButton->setEnabled(value < range.getMax());
            _decButton->setEnabled(value > range.getMin());
        }

        struct FloatIncButtons::Private
        {
            std::shared_ptr<FloatModel> model;
            std::shared_ptr<observer::ValueObserver<float> > valueObserver;
            std::shared_ptr<observer::ValueObserver<math::FloatRange> > rangeObserver;
        };

        void FloatIncButtons::_init(
            const std::shared_ptr<FloatModel>& model,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IncButtons::_init(context, parent);
            setName("tl::ui::FloatIncButtons");
            TLRENDER_P();

            p.model = model;

            _modelUpdate();

            _incButton->setClickedCallback(
                [this]
                {
                    _p->model->incrementStep();
                });

            _decButton->setClickedCallback(
                [this]
                {
                    _p->model->decrementStep();
                });

            p.valueObserver = observer::ValueObserver<float>::create(
                p.model->observeValue(),
                [this](float)
                {
                    _modelUpdate();
                });

            p.rangeObserver = observer::ValueObserver<math::FloatRange>::create(
                p.model->observeRange(),
                [this](const math::FloatRange&)
                {
                    _modelUpdate();
                });
        }

        FloatIncButtons::FloatIncButtons() :
            _p(new Private)
        {}

        FloatIncButtons::~FloatIncButtons()
        {}

        std::shared_ptr<FloatIncButtons> FloatIncButtons::create(
            const std::shared_ptr<FloatModel>& model,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FloatIncButtons>(new FloatIncButtons);
            out->_init(model, context, parent);
            return out;
        }

        const std::shared_ptr<FloatModel>& FloatIncButtons::getModel() const
        {
            return _p->model;
        }

        void FloatIncButtons::_modelUpdate()
        {
            TLRENDER_P();
            const float value = p.model->getValue();
            const math::FloatRange& range = p.model->getRange();
            _incButton->setEnabled(value < range.getMax());
            _decButton->setEnabled(value > range.getMin());
        }

        struct DoubleIncButtons::Private
        {
            std::shared_ptr<DoubleModel> model;
            std::shared_ptr<observer::ValueObserver<double> > valueObserver;
            std::shared_ptr<observer::ValueObserver<math::DoubleRange> > rangeObserver;
        };

        void DoubleIncButtons::_init(
            const std::shared_ptr<DoubleModel>& model,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IncButtons::_init(context, parent);
            setName("tl::ui::DoubleIncButtons");
            TLRENDER_P();

            p.model = model;

            _modelUpdate();

            _incButton->setClickedCallback(
                [this]
                {
                    _p->model->incrementStep();
                });

            _decButton->setClickedCallback(
                [this]
                {
                    _p->model->decrementStep();
                });

            p.valueObserver = observer::ValueObserver<double>::create(
                p.model->observeValue(),
                [this](double)
                {
                    _modelUpdate();
                });

            p.rangeObserver = observer::ValueObserver<math::DoubleRange>::create(
                p.model->observeRange(),
                [this](const math::DoubleRange&)
                {
                    _modelUpdate();
                });
        }

        DoubleIncButtons::DoubleIncButtons() :
            _p(new Private)
        {}

        DoubleIncButtons::~DoubleIncButtons()
        {}

        std::shared_ptr<DoubleIncButtons> DoubleIncButtons::create(
            const std::shared_ptr<DoubleModel>& model,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<DoubleIncButtons>(new DoubleIncButtons);
            out->_init(model, context, parent);
            return out;
        }

        const std::shared_ptr<DoubleModel>& DoubleIncButtons::getModel() const
        {
            return _p->model;
        }

        void DoubleIncButtons::_modelUpdate()
        {
            TLRENDER_P();
            const double value = p.model->getValue();
            const math::DoubleRange& range = p.model->getRange();
            _incButton->setEnabled(value < range.getMax());
            _decButton->setEnabled(value > range.getMin());
        }
    }
}
