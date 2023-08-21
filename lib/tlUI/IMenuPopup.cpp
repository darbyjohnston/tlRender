// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IMenuPopup.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/EventLoop.h>
#include <tlUI/ScrollWidget.h>

namespace tl
{
    namespace ui
    {
        namespace
        {
            class MenuWidget : public IWidget
            {
                TLRENDER_NON_COPYABLE(MenuWidget);

            protected:
                void _init(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                MenuWidget();

            public:
                virtual ~MenuWidget();

                static std::shared_ptr<MenuWidget> create(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setGeometry(const math::Box2i&) override;
                void sizeHintEvent(const SizeHintEvent&) override;
            };

            void MenuWidget::_init(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidget::_init("tl::ui::MenuWidget", context, parent);
                _setMouseHover(true);
                _setMousePress(true);
            }

            MenuWidget::MenuWidget()
            {}

            MenuWidget::~MenuWidget()
            {}

            std::shared_ptr<MenuWidget> MenuWidget::create(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<MenuWidget>(new MenuWidget);
                out->_init(context, parent);
                return out;
            }

            void MenuWidget::setGeometry(const math::Box2i& value)
            {
                IWidget::setGeometry(value);
                if (!_children.empty())
                {
                    _children.front()->setGeometry(value);
                }
            }

            void MenuWidget::sizeHintEvent(const SizeHintEvent& value)
            {
                IWidget::sizeHintEvent(value);
                if (!_children.empty())
                {
                    _sizeHint = _children.front()->getSizeHint();
                }
            }
        }

        struct IMenuPopup::Private
        {
            MenuPopupStyle popupStyle = MenuPopupStyle::Menu;
            ColorRole popupRole = ColorRole::Window;
            math::Box2i buttonGeometry;
            bool open = false;
            std::function<void(void)> closeCallback;
            std::shared_ptr<IWidget> widget;
            std::shared_ptr<ScrollWidget> scrollWidget;
            std::shared_ptr<MenuWidget> menuWidget;

            struct SizeData
            {
                int shadow = 0;
            };
            SizeData size;
        };

        void IMenuPopup::_init(
            const std::string& objectName,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IPopup::_init(objectName, context, parent);
            TLRENDER_P();

            p.scrollWidget = ScrollWidget::create(
                context,
                ScrollType::Menu);
            
            p.menuWidget = MenuWidget::create(context, shared_from_this());
            p.scrollWidget->setParent(p.menuWidget);
        }

        IMenuPopup::IMenuPopup() :
            _p(new Private)
        {}

        IMenuPopup::~IMenuPopup()
        {}

        void IMenuPopup::open(
            const std::shared_ptr<EventLoop>& eventLoop,
            const math::Box2i& buttonGeometry)
        {
            TLRENDER_P();
            p.buttonGeometry = buttonGeometry;
            p.open = true;
            eventLoop->addWidget(shared_from_this());
        }

        bool IMenuPopup::isOpen() const
        {
            return _p->open;
        }

        void IMenuPopup::close()
        {
            TLRENDER_P();
            p.open = false;
            if (auto eventLoop = getEventLoop().lock())
            {
                eventLoop->removeWidget(shared_from_this());
            }
            if (p.closeCallback)
            {
                p.closeCallback();
            }
        }

        void IMenuPopup::setCloseCallback(const std::function<void(void)>& value)
        {
            _p->closeCallback = value;
        }

        void IMenuPopup::setPopupStyle(MenuPopupStyle value)
        {
            TLRENDER_P();
            p.popupStyle = value;
        }

        void IMenuPopup::setPopupRole(ColorRole value)
        {
            TLRENDER_P();
            if (value == p.popupRole)
                return;
            p.popupRole = value;
            _updates |= Update::Draw;
        }

        void IMenuPopup::setWidget(const std::shared_ptr<IWidget>& value)
        {
            TLRENDER_P();
            p.widget = value;
            p.scrollWidget->setWidget(p.widget);
        }

        void IMenuPopup::setGeometry(const math::Box2i& value)
        {
            IPopup::setGeometry(value);
            TLRENDER_P();
            math::Size2i sizeHint = p.menuWidget->getSizeHint();
            std::list<math::Box2i> boxes;
            switch (p.popupStyle)
            {
            case MenuPopupStyle::Menu:
                boxes.push_back(math::Box2i(
                    p.buttonGeometry.min.x,
                    p.buttonGeometry.max.y + 1,
                    sizeHint.w,
                    sizeHint.h));
                boxes.push_back(math::Box2i(
                    p.buttonGeometry.max.x + 1 - sizeHint.w,
                    p.buttonGeometry.max.y + 1,
                    sizeHint.w,
                    sizeHint.h));
                boxes.push_back(math::Box2i(
                    p.buttonGeometry.min.x,
                    p.buttonGeometry.min.y - sizeHint.h,
                    sizeHint.w,
                    sizeHint.h));
                boxes.push_back(math::Box2i(
                    p.buttonGeometry.max.x + 1 - sizeHint.w,
                    p.buttonGeometry.min.y - sizeHint.h,
                    sizeHint.w,
                    sizeHint.h));
                break;
            case MenuPopupStyle::SubMenu:
                boxes.push_back(math::Box2i(
                    p.buttonGeometry.max.x,
                    p.buttonGeometry.min.y,
                    sizeHint.w,
                    sizeHint.h));
                boxes.push_back(math::Box2i(
                    p.buttonGeometry.min.x - sizeHint.w,
                    p.buttonGeometry.min.y,
                    sizeHint.w,
                    sizeHint.h));
                break;
            }
            struct Intersect
            {
                math::Box2i original;
                math::Box2i intersected;
            };
            std::vector<Intersect> intersect;
            for (const auto& box : boxes)
            {
                intersect.push_back({ box, box.intersect(value) });
            }
            std::stable_sort(
                intersect.begin(),
                intersect.end(),
                [](const Intersect& a, const Intersect& b)
                {
                    return
                        a.intersected.getSize().getArea() >
                        b.intersected.getSize().getArea();
                });
            math::Box2i g = intersect.front().intersected;
            p.menuWidget->setGeometry(g);
        }

        void IMenuPopup::sizeHintEvent(const SizeHintEvent& event)
        {
            IPopup::sizeHintEvent(event);
            TLRENDER_P();
            p.size.shadow = event.style->getSizeRole(SizeRole::Shadow, event.displayScale);
        }

        void IMenuPopup::drawEvent(
            const math::Box2i& drawRect,
            const DrawEvent& event)
        {
            IPopup::drawEvent(drawRect, event);
            TLRENDER_P();
            //event.render->drawRect(
            //    _geometry,
            //    image::Color4f(0.F, 0.F, 0.F, .2F));
            const math::Box2i& g = p.menuWidget->getGeometry();
            if (g.isValid())
            {
                const math::Box2i g2(
                    g.min.x - p.size.shadow,
                    g.min.y,
                    g.w() + p.size.shadow * 2,
                    g.h() + p.size.shadow);
                event.render->drawColorMesh(
                    shadow(g2, p.size.shadow),
                    math::Vector2i(),
                    image::Color4f(1.F, 1.F, 1.F));

                event.render->drawRect(g, event.style->getColorRole(p.popupRole));
            }
        }
    }
}
