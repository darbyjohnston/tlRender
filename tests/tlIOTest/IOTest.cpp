// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIOTest/IOTest.h>

#include <tlIO/System.h>

#include <ftk/Core/Format.h>
#include <ftk/Core/String.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        IOTest::IOTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "IOTest::IOTest")
        {}

        std::shared_ptr<IOTest> IOTest::create(const std::shared_ptr<ftk::Context>& context)
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
                FTK_ASSERT(!time::isValid(v.time));
                FTK_ASSERT(!v.image);
            }
            {
                const auto time = OTIO_NS::RationalTime(1.0, 24.0);
                const uint16_t layer = 1;
                const auto image = ftk::Image::create(160, 80, ftk::ImageType::L_U8);
                const VideoData v(time, layer, image);
                FTK_ASSERT(time.strictly_equal(v.time));
                FTK_ASSERT(layer == v.layer);
                FTK_ASSERT(image == v.image);
            }
            {
                const auto time = OTIO_NS::RationalTime(1.0, 24.0);
                const uint16_t layer = 1;
                const auto image = ftk::Image::create(16, 16, ftk::ImageType::L_U8);
                const VideoData a(time, layer, image);
                VideoData b(time, layer, image);
                FTK_ASSERT(a == b);
                b.time = OTIO_NS::RationalTime(2.0, 24.0);
                FTK_ASSERT(a != b);
                FTK_ASSERT(a < b);
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
                ftk::ImageInfo getInfo(
                    const ftk::ImageInfo&,
                    const io::Options & = io::Options()) const override
                {
                    return ftk::ImageInfo();
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
                ss << "Plugins: " << ftk::join(plugins, ", ");
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
                    FTK_ASSERT(readSystem->getPlugin(file::Path("test" + plugin.first)) == plugin.second);
                }
                FTK_ASSERT(!readSystem->getPlugin(file::Path()));
                FTK_ASSERT(!readSystem->getPlugin<DummyReadPlugin>());
            }
            {
                std::vector<std::string> extensions;
                for (const auto& extension : readSystem->getExtensions())
                {
                    extensions.push_back(extension);
                }
                std::stringstream ss;
                ss << "Extensions: " << ftk::join(extensions, ", ");
                _print(ss.str());
            }
            FTK_ASSERT(!readSystem->read(file::Path()));
            auto writeSystem = _context->getSystem<WriteSystem>();
            FTK_ASSERT(!writeSystem->write(file::Path(), Info()));
        }
    }
}
