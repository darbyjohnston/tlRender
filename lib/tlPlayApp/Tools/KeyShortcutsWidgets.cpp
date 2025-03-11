// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Tools/SettingsToolPrivate.h>

#include <tlPlayApp/App.h>

#include <dtk/ui/GroupBox.h>
#include <dtk/ui/CheckBox.h>
#include <dtk/ui/DrawUtil.h>
#include <dtk/ui/FormLayout.h>
#include <dtk/ui/Label.h>
#include <dtk/ui/RowLayout.h>

namespace tl
{
    namespace play
    {
        struct KeyShortcutWidget::Private
        {
            KeyShortcut shortcut;
            bool collision = false;

            std::shared_ptr<dtk::Label> label;

            std::function<void(const KeyShortcut&)> callback;

            struct SizeData
            {
                int border = 0;
            };
            SizeData size;
        };

        void KeyShortcutWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::KeyShortcutWidget", parent);
            DTK_P();
            
            setHStretch(dtk::Stretch::Expanding);
            setAcceptsKeyFocus(true);
            _setMouseHoverEnabled(true);
            _setMousePressEnabled(true);

            p.label = dtk::Label::create(context, shared_from_this());
            p.label->setMarginRole(dtk::SizeRole::MarginInside);

            _widgetUpdate();
        }

        KeyShortcutWidget::KeyShortcutWidget() :
            _p(new Private)
        {}

        KeyShortcutWidget::~KeyShortcutWidget()
        {}

        std::shared_ptr<KeyShortcutWidget> KeyShortcutWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<KeyShortcutWidget>(new KeyShortcutWidget);
            out->_init(context, parent);
            return out;
        }

        void KeyShortcutWidget::setShortcut(const KeyShortcut& value)
        {
            DTK_P();
            if (value == p.shortcut)
                return;
            p.shortcut = value;
            _widgetUpdate();
        }

        void KeyShortcutWidget::setCallback(const std::function<void(const KeyShortcut&)>& value)
        {
            _p->callback = value;
        }

        void KeyShortcutWidget::setCollision(bool value)
        {
            DTK_P();
            if (value == p.collision)
                return;
            p.collision = value;
            _widgetUpdate();
        }

        void KeyShortcutWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            DTK_P();
            const dtk::Box2I g = dtk::margin(value, -p.size.border);
            p.label->setGeometry(g);
        }

        void KeyShortcutWidget::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            DTK_P();
            p.size.border = event.style->getSizeRole(dtk::SizeRole::Border, event.displayScale);
            _setSizeHint(_p->label->getSizeHint() + p.size.border * 2);
        }

        void KeyShortcutWidget::drawEvent(const dtk::Box2I& drawRect, const dtk::DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            DTK_P();

            const dtk::Box2I& g = getGeometry();
            event.render->drawMesh(
                dtk::border(g, p.size.border),
                event.style->getColorRole(hasKeyFocus() ? dtk::ColorRole::KeyFocus : dtk::ColorRole::Border));

            const dtk::Box2I g2 = dtk::margin(g, -p.size.border);
            event.render->drawRect(
                g2,
                event.style->getColorRole(p.collision ? dtk::ColorRole::Red : dtk::ColorRole::Base));

            if (_isMouseInside())
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(dtk::ColorRole::Hover));
            }
        }

        void KeyShortcutWidget::mouseEnterEvent(dtk::MouseEnterEvent& event)
        {
            IWidget::mouseEnterEvent(event);
            _setDrawUpdate();
        }

        void KeyShortcutWidget::mouseLeaveEvent()
        {
            IWidget::mouseLeaveEvent();
            _setDrawUpdate();
        }

        void KeyShortcutWidget::mousePressEvent(dtk::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            if (0 == event.button)
            {
                event.accept = true;
                takeKeyFocus();
                _setDrawUpdate();
            }
        }

        void KeyShortcutWidget::mouseReleaseEvent(dtk::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            if (0 == event.button)
            {
                event.accept = true;
            }
        }

        void KeyShortcutWidget::keyFocusEvent(bool value)
        {
            IWidget::keyFocusEvent(value);
            _setDrawUpdate();
        }

        void KeyShortcutWidget::keyPressEvent(dtk::KeyEvent& event)
        {
            IWidget::keyPressEvent(event);
            DTK_P();
            switch (event.key)
            {
            case dtk::Key::Unknown: break;
            case dtk::Key::Escape:
                event.accept = true;
                releaseKeyFocus();
                break;
            case dtk::Key::Enter: break;
            case dtk::Key::Tab: break;
            case dtk::Key::CapsLock: break;
            case dtk::Key::ScrollLock: break;
            case dtk::Key::NumLock: break;
            case dtk::Key::LeftShift: break;
            case dtk::Key::LeftControl: break;
            case dtk::Key::LeftAlt: break;
            case dtk::Key::LeftSuper: break;
            case dtk::Key::RightShift: break;
            case dtk::Key::RightControl: break;
            case dtk::Key::RightAlt: break;
            case dtk::Key::RightSuper: break;
            default:
                if (hasKeyFocus())
                {
                    event.accept = true;
                    p.shortcut.key = event.key;
                    p.shortcut.modifiers = event.modifiers;
                    if (p.callback)
                    {
                        p.callback(p.shortcut);
                    }
                    _widgetUpdate();
                }
                break;
            }
        }

        void KeyShortcutWidget::keyReleaseEvent(dtk::KeyEvent& event)
        {
            IWidget::keyReleaseEvent(event);
            event.accept = true;
        }

        void KeyShortcutWidget::_widgetUpdate()
        {
            DTK_P();
            p.label->setText(dtk::getShortcutLabel(p.shortcut.key, p.shortcut.modifiers));
        }

        struct KeyShortcutsSettingsWidget::Private
        {
            std::shared_ptr<SettingsModel> model;
            struct Group
            {
                std::string name;
                std::vector<KeyShortcut> shortcuts;

                bool operator == (const Group& other) const
                {
                    bool out =
                        name == other.name &&
                        shortcuts.size() == other.shortcuts.size();
                    for (size_t i = 0; out && i < shortcuts.size(); ++i)
                    {
                        out &= shortcuts[i].name == other.shortcuts[i].name;
                    }
                    return out;
                }

                bool operator != (const Group& other) const
                {
                    return !(*this == other);
                }
            };
            std::vector<Group> groups;

            std::map<std::string, std::shared_ptr<KeyShortcutWidget> > widgets;
            std::vector<std::shared_ptr<dtk::GroupBox> > groupBoxes;
            std::shared_ptr<dtk::VerticalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<KeyShortcutsSettings> > settingsObserver;
        };

        void KeyShortcutsSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::KeyShortcutsSettingsWidget", parent);
            DTK_P();

            p.model = app->getSettingsModel();

            p.layout = dtk::VerticalLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);

            p.settingsObserver = dtk::ValueObserver<KeyShortcutsSettings>::create(
                p.model->observeKeyShortcuts(),
                [this](const KeyShortcutsSettings& value)
                {
                    _widgetUpdate(value);
                });
        }

        KeyShortcutsSettingsWidget::KeyShortcutsSettingsWidget() :
            _p(new Private)
        {}

        KeyShortcutsSettingsWidget::~KeyShortcutsSettingsWidget()
        {}

        std::shared_ptr<KeyShortcutsSettingsWidget> KeyShortcutsSettingsWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<KeyShortcutsSettingsWidget>(new KeyShortcutsSettingsWidget);
            out->_init(context, app, parent);
            return out;
        }

        void KeyShortcutsSettingsWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void KeyShortcutsSettingsWidget::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }

        void KeyShortcutsSettingsWidget::_widgetUpdate(const KeyShortcutsSettings& settings)
        {
            DTK_P();

            // Create groups of shortcuts.
            std::vector<Private::Group> groups;
            for (const auto& shortcut : settings.shortcuts)
            {
                const auto s = dtk::split(shortcut.name, '/');
                if ((!s.empty() && !groups.empty() && s.front() != groups.back().name) ||
                    (!s.empty() && groups.empty()))
                {
                    Private::Group group;
                    group.name = s.front();
                    groups.push_back(group);
                }
                if (s.size() > 1 && !groups.empty())
                {
                    groups.back().shortcuts.push_back(shortcut);
                }
            }

            // Find collisions.
            std::map<std::pair<dtk::Key, int>, std::vector<std::string> > collisions;
            for (const auto& shortcut : settings.shortcuts)
            {
                if (shortcut.key != dtk::Key::Unknown)
                {
                    collisions[std::make_pair(shortcut.key, shortcut.modifiers)].push_back(shortcut.name);
                }
            }

            if (groups != p.groups)
            {
                p.groups = groups;

                // Delete the old widgets.
                p.widgets.clear();
                for (auto groupBox : p.groupBoxes)
                {
                    groupBox->setParent(nullptr);
                }
                p.groupBoxes.clear();

                // Create the new widgets.
                if (auto context = getContext())
                {
                    for (const auto& group : p.groups)
                    {
                        auto groupBox = dtk::GroupBox::create(context, group.name, p.layout);
                        p.groupBoxes.push_back(groupBox);
                        auto formLayout = dtk::FormLayout::create(context, groupBox);
                        formLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
                        for (const auto& shortcut : group.shortcuts)
                        {
                            auto widget = KeyShortcutWidget::create(context);
                            widget->setShortcut(shortcut);
                            p.widgets[shortcut.name] = widget;
                            formLayout->addRow(shortcut.text + ":", widget);
                            widget->setCallback(
                                [this](const KeyShortcut& value)
                                {
                                    DTK_P();
                                    auto settings = p.model->getKeyShortcuts();
                                    const auto shortcut = value;
                                    const auto i = std::find_if(
                                        settings.shortcuts.begin(),
                                        settings.shortcuts.end(),
                                        [shortcut](const KeyShortcut& other)
                                        {
                                            return shortcut.name == other.name;
                                        });
                                    if (i != settings.shortcuts.end())
                                    {
                                        *i = value;
                                        p.model->setKeyShortcuts(settings);
                                    }
                                });
                        }
                    }
                }
            }

            // Update the values.
            for (const auto& group : p.groups)
            {
                for (const auto& shortcut : group.shortcuts)
                {
                    const auto i = p.widgets.find(shortcut.name);
                    const auto j = std::find_if(
                        settings.shortcuts.begin(),
                        settings.shortcuts.end(),
                        [shortcut](const KeyShortcut& value)
                        {
                            return shortcut.name == value.name;
                        });
                    if (i != p.widgets.end() && j != settings.shortcuts.end())
                    {
                        i->second->setShortcut(*j);
                        bool collision = false;
                        const auto k = collisions.find(std::make_pair(j->key, j->modifiers));
                        if (k != collisions.end())
                        {
                            collision = k->second.size() > 1;
                        }
                        i->second->setCollision(collision);
                    }
                }
            }
        }
    }
}
