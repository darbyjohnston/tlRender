// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrCore/SoftwareRender.h>

#include <tlrCore/FontSystem.h>
#include <tlrCore/Mesh.h>

using namespace tlr::core;

namespace tlr
{
    namespace render
    {
        namespace
        {
            std::shared_ptr<imaging::Image> convert_to_RGBA_F32(
                const std::shared_ptr<imaging::Image>& image,
                imaging::YUVRange yuvRange = imaging::YUVRange::Full)
            {
                const imaging::Size size = image->getSize();
                const size_t w = size.w;
                const size_t h = size.h;
                auto out = imaging::Image::create(imaging::Info(size, imaging::PixelType::RGBA_F32));
                switch (image->getPixelType())
                {
                case imaging::PixelType::L_U8:
                {
                    imaging::F32_T* outP = reinterpret_cast<imaging::F32_T*>(out->getData());
                    const float max = imaging::U8Range.getMax();
                    for (size_t y = 0; y < h; ++y)
                    {
                        const imaging::U8_T* p = image->getData() + y * w;
                        for (size_t x = 0; x < w; ++x, ++p, outP += 4)
                        {
                            outP[0] = outP[1] = outP[2] = *p / max;
                            outP[3] = 1.F;
                        }
                    }
                    break;
                }
                case imaging::PixelType::LA_U8:
                {
                    imaging::F32_T* outP = reinterpret_cast<imaging::F32_T*>(out->getData());
                    const float max = imaging::U8Range.getMax();
                    for (size_t y = 0; y < h; ++y)
                    {
                        const imaging::U8_T* p = image->getData() + y * w * 2;
                        for (size_t x = 0; x < w; ++x, p += 2, outP += 4)
                        {
                            outP[0] = outP[1] = outP[2] = p[0] / max;
                            outP[3] = p[1] / max;
                        }
                    }
                    break;
                }
                case imaging::PixelType::RGB_U8:
                {
                    imaging::F32_T* outP = reinterpret_cast<imaging::F32_T*>(out->getData());
                    const float max = imaging::U8Range.getMax();
                    for (size_t y = 0; y < h; ++y)
                    {
                        const imaging::U8_T* p = image->getData() + y * w * 3;
                        for (size_t x = 0; x < w; ++x, p += 3, outP += 4)
                        {
                            outP[0] = p[0] / max;
                            outP[1] = p[1] / max;
                            outP[2] = p[2] / max;
                            outP[3] = 1.F;
                        }
                    }
                    break;
                }
                case imaging::PixelType::RGBA_U8:
                {
                    imaging::F32_T* outP = reinterpret_cast<imaging::F32_T*>(out->getData());
                    const float max = imaging::U8Range.getMax();
                    for (size_t y = 0; y < h; ++y)
                    {
                        const imaging::U8_T* p = image->getData() + y * w * 4;
                        for (size_t x = 0; x < w; ++x, p += 4, outP += 4)
                        {
                            outP[0] = p[0] / max;
                            outP[1] = p[1] / max;
                            outP[2] = p[2] / max;
                            outP[3] = p[3] / max;
                        }
                    }
                    break;
                }
                case imaging::PixelType::RGB_F32:
                {
                    imaging::F32_T* outP = reinterpret_cast<imaging::F32_T*>(out->getData());
                    for (size_t y = 0; y < h; ++y)
                    {
                        const imaging::F32_T* p = reinterpret_cast<imaging::F32_T*>(image->getData()) + y * w * 3;
                        for (size_t x = 0; x < w; ++x, p += 3, outP += 4)
                        {
                            outP[0] = p[0];
                            outP[1] = p[1];
                            outP[2] = p[2];
                            outP[3] = 1.F;
                        }
                    }
                    break;
                }
                case imaging::PixelType::RGBA_F32:
                {
                    imaging::F32_T* outP = reinterpret_cast<imaging::F32_T*>(out->getData());
                    for (size_t y = 0; y < h; ++y)
                    {
                        const imaging::F32_T* p = reinterpret_cast<imaging::F32_T*>(image->getData()) + y * w * 4;
                        for (size_t x = 0; x < w; ++x, p += 4, outP += 4)
                        {
                            outP[0] = p[0];
                            outP[1] = p[1];
                            outP[2] = p[2];
                            outP[3] = p[3];
                        }
                    }
                    break;
                }
                case imaging::PixelType::YUV_420P:
                {
                    const imaging::U8_T* cbp = image->getData() + h * w;
                    const size_t w2 = w / 2;
                    const size_t h2 = h / 2;
                    const imaging::U8_T* crp = cbp + h2 * w2;
                    imaging::F32_T* outP = reinterpret_cast<imaging::F32_T*>(out->getData());
                    const float max = imaging::U8Range.getMax();
                    for (size_t y = 0; y < h; ++y)
                    {
                        const imaging::U8_T* p = image->getData() + y * w;
                        for (size_t x = 0; x < w; ++x, ++p, outP += 4)
                        {
                            const float _y = *p / max;
                            const float cb = *(cbp + y / 2 * w2 + x / 2) / max - .5F;
                            const float cr = *(crp + y / 2 * w2 + x / 2) / max - .5F;
                            switch (yuvRange)
                            {
                            case imaging::YUVRange::Full:
                                outP[0] = _y + ( 0.F    * cb) + ( 1.4F   * cr);
                                outP[1] = _y + (-0.343F * cb) + (-0.711F * cr);
                                outP[2] = _y + ( 1.765F * cb) + ( 0.F    * cr);
                                break;
                            case imaging::YUVRange::Video:
                                outP[0] = (1.164F * _y) + ( 0.F    * cb) + ( 1.793F * cr);
                                outP[1] = (1.164F * _y) + (-0.213F * cb) + (-0.533F * cr);
                                outP[2] = (1.164F * _y) + ( 2.112F * cb) + ( 0.F    * cr);
                                break;
                            default: break;
                            }
                            outP[3] = 1.F;
                        }
                    }
                    break;
                }
                default:
                    out->zero();
                    break;
                }
                return out;
            }

            std::shared_ptr<imaging::Image> convert_from_RGBA_F32(
                const std::shared_ptr<imaging::Image>& image,
                imaging::PixelType pixelType)
            {
                const imaging::Size size = image->getSize();
                const size_t w = size.w;
                const size_t h = size.h;
                auto out = imaging::Image::create(imaging::Info(size, pixelType));
                switch (pixelType)
                {
                case imaging::PixelType::RGB_U8:
                {
                    imaging::U8_T* outP = out->getData();
                    const float min = imaging::U8Range.getMin();
                    const float max = imaging::U8Range.getMax();
                    for (size_t y = 0; y < h; ++y)
                    {
                        const imaging::F32_T* p = reinterpret_cast<imaging::F32_T*>(image->getData()) + (h - 1 - y) * w * 3;
                        for (size_t x = 0; x < w; ++x, p += 3, outP += 3)
                        {
                            outP[0] = math::clamp(p[0] * max, min, max);
                            outP[1] = math::clamp(p[1] * max, min, max);
                            outP[2] = math::clamp(p[2] * max, min, max);
                        }
                    }
                    break;
                }
                case imaging::PixelType::RGBA_U8:
                {
                    imaging::U8_T* outP = out->getData();
                    const float min = imaging::U8Range.getMin();
                    const float max = imaging::U8Range.getMax();
                    for (size_t y = 0; y < h; ++y)
                    {
                        const imaging::F32_T* p = reinterpret_cast<imaging::F32_T*>(image->getData()) + (h - 1 - y) * w * 3;
                        for (size_t x = 0; x < w; ++x, p += 3, outP += 4)
                        {
                            outP[0] = math::clamp(p[0] * max, min, max);
                            outP[1] = math::clamp(p[1] * max, min, max);
                            outP[2] = math::clamp(p[2] * max, min, max);
                            outP[3] = max;
                        }
                    }
                    break;
                }
                default:
                    out->zero();
                    break;
                }
                return out;
            }

            imaging::Color4f sample_RGBA_F32(const imaging::F32_T* p, size_t w, size_t h, float x, float y)
            {
                const size_t x0 = x * (static_cast<int>(w) - 1);
                const size_t x1 = std::min(static_cast<int>(x0) + 1, static_cast<int>(w) - 1);
                const size_t y0 = y * (static_cast<int>(h) - 1);
                const size_t y1 = std::min(static_cast<int>(y0) + 1, static_cast<int>(h) - 1);
                const imaging::F32_T* p0 = p + y0 * w * 4 + x0 * 4;
                const imaging::F32_T* p1 = p + y0 * w * 4 + x1 * 4;
                const imaging::F32_T* p2 = p + y1 * w * 4 + x0 * 4;
                const imaging::F32_T* p3 = p + y1 * w * 4 + x1 * 4;
                const float a = x * (static_cast<int>(w) - 1) - x0;
                const float b = y * (static_cast<int>(h) - 1) - y0;
                return imaging::Color4f(
                    (p0[0] * (1.F - a) + p1[0] * a) * (1.F - b) + (p2[0] * (1.F - a) + p3[0] * a) * b,
                    (p0[1] * (1.F - a) + p1[1] * a) * (1.F - b) + (p2[1] * (1.F - a) + p3[1] * a) * b,
                    (p0[2] * (1.F - a) + p1[2] * a) * (1.F - b) + (p2[2] * (1.F - a) + p3[2] * a) * b,
                    (p0[3] * (1.F - a) + p1[3] * a) * (1.F - b) + (p2[3] * (1.F - a) + p3[3] * a) * b);
            }

            void drawMesh(
                const geom::TriangleMesh2& mesh,
                const std::shared_ptr<imaging::Image>& texture,
                const imaging::Color4f& color,
                const std::shared_ptr<imaging::Image>& frameBuffer)
            {
                if (!mesh.v.empty())
                {
                    math::BBox2i bbox;
                    bbox.min.x = floor(mesh.v[0].x);
                    bbox.min.y = floor(mesh.v[0].y);
                    bbox.max.x = ceil(mesh.v[0].x);
                    bbox.max.y = ceil(mesh.v[0].y);
                    for (const auto& v : mesh.v)
                    {
                        bbox.min.x = std::min(bbox.min.x, static_cast<int>(floor(v.x)));
                        bbox.min.y = std::min(bbox.min.y, static_cast<int>(floor(v.y)));
                        bbox.max.x = std::max(bbox.max.x, static_cast<int>(ceil(v.x)));
                        bbox.max.y = std::max(bbox.max.y, static_cast<int>(ceil(v.y)));
                    }

                    const size_t fbW = frameBuffer->getWidth();
                    const size_t fbH = frameBuffer->getHeight();
                    bbox = bbox.intersect(math::BBox2i(0, 0, fbW, fbH));
                    if (bbox.isValid())
                    {
                        imaging::Color4f textureColor;
                        const imaging::Size textureSize = texture ? texture->getSize() : imaging::Size();
                        const imaging::F32_T* textureP = texture ? reinterpret_cast<imaging::F32_T*>(texture->getData()) : nullptr;
                        for (size_t y = bbox.min.y; y <= bbox.max.y; ++y)
                        {
                            imaging::F32_T* fbP = reinterpret_cast<imaging::F32_T*>(frameBuffer->getData()) +
                                y * fbW * 3 +
                                static_cast<size_t>(bbox.min.x) * 3;
                            for (size_t x = bbox.min.x; x <= bbox.max.x; ++x, fbP += 3)
                            {
                                for (const auto& t : mesh.triangles)
                                {
                                    float w0 = geom::edge(glm::vec2(x, y), mesh.v[t.v[2].v], mesh.v[t.v[1].v]);
                                    float w1 = geom::edge(glm::vec2(x, y), mesh.v[t.v[0].v], mesh.v[t.v[2].v]);
                                    float w2 = geom::edge(glm::vec2(x, y), mesh.v[t.v[1].v], mesh.v[t.v[0].v]);
                                    if (w0 >= 0.F && w1 >= 0.F && w2 >= 0.F)
                                    {
                                        const float area = geom::edge(
                                            mesh.v[t.v[2].v],
                                            mesh.v[t.v[1].v],
                                            mesh.v[t.v[0].v]);
                                        w0 /= area;
                                        w1 /= area;
                                        w2 /= area;
                                        const float u = w0 * mesh.t[t.v[0].t].x + w1 * mesh.t[t.v[1].t].x + w2 * mesh.t[t.v[2].t].x;
                                        const float v = w0 * mesh.t[t.v[0].t].y + w1 * mesh.t[t.v[1].t].y + w2 * mesh.t[t.v[2].t].y;
                                        imaging::Color4f pixel = color;
                                        if (textureP)
                                        {
                                            textureColor = sample_RGBA_F32(textureP, textureSize.w, textureSize.h, u, v);
                                            pixel.r *= textureColor.r;
                                            pixel.g *= textureColor.g;
                                            pixel.b *= textureColor.b;
                                            pixel.a *= textureColor.a;
                                        }
                                        fbP[0] = pixel.r + fbP[0] * (1.F - pixel.a);
                                        fbP[1] = pixel.g + fbP[1] * (1.F - pixel.a);
                                        fbP[2] = pixel.b + fbP[2] * (1.F - pixel.a);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        struct SoftwareRender::Private
        {
            std::shared_ptr<imaging::Image> frameBuffer;
        };

        void SoftwareRender::_init(const std::shared_ptr<Context>& context)
        {
            IRender::_init(context);
        }

        SoftwareRender::SoftwareRender() :
            _p(new Private)
        {}

        SoftwareRender::~SoftwareRender()
        {}

        std::shared_ptr<SoftwareRender> SoftwareRender::create(const std::shared_ptr<Context>& context)
        {
            auto out = std::shared_ptr<SoftwareRender>(new SoftwareRender);
            out->_init(context);
            return out;
        }

        const std::shared_ptr<imaging::Image> SoftwareRender::getFrameBuffer() const
        {
            return _p->frameBuffer;
        }

        std::shared_ptr<imaging::Image> SoftwareRender::copyFrameBuffer(imaging::PixelType pixelType) const
        {
            std::shared_ptr<imaging::Image> out;
            if (_p->frameBuffer)
            {
                out = convert_from_RGBA_F32(_p->frameBuffer, pixelType);
            }
            return out;
        }

        void SoftwareRender::setTextureCacheSize(size_t)
        {}

        void SoftwareRender::setColorConfig(const imaging::ColorConfig&)
        {}

        void SoftwareRender::begin(const imaging::Size& size)
        {
            TLR_PRIVATE_P();
            if (!p.frameBuffer || (p.frameBuffer && size != p.frameBuffer->getSize()))
            {
                p.frameBuffer = imaging::Image::create(imaging::Info(size, imaging::PixelType::RGB_F32));
            }
            p.frameBuffer->zero();
        }

        void SoftwareRender::end()
        {}

        void SoftwareRender::drawRect(
            const math::BBox2i& bbox,
            const imaging::Color4f& color)
        {
            TLR_PRIVATE_P();

            geom::TriangleMesh2 mesh;
            mesh.v.push_back(glm::vec2(bbox.min.x, bbox.min.y));
            mesh.v.push_back(glm::vec2(bbox.max.x, bbox.min.y));
            mesh.v.push_back(glm::vec2(bbox.max.x, bbox.max.y));
            mesh.v.push_back(glm::vec2(bbox.min.x, bbox.max.y));
            mesh.t.push_back(glm::vec2(0.F, 0.F));
            mesh.t.push_back(glm::vec2(1.F, 0.F));
            mesh.t.push_back(glm::vec2(1.F, 1.F));
            mesh.t.push_back(glm::vec2(0.F, 1.F));
            geom::Triangle2 triangle;
            triangle.v[0].v = 0;
            triangle.v[1].v = 1;
            triangle.v[2].v = 2;
            triangle.v[0].t = 0;
            triangle.v[1].t = 1;
            triangle.v[2].t = 2;
            mesh.triangles.push_back(triangle);
            triangle.v[0].v = 2;
            triangle.v[1].v = 3;
            triangle.v[2].v = 0;
            triangle.v[0].t = 2;
            triangle.v[1].t = 3;
            triangle.v[2].t = 0;
            mesh.triangles.push_back(triangle);
            drawMesh(mesh, nullptr, color, p.frameBuffer);
        }

        namespace
        {
            void _drawImage(
                const std::shared_ptr<imaging::Image>& image,
                const math::BBox2i& bbox,
                const imaging::Color4f& color,
                const render::ImageOptions& imageOptions,
                const std::shared_ptr<imaging::Image>& frameBuffer)
            {
                const auto& info = image->getInfo();
                imaging::YUVRange yuvRange = info.yuvRange;
                switch (imageOptions.yuvRange)
                {
                case render::YUVRange::Full:  yuvRange = imaging::YUVRange::Full;  break;
                case render::YUVRange::Video: yuvRange = imaging::YUVRange::Video; break;
                default: break;
                }

                if (auto image_RGBA_F32 = convert_to_RGBA_F32(image, yuvRange))
                {
                    geom::TriangleMesh2 mesh;
                    mesh.v.push_back(glm::vec2(bbox.min.x, bbox.min.y));
                    mesh.v.push_back(glm::vec2(bbox.max.x, bbox.min.y));
                    mesh.v.push_back(glm::vec2(bbox.max.x, bbox.max.y));
                    mesh.v.push_back(glm::vec2(bbox.min.x, bbox.max.y));
                    mesh.t.push_back(glm::vec2(0.F, 0.F));
                    mesh.t.push_back(glm::vec2(1.F, 0.F));
                    mesh.t.push_back(glm::vec2(1.F, 1.F));
                    mesh.t.push_back(glm::vec2(0.F, 1.F));
                    geom::Triangle2 triangle;
                    triangle.v[0].v = 0;
                    triangle.v[1].v = 1;
                    triangle.v[2].v = 2;
                    triangle.v[0].t = 0;
                    triangle.v[1].t = 1;
                    triangle.v[2].t = 2;
                    mesh.triangles.push_back(triangle);
                    triangle.v[0].v = 2;
                    triangle.v[1].v = 3;
                    triangle.v[2].v = 0;
                    triangle.v[0].t = 2;
                    triangle.v[1].t = 3;
                    triangle.v[2].t = 0;
                    mesh.triangles.push_back(triangle);
                    drawMesh(mesh, image_RGBA_F32, color, frameBuffer);
                }
            }
        }

        void SoftwareRender::drawImage(
            const std::shared_ptr<imaging::Image>& image,
            const math::BBox2i& bbox,
            const imaging::Color4f& color,
            const render::ImageOptions& imageOptions)
        {
            _drawImage(image, bbox, color, imageOptions, _p->frameBuffer);
        }

        void SoftwareRender::drawVideo(
            const timeline::VideoData& data,
            const render::ImageOptions& imageOptions)
        {
            TLR_PRIVATE_P();
            for (const auto& i : data.layers)
            {
                switch (i.transition)
                {
                case timeline::Transition::Dissolve:
                {
                    auto buffer = imaging::Image::create(p.frameBuffer->getInfo());
                    buffer->zero();

                    ImageOptions imageOptionsTmp;
                    imageOptionsTmp.yuvRange = imageOptions.yuvRange;
                    if (i.image)
                    {
                        const float t = 1.F - i.transitionValue;
                        _drawImage(
                            i.image,
                            imaging::getBBox(i.image->getAspect(), buffer->getSize()),
                            imaging::Color4f(t, t, t, t),
                            imageOptionsTmp,
                            buffer);
                    }
                    if (i.imageB)
                    {
                        const float tB = i.transitionValue;
                        _drawImage(
                            i.imageB,
                            imaging::getBBox(i.imageB->getAspect(), buffer->getSize()),
                            imaging::Color4f(tB, tB, tB, tB),
                            imageOptionsTmp,
                            buffer);
                    }

                    _drawImage(
                        buffer,
                        imaging::getBBox(buffer->getAspect(), p.frameBuffer->getSize()),
                        imaging::Color4f(1.F, 1.F, 1.F),
                        imageOptions,
                        p.frameBuffer);
                    break;
                }
                default:
                    if (i.image)
                    {
                        _drawImage(
                            i.image,
                            imaging::getBBox(i.image->getAspect(), p.frameBuffer->getSize()),
                            imaging::Color4f(1.F, 1.F, 1.F),
                            imageOptions,
                            p.frameBuffer);
                    }
                    break;
                }
            }
        }

        void SoftwareRender::drawText(
            const std::vector<std::shared_ptr<imaging::Glyph> >& glyphs,
            const glm::ivec2& pos,
            const imaging::Color4f& color)
        {
            TLR_PRIVATE_P();
            int x = 0;
            int32_t rsbDeltaPrev = 0;
            uint8_t textureIndex = 0;
            for (const auto& glyph : glyphs)
            {
                if (glyph)
                {
                    if (rsbDeltaPrev - glyph->lsbDelta > 32)
                    {
                        x -= 1;
                    }
                    else if (rsbDeltaPrev - glyph->lsbDelta < -31)
                    {
                        x += 1;
                    }
                    rsbDeltaPrev = glyph->rsbDelta;

                    if (!glyph->data.empty())
                    {
                        auto image_RGBA_F32 = imaging::Image::create(imaging::Info(
                            glyph->width,
                            glyph->height,
                            imaging::PixelType::RGBA_F32));
                        for (size_t glyphY = 0; glyphY < glyph->height; ++glyphY)
                        {
                            const uint8_t* glyphP = glyph->data.data() + glyphY * glyph->width;
                            float* imageP = reinterpret_cast<float*>(image_RGBA_F32->getData()) + glyphY * glyph->width * 4;
                            for (size_t glyphX = 0; glyphX < glyph->width; ++glyphX, ++glyphP, imageP += 4)
                            {
                                imageP[0] = color.r * (*glyphP / 255.0);
                                imageP[1] = color.g * (*glyphP / 255.0);
                                imageP[2] = color.b * (*glyphP / 255.0);
                                imageP[3] = color.a * (*glyphP / 255.0);
                            }
                        }

                        const glm::ivec2& offset = glyph->offset;
                        const math::BBox2i bbox(pos.x + x + offset.x, pos.y - offset.y, glyph->width, glyph->height);

                        geom::TriangleMesh2 mesh;
                        mesh.v.push_back(glm::vec2(bbox.min.x, bbox.min.y));
                        mesh.v.push_back(glm::vec2(bbox.max.x, bbox.min.y));
                        mesh.v.push_back(glm::vec2(bbox.max.x, bbox.max.y));
                        mesh.v.push_back(glm::vec2(bbox.min.x, bbox.max.y));
                        mesh.t.push_back(glm::vec2(0.F, 0.F));
                        mesh.t.push_back(glm::vec2(1.F, 0.F));
                        mesh.t.push_back(glm::vec2(1.F, 1.F));
                        mesh.t.push_back(glm::vec2(0.F, 1.F));
                        geom::Triangle2 triangle;
                        triangle.v[0].v = 0;
                        triangle.v[1].v = 1;
                        triangle.v[2].v = 2;
                        triangle.v[0].t = 0;
                        triangle.v[1].t = 1;
                        triangle.v[2].t = 2;
                        mesh.triangles.push_back(triangle);
                        triangle.v[0].v = 2;
                        triangle.v[1].v = 3;
                        triangle.v[2].v = 0;
                        triangle.v[0].t = 2;
                        triangle.v[1].t = 3;
                        triangle.v[2].t = 0;
                        mesh.triangles.push_back(triangle);
                        drawMesh(mesh, image_RGBA_F32, color, p.frameBuffer);
                    }

                    x += glyph->advance;
                }
            }           
        }
    }
}