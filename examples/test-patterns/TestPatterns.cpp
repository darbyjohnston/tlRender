// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "TestPatterns.h"

#include <feather-tk/core/Context.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/Math.h>

#include <cmath>

namespace tl
{
    namespace examples
    {
        namespace test_patterns
        {
            void ITestPattern::_init(
                const std::shared_ptr<feather_tk::Context>& context,
                const std::string& name,
                const feather_tk::Size2I& size)
            {
                _context = context;
                _name = name;
                _size = size;
            }

            ITestPattern::ITestPattern()
            {}

            ITestPattern::~ITestPattern()
            {}

            const std::string& ITestPattern::getName() const
            {
                return _name;
            }

            void CountTestPattern::_init(
                const std::shared_ptr<feather_tk::Context>& context,
                const feather_tk::Size2I& size)
            {
                ITestPattern::_init(context, getClassName(), size);

                auto fontSystem = context->getSystem<feather_tk::FontSystem>();
                _secondsFontInfo = feather_tk::FontInfo(
                    "NotoSansMono-Regular",
                    _size.h / 2.F);
                _secondsFontMetrics = fontSystem->getMetrics(_secondsFontInfo);

                _framesFontInfo = feather_tk::FontInfo(
                    "NotoSansMono-Regular",
                    _secondsFontInfo.size / 4.F);
                _framesFontMetrics = fontSystem->getMetrics(_framesFontInfo);
            }

            CountTestPattern::~CountTestPattern()
            {}

            std::string CountTestPattern::getClassName()
            {
                return "Count";
            }

            std::shared_ptr<CountTestPattern> CountTestPattern::create(
                const std::shared_ptr<feather_tk::Context>& context,
                const feather_tk::Size2I& size)
            {
                auto out = std::shared_ptr<CountTestPattern>(new CountTestPattern);
                out->_init(context, size);
                return out;
            }

            void CountTestPattern::render(
                const std::shared_ptr<timeline::IRender>& render,
                const OTIO_NS::RationalTime& time)
            {
                if (auto context = _context.lock())
                {
                    const OTIO_NS::RationalTime seconds = time.rescaled_to(1.0);
                    const int wholeSeconds = static_cast<int>(seconds.value());
                    const int frames = static_cast<int>(time.value()) % static_cast<int>(time.rate());

                    const std::string secondsString = feather_tk::Format("{0}").arg(wholeSeconds);
                    auto fontSystem = context->getSystem<feather_tk::FontSystem>();
                    const feather_tk::Size2I secondsSize = fontSystem->getSize(secondsString, _secondsFontInfo);
                    const feather_tk::V2I secondsPos(
                        _size.w / 2.F - secondsSize.w / 2.F,
                        _size.h / 2.F - secondsSize.h / 2.F);

                    const std::string framesString = feather_tk::Format("{0}").arg(frames);
                    const feather_tk::Size2I framesSize = fontSystem->getSize(framesString, _framesFontInfo);
                    const feather_tk::V2I framesPos(
                        _size.w / 2.F - framesSize.w / 2.F,
                        secondsPos.y + secondsSize.h);

                    render->drawRect(
                        feather_tk::Box2I(0, 0, _size.w, _size.h),
                        feather_tk::Color4F(.1F, .1F, .1F));

                    /*render->drawRect(
                        feather_tk::Box2I(secondsPos.x, secondsPos.y, secondsSize.x, secondsSize.y),
                        feather_tk::Color4F(.5F, 0.F, 0.F));
                    render->drawRect(
                        feather_tk::Box2I(framesPos.x, framesPos.y, framesSize.x, framesSize.y),
                        feather_tk::Color4F(0.F, .5F, 0.F));*/

                    const size_t resolution = 100;
                    feather_tk::TriMesh2F mesh;
                    mesh.v.push_back(feather_tk::V2F(
                        _size.w / 2.F,
                        _size.h / 2.F));
                    for (int i = 0; i < resolution; ++i)
                    {
                        const float f = i / static_cast<float>(resolution - 1);
                        const float a = f * feather_tk::pi2;
                        const float r = secondsSize.h / 2.F + framesSize.h + 10.F;
                        mesh.v.push_back(feather_tk::V2F(
                            _size.w / 2.F + std::cos(a) * r,
                            _size.h / 2.F + std::sin(a) * r));
                    }
                    for (int i = 1; i < resolution; ++i)
                    {
                        mesh.triangles.push_back(feather_tk::Triangle2({
                            feather_tk::Vertex2(1),
                            feather_tk::Vertex2(i + 1),
                            feather_tk::Vertex2(i - 1 + 1) }));
                    }
                    mesh.triangles.push_back(feather_tk::Triangle2({
                        feather_tk::Vertex2(1),
                        feather_tk::Vertex2(1 + resolution - 1),
                        feather_tk::Vertex2(1 + 1) }));
                    render->drawMesh(
                        mesh,
                        feather_tk::Color4F(.2F, .2F, .2F));

                    mesh.v.clear();
                    mesh.triangles.clear();
                    mesh.v.push_back(feather_tk::V2F(
                        _size.w / 2.F,
                        _size.h / 2.F));
                    for (int i = 0; i < resolution; ++i)
                    {
                        const float v = frames / time.rate();
                        const float f = i / static_cast<float>(resolution - 1);
                        const float a = v * f * feather_tk::pi2 - feather_tk::pi / 2.F;
                        const float r = secondsSize.h / 2.F + framesSize.h + 10.F;
                        mesh.v.push_back(feather_tk::V2F(
                            _size.w / 2.F + std::cos(a) * r,
                            _size.h / 2.F + std::sin(a) * r));
                    }
                    for (int i = 1; i < resolution; ++i)
                    {
                        mesh.triangles.push_back(feather_tk::Triangle2({
                            feather_tk::Vertex2(1),
                            feather_tk::Vertex2(i + 1),
                            feather_tk::Vertex2((i - 1) + 1) }));
                    }
                    render->drawMesh(
                        mesh,
                        feather_tk::Color4F(.3F, .3F, .3F));

                    render->drawText(
                        fontSystem->getGlyphs(secondsString, _secondsFontInfo),
                        _secondsFontMetrics,
                        secondsPos,
                        feather_tk::Color4F(1.F, 1.F, 1.F));

                    render->drawText(
                        fontSystem->getGlyphs(framesString, _framesFontInfo),
                        _framesFontMetrics,
                        framesPos,
                        feather_tk::Color4F(1.F, 1.F, 1.F));
                }
            }

            void SwatchesTestPattern::_init(
                const std::shared_ptr<feather_tk::Context>& context,
                const feather_tk::Size2I& size)
            {
                ITestPattern::_init(context, getClassName(), size);

                const feather_tk::ImageInfo info(_size.w, 1, feather_tk::ImageType::L_F32);
                _gradient = feather_tk::Image::create(info);
                float* data = reinterpret_cast<float*>(_gradient->getData());
                for (float* p = data, v = 0.F; p < data + _size.w; ++p, v += 1.F / _size.w)
                {
                    *p = v;
                }
            }

            SwatchesTestPattern::~SwatchesTestPattern()
            {}

            std::string SwatchesTestPattern::getClassName()
            {
                return "Swatches";
            }

            std::shared_ptr<SwatchesTestPattern> SwatchesTestPattern::create(
                const std::shared_ptr<feather_tk::Context>& context,
                const feather_tk::Size2I& size)
            {
                auto out = std::shared_ptr<SwatchesTestPattern>(new SwatchesTestPattern);
                out->_init(context, size);
                return out;
            }

            void SwatchesTestPattern::render(
                const std::shared_ptr<timeline::IRender>& render,
                const OTIO_NS::RationalTime& time)
            {
                const std::array<feather_tk::Color4F, 8> colors =
                {
                    feather_tk::Color4F(0.F, 0.F, 0.F),
                    feather_tk::Color4F(1.F, 0.F, 0.F),
                    feather_tk::Color4F(1.F, 1.F, 0.F),
                    feather_tk::Color4F(0.F, 1.F, 0.F),
                    feather_tk::Color4F(0.F, 1.F, 1.F),
                    feather_tk::Color4F(0.F, 0.F, 1.F),
                    feather_tk::Color4F(1.F, 0.F, 1.F),
                    feather_tk::Color4F(1.F, 1.F, 1.F)
                };
                const int swatchWidth = _size.w / colors.size();
                for (int x = 0, i = 0; x < _size.w; x += swatchWidth, ++i)
                {
                    render->drawRect(
                        feather_tk::Box2I(x, 0, swatchWidth, _size.h / 2),
                        colors[i]);
                }
                render->drawImage(_gradient, feather_tk::Box2I(0, _size.h / 2, _size.w, _size.h / 2));
            }

            GridTestPattern::~GridTestPattern()
            {}

            std::string GridTestPattern::getClassName()
            {
                return "Grid";
            }

            std::shared_ptr<GridTestPattern> GridTestPattern::create(
                const std::shared_ptr<feather_tk::Context>& context,
                const feather_tk::Size2I& size)
            {
                auto out = std::shared_ptr<GridTestPattern>(new GridTestPattern);
                out->_init(context, getClassName(), size);
                return out;
            }

            void GridTestPattern::render(
                const std::shared_ptr<timeline::IRender>& render,
                const OTIO_NS::RationalTime& time)
            {
                int cellSize = 2;
                switch (static_cast<int>(time.value() / 24.0) % 3)
                {
                case 1: cellSize = 10; break;
                case 2: cellSize = 100; break;
                }
                {
                    feather_tk::TriMesh2F mesh;
                    for (int x = 0, i = 0; x < _size.w; x += cellSize, ++i)
                    {
                        mesh.v.push_back(feather_tk::V2F(x, 0.F));
                        mesh.v.push_back(feather_tk::V2F(x + 1, 0.F));
                        mesh.v.push_back(feather_tk::V2F(x + 1, _size.h));
                        mesh.v.push_back(feather_tk::V2F(x, _size.h));
                        mesh.triangles.push_back(feather_tk::Triangle2({
                            feather_tk::Vertex2(i * 4 + 1),
                            feather_tk::Vertex2(i * 4 + 2),
                            feather_tk::Vertex2(i * 4 + 3) }));
                        mesh.triangles.push_back(feather_tk::Triangle2({
                            feather_tk::Vertex2(i * 4 + 3),
                            feather_tk::Vertex2(i * 4 + 4),
                            feather_tk::Vertex2(i * 4 + 1) }));
                    }
                    render->drawMesh(
                        mesh,
                        feather_tk::Color4F(1.F, 1.F, 1.F));
                }
                {
                    feather_tk::TriMesh2F mesh;
                    for (int y = 0, i = 0; y < _size.h; y += cellSize, ++i)
                    {
                        mesh.v.push_back(feather_tk::V2F(0.F, y));
                        mesh.v.push_back(feather_tk::V2F(_size.w, y));
                        mesh.v.push_back(feather_tk::V2F(_size.w, y + 1));
                        mesh.v.push_back(feather_tk::V2F(0.F, y + 1));
                        mesh.triangles.push_back(feather_tk::Triangle2({
                            feather_tk::Vertex2(i * 4 + 1),
                            feather_tk::Vertex2(i * 4 + 2),
                            feather_tk::Vertex2(i * 4 + 3) }));
                        mesh.triangles.push_back(feather_tk::Triangle2({
                            feather_tk::Vertex2(i * 4 + 3),
                            feather_tk::Vertex2(i * 4 + 4),
                            feather_tk::Vertex2(i * 4 + 1) }));
                    }
                    render->drawMesh(
                        mesh,
                        feather_tk::Color4F(1.F, 1.F, 1.F));
                }
            }

            std::shared_ptr<ITestPattern> TestPatternFactory::create(
                const std::shared_ptr<feather_tk::Context>& context,
                const std::string& name,
                const feather_tk::Size2I& size)
            {
                std::shared_ptr<ITestPattern> out;
                if (name == CountTestPattern::getClassName())
                {
                    out = CountTestPattern::create(context, size);
                }
                else if (name == SwatchesTestPattern::getClassName())
                {
                    out = SwatchesTestPattern::create(context, size);
                }
                else if (name == GridTestPattern::getClassName())
                {
                    out = GridTestPattern::create(context, size);
                }
                return out;
            };
        }
    }
}
