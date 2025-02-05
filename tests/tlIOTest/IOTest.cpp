// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIOTest/IOTest.h>

#include <tlIO/System.h>

#include <dtk/core/Format.h>
#include <dtk/core/String.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        IOTest::IOTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "IOTest::IOTest")
        {}

        std::shared_ptr<IOTest> IOTest::create(const std::shared_ptr<dtk::Context>& context)
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
                DTK_ASSERT(!time::isValid(v.time));
                DTK_ASSERT(!v.image);
            }
            {
                const auto time = OTIO_NS::RationalTime(1.0, 24.0);
                const uint16_t layer = 1;
                const auto image = dtk::Image::create(160, 80, dtk::ImageType::L_U8);
                const VideoData v(time, layer, image);
                DTK_ASSERT(time.strictly_equal(v.time));
                DTK_ASSERT(layer == v.layer);
                DTK_ASSERT(image == v.image);
            }
            {
                const auto time = OTIO_NS::RationalTime(1.0, 24.0);
                const uint16_t layer = 1;
                const auto image = dtk::Image::create(16, 16, dtk::ImageType::L_U8);
                const VideoData a(time, layer, image);
                VideoData b(time, layer, image);
                DTK_ASSERT(a == b);
                b.time = OTIO_NS::RationalTime(2.0, 24.0);
                DTK_ASSERT(a != b);
                DTK_ASSERT(a < b);
            }
        }

        namespace
        {
            class DummyPlugin : public IPlugin
            {
            public:
                std::shared_ptr<IRead> read(
                    const file::Path&,
                    const Options& = Options()) override
                {
                    return nullptr;
                }

                image::Info getWriteInfo(
                    const image::Info&,
                    const io::Options& = io::Options()) const override
                {
                    return image::Info();
                }

                std::shared_ptr<IWrite> write(
                    const file::Path&,
                    const Info&,
                    const Options& = Options()) override
                {
                    return nullptr;
                }
            };
        }

        void IOTest::_ioSystem()
        {
            auto system = _context->getSystem<System>();
            {
                std::vector<std::string> plugins;
                for (const auto& plugin : system->getPlugins())
                {
                    plugins.push_back(plugin->getName());
                }
                std::stringstream ss;
                ss << "Plugins: " << dtk::join(plugins, ", ");
                _print(ss.str());
            }
            {
                std::map<std::string, std::shared_ptr<IPlugin> > plugins;
                for (const auto& plugin : system->getPlugins())
                {
                    const auto& extensions = plugin->getExtensions();
                    if (!extensions.empty())
                    {
                        plugins[*(extensions.begin())] = plugin;
                    }
                }
                for (const auto& plugin : plugins)
                {
                    DTK_ASSERT(system->getPlugin(file::Path("test" + plugin.first)) == plugin.second);
                }
                DTK_ASSERT(!system->getPlugin(file::Path()));
                DTK_ASSERT(!system->getPlugin<DummyPlugin>());
            }
            {
                std::vector<std::string> extensions;
                for (const auto& extension : system->getExtensions())
                {
                    extensions.push_back(extension);
                }
                std::stringstream ss;
                ss << "Extensions: " << dtk::join(extensions, ", ");
                _print(ss.str());
            }
            DTK_ASSERT(!system->read(file::Path()));
            DTK_ASSERT(!system->write(file::Path(), Info()));
        }
    }
}
