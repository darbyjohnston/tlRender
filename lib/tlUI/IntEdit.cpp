// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IntEdit.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/GeometryUtil.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        struct IntEdit::Private
        {
            std::shared_ptr<IntModel> model;
            std::string text;
            std::string format;
            int digits = 3;
            imaging::FontInfo fontInfo;

            struct Size
            {
                imaging::FontInfo fontInfo;
                imaging::FontMetrics fontMetrics;
                math::Vector2i textSize;
                math::Vector2i formatSize;
                int margin = 0;
                int border = 0;
            };
            Size size;

            std::shared_ptr<observer::ValueObserver<int> > valueObserver;
            std::shared_ptr<observer::ValueObserver<math::IntRange> > rangeObserver;
        };

        void IntEdit::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::IntEdit", context, parent);
            TLRENDER_P();
            _hAlign = HAlign::Right;
            setModel(IntModel::create(context));
            p.fontInfo.family = "NotoMono-Regular";
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
                        _textUpdate();
                    });
                p.rangeObserver = observer::ValueObserver<math::IntRange>::create(
                    p.model->observeRange(),
                    [this](const math::IntRange&)
                    {
                        _textUpdate();
                    });
            }
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

        void IntEdit::setFontInfo(const imaging::FontInfo& value)
        {
            TLRENDER_P();
            if (value == p.fontInfo)
                return;
            p.fontInfo = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IntEdit::sizeEvent(const SizeEvent& event)
        {
            IWidget::sizeEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(SizeRole::MarginInside) * event.contentScale;
            p.size.border = event.style->getSizeRole(SizeRole::Border) * event.contentScale;

            p.size.fontInfo = p.fontInfo;
            p.size.fontInfo.size *= event.contentScale;
            p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
            p.size.textSize = event.fontSystem->measure(p.text, p.size.fontInfo);
            p.size.formatSize = event.fontSystem->measure(p.format, p.size.fontInfo);

            _sizeHint.x = p.size.formatSize.x + p.size.margin * 2;
            _sizeHint.y = p.size.fontMetrics.lineHeight + p.size.margin * 2;
        }

        void IntEdit::drawEvent(const DrawEvent& event)
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
                border(g, p.size.border),
                event.style->getColorRole(ColorRole::Border));

            event.render->drawRect(
                g.margin(-p.size.border),
                event.style->getColorRole(ColorRole::Base));
            
            math::BBox2i g2 = g.margin(-p.size.margin);
            math::Vector2i pos(
                g2.x() + g2.w() - p.size.textSize.x,
                g2.y() + g2.h() / 2 - p.size.fontMetrics.lineHeight / 2 +
                p.size.fontMetrics.ascender);
            event.render->drawText(
                event.fontSystem->getGlyphs(p.text, p.size.fontInfo),
                pos,
                event.style->getColorRole(ColorRole::Text));
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
            p.text = text;
            p.format = format;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }
    }
}
