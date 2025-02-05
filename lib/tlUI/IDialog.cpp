// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/IDialog.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/IWindow.h>

namespace tl
{
    namespace ui
    {
        struct IDialog::Private
        {
            bool open = false;
            std::function<void(void)> closeCallback;
            
            struct SizeData
            {
                bool sizeInit = true;
                int margin = 0;
                int border = 0;
                int shadow = 0;
            };
            SizeData size;
        };

        void IDialog::_init(
            const std::string& objectName,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IPopup::_init(objectName, context, parent);
        }

        IDialog::IDialog() :
            _p(new Private)
        {}

        IDialog::~IDialog()
        {}

        void IDialog::open(const std::shared_ptr<IWindow>& window)
        {
            TLRENDER_P();
            p.open = true;
            setParent(window);
            takeKeyFocus();
        }

        bool IDialog::isOpen() const
        {
            return _p->open;
        }

        void IDialog::close()
        {
            TLRENDER_P();
            p.open = false;
            setParent(nullptr);
            if (p.closeCallback)
            {
                p.closeCallback();
            }
        }

        void IDialog::setCloseCallback(const std::function<void(void)>& value)
        {
            _p->closeCallback = value;
        }

        void IDialog::setGeometry(const dtk::Box2I& value)
        {
            IPopup::setGeometry(value);
            TLRENDER_P();
            if (!_children.empty())
            {
                const dtk::Box2I g = dtk::margin(value, -p.size.margin);
                const dtk::Size2I& sizeHint = _children.front()->getSizeHint();
                dtk::V2I size;
                size.x = std::min(sizeHint.w, g.w());
                size.y = std::min(sizeHint.h, g.h());
                if (Stretch::Expanding == _children.front()->getHStretch())
                {
                    size.x = g.w();
                }
                if (Stretch::Expanding == _children.front()->getVStretch())
                {
                    size.y = g.h();
                }
                _children.front()->setGeometry(dtk::Box2I(
                    g.x() + g.w() / 2 - size.x / 2,
                    g.y() + g.h() / 2 - size.y / 2,
                    size.x,
                    size.y));
            }
        }

        void IDialog::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IPopup::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(SizeRole::MarginDialog, _displayScale);
                p.size.border = event.style->getSizeRole(SizeRole::Border, _displayScale);
                p.size.shadow = event.style->getSizeRole(SizeRole::Shadow, _displayScale);
            }
            p.size.sizeInit = false;
        }

        void IDialog::drawEvent(
            const dtk::Box2I& drawRect,
            const DrawEvent& event)
        {
            IPopup::drawEvent(drawRect, event);
            TLRENDER_P();
            //event.render->drawRect(
            //    _geometry,
            //    dtk::Color4F(0.F, 0.F, 0.F, .2F));
            if (!_children.empty())
            {
                const dtk::Box2I g = _children.front()->getGeometry();
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
                
                event.render->drawRect(g, event.style->getColorRole(ColorRole::Window));
            }
        }
    }
}
