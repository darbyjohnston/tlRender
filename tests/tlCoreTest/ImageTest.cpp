// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCoreTest/ImageTest.h>

#include <tlCore/Image.h>

#include <dtk/core/Format.h>

using namespace tl::image;

namespace tl
{
    namespace core_tests
    {
        ImageTest::ImageTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "core_tests::ImageTest")
        {}

        std::shared_ptr<ImageTest> ImageTest::create(const std::shared_ptr<dtk::Context>& context)
        {
            return std::shared_ptr<ImageTest>(new ImageTest(context));
        }

        void ImageTest::run()
        {
            _size();
            _enums();
            _util();
            _info();
            _image();
            _serialize();
        }

        void ImageTest::_size()
        {
            {
                const Size size;
                DTK_ASSERT(0 == size.w);
                DTK_ASSERT(0 == size.h);
                DTK_ASSERT(1.F == size.pixelAspectRatio);
                DTK_ASSERT(!size.isValid());
                DTK_ASSERT(0.F == size.getAspect());
            }
            {
                const Size size(1, 2);
                DTK_ASSERT(1 == size.w);
                DTK_ASSERT(2 == size.h);
                DTK_ASSERT(1.F == size.pixelAspectRatio);
                DTK_ASSERT(size.isValid());
                DTK_ASSERT(.5F == size.getAspect());
            }
            {
                DTK_ASSERT(Size(1, 2) == Size(1, 2));
                DTK_ASSERT(Size(1, 2) != Size(1, 3));
                DTK_ASSERT(Size(1, 2) < Size(1, 3));
            }
            {
                const Size size(1, 2, 2.F);
                std::stringstream ss;
                ss << size;
                Size size2;
                ss >> size2;
                DTK_ASSERT(size == size2);
            }
            {
                DTK_ASSERT(math::Box2i(0, 0, 100, 100) == getBox(1.F, math::Box2i(0, 0, 100, 100)));
                DTK_ASSERT(math::Box2i(50, 0, 100, 100) == getBox(1.F, math::Box2i(0, 0, 200, 100)));
                DTK_ASSERT(math::Box2i(0, 50, 100, 100) == getBox(1.F, math::Box2i(0, 0, 100, 200)));
            }
        }

        void ImageTest::_enums()
        {
            _enum<PixelType>("PixelType", getPixelTypeEnums);
            _enum<VideoLevels>("VideoLevels", getVideoLevelsEnums);
            _enum<YUVCoefficients>("YUVCoefficients", getYUVCoefficientsEnums);
            for (auto i : getYUVCoefficientsEnums())
            {
                _print(dtk::Format("%0: %1").arg(getLabel(i)).arg(getYUVCoefficients(i)));
            }
        }

        void ImageTest::_info()
        {
            {
                const Info info;
                DTK_ASSERT(Size() == info.size);
                DTK_ASSERT(PixelType::None == info.pixelType);
                DTK_ASSERT(!info.isValid());
            }
            {
                const Info info(Size(1, 2), PixelType::L_U8);
                DTK_ASSERT(Size(1, 2) == info.size);
                DTK_ASSERT(PixelType::L_U8 == info.pixelType);
                DTK_ASSERT(info.isValid());
            }
            {
                const Info info(1, 2, PixelType::L_U8);
                DTK_ASSERT(Size(1, 2) == info.size);
                DTK_ASSERT(PixelType::L_U8 == info.pixelType);
                DTK_ASSERT(info.isValid());
            }
            {
                Info info(1, 2, PixelType::L_U8);
                info.layout.alignment = 1;
                DTK_ASSERT(getDataByteCount(info) == 2);
            }
            {
                Info info(1, 2, PixelType::L_U8);
                info.layout.alignment = 2;
                DTK_ASSERT(getDataByteCount(info) == 4);
            }
            {
                Info info(1, 2, PixelType::L_U8);
                info.layout.alignment = 4;
                DTK_ASSERT(getDataByteCount(info) == 8);
            }
            {
                Info info(1, 2, PixelType::L_U16);
                info.layout.alignment = 4;
                DTK_ASSERT(getDataByteCount(info) == 8);
            }
            {
                DTK_ASSERT(Info(1, 2, PixelType::L_U8) == Info(1, 2, PixelType::L_U8));
                DTK_ASSERT(Info(1, 2, PixelType::L_U8) != Info(1, 2, PixelType::L_U16));
            }
        }

        void ImageTest::_util()
        {
            for (auto i : getPixelTypeEnums())
            {
                std::stringstream ss;
                ss << i << " channel count: " << getChannelCount(i);
                _print(ss.str());
            }
            for (auto i : getPixelTypeEnums())
            {
                std::stringstream ss;
                ss << i << " bit depth: " << getBitDepth(i);
                _print(ss.str());
            }
            for (size_t c : { 1, 2, 3, 4 })
            {
                for (size_t b : { 8, 10, 16, 32 })
                {
                    std::stringstream ss;
                    ss << c << "/" << b << " int type: " << getIntType(c, b);
                    _print(ss.str());
                }
            }
            for (size_t c : { 1, 2, 3, 4 })
            {
                for (size_t b : { 16, 32 })
                {
                    std::stringstream ss;
                    ss << c << "/" << b << " float type: " << getFloatType(c, b);
                    _print(ss.str());
                }
            }
            {
                DTK_ASSERT(getClosest(
                    PixelType::None, {}) == PixelType::None);
                DTK_ASSERT(getClosest(
                    PixelType::L_U16, { PixelType::L_U8 }) == PixelType::L_U8);
                DTK_ASSERT(getClosest(
                    PixelType::L_U16, { PixelType::L_U8, PixelType::L_U16 }) == PixelType::L_U16);
                DTK_ASSERT(getClosest(
                    PixelType::L_U16, { PixelType::L_U8, PixelType::L_U16, PixelType::L_U32 }) == PixelType::L_U16);
                DTK_ASSERT(getClosest(
                    PixelType::RGB_U16, { PixelType::L_U8, PixelType::L_U16, PixelType::L_U32 }) == PixelType::L_U16);
                DTK_ASSERT(getClosest(
                    PixelType::L_U16, { PixelType::RGB_U8, PixelType::RGB_U16, PixelType::RGB_U32 }) == PixelType::RGB_U16);
            }
            for (auto i : getPixelTypeEnums())
            {
                const Info info(1, 2, i);
                std::stringstream ss;
                ss << info.size << " " << info.pixelType << " data byte count: " << getDataByteCount(info);
                _print(ss.str());
            }
        }

        void ImageTest::_image()
        {
            {
                const Info info(1, 2, PixelType::L_U8);
                auto image = Image::create(info);
                image->zero();
                DTK_ASSERT(image->getInfo() == info);
                DTK_ASSERT(image->getSize() == info.size);
                DTK_ASSERT(image->getWidth() == info.size.w);
                DTK_ASSERT(image->getHeight() == info.size.h);
                DTK_ASSERT(image->getAspect() == .5F);
                DTK_ASSERT(image->getPixelType() == info.pixelType);
                DTK_ASSERT(image->isValid());
                DTK_ASSERT(image->getData());
                DTK_ASSERT(static_cast<const Image*>(image.get())->getData());
            }
            {
                auto image = Image::create(Size(1, 2), PixelType::L_U8);
                DTK_ASSERT(image->getWidth() == 1);
                DTK_ASSERT(image->getHeight() == 2);
                DTK_ASSERT(image->getPixelType() == PixelType::L_U8);
            }
            {
                auto image = Image::create(1, 2, PixelType::L_U8);
                DTK_ASSERT(image->getWidth() == 1);
                DTK_ASSERT(image->getHeight() == 2);
                DTK_ASSERT(image->getPixelType() == PixelType::L_U8);
            }
        }

        void ImageTest::_serialize()
        {
            {
                const Size s(1, 2);
                nlohmann::json json;
                to_json(json, s);
                Size s2;
                from_json(json, s2);
                DTK_ASSERT(s == s2);
            }
            {
                const Size s(1, 2);
                std::stringstream ss;
                ss << s;
                Size s2;
                ss >> s2;
                DTK_ASSERT(s == s2);
            }
            {
                const Size s(1, 2, 2.F);
                std::stringstream ss;
                ss << s;
                Size s2;
                ss >> s2;
                DTK_ASSERT(s == s2);
            }
            try
            {
                Size s;
                std::stringstream ss;
                ss >> s;
                DTK_ASSERT(false);
            }
            catch (const std::exception&)
            {}
            try
            {
                Size s;
                std::stringstream ss("...");
                ss >> s;
                DTK_ASSERT(false);
            }
            catch (const std::exception&)
            {}
        }
    }
}
