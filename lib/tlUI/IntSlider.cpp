// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/IntSlider.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct IntSlider::Private
        {
            std::shared_ptr<IntModel> model;

            struct SizeData
            {
                bool sizeInit = true;
                int size = 0;
                int border = 0;
                int handle = 0;
                image::FontMetrics fontMetrics;
            };
            SizeData size;

            std::function<void(int)> callback;

            std::shared_ptr<observer::ValueObserver<int> > valueObserver;
            std::shared_ptr<observer::ValueObserver<math::IntRange> > rangeObserver;
        };

        void IntSlider::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IntModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::IntSlider", context, parent);
            TLRENDER_P();

            setAcceptsKeyFocus(true);
            setHStretch(Stretch::Expanding);
            _setMouseHover(true);
            _setMousePress(true);

            p.model = model;
            if (!p.model)
            {
                p.model = IntModel::create(context);
            }

            p.valueObserver = observer::ValueObserver<int>::create(
                p.model->observeValue(),
                [this](int value)
                {
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                    if (_p->callback)
                    {
                        _p->callback(value);
                    }
                });

            p.rangeObserver = observer::ValueObserver<math::IntRange>::create(
                p.model->observeRange(),
                [this](const math::IntRange&)
                {
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                });
        }

        IntSlider::IntSlider() :
            _p(new Private)
        {}

        IntSlider::~IntSlider()
        {}

        std::shared_ptr<IntSlider> IntSlider::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IntModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<IntSlider>(new IntSlider);
            out->_init(context, model, parent);
            return out;
        }

        int IntSlider::getValue() const
        {
            return _p->model->getValue();
        }

        void IntSlider::setValue(int value)
        {
            _p->model->setValue(value);
        }

        void IntSlider::setCallback(const std::function<void(int)>& value)
        {
            _p->callback = value;
        }

        const math::IntRange& IntSlider::getRange() const
        {
            return _p->model->getRange();
        }

        void IntSlider::setRange(const math::IntRange& value)
        {
            _p->model->setRange(value);
        }

        void IntSlider::setStep(int value)
        {
            _p->model->setStep(value);
        }

        void IntSlider::setLargeStep(int value)
        {
            _p->model->setLargeStep(value);
        }

        void IntSlider::setDefaultValue(int value)
        {
            _p->model->setDefaultValue(value);
        }

        const std::shared_ptr<IntModel>& IntSlider::getModel() const
        {
            return _p->model;
        }

        void IntSlider::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.size = event.style->getSizeRole(SizeRole::Slider, _displayScale);
                p.size.border = event.style->getSizeRole(SizeRole::Border, _displayScale);
                p.size.handle = event.style->getSizeRole(SizeRole::Handle, _displayScale);
                auto fontInfo = event.style->getFontRole(FontRole::Label, _displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(fontInfo);
            }
            p.size.sizeInit = false;

            _sizeHint.w =
                p.size.size +
                p.size.border * 6;
            _sizeHint.h =
                p.size.fontMetrics.lineHeight +
                p.size.border * 6;
        }

        void IntSlider::drawEvent(
            const math::Box2i& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::Box2i& g = _geometry;

            if (_keyFocus)
            {
                event.render->drawMesh(
                    border(g, p.size.border * 2),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::KeyFocus));
            }
            else
            {
                event.render->drawMesh(
                    border(g.margin(-p.size.border), p.size.border),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::Border));
            }

            event.render->drawRect(
                g.margin(-p.size.border * 2),
                event.style->getColorRole(ColorRole::Base));

            const math::Box2i g2 = _getSliderGeometry();
            //event.render->drawRect(
            //    g2,
            //    image::Color4f(1.F, 0.F, 0.F, .5F));
            int pos = 0;
            if (p.model)
            {
                pos = _valueToPos(p.model->getValue());
            }
            const math::Box2i g3(
                pos - p.size.handle / 2,
                g2.y(),
                p.size.handle,
                g2.h());
            event.render->drawRect(
                g3,
                event.style->getColorRole(ColorRole::Button));
            if (_mouse.press)
            {
                event.render->drawRect(
                    g3,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (_mouse.inside)
            {
                event.render->drawRect(
                    g3,
                    event.style->getColorRole(ColorRole::Hover));
            }
        }

        void IntSlider::mouseEnterEvent()
        {
            IWidget::mouseEnterEvent();
            _updates |= Update::Draw;
        }

        void IntSlider::mouseLeaveEvent()
        {
            IWidget::mouseLeaveEvent();
            _updates |= Update::Draw;
        }

        void IntSlider::mouseMoveEvent(MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            TLRENDER_P();
            if (_mouse.press && p.model)
            {
                p.model->setValue(_posToValue(_mouse.pos.x));
            }
        }

        void IntSlider::mousePressEvent(MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            TLRENDER_P();
            if (p.model)
            {
                p.model->setValue(_posToValue(_mouse.pos.x));
            }
            takeKeyFocus();
            _updates |= Update::Draw;
        }

        void IntSlider::mouseReleaseEvent(MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            _updates |= Update::Draw;
        }

        void IntSlider::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
            if (isEnabled() && p.model)
            {
                switch (event.key)
                {
                case Key::Left:
                case Key::Down:
                    event.accept = true;
                    p.model->decrementStep();
                    break;
                case Key::Right:
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
                case Key::End:
                    event.accept = true;
                    p.model->setValue(p.model->getRange().getMin());
                    break;
                case Key::Home:
                    event.accept = true;
                    p.model->setValue(p.model->getRange().getMax());
                    break;
                case Key::Escape:
                    if (hasKeyFocus())
                    {
                        event.accept = true;
                        releaseKeyFocus();
                    }
                    break;
                default: break;
                }
            }
        }

        void IntSlider::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }

        math::Box2i IntSlider::_getSliderGeometry() const
        {
            TLRENDER_P();
            return _geometry.margin(
                -(p.size.border * 3 + p.size.handle / 2),
                -(p.size.border * 3),
                -(p.size.border * 3 + p.size.handle / 2),
                -(p.size.border * 3));
        }

        int IntSlider::_posToValue(int pos) const
        {
            TLRENDER_P();
            const math::Box2i g = _getSliderGeometry();
            const math::IntRange range = p.model ?
                p.model->getRange() :
                math::IntRange();
            const float inc = g.w() /
                static_cast<float>(range.getMax() - range.getMin());
            const float v = (pos + inc / 2 - g.x()) /
                static_cast<float>(g.w());
            const int out = range.getMin() +
                (range.getMax() - range.getMin()) * v;
            return out;
        }

        int IntSlider::_valueToPos(int value) const
        {
            TLRENDER_P();
            const math::Box2i g = _getSliderGeometry();
            const math::IntRange range = p.model ?
                p.model->getRange() :
                math::IntRange();
            float v = 0.F;
            if (range.getMin() != range.getMax())
            {
                v = (value - range.getMin()) /
                    static_cast<float>(range.getMax() - range.getMin());
            }
            const int out = g.x() + g.w() * v;
            return out;
        }
    }
}
