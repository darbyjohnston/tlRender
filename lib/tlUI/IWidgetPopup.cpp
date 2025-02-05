// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/IWidgetPopup.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/IWindow.h>

namespace tl
{
    namespace ui
    {
        namespace
        {
            class ContainerWidget : public IWidget
            {
                TLRENDER_NON_COPYABLE(ContainerWidget);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                ContainerWidget();

            public:
                virtual ~ContainerWidget();

                static std::shared_ptr<ContainerWidget> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setGeometry(const dtk::Box2I&) override;
                void sizeHintEvent(const SizeHintEvent&) override;
            };

            void ContainerWidget::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidget::_init("tl::ui::ContainerWidget", context, parent);
                _setMouseHover(true);
                _setMousePress(true);
            }

            ContainerWidget::ContainerWidget()
            {}

            ContainerWidget::~ContainerWidget()
            {}

            std::shared_ptr<ContainerWidget> ContainerWidget::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<ContainerWidget>(new ContainerWidget);
                out->_init(context, parent);
                return out;
            }

            void ContainerWidget::setGeometry(const dtk::Box2I& value)
            {
                IWidget::setGeometry(value);
                if (!_children.empty())
                {
                    _children.front()->setGeometry(value);
                }
            }

            void ContainerWidget::sizeHintEvent(const SizeHintEvent& event)
            {
                IWidget::sizeHintEvent(event);
                if (!_children.empty())
                {
                    _sizeHint = _children.front()->getSizeHint();
                }
            }
        }

        struct IWidgetPopup::Private
        {
            ColorRole popupRole = ColorRole::Window;
            dtk::Box2I buttonGeometry;
            bool open = false;
            std::function<void(void)> closeCallback;
            std::shared_ptr<IWidget> widget;
            std::shared_ptr<ContainerWidget> containerWidget;

            struct SizeData
            {
                int border = 0;
                int shadow = 0;
            };
            SizeData size;
        };

        void IWidgetPopup::_init(
            const std::string& objectName,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IPopup::_init(objectName, context, parent);
            TLRENDER_P();
            p.containerWidget = ContainerWidget::create(context, shared_from_this());
        }

        IWidgetPopup::IWidgetPopup() :
            _p(new Private)
        {}

        IWidgetPopup::~IWidgetPopup()
        {}

        void IWidgetPopup::open(
            const std::shared_ptr<IWindow>& window,
            const dtk::Box2I& buttonGeometry)
        {
            TLRENDER_P();
            p.buttonGeometry = buttonGeometry;
            p.open = true;
            setParent(window);
            takeKeyFocus();
        }

        bool IWidgetPopup::isOpen() const
        {
            return _p->open;
        }

        void IWidgetPopup::close()
        {
            TLRENDER_P();
            p.open = false;
            setParent(nullptr);
            if (p.closeCallback)
            {
                p.closeCallback();
            }
        }

        void IWidgetPopup::setCloseCallback(const std::function<void(void)>& value)
        {
            _p->closeCallback = value;
        }

        void IWidgetPopup::setPopupRole(ColorRole value)
        {
            TLRENDER_P();
            if (value == p.popupRole)
                return;
            p.popupRole = value;
            _updates |= Update::Draw;
        }

        void IWidgetPopup::setWidget(const std::shared_ptr<IWidget>& value)
        {
            TLRENDER_P();
            if (p.widget)
            {
                p.widget->setParent(nullptr);
            }
            p.widget = value;
            if (p.widget)
            {
                p.widget->setParent(p.containerWidget);
            }
        }

        void IWidgetPopup::setGeometry(const dtk::Box2I& value)
        {
            IPopup::setGeometry(value);
            TLRENDER_P();
            dtk::Size2I sizeHint = p.containerWidget->getSizeHint();
            std::list<dtk::Box2I> boxes;
            boxes.push_back(dtk::Box2I(
                p.buttonGeometry.min.x,
                p.buttonGeometry.max.y + 1,
                sizeHint.w,
                sizeHint.h));
            boxes.push_back(dtk::Box2I(
                p.buttonGeometry.max.x + 1 - sizeHint.w,
                p.buttonGeometry.max.y + 1,
                sizeHint.w,
                sizeHint.h));
            boxes.push_back(dtk::Box2I(
                p.buttonGeometry.min.x,
                p.buttonGeometry.min.y - sizeHint.h,
                sizeHint.w,
                sizeHint.h));
            boxes.push_back(dtk::Box2I(
                p.buttonGeometry.max.x + 1 - sizeHint.w,
                p.buttonGeometry.min.y - sizeHint.h,
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
            p.containerWidget->setGeometry(g);
        }

        void IWidgetPopup::sizeHintEvent(const SizeHintEvent& event)
        {
            IPopup::sizeHintEvent(event);
            TLRENDER_P();
            p.size.border = event.style->getSizeRole(SizeRole::Border, _displayScale);
            p.size.shadow = event.style->getSizeRole(SizeRole::Shadow, _displayScale);
        }

        void IWidgetPopup::drawEvent(
            const dtk::Box2I& drawRect,
            const DrawEvent& event)
        {
            IPopup::drawEvent(drawRect, event);
            TLRENDER_P();
            //event.render->drawRect(
            //    _geometry,
            //    dtk::Color4F(0.F, 0.F, 0.F, .2F));
            const dtk::Box2I g = margin(p.containerWidget->getGeometry(), p.size.border);
            if (g.isValid())
            {
                const dtk::Box2I g2(
                    g.min.x - p.size.shadow,
                    g.min.y,
                    g.w() + p.size.shadow * 2,
                    g.h() + p.size.shadow);
                event.render->drawColorMesh(
                    shadow(g2, p.size.shadow),
                    dtk::Color4F(1.F, 1.F, 1.F));

                event.render->drawMesh(
                    border(g, p.size.border),
                    event.style->getColorRole(ColorRole::Border));

                event.render->drawRect(
                    dtk::margin(g, -p.size.border),
                    event.style->getColorRole(p.popupRole));
            }
        }
    }
}
