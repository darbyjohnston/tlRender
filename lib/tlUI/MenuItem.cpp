// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/MenuBar.h>

namespace tl
{
    namespace ui
    {
        struct MenuItem::Private
        {
            std::string text;
            std::weak_ptr<MenuItem> parent;
            std::list<std::shared_ptr<MenuItem> > children;
        };

        void MenuItem::_init(
            const std::string& text,
            const std::shared_ptr<MenuItem>& parent)
        {
            TLRENDER_P();
            p.text = text;
            if (parent)
            {
                parent->_p->children.push_back(shared_from_this());
            }
            p.parent = parent;
        }

        MenuItem::MenuItem() :
            _p(new Private)
        {}

        MenuItem::~MenuItem()
        {}
        
        std::shared_ptr<MenuItem> MenuItem::create(
            const std::string& text,
            const std::shared_ptr<MenuItem>& parent)
        {
            auto out = std::shared_ptr<MenuItem>(new MenuItem);
            out->_init(text, parent);
            return out;
        }
    }
}
