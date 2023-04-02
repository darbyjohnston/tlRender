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
            int margin = 0;
            int border = 0;
            int handle = 0;
            bool inside = false;
            math::Vector2i mousePos;
            bool pressed = false;
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

        void FloatSlider::sizeEvent(const SizeEvent& event)
        {
            IWidget::sizeEvent(event);
            TLRENDER_P();

            p.margin = event.style->getSizeRole(SizeRole::MarginInside) * event.contentScale;
            p.border = event.style->getSizeRole(SizeRole::Border) * event.contentScale;
            p.handle = event.style->getSizeRole(SizeRole::Handle) * event.contentScale;

            imaging::FontInfo fontInfo;
            fontInfo.size *= event.contentScale;
            auto fontMetrics = event.fontSystem->getMetrics(fontInfo);

            _sizeHint.x = event.style->getSizeRole(SizeRole::ScrollArea) + p.margin * 2;
            _sizeHint.y = fontMetrics.lineHeight + p.margin * 2;
        }

        void FloatSlider::drawEvent(const DrawEvent& event)
        {
            IWidget::drawEvent(event);
            TLRENDER_P();

            math::BBox2i g = _geometry;

            event.render->drawMesh(
                border(g, p.border),
                event.style->getColorRole(ColorRole::Border));

            event.render->drawRect(
                g.margin(-p.border),
                event.style->getColorRole(ColorRole::Base));

            math::BBox2i g2 = _getSliderGeometry();
            //event.render->drawRect(
            //    g2,
            //    imaging::Color4f(1.F, 0.F, 0.F, .5F));
            int pos = 0;
            if (p.model)
            {
                pos = _valueToPos(p.model->getValue());
            }
            math::BBox2i g3(
                pos - p.handle / 2,
                g2.y(),
                p.handle,
                g2.h());
            event.render->drawRect(
                g3,
                event.style->getColorRole(ColorRole::Button));
            if (p.pressed)
            {
                event.render->drawRect(
                    g3,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (p.inside)
            {
                event.render->drawRect(
                    g3,
                    event.style->getColorRole(ColorRole::Hover));
            }
        }

        void FloatSlider::enterEvent()
        {
            TLRENDER_P();
            p.inside = true;
            _updates |= Update::Draw;
        }

        void FloatSlider::leaveEvent()
        {
            TLRENDER_P();
            p.inside = false;
            _updates |= Update::Draw;
        }

        void FloatSlider::mouseMoveEvent(MouseMoveEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mousePos = event.pos;
            if (p.pressed && p.model)
            {
                p.model->setValue(_posToValue(p.mousePos.x));
            }
        }

        void FloatSlider::mousePressEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.pressed = true;
            if (p.model)
            {
                p.model->setValue(_posToValue(p.mousePos.x));
            }
            _updates |= Update::Draw;
        }

        void FloatSlider::mouseReleaseEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.pressed = false;
            _updates |= Update::Draw;
        }

        math::BBox2i FloatSlider::_getSliderGeometry() const
        {
            TLRENDER_P();
            return _geometry.margin(
                -(p.margin + p.handle / 2),
                -p.margin,
                -(p.margin + p.handle / 2),
                -p.margin);
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
