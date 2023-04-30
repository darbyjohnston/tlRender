// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/FloatSlider.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct FloatSlider::Private
        {
            std::shared_ptr<FloatModel> model;

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

            std::shared_ptr<observer::ValueObserver<float> > valueObserver;
            std::shared_ptr<observer::ValueObserver<math::FloatRange> > rangeObserver;
        };

        void FloatSlider::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::FloatSlider", context, parent);
            TLRENDER_P();
            setModel(FloatModel::create(context));
        }

        FloatSlider::FloatSlider() :
            _p(new Private)
        {}

        FloatSlider::~FloatSlider()
        {}

        std::shared_ptr<FloatSlider> FloatSlider::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FloatSlider>(new FloatSlider);
            out->_init(context, parent);
            return out;
        }

        const std::shared_ptr<FloatModel>& FloatSlider::getModel() const
        {
            return _p->model;
        }

        void FloatSlider::setModel(const std::shared_ptr<FloatModel>& value)
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
                        _updates |= Update::Size;
                        _updates |= Update::Draw;
                    });
                p.rangeObserver = observer::ValueObserver<math::FloatRange>::create(
                    p.model->observeRange(),
                    [this](const math::FloatRange&)
                    {
                        _updates |= Update::Size;
                        _updates |= Update::Draw;
                    });
            }
        }

        bool FloatSlider::acceptsKeyFocus() const
        {
            return true;
        }

        void FloatSlider::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);
            p.size.handle = event.style->getSizeRole(SizeRole::Handle, event.displayScale);

            auto fontInfo = imaging::FontInfo();
            fontInfo.size *= event.displayScale;
            p.size.fontMetrics = event.fontSystem->getMetrics(fontInfo);

            _sizeHint.x =
                event.style->getSizeRole(SizeRole::ScrollArea, event.displayScale) +
                p.size.border * 4;
            _sizeHint.y =
                p.size.fontMetrics.lineHeight +
                p.size.border * 4;
        }

        void FloatSlider::drawEvent(const DrawEvent& event)
        {
            IWidget::drawEvent(event);
            TLRENDER_P();

            const math::BBox2i g = _geometry;

            event.render->drawMesh(
                border(g, p.size.border),
                math::Vector2i(),
                event.style->getColorRole(event.focusWidget == shared_from_this() ?
                    ColorRole::Checked :
                    ColorRole::Border));

            event.render->drawRect(
                g.margin(-p.size.border),
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

        void FloatSlider::enterEvent()
        {
            TLRENDER_P();
            p.mouse.inside = true;
            _updates |= Update::Draw;
        }

        void FloatSlider::leaveEvent()
        {
            TLRENDER_P();
            p.mouse.inside = false;
            _updates |= Update::Draw;
        }

        void FloatSlider::mouseMoveEvent(MouseMoveEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pos = event.pos;
            if (p.mouse.pressed && p.model)
            {
                p.model->setValue(_posToValue(p.mouse.pos.x));
            }
        }

        void FloatSlider::mousePressEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pressed = true;
            if (p.model)
            {
                p.model->setValue(_posToValue(p.mouse.pos.x));
            }
            takeFocus();
            _updates |= Update::Draw;
        }

        void FloatSlider::mouseReleaseEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pressed = false;
            _updates |= Update::Draw;
        }

        void FloatSlider::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
            if (p.model)
            {
                switch (event.key)
                {
                case Key::Left:
                case Key::Down:
                    event.accept = true;
                    p.model->subtractStep();
                    break;
                case Key::Right:
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
                case Key::End:
                    event.accept = true;
                    p.model->setValue(p.model->getRange().getMin());
                    break;
                case Key::Home:
                    event.accept = true;
                    p.model->setValue(p.model->getRange().getMax());
                    break;
                }
            }
        }

        void FloatSlider::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }

        math::BBox2i FloatSlider::_getSliderGeometry() const
        {
            TLRENDER_P();
            return _geometry.margin(
                -(p.size.border * 2 + p.size.handle / 2),
                -p.size.border * 2,
                -(p.size.border * 2 + p.size.handle / 2),
                -p.size.border * 2);
        }

        float FloatSlider::_posToValue(int pos) const
        {
            TLRENDER_P();
            const math::BBox2i g = _getSliderGeometry();
            const float v = (pos - g.x()) / static_cast<float>(g.w());
            float out = 0.F;
            if (p.model)
            {
                const math::FloatRange& range = p.model->getRange();
                out = range.getMin() + (range.getMax() - range.getMin()) * v;
            }
            return out;
        }

        int FloatSlider::_valueToPos(float value) const
        {
            TLRENDER_P();
            const math::BBox2i g = _getSliderGeometry();
            float v = 0.F;
            if (p.model)
            {
                const math::FloatRange& range = p.model->getRange();
                if (range.getMin() != range.getMax())
                {
                    v = (value - range.getMin()) /
                        static_cast<float>(range.getMax() - range.getMin());
                }
            }
            return g.x() + g.w() * v;
        }
    }
}
