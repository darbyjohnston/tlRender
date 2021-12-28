// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/SoftwareRender.h>

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
                imaging::YUVRange yuvRange)
            {
                const imaging::Size size = image->getSize();
                const size_t w = size.w;
                const size_t h = size.h;
                auto out = imaging::Image::create(imaging::Info(size, imaging::PixelType::RGBA_F32));
                switch (image->getPixelType())
                {
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
                        const imaging::F32_T* p = reinterpret_cast<imaging::F32_T*>(image->getData()) + y * w * 3;
                        for (size_t x = 0; x < w; ++x, p += 3, outP += 3)
                        {
                            outP[0] = math::clamp(p[0] * max, min, max);
                            outP[1] = math::clamp(p[1] * max, min, max);
                            outP[2] = math::clamp(p[2] * max, min, max);
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

            struct Pixel
            {
                float r;
                float g;
                float b;
                float a;
            };

            Pixel sample_RGBA_F32(const imaging::F32_T* p, size_t w, size_t h, float x, float y)
            {
                const size_t x0 = floor(x * (w - 1));
                const size_t x1 = ceil(x * (w - 1));
                const size_t y0 = floor(y * (h - 1));
                const size_t y1 = ceil(y * (h - 1));
                const imaging::F32_T* p0 = p + y0 * w * 4 + x0 * 4;
                const imaging::F32_T* p1 = p + y0 * w * 4 + x1 * 4;
                const imaging::F32_T* p2 = p + y1 * w * 4 + x0 * 4;
                const imaging::F32_T* p3 = p + y1 * w * 4 + x1 * 4;
                const float a = x * (w - 1) - x0;
                const float b = y * (h - 1) - y0;
                return
                {
                    (p0[0] * (1.F - a) + p1[0] * a) * (1.F - b) +
                    (p2[0] * (1.F - a) + p3[0] * a) * b,
                    (p0[1] * (1.F - a) + p1[1] * a) * (1.F - b) +
                    (p2[1] * (1.F - a) + p3[1] * a) * b,
                    (p0[2] * (1.F - a) + p1[2] * a) * (1.F - b) +
                    (p2[2] * (1.F - a) + p3[2] * a) * b,
                    (p0[3] * (1.F - a) + p1[3] * a) * (1.F - b) +
                    (p2[3] * (1.F - a) + p3[3] * a) * b
                };
            }

            void drawImage_RGBA_F32(
                const std::shared_ptr<imaging::Image>& fb,
                const math::BBox2i& vp,
                const std::shared_ptr<imaging::Image>& image,
                const math::BBox2f& bbox)
            {
                const imaging::Size& fbSize = fb->getSize();
                const imaging::Size& imageSize = image->getSize();
                const imaging::PixelType imagePixelType = image->getPixelType();
                const imaging::F32_T* imageP = reinterpret_cast<imaging::F32_T*>(image->getData());

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

                for (size_t y = vp.min.y; y <= vp.max.y; ++y)
                {
                    float* fbP = reinterpret_cast<float*>(fb->getData()) +
                        (static_cast<size_t>(fbSize.h) - 1 - y) * static_cast<size_t>(fbSize.w) * 3 +
                        static_cast<size_t>(vp.min.x) * 3;
                    for (size_t x = vp.min.x; x <= vp.max.x; ++x)
                    {
                        for (const auto& t : mesh.triangles)
                        {
                            float w0 = geom::edge(glm::vec2(x, y), mesh.v[t.v[2].v], mesh.v[t.v[1].v]);
                            float w1 = geom::edge(glm::vec2(x, y), mesh.v[t.v[0].v], mesh.v[t.v[2].v]);
                            float w2 = geom::edge(glm::vec2(x, y), mesh.v[t.v[1].v], mesh.v[t.v[0].v]);
                            if (w0 >= 0 && w1 >= 0 && w2 >= 0)
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
                                const Pixel pixel = sample_RGBA_F32(imageP, imageSize.w, imageSize.h, u, v);
                                fbP[0] = pixel.r + fbP[0] * (1.F - pixel.a);
                                fbP[1] = pixel.g + fbP[1] * (1.F - pixel.a);
                                fbP[2] = pixel.b + fbP[2] * (1.F - pixel.a);
                            }
                        }
                        fbP += 3;
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
            const math::BBox2f&,
            const imaging::Color4f&)
        {}

        void SoftwareRender::drawImage(
            const std::shared_ptr<imaging::Image>& image,
            const math::BBox2f& bbox,
            const imaging::Color4f& color,
            const render::ImageOptions& imageOptions)
        {
            TLR_PRIVATE_P();

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
                const size_t stripCount = 8;
                std::vector<math::BBox2i> strips;
                const imaging::Size& fbSize = p.frameBuffer->getSize();
                const size_t stripHeight = fbSize.h / stripCount;
                size_t y = 0;
                for (size_t i = 0; i < stripCount - 1; ++i, y += stripHeight)
                {
                    strips.push_back(math::BBox2i(0, y, fbSize.w, stripHeight));
                }
                strips.push_back(math::BBox2i(0, y, fbSize.w, fbSize.h - y));

                std::vector<std::future<void> > stripFutures;
                for (const auto& strip : strips)
                {
                    stripFutures.push_back(std::async(
                        std::launch::async,
                        [this, strip, image_RGBA_F32, bbox]
                        {
                            drawImage_RGBA_F32(_p->frameBuffer, strip, image_RGBA_F32, bbox);
                        }));
                }
                for (auto& future : stripFutures)
                {
                    future.get();
                }
            }
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
                    break;
                default:
                    if (i.image)
                    {
                        drawImage(
                            i.image,
                            imaging::getBBox(i.image->getAspect(), p.frameBuffer->getSize()),
                            imaging::Color4f(1.F, 1.F, 1.F),
                            imageOptions);
                    }
                    break;
                }
            }
        }

        void SoftwareRender::drawText(
            const std::vector<std::shared_ptr<imaging::Glyph> >& glyphs,
            const glm::vec2& position,
            const imaging::Color4f&)
        {}
    }
}