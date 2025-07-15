// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIOTest/IOTest.h>

#include <tlIO/System.h>

#include <feather-tk/core/Format.h>
#include <feather-tk/core/String.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        IOTest::IOTest(const std::shared_ptr<feather_tk::Context>& context) :
            ITest(context, "IOTest::IOTest")
        {}

        std::shared_ptr<IOTest> IOTest::create(const std::shared_ptr<feather_tk::Context>& context)
        {
            return std::shared_ptr<IOTest>(new IOTest(context));
        }

        void IOTest::run()
        {
            _videoData();
            _ioSystem();
        }

        void IOTest::_videoData()
        {
            {
                const VideoData v;
                FEATHER_TK_ASSERT(!time::isValid(v.time));
                FEATHER_TK_ASSERT(!v.image);
            }
            {
                const auto time = OTIO_NS::RationalTime(1.0, 24.0);
                const uint16_t layer = 1;
                const auto image = feather_tk::Image::create(160, 80, feather_tk::ImageType::L_U8);
                const VideoData v(time, layer, image);
                FEATHER_TK_ASSERT(time.strictly_equal(v.time));
                FEATHER_TK_ASSERT(layer == v.layer);
                FEATHER_TK_ASSERT(image == v.image);
            }
            {
                const auto time = OTIO_NS::RationalTime(1.0, 24.0);
                const uint16_t layer = 1;
                const auto image = feather_tk::Image::create(16, 16, feather_tk::ImageType::L_U8);
                const VideoData a(time, layer, image);
                VideoData b(time, layer, image);
                FEATHER_TK_ASSERT(a == b);
                b.time = OTIO_NS::RationalTime(2.0, 24.0);
                FEATHER_TK_ASSERT(a != b);
                FEATHER_TK_ASSERT(a < b);
            }
        }

        namespace
        {
            class DummyReadPlugin : public IReadPlugin
            {
            public:
                std::shared_ptr<IRead> read(
                    const file::Path&,
                    const Options& = Options()) override
                {
                    return nullptr;
                }
            };

            class DummyWritePlugin : public IWritePlugin
            {
            public:
                feather_tk::ImageInfo getInfo(
                    const feather_tk::ImageInfo&,
                    const io::Options & = io::Options()) const override
                {
                    return feather_tk::ImageInfo();
                }

                std::shared_ptr<IWrite> write(
                    const file::Path&,
                    const Info&,
                    const Options & = Options()) override
                {
                    return nullptr;
                }
            };
        }

        void IOTest::_ioSystem()
        {
            auto readSystem = _context->getSystem<ReadSystem>();
            {
                std::vector<std::string> plugins;
                for (const auto& plugin : readSystem->getPlugins())
                {
                    plugins.push_back(plugin->getName());
                }
                std::stringstream ss;
                ss << "Plugins: " << feather_tk::join(plugins, ", ");
                _print(ss.str());
            }
            {
                std::map<std::string, std::shared_ptr<IPlugin> > plugins;
                for (const auto& plugin : readSystem->getPlugins())
                {
                    const auto& extensions = plugin->getExtensions();
                    if (!extensions.empty())
                    {
                        plugins[*(extensions.begin())] = plugin;
                    }
                }
                for (const auto& plugin : plugins)
                {
                    FEATHER_TK_ASSERT(readSystem->getPlugin(file::Path("test" + plugin.first)) == plugin.second);
                }
                FEATHER_TK_ASSERT(!readSystem->getPlugin(file::Path()));
                FEATHER_TK_ASSERT(!readSystem->getPlugin<DummyReadPlugin>());
            }
            {
                std::vector<std::string> extensions;
                for (const auto& extension : readSystem->getExtensions())
                {
                    extensions.push_back(extension);
                }
                std::stringstream ss;
                ss << "Extensions: " << feather_tk::join(extensions, ", ");
                _print(ss.str());
            }
            FEATHER_TK_ASSERT(!readSystem->read(file::Path()));
            auto writeSystem = _context->getSystem<WriteSystem>();
            FEATHER_TK_ASSERT(!writeSystem->write(file::Path(), Info()));
        }
    }
}
