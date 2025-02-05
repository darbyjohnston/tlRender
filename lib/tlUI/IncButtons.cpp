// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
            const std::shared_ptr<dtk::Context>& context,
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
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<IncButton>(new IncButton);
            out->_init(context, parent);
            return out;
        }

        void IncButton::sizeHintEvent(const SizeHintEvent& event)
        {
            IButton::sizeHintEvent(event);
            DTK_P();

            //p.size.margin = event.style->getSizeRole(SizeRole::MarginInside, _displayScale);

            _sizeHint = dtk::Size2I();
            if (_iconImage)
            {
                _sizeHint.w = _iconImage->getWidth();
                _sizeHint.h = _iconImage->getHeight();
            }
            _sizeHint.w += p.size.margin * 2;
            _sizeHint.h += p.size.margin * 2;
        }

        void IncButton::drawEvent(
            const dtk::Box2I& drawRect,
            const DrawEvent& event)
        {
            IButton::drawEvent(drawRect, event);
            DTK_P();

            const dtk::Box2I& g = _geometry;

            const ColorRole colorRole = _checked ?
                ColorRole::Checked :
                _buttonRole;
            if (colorRole != ColorRole::None)
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(colorRole));
            }

            if (_mouse.press && dtk::contains(_geometry, _mouse.pos))
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
                  dtk::Box2I(
                      x,
                      g.y() + g.h() / 2 - iconSize.h / 2,
                      iconSize.w,
                      iconSize.h));
            }
        }

        void IncButtons::_init(
            const std::shared_ptr<dtk::Context>& context,
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
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<IncButtons>(new IncButtons);
            out->_init(context, parent);
            return out;
        }

        void IncButtons::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _incButton->setGeometry(dtk::Box2I(
                value.min.x,
                value.min.y,
                value.w(),
                value.h() / 2));
            _decButton->setGeometry(dtk::Box2I(
                value.min.x,
                value.max.y - value.h() / 2,
                value.w(),
                value.h() / 2));
        }

        void IncButtons::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            const dtk::Size2I incSizeHint = _incButton->getSizeHint();
            const dtk::Size2I decSizeHint = _decButton->getSizeHint();
            _sizeHint.w = std::max(incSizeHint.w, decSizeHint.w);
            _sizeHint.h = incSizeHint.h + decSizeHint.h;
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
            std::shared_ptr<dtk::ValueObserver<int> > valueObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::RangeI> > rangeObserver;
        };

        void IntIncButtons::_init(
            const std::shared_ptr<IntModel>& model,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IncButtons::_init(context, parent);
            setObjectName("tl::ui::IntIncButtons");
            DTK_P();

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

            p.valueObserver = dtk::ValueObserver<int>::create(
                p.model->observeValue(),
                [this](int)
                {
                    _modelUpdate();
                });

            p.rangeObserver = dtk::ValueObserver<dtk::RangeI>::create(
                p.model->observeRange(),
                [this](const dtk::RangeI&)
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
            const std::shared_ptr<dtk::Context>& context,
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
            DTK_P();
            const int value = p.model->getValue();
            const dtk::RangeI& range = p.model->getRange();
            _incButton->setEnabled(value < range.max());
            _decButton->setEnabled(value > range.min());
        }

        struct FloatIncButtons::Private
        {
            std::shared_ptr<FloatModel> model;
            std::shared_ptr<dtk::ValueObserver<float> > valueObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::RangeF> > rangeObserver;
        };

        void FloatIncButtons::_init(
            const std::shared_ptr<FloatModel>& model,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IncButtons::_init(context, parent);
            setObjectName("tl::ui::FloatIncButtons");
            DTK_P();

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

            p.valueObserver = dtk::ValueObserver<float>::create(
                p.model->observeValue(),
                [this](float)
                {
                    _modelUpdate();
                });

            p.rangeObserver = dtk::ValueObserver<dtk::RangeF>::create(
                p.model->observeRange(),
                [this](const dtk::RangeF&)
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
            const std::shared_ptr<dtk::Context>& context,
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
            DTK_P();
            const float value = p.model->getValue();
            const dtk::RangeF& range = p.model->getRange();
            _incButton->setEnabled(value < range.max());
            _decButton->setEnabled(value > range.min());
        }

        struct DoubleIncButtons::Private
        {
            std::shared_ptr<DoubleModel> model;
            std::shared_ptr<dtk::ValueObserver<double> > valueObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::RangeD> > rangeObserver;
        };

        void DoubleIncButtons::_init(
            const std::shared_ptr<DoubleModel>& model,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IncButtons::_init(context, parent);
            setObjectName("tl::ui::DoubleIncButtons");
            DTK_P();

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

            p.valueObserver = dtk::ValueObserver<double>::create(
                p.model->observeValue(),
                [this](double)
                {
                    _modelUpdate();
                });

            p.rangeObserver = dtk::ValueObserver<dtk::RangeD>::create(
                p.model->observeRange(),
                [this](const dtk::RangeD&)
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
            const std::shared_ptr<dtk::Context>& context,
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
            DTK_P();
            const double value = p.model->getValue();
            const dtk::RangeD& range = p.model->getRange();
            _incButton->setEnabled(value < range.max());
            _decButton->setEnabled(value > range.min());
        }
    }
}
