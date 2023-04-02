// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/GroupBox.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct GroupBox::Private
        {
            std::string text;
            imaging::FontInfo fontInfo;
            math::Vector2i textSize;
            int lineHeight = 0;
            int ascender = 0;
            int margin = 0;
            int spacing = 0;
            int border = 0;
        };

        void GroupBox::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::GroupBox", context, parent);
        }

        GroupBox::GroupBox() :
            _p(new Private)
        {}

        GroupBox::~GroupBox()
        {}

        std::shared_ptr<GroupBox> GroupBox::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<GroupBox>(new GroupBox);
            out->_init(context, parent);
            return out;
        }

        void GroupBox::setText(const std::string& value)
        {
            TLRENDER_P();
            if (value == p.text)
                return;
            p.text = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void GroupBox::setFontInfo(const imaging::FontInfo& value)
        {
            TLRENDER_P();
            if (value == p.fontInfo)
                return;
            p.fontInfo = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void GroupBox::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            math::BBox2i g = value;
            g.min.y += p.lineHeight + p.spacing;
            g = g.margin(-p.margin);
            for (const auto& child : _children)
            {
                child->setGeometry(g);
            }
        }

        void GroupBox::sizeEvent(const SizeEvent& event)
        {
            IWidget::sizeEvent(event);
            TLRENDER_P();

            p.margin = event.style->getSizeRole(SizeRole::Margin) * event.contentScale;
            p.spacing = event.style->getSizeRole(SizeRole::SpacingSmall) * event.contentScale;
            p.border = event.style->getSizeRole(SizeRole::Border) * event.contentScale;

            auto fontInfo = p.fontInfo;
            fontInfo.size *= event.contentScale;
            p.textSize = event.fontSystem->measure(p.text, fontInfo);
            auto fontMetrics = event.fontSystem->getMetrics(fontInfo);
            p.lineHeight = fontMetrics.lineHeight;
            p.ascender = fontMetrics.ascender;
            _sizeHint.x = 0;
            _sizeHint.y = 0;
            for (const auto& child : _children)
            {
                const math::Vector2i& sizeHint = child->getSizeHint();
                _sizeHint.x = std::max(_sizeHint.x, sizeHint.x);
                _sizeHint.y = std::max(_sizeHint.y, sizeHint.y);
            }
            _sizeHint.x += p.margin * 2;
            _sizeHint.y += p.margin * 2;
            _sizeHint.x = std::max(_sizeHint.x, p.textSize.x);
            _sizeHint.y += p.lineHeight + p.spacing;
        }

        void GroupBox::drawEvent(const DrawEvent& event)
        {
            IWidget::drawEvent(event);
            TLRENDER_P();

            math::BBox2i g = _geometry;

            auto fontInfo = p.fontInfo;
            fontInfo.size *= event.contentScale;
            event.render->drawText(
                event.fontSystem->getGlyphs(p.text, fontInfo),
                math::Vector2i(g.x(), g.y() + p.ascender),
                event.style->getColorRole(ColorRole::Text));

            g.min.y += p.lineHeight + p.spacing;
            event.render->drawMesh(
                border(g, p.border, p.margin / 2),
                event.style->getColorRole(ColorRole::Border));
        }
    }
}
