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
                switch (oiio.nchannels)
                {
                case 1:
                    switch (oiio.format.basetype)
                    {
                    case OIIO::TypeDesc::UINT8:  out = ftk::ImageType::L_U8;  break;
                    case OIIO::TypeDesc::UINT16: out = ftk::ImageType::L_U16; break;
                    case OIIO::TypeDesc::UINT32: out = ftk::ImageType::L_U32; break;
                    case OIIO::TypeDesc::HALF:   out = ftk::ImageType::L_F16; break;
                    case OIIO::TypeDesc::FLOAT:  out = ftk::ImageType::L_F32; break;
                    default: break;
                    }
                    break;
                case 2:
                    switch (oiio.format.basetype)
                    {
                    case OIIO::TypeDesc::UINT8:  out = ftk::ImageType::LA_U8;  break;
                    case OIIO::TypeDesc::UINT16: out = ftk::ImageType::LA_U16; break;
                    case OIIO::TypeDesc::UINT32: out = ftk::ImageType::LA_U32; break;
                    case OIIO::TypeDesc::HALF:   out = ftk::ImageType::LA_F16; break;
                    case OIIO::TypeDesc::FLOAT:  out = ftk::ImageType::LA_F32; break;
                    default: break;
                    }
                    break;
                case 3:
                    switch (oiio.format.basetype)
                    {
                    case OIIO::TypeDesc::UINT8:  out = ftk::ImageType::RGB_U8;  break;
                    case OIIO::TypeDesc::UINT16: out = ftk::ImageType::RGB_U16; break;
                    case OIIO::TypeDesc::UINT32: out = ftk::ImageType::RGB_U32; break;
                    case OIIO::TypeDesc::HALF:   out = ftk::ImageType::RGB_F16; break;
                    case OIIO::TypeDesc::FLOAT:  out = ftk::ImageType::RGB_F32; break;
                    default: break;
                    }
                    break;
                case 4:
                    switch (oiio.format.basetype)
                    {
                    case OIIO::TypeDesc::UINT8:  out = ftk::ImageType::RGBA_U8;  break;
                    case OIIO::TypeDesc::UINT16: out = ftk::ImageType::RGBA_U16; break;
                    case OIIO::TypeDesc::UINT32: out = ftk::ImageType::RGBA_U32; break;
                    case OIIO::TypeDesc::HALF:   out = ftk::ImageType::RGBA_F16; break;
                    case OIIO::TypeDesc::FLOAT:  out = ftk::ImageType::RGBA_F32; break;
                    default: break;
                    }
                    break;
                default: break;
                }
                return out;
            }
        }

        io::Info Read::_getInfo(
            const std::string& fileName,
            const ftk::InMemoryFile* memory)
        {
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
                std::stringstream ss;
                ss << "Cannot open file: " << fileName;
                throw std::runtime_error(ss.str());
            }

            const auto& oiioSpec = oiioInput->spec();
            const ftk::ImageType imageType = fromOIIO(oiioSpec);
            if (ftk::ImageType::None == imageType)
            {
                std::stringstream ss;
                ss << "Unsupported file: " << fileName;
                throw std::runtime_error(ss.str());
            }

            io::Info out;
            ftk::ImageInfo imageInfo(oiioSpec.width, oiioSpec.height, imageType);
            imageInfo.layout.mirror.y = true;
            out.video.push_back(imageInfo);
            out.videoTime = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                OTIO_NS::RationalTime(_startFrame, _defaultSpeed),
                OTIO_NS::RationalTime(_endFrame, _defaultSpeed));
            return out;
        }

        io::VideoData Read::_readVideo(
            const std::string& fileName,
            const ftk::InMemoryFile* memory,
            const OTIO_NS::RationalTime& time,
            const io::Options&)
        {
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
                std::stringstream ss;
                ss << "Cannot open file: " << fileName;
                throw std::runtime_error(ss.str());
            }

            const auto& oiioSpec = oiioInput->spec();
            const ftk::ImageType imageType = fromOIIO(oiioSpec);
            if (ftk::ImageType::None == imageType)
            {
                std::stringstream ss;
                ss << "Unsupported file: " << fileName;
                throw std::runtime_error(ss.str());
            }

            ftk::ImageInfo imageInfo(oiioSpec.width, oiioSpec.height, imageType);
            imageInfo.layout.mirror.y = true;
            ftk::ImageTags tags;
            for (const auto& i : oiioSpec.extra_attribs)
            {
                tags[std::string(i.name())] = i.get_string();
            }

            io::VideoData out;
            out.time = time;
            out.image = ftk::Image::create(imageInfo);
            out.image->setTags(tags);
            oiioInput->read_image(
                0,
                0,
                0,
                oiioSpec.nchannels,
                oiioSpec.format,
                out.image->getData());
            return out;
        }
    }
}
