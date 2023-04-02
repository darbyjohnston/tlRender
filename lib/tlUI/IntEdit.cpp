// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IntEdit.h>

#include <tlUI/DrawUtil.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        struct IntEdit::Private
        {
            std::shared_ptr<IntModel> model;
            std::string text;
            int digits = 3;
            imaging::FontInfo fontInfo;
            math::Vector2i textSize;
            int lineHeight = 0;
            int ascender = 0;
            int margin = 0;
            int border = 0;
            std::shared_ptr<observer::ValueObserver<int> > valueObserver;
        };

        void IntEdit::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::IntEdit", context, parent);
            TLRENDER_P();
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
            p.model = value;
            if (p.model)
            {
                p.valueObserver = observer::ValueObserver<int>::create(
                    p.model->observeValue(),
                    [this](int)
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
            _updates |= Update::Size;
            _updates |= Update::Draw;
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

            p.margin = event.style->getSizeRole(SizeRole::MarginInside) * event.contentScale;
            p.border = event.style->getSizeRole(SizeRole::Border) * event.contentScale;

            auto fontInfo = p.fontInfo;
            fontInfo.size *= event.contentScale;
            p.textSize = event.fontSystem->measure(p.text, fontInfo);
            auto fontMetrics = event.fontSystem->getMetrics(fontInfo);
            p.lineHeight = fontMetrics.lineHeight;
            p.ascender = fontMetrics.ascender;
            std::string digits;
            for (int i = 0; i < p.digits; ++i)
            {
                digits.append("0");
            }
            math::Vector2i digitsSize = event.fontSystem->measure(digits, fontInfo);

            _sizeHint.x = digitsSize.x + p.margin * 2;
            _sizeHint.y = p.lineHeight + p.margin * 2;
        }

        void IntEdit::drawEvent(const DrawEvent& event)
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

            math::BBox2i g2 = g.margin(-p.margin);
            auto fontInfo = p.fontInfo;
            fontInfo.size *= event.contentScale;
            event.render->drawText(
                event.fontSystem->getGlyphs(p.text, fontInfo),
                math::Vector2i(g2.x() + g2.w() - p.textSize.x, g2.y() + p.ascender),
                event.style->getColorRole(ColorRole::Text));
        }

        void IntEdit::_textUpdate()
        {
            TLRENDER_P();
            std::string text;
            if (p.model)
            {
                text = string::Format("{0}").arg(p.model->getValue());
            }
            p.text = text;
        }
    }
}
