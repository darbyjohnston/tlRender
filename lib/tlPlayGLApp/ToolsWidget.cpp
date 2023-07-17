// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/ToolsWidget.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/AudioToolWidget.h>
#include <tlPlayGLApp/ColorToolWidget.h>
#include <tlPlayGLApp/CompareToolWidget.h>
#include <tlPlayGLApp/DevicesToolWidget.h>
#include <tlPlayGLApp/FilesToolWidget.h>
#include <tlPlayGLApp/InfoToolWidget.h>
#include <tlPlayGLApp/MessagesToolWidget.h>
#include <tlPlayGLApp/SettingsToolWidget.h>
#include <tlPlayGLApp/SystemLogToolWidget.h>

#include <tlUI/RowLayout.h>
#include <tlUI/StackLayout.h>

namespace tl
{
    namespace play_gl
    {
        struct ToolsWidget::Private
        {
            std::map<Tool, std::shared_ptr<IToolWidget> > toolWidgets;
            std::shared_ptr<ui::StackLayout> layout;
            std::shared_ptr<observer::ValueObserver<int> > activeObserver;
        };

        void ToolsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(
                "tl::play_gl::ToolsWidget",
                context,
                parent);
            TLRENDER_P();

            p.toolWidgets[Tool::Audio] = AudioToolWidget::create(app, context);
            p.toolWidgets[Tool::Color] = ColorToolWidget::create(app, context);
            p.toolWidgets[Tool::Compare] = CompareToolWidget::create(app, context);
            p.toolWidgets[Tool::Devices] = DevicesToolWidget::create(app, context);
            p.toolWidgets[Tool::Files] = FilesToolWidget::create(app, context);
            p.toolWidgets[Tool::Info] = InfoToolWidget::create(app, context);
            p.toolWidgets[Tool::Messages] = MessagesToolWidget::create(app, context);
            p.toolWidgets[Tool::Settings] = SettingsToolWidget::create(app, context);
            p.toolWidgets[Tool::SystemLog] = SystemLogToolWidget::create(app, context);

            p.layout = ui::StackLayout::create(context, shared_from_this());
            for (const auto& widget : p.toolWidgets)
            {
                widget.second->setParent(p.layout);
            }

            p.activeObserver = observer::ValueObserver<int>::create(
                app->getToolsModel()->observeActiveTool(),
                [this](int value)
                {
                    _p->layout->setCurrentIndex(value);
                    setVisible(value != -1);
                });
        }

        ToolsWidget::ToolsWidget() :
            _p(new Private)
        {}

        ToolsWidget::~ToolsWidget()
        {}

        std::shared_ptr<ToolsWidget> ToolsWidget::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<ToolsWidget>(new ToolsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void ToolsWidget::setGeometry(const math::BBox2i & value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void ToolsWidget::sizeHintEvent(const ui::SizeHintEvent & event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        std::map<Tool, bool> ToolsWidget::_getToolsVisible() const
        {
            TLRENDER_P();
            std::map<Tool, bool> out;
            for (const auto& i : p.toolWidgets)
            {
                out[i.first] = i.second->isVisible(false);
            }
            return out;
        }
    }
}
