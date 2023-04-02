// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/FloatEdit.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/GeometryUtil.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        struct FloatEdit::Private
        {
            std::shared_ptr<FloatModel> model;
            std::string text;
            std::string format;
            int digits = 3;
            int precision = 2;
            imaging::FontInfo fontInfo;

            struct SizeCache
            {
                imaging::FontInfo fontInfo;
                imaging::FontMetrics fontMetrics;
                math::Vector2i textSize;
                math::Vector2i formatSize;
                int margin = 0;
                int border = 0;
            };
            SizeCache sizeCache;

            std::shared_ptr<observer::ValueObserver<float> > valueObserver;
            std::shared_ptr<observer::ValueObserver<math::FloatRange> > rangeObserver;
        };

        void FloatEdit::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::FloatEdit", context, parent);
            TLRENDER_P();
            _hAlign = HAlign::Right;
            setModel(FloatModel::create(context));
            p.fontInfo.family = "NotoMono-Regular";
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
                        _textUpdate();
                    });
                p.rangeObserver = observer::ValueObserver<math::FloatRange>::create(
                    p.model->observeRange(),
                    [this](const math::FloatRange&)
                    {
                        _textUpdate();
                    });
            }
            _textUpdate();
        }

        void FloatEdit::setDigits(int value)
        {
            TLRENDER_P();
            if (value == p.digits)
                return;
            p.digits = value;
            _textUpdate();
        }

        void FloatEdit::setPrecision(int value)
        {
            TLRENDER_P();
            if (value == p.precision)
                return;
            p.precision = value;
            _textUpdate();
        }

        void FloatEdit::setFontInfo(const imaging::FontInfo& value)
        {
            TLRENDER_P();
            if (value == p.fontInfo)
                return;
            p.fontInfo = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void FloatEdit::sizeEvent(const SizeEvent& event)
        {
            IWidget::sizeEvent(event);
            TLRENDER_P();

            p.sizeCache.margin = event.style->getSizeRole(SizeRole::MarginInside) * event.contentScale;
            p.sizeCache.border = event.style->getSizeRole(SizeRole::Border) * event.contentScale;

            p.sizeCache.fontInfo = p.fontInfo;
            p.sizeCache.fontInfo.size *= event.contentScale;
            p.sizeCache.fontMetrics = event.fontSystem->getMetrics(p.sizeCache.fontInfo);
            p.sizeCache.textSize = event.fontSystem->measure(p.text, p.sizeCache.fontInfo);
            p.sizeCache.formatSize = event.fontSystem->measure(p.format, p.sizeCache.fontInfo);

            _sizeHint.x = p.sizeCache.formatSize.x + p.sizeCache.margin * 2;
            _sizeHint.y = p.sizeCache.fontMetrics.lineHeight + p.sizeCache.margin * 2;
        }

        void FloatEdit::drawEvent(const DrawEvent& event)
        {
            IWidget::drawEvent(event);
            TLRENDER_P();

            math::BBox2i g = align(
                _geometry,
                _sizeHint,
                Stretch::Expanding,
                Stretch::Expanding,
                _hAlign,
                _vAlign);

            event.render->drawMesh(
                border(g, p.sizeCache.border),
                event.style->getColorRole(ColorRole::Border));

            event.render->drawRect(
                g.margin(-p.sizeCache.border),
                event.style->getColorRole(ColorRole::Base));

            math::BBox2i g2 = g.margin(-p.sizeCache.margin);
            math::Vector2i pos(
                g2.x() + g2.w() - p.sizeCache.textSize.x,
                g2.y() + g2.h() / 2 - p.sizeCache.fontMetrics.lineHeight / 2 +
                p.sizeCache.fontMetrics.ascender);
            event.render->drawText(
                event.fontSystem->getGlyphs(p.text, p.sizeCache.fontInfo),
                pos,
                event.style->getColorRole(ColorRole::Text));
        }

        void FloatEdit::_textUpdate()
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
            p.text = text;
            p.format = format;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }
    }
}
