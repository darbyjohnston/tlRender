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
            struct Pixel
            {
                float r;
                float g;
                float b;
                float a;
            };

            Pixel sample_RGBA_U8(const imaging::U8_T* p, size_t w, size_t h, float x, float y)
            {
                const float max = imaging::U8Range.getMax();

                const size_t x0 = floor(x * (w - 1));
                const size_t x1 = ceil(x * (w - 1));
                const size_t y0 = floor(y * (h - 1));
                const size_t y1 = ceil(y * (h - 1));
                const imaging::U8_T* p0 = p + y0 * w * 4 + x0 * 4;
                const imaging::U8_T* p1 = p + y0 * w * 4 + x1 * 4;
                const imaging::U8_T* p2 = p + y1 * w * 4 + x0 * 4;
                const imaging::U8_T* p3 = p + y1 * w * 4 + x1 * 4;
                const float a = x * (w - 1) - x0;
                const float b = y * (h - 1) - y0;

                return {
                    (p0[0] / max * (1.F - a) + p1[0] / max * a) * (1.F - b) +
                    (p2[0] / max * (1.F - a) + p3[0] / max * a) * b,
                    (p0[1] / max * (1.F - a) + p1[1] / max * a) * (1.F - b) +
                    (p2[1] / max * (1.F - a) + p3[1] / max * a) * b,
                    (p0[2] / max * (1.F - a) + p1[2] / max * a) * (1.F - b) +
                    (p2[2] / max * (1.F - a) + p3[2] / max * a) * b,
                    (p0[3] / max * (1.F - a) + p1[3] / max * a) * (1.F - b) +
                    (p2[3] / max * (1.F - a) + p3[3] / max * a) * b
                };
            }

            Pixel sample_YUV_420P(const imaging::U8_T* p, size_t w, size_t h, float x, float y)
            {
                const float max = imaging::U8Range.getMax();

                size_t x0 = floor(x * (w - 1));
                size_t x1 = ceil(x * (w - 1));
                size_t y0 = floor(y * (h - 1));
                size_t y1 = ceil(y * (h - 1));
                const imaging::U8_T* p0 = p + y0 * w + x0;
                const imaging::U8_T* p1 = p + y0 * w + x1;
                const imaging::U8_T* p2 = p + y1 * w + x0;
                const imaging::U8_T* p3 = p + y1 * w + x1;
                float a = x * (w - 1) - x0;
                float b = y * (h - 1) - y0;
                const float _y =
                    (*p0 / max * (1.F - a) + *p1 / max * a) * (1.F - b) +
                    (*p2 / max * (1.F - a) + *p3 / max * a) * b;

                const size_t w2 = w / 2;
                const size_t h2 = h / 2;
                x0 = floor(x * (w2 - 1));
                x1 = ceil(x * (w2 - 1));
                y0 = floor(y * (h2 - 1));
                y1 = ceil(y * (h2 - 1));
                p0 = p + h * w + y0 * w2 + x0;
                p1 = p + h * w + y0 * w2 + x1;
                p2 = p + h * w + y1 * w2 + x0;
                p3 = p + h * w + y1 * w2 + x1;
                a = x * (w2 - 1) - x0;
                b = y * (h2 - 1) - y0;
                const float cb =
                    (*p0 / max * (1.F - a) + *p1 / max * a) * (1.F - b) +
                    (*p2 / max * (1.F - a) + *p3 / max * a) * b -
                    .5F;

                p0 = p + h * w + h2 * w2 + y0 * w2 + x0;
                p1 = p + h * w + h2 * w2 + y0 * w2 + x1;
                p2 = p + h * w + h2 * w2 + y1 * w2 + x0;
                p3 = p + h * w + h2 * w2 + y1 * w2 + x1;
                const float cr =
                    (*p0 / max * (1.F - a) + *p1 / max * a) * (1.F - b) +
                    (*p2 / max * (1.F - a) + *p3 / max * a) * b -
                    .5F;

                return {
                    _y + (0.F * cb) + (1.4F * cr),
                    _y + (-.343F * cb) + (-.711F * cr),
                    _y + (1.765F * cb) + (0.F * cr),
                    1.F
                };
            }

            Pixel sample(const imaging::U8_T* p, size_t w, size_t h, imaging::PixelType t, float x, float y)
            {
                switch (t)
                {
                case imaging::PixelType::RGBA_U8: return sample_RGBA_U8(p, w, h, x, y);
                case imaging::PixelType::YUV_420P: return sample_YUV_420P(p, w, h, x, y);
                default: break;
                }
                return { 0.F, 0.F, 0.F, 0.F };
            }

            void renderImage(
                const std::shared_ptr<imaging::Image>& fb,
                const math::BBox2i& vp,
                const std::shared_ptr<imaging::Image>& image,
                const math::BBox2f& bbox)
            {
                const imaging::Size& fbSize = fb->getSize();
                const imaging::Size& videoSize = image->getSize();
                const imaging::PixelType videoType = image->getPixelType();
                const uint8_t* videoP = image->getData();

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
                    float* fbP = reinterpret_cast<float*>(fb->getData()) + (fbSize.h - 1 - y) * fbSize.w * 3 + vp.min.x * 3;
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
                                const Pixel pixel = sample(videoP, videoSize.w, videoSize.h, videoType, u, v);
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
            const render::ImageOptions& options)
        {
            TLR_PRIVATE_P();
            const size_t stripCount = 24;
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
                    [this, strip, image, bbox]
                    {
                        renderImage(_p->frameBuffer, strip, image, bbox);
                    }));
            }
            for (auto& future : stripFutures)
            {
                future.get();
            }
        }

        void SoftwareRender::drawVideo(
            const timeline::VideoData&,
            const render::ImageOptions&)
        {}

        void SoftwareRender::drawText(
            const std::vector<std::shared_ptr<imaging::Glyph> >& glyphs,
            const glm::vec2& position,
            const imaging::Color4f&)
        {}
    }
}