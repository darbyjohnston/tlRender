// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/Video.h>

#include <tlTimeline/IRender.h>

#include <tlCore/Error.h>
#include <tlCore/FontSystem.h>
#include <tlCore/String.h>

#include <opentimelineio/transition.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(
            Transition,
            "None",
            "Dissolve");
        TLRENDER_ENUM_SERIALIZE_IMPL(Transition);

        Transition toTransition(const std::string& value)
        {
            Transition out = Transition::None;
            if (otio::Transition::Type::SMPTE_Dissolve == value)
            {
                out = Transition::Dissolve;
            }
            return out;
        }

        void IPrimitive::_init(
            const otio::AnyDictionary&,
            const std::shared_ptr<system::Context>& context)
        {
            _context = context;
        }

        IPrimitive::IPrimitive()
        {}

        IPrimitive::~IPrimitive()
        {}

        struct RectPrimitive::Private
        {
            math::BBox2i bbox;
            imaging::Color4f color = imaging::Color4f(1.F, 1.F, 1.F);
        };

        void RectPrimitive::_init(
            const otio::AnyDictionary& value,
            const std::shared_ptr<system::Context>& context)
        {
            IPrimitive::_init(value, context);
            TLRENDER_P();
            for (const auto& l : value)
            {
                if ("bbox" == l.first)
                {
                    const auto s = otio::any_cast<std::string const&>(l.second);
                    std::stringstream ss(s);
                    ss >> p.bbox;
                }
                else if ("color" == l.first)
                {
                    const auto s = otio::any_cast<std::string const&>(l.second);
                    std::stringstream ss(s);
                    ss >> p.color;
                }
            }
        }

        RectPrimitive::RectPrimitive() :
            _p(new Private)
        {}

        RectPrimitive::~RectPrimitive()
        {}

        std::shared_ptr<RectPrimitive> RectPrimitive::create(
            const otio::AnyDictionary& value,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<RectPrimitive>(new RectPrimitive);
            out->_init(value, context);
            return out;
        }

        void RectPrimitive::render(
            const std::shared_ptr<imaging::FontSystem>&,
            const std::shared_ptr<IRender>& render)
        {
            TLRENDER_P();
            render->drawRect(p.bbox, p.color);
        }

        struct TextPrimitive::Private
        {
            std::string text;
            imaging::FontInfo fontInfo;
            math::Vector2i position;
            imaging::Color4f color = imaging::Color4f(1.F, 1.F, 1.F);
        };

        void TextPrimitive::_init(
            const otio::AnyDictionary& value,
            const std::shared_ptr<system::Context>& context)
        {
            IPrimitive::_init(value, context);
            TLRENDER_P();
            for (const auto& l : value)
            {
                if ("text" == l.first)
                {
                    p.text = otio::any_cast<std::string const&>(l.second);
                }
                else if ("fontSize" == l.first)
                {
                    const auto s = otio::any_cast<std::string const&>(l.second);
                    std::stringstream ss(s);
                    ss >> p.fontInfo.size;
                }
                else if ("position" == l.first)
                {
                    const auto s = otio::any_cast<std::string const&>(l.second);
                    std::stringstream ss(s);
                    ss >> p.position;
                }
                else if ("color" == l.first)
                {
                    const auto s = otio::any_cast<std::string const&>(l.second);
                    std::stringstream ss(s);
                    ss >> p.color;
                }
            }
        }

        TextPrimitive::TextPrimitive() :
            _p(new Private)
        {}

        TextPrimitive::~TextPrimitive()
        {}

        std::shared_ptr<TextPrimitive> TextPrimitive::create(
            const otio::AnyDictionary& value,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<TextPrimitive>(new TextPrimitive);
            out->_init(value, context);
            return out;
        }

        void TextPrimitive::render(
            const std::shared_ptr<imaging::FontSystem>& fontSystem,
            const std::shared_ptr<IRender>& render)
        {
            TLRENDER_P();
            render->drawText(fontSystem->getGlyphs(p.text, p.fontInfo), p.position, p.color);
        }

        struct PrimitiveFactory::Private
        {
            std::weak_ptr<system::Context> context;
        };

        void PrimitiveFactory::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            p.context = context;
        }

        PrimitiveFactory::PrimitiveFactory() :
            _p(new Private)
        {}

        std::shared_ptr<PrimitiveFactory> PrimitiveFactory::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<PrimitiveFactory>(new PrimitiveFactory);
            out->_init(context);
            return out;
        }

        void PrimitiveFactory::read(
            const otio::AnyDictionary& value,
            std::vector<std::shared_ptr<IPrimitive> >& out)
        {
            TLRENDER_P();
            if (auto context = p.context.lock())
            {
                try
                {
                    for (const auto& i : value)
                    {
                        if ("tlRender" == i.first)
                        {
                            for (const auto& j : otio::any_cast<otio::AnyVector const&>(i.second))
                            {
                                const auto& dict = otio::any_cast<otio::AnyDictionary const&>(j);
                                for (const auto& k : dict)
                                {
                                    if ("id" == k.first)
                                    {
                                        const auto& s = otio::any_cast<std::string const&>(k.second);
                                        if ("rect" == s)
                                        {
                                            out.push_back(RectPrimitive::create(dict, context));
                                        }
                                        else if ("text" == s)
                                        {
                                            out.push_back(TextPrimitive::create(dict, context));
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                catch (const linb::bad_any_cast&)
                {
                    //! \todo How should this be handled?
                }
            }
        }
    }
}