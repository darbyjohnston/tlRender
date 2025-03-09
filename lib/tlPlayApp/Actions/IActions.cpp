// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/IActions.h>

#include <tlPlayApp/App.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace play
    {
        struct IActions::Private
        {
            std::shared_ptr<dtk::ValueObserver<KeyShortcutsSettings> > keyShortcutsSettingsObserver;
        };

        void IActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::string& name)
        {
            DTK_P();

            _name = name;

            p.keyShortcutsSettingsObserver = dtk::ValueObserver<KeyShortcutsSettings>::create(
                app->getSettingsModel()->observeKeyShortcuts(),
                [this](const KeyShortcutsSettings& value)
                {
                    _keyShortcutsUpdate(value);
                });
        }

        IActions::IActions() :
            _p(new Private)
        {}

        IActions::~IActions()
        {}

        const std::map<std::string, std::shared_ptr<dtk::Action> >& IActions::getActions() const
        {
            return _actions;
        }

        void IActions::_keyShortcutsUpdate(const KeyShortcutsSettings& value)
        {
            for (const auto& i : _actions)
            {
                const std::string name = dtk::Format("{0}/{1}").arg(_name).arg(i.first);
                const auto j = std::find_if(
                    value.shortcuts.begin(),
                    value.shortcuts.end(),
                    [name](const KeyShortcut& value)
                    {
                        return name == value.name;
                    });
                if (j != value.shortcuts.end())
                {
                    i.second->setShortcut(j->key);
                    i.second->setShortcutModifiers(j->modifiers);
                    const auto k = _tooltips.find(i.first);
                    if (k != _tooltips.end())
                    {
                        i.second->setTooltip(dtk::Format(
                            "{0}\n"
                            "\n"
                            "Shortcut: {1}").
                            arg(k->second).
                            arg(dtk::getShortcutLabel(j->key, j->modifiers)));
                    }
                }
            }
        }
    }
}
