// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/ToolTip.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/IWindow.h>
#include <tlUI/Label.h>

namespace tl
{
    namespace ui
    {
        struct ToolTip::Private
        {
            dtk::V2I pos;

            std::shared_ptr<Label> label;

            struct SizeData
            {
                bool sizeInit = true;
                int border = 0;
                int handle = 0;
                int shadow = 0;
            };
            SizeData size;
        };

        void ToolTip::_init(
            const std::string& text,
            const dtk::V2I& pos,
            const std::shared_ptr<IWidget>& window,
            const std::shared_ptr<dtk::Context>& context)
        {
            IPopup::_init("tl::ui::ToolTip", context, nullptr);
            DTK_P();

            p.pos = pos;

            p.label = Label::create(context, shared_from_this());
            p.label->setText(text);
            p.label->setTextRole(ColorRole::ToolTipText);
            p.label->setMarginRole(SizeRole::MarginSmall);

            setParent(window);
        }

        ToolTip::ToolTip() :
            _p(new Private)
        {}

        ToolTip::~ToolTip()
        {}

        std::shared_ptr<ToolTip> ToolTip::create(
            const std::string& text,
            const dtk::V2I& pos,
            const std::shared_ptr<IWidget>& window,
            const std::shared_ptr<dtk::Context>& context)
        {
            auto out = std::shared_ptr<ToolTip>(new ToolTip);
            out->_init(text, pos, window, context);
            return out;
        }

        void ToolTip::close()
        {
            setParent(nullptr);
        }

        void ToolTip::setGeometry(const dtk::Box2I& value)
        {
            IPopup::setGeometry(value);
            DTK_P();
            dtk::Size2I sizeHint = p.label->getSizeHint();
            std::list<dtk::Box2I> boxes;
            boxes.push_back(dtk::Box2I(
                p.pos.x + p.size.handle,
                p.pos.y + p.size.handle,
                sizeHint.w,
                sizeHint.h));
            boxes.push_back(dtk::Box2I(
                p.pos.x - p.size.handle - sizeHint.w,
                p.pos.y + p.size.handle,
                sizeHint.w,
                sizeHint.h));
            boxes.push_back(dtk::Box2I(
                p.pos.x + p.size.handle,
                p.pos.y - p.size.handle - sizeHint.h,
                sizeHint.w,
                sizeHint.h));
            boxes.push_back(dtk::Box2I(
                p.pos.x - p.size.handle - sizeHint.w,
                p.pos.y - p.size.handle - sizeHint.h,
                sizeHint.w,
                sizeHint.h));
            struct Intersect
            {
                dtk::Box2I original;
                dtk::Box2I intersected;
            };
            std::vector<Intersect> intersect;
            for (const auto& box : boxes)
            {
                intersect.push_back({ box, dtk::intersect(box, value) });
            }
            std::stable_sort(
                intersect.begin(),
                intersect.end(),
                [](const Intersect& a, const Intersect& b)
                {
                    return
                        dtk::area(a.intersected.size()) >
                        dtk::area(b.intersected.size());
                });
            dtk::Box2I g = intersect.front().intersected;
            p.label->setGeometry(g);
        }

        void ToolTip::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IPopup::sizeHintEvent(event);
            DTK_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.border = event.style->getSizeRole(SizeRole::Border, _displayScale);
                p.size.handle = event.style->getSizeRole(SizeRole::Handle, _displayScale);
                p.size.shadow = event.style->getSizeRole(SizeRole::Shadow, _displayScale);
            }
            p.size.sizeInit = false;
        }

        void ToolTip::drawEvent(
            const dtk::Box2I& drawRect,
            const DrawEvent& event)
        {
            IPopup::drawEvent(drawRect, event);
            DTK_P();
            //event.render->drawRect(
            //    _geometry,
            //    dtk::Color4F(0.F, 0.F, 0.F, .2F));
            const dtk::Box2I g = p.label->getGeometry();
            const dtk::Box2I g2(
                g.min.x - p.size.shadow,
                g.min.y,
                g.w() + p.size.shadow * 2,
                g.h() + p.size.shadow);
            event.render->drawColorMesh(
                shadow(g2, p.size.shadow),
                dtk::Color4F(1.F, 1.F, 1.F));

            event.render->drawMesh(
                border(dtk::margin(g, p.size.border), p.size.border),
                event.style->getColorRole(ColorRole::Border));
                
            event.render->drawRect(
                g,
                event.style->getColorRole(ColorRole::ToolTipWindow));
        }
    }
}
