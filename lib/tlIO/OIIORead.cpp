// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/OIIO.h>

#include <ftk/Core/Format.h>
#include <ftk/Core/Memory.h>
#include <ftk/Core/String.h>

#include <OpenImageIO/filesystem.h>
#include <OpenImageIO/imagebufalgo.h>

namespace tl
{
    namespace oiio
    {
        void Read::_init(
            const file::Path& path,
            const std::vector<ftk::InMemoryFile>& memory,
            const io::Options& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            ISequenceRead::_init(path, memory, options, logSystem);
        }

        Read::Read()
        {}

        Read::~Read()
        {
            _finish();
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const io::Options& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const std::vector<ftk::InMemoryFile>& memory,
            const io::Options& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, logSystem);
            return out;
        }

        namespace
        {
            ftk::ImageType fromOIIO(const OIIO::ImageSpec& oiio)
            {
                ftk::ImageType out = ftk::ImageType::None;
                if (1 == oiio.nchannels)
                {
                    switch (oiio.format.basetype)
                    {
                    case OIIO::TypeDesc::UINT8:  out = ftk::ImageType::L_U8;  break;
                    case OIIO::TypeDesc::UINT16: out = ftk::ImageType::L_U16; break;
                    case OIIO::TypeDesc::UINT32: out = ftk::ImageType::L_U32; break;
                    case OIIO::TypeDesc::HALF:   out = ftk::ImageType::L_F16; break;
                    case OIIO::TypeDesc::FLOAT:  out = ftk::ImageType::L_F32; break;
                    default: break;
                    }
                }
                else if (2 == oiio.nchannels)
                {
                    switch (oiio.format.basetype)
                    {
                    case OIIO::TypeDesc::UINT8:  out = ftk::ImageType::LA_U8;  break;
                    case OIIO::TypeDesc::UINT16: out = ftk::ImageType::LA_U16; break;
                    case OIIO::TypeDesc::UINT32: out = ftk::ImageType::LA_U32; break;
                    case OIIO::TypeDesc::HALF:   out = ftk::ImageType::LA_F16; break;
                    case OIIO::TypeDesc::FLOAT:  out = ftk::ImageType::LA_F32; break;
                    default: break;
                    }
                }
                else if (3 == oiio.nchannels)
                {
                    switch (oiio.format.basetype)
                    {
                    case OIIO::TypeDesc::UINT8:  out = ftk::ImageType::RGB_U8;  break;
                    case OIIO::TypeDesc::UINT16: out = ftk::ImageType::RGB_U16; break;
                    case OIIO::TypeDesc::UINT32: out = ftk::ImageType::RGB_U32; break;
                    case OIIO::TypeDesc::HALF:   out = ftk::ImageType::RGB_F16; break;
                    case OIIO::TypeDesc::FLOAT:  out = ftk::ImageType::RGB_F32; break;
                    default: break;
                    }
                }
                else if (oiio.nchannels >= 4)
                {
                    switch (oiio.format.basetype)
                    {
                    case OIIO::TypeDesc::UINT8:  out = ftk::ImageType::RGBA_U8;  break;
                    case OIIO::TypeDesc::UINT16: out = ftk::ImageType::RGBA_U16; break;
                    case OIIO::TypeDesc::UINT32: out = ftk::ImageType::RGBA_U32; break;
                    case OIIO::TypeDesc::HALF:   out = ftk::ImageType::RGBA_F16; break;
                    case OIIO::TypeDesc::FLOAT:  out = ftk::ImageType::RGBA_F32; break;
                    default: break;
                    }
                }
                return out;
            }
        }

        io::Info Read::_getInfo(
            const std::string& fileName,
            const ftk::InMemoryFile* memory)
        {
            // Open the file.
            std::unique_ptr<OIIO::Filesystem::IOMemReader> oiioMemReader;
            if (memory)
            {
                oiioMemReader.reset(new OIIO::Filesystem::IOMemReader(memory->p, memory->size));
            }
            const auto oiioInput = OIIO::ImageInput::open(
                fileName,
                nullptr,
                oiioMemReader.get());
            if (!oiioInput)
            {
                throw std::runtime_error(OIIO::geterror());
            }

            // Get file information.
            io::Info out;
            auto oiioSpec = oiioInput->spec();
            for (const auto& i : oiioSpec.extra_attribs)
            {
                out.tags[std::string(i.name())] = i.get_string();
            }
            for (int sub = 0; oiioInput->seek_subimage(sub, 0); ++sub)
            {
                oiioSpec = oiioInput->spec();
                const ftk::ImageType imageType = fromOIIO(oiioSpec);
                if (ftk::ImageType::None == imageType)
                {
                    std::stringstream ss;
                    ss << "Unsupported file: " << fileName;
                    throw std::runtime_error(ss.str());
                }
                ftk::ImageInfo imageInfo(oiioSpec.width, oiioSpec.height, imageType);
                imageInfo.name = "";
                for (int j = 0; j < oiioSpec.nchannels; ++j)
                {
                    imageInfo.name += oiioSpec.channelnames[j];
                }
                imageInfo.layout.mirror.y = true;
                out.video.push_back(imageInfo);
            }
            out.videoTime = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                OTIO_NS::RationalTime(_startFrame, _defaultSpeed),
                OTIO_NS::RationalTime(_endFrame, _defaultSpeed));
            return out;
        }

        io::VideoData Read::_readVideo(
            const std::string& fileName,
            const ftk::InMemoryFile* memory,
            const OTIO_NS::RationalTime& time,
            const io::Options& options)
        {
            // Open the file.
            std::unique_ptr<OIIO::Filesystem::IOMemReader> oiioMemReader;
            if (memory)
            {
                oiioMemReader.reset(new OIIO::Filesystem::IOMemReader(memory->p, memory->size));
            }
            const auto oiioInput = OIIO::ImageInput::open(
                fileName,
                nullptr,
                oiioMemReader.get());
            if (!oiioInput)
            {
                throw std::runtime_error(OIIO::geterror());
            }

            // Find the layer.
            int layer = 0;
            const auto i = options.find("Layer");
            if (i != options.end())
            {
                layer = std::atoi(i->second.c_str());
            }
            if (!oiioInput->seek_subimage(layer, 0))
            {
                std::stringstream ss;
                ss << "Cannot open layer: " << layer;
                throw std::runtime_error(ss.str());
            }

            // Get file information.
            const auto& oiioSpec = oiioInput->spec();
            const ftk::ImageType imageType = fromOIIO(oiioSpec);
            if (ftk::ImageType::None == imageType)
            {
                std::stringstream ss;
                ss << "Unsupported file: " << fileName;
                throw std::runtime_error(ss.str());
            }

            // Get the tags.
            ftk::ImageInfo imageInfo(oiioSpec.width, oiioSpec.height, imageType);
            imageInfo.layout.mirror.y = true;
            ftk::ImageTags tags;
            for (const auto& i : oiioSpec.extra_attribs)
            {
                tags[std::string(i.name())] = i.get_string();
            }

            // Read the image.
            io::VideoData out;
            out.time = time;
            out.image = ftk::Image::create(imageInfo);
            out.image->setTags(tags);
            if (!oiioInput->read_image(
                layer,
                0,
                0,
                ftk::getChannelCount(imageType),
                oiioSpec.format,
                out.image->getData()))
            {
                throw std::runtime_error(OIIO::geterror());
            }
            return out;
        }
    }
}
