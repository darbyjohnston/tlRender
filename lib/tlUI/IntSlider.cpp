// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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
                int border = 0;
                int handle = 0;
                imaging::FontMetrics fontMetrics;
            };
            SizeData size;

            struct MouseData
            {
                bool inside = false;
                math::Vector2i pos;
                bool pressed = false;
            };
            MouseData mouse;

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

            setMouseHover(true);
            setAcceptsKeyFocus(true);
            setHStretch(Stretch::Expanding);

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

        void IntSlider::setVisible(bool value)
        {
            const bool changed = value != _visible;
            IWidget::setVisible(value);
            if (changed && !_visible)
            {
                _resetMouse();
            }
        }

        void IntSlider::setEnabled(bool value)
        {
            const bool changed = value != _enabled;
            IWidget::setEnabled(value);
            if (changed && !_enabled)
            {
                _resetMouse();
            }
        }

        void IntSlider::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);
            p.size.handle = event.style->getSizeRole(SizeRole::Handle, event.displayScale);

            auto fontInfo = event.style->getFontRole(FontRole::Label, event.displayScale);
            p.size.fontMetrics = event.fontSystem->getMetrics(fontInfo);

            _sizeHint.x =
                event.style->getSizeRole(SizeRole::Slider, event.displayScale) +
                p.size.border * 6;
            _sizeHint.y =
                p.size.fontMetrics.lineHeight +
                p.size.border * 6;
        }

        void IntSlider::clipEvent(
            const math::BBox2i& clipRect,
            bool clipped,
            const ClipEvent& event)
        {
            const bool changed = clipped != _clipped;
            IWidget::clipEvent(clipRect, clipped, event);
            if (changed && clipped)
            {
                _resetMouse();
            }
        }

        void IntSlider::drawEvent(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::BBox2i& g = _geometry;

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

            const math::BBox2i g2 = _getSliderGeometry();
            //event.render->drawRect(
            //    g2,
            //    imaging::Color4f(1.F, 0.F, 0.F, .5F));
            int pos = 0;
            if (p.model)
            {
                pos = _valueToPos(p.model->getValue());
            }
            const math::BBox2i g3(
                pos - p.size.handle / 2,
                g2.y(),
                p.size.handle,
                g2.h());
            event.render->drawRect(
                g3,
                event.style->getColorRole(ColorRole::Button));
            if (p.mouse.pressed)
            {
                event.render->drawRect(
                    g3,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (p.mouse.inside)
            {
                event.render->drawRect(
                    g3,
                    event.style->getColorRole(ColorRole::Hover));
            }
        }

        void IntSlider::mouseEnterEvent()
        {
            TLRENDER_P();
            p.mouse.inside = true;
            _updates |= Update::Draw;
        }

        void IntSlider::mouseLeaveEvent()
        {
            TLRENDER_P();
            p.mouse.inside = false;
            _updates |= Update::Draw;
        }

        void IntSlider::mouseMoveEvent(MouseMoveEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pos = event.pos;
            if (p.mouse.pressed && p.model)
            {
                p.model->setValue(_posToValue(p.mouse.pos.x));
            }
        }

        void IntSlider::mousePressEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pressed = true;
            if (p.model)
            {
                p.model->setValue(_posToValue(p.mouse.pos.x));
            }
            takeKeyFocus();
            _updates |= Update::Draw;
        }

        void IntSlider::mouseReleaseEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pressed = false;
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

        math::BBox2i IntSlider::_getSliderGeometry() const
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
            const math::BBox2i g = _getSliderGeometry();
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
            const math::BBox2i g = _getSliderGeometry();
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

        void IntSlider::_resetMouse()
        {
            TLRENDER_P();
            if (p.mouse.pressed || p.mouse.inside)
            {
                p.mouse.pressed = false;
                p.mouse.inside = false;
                _updates |= Update::Draw;
            }
        }
    }
}
