// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/DPX.h>

#include <tlrCore/Cineon.h>
#include <tlrCore/Error.h>
#include <tlrCore/Memory.h>
#include <tlrCore/StringFormat.h>

#include <array>
#include <cstring>
#include <sstream>

namespace tlr
{
    namespace dpx
    {
        TLR_ENUM_IMPL(
            Version,
            "1.0",
            "2.0");

        TLR_ENUM_IMPL(
            Endian,
            "Auto",
            "MSB",
            "LSB");

        TLR_ENUM_IMPL(
            Orient,
            "LeftRightTopBottom",
            "RightLeftTopBottom",
            "LeftRightBottomTop",
            "RightLeftBottomTop",
            "TopBottomLeftRight",
            "TopBottomRightLeft",
            "BottomTopLeftRight",
            "BottomTopRightLeft");

        TLR_ENUM_IMPL(
            Transfer,
            "User",
            "FilmPrint",
            "Linear",
            "Log",
            "Video",
            "SMPTE_274M",
            "ITU_R_709_4",
            "ITU_R_601_5_B_OR_G",
            "ITU_R_601_5_M",
            "NTSC",
            "PAL",
            "Z",
            "ZHomogeneous");

        TLR_ENUM_IMPL(
            Components,
            "Pack",
            "TypeA",
            "TypeB");

        namespace
        {
            void zero(char* in, int size)
            {
                memset(in, 0, size);
            }
        }

        Header::Header()
        {
            memset(&file, 0xff, sizeof(Header::File));
            zero(file.version, 8);
            zero(file.name, 100);
            zero(file.time, 24);
            zero(file.creator, 100);
            zero(file.project, 200);
            zero(file.copyright, 200);

            memset(&image, 0xff, sizeof(Header::Image));

            memset(&source, 0xff, sizeof(Header::Source));
            zero(source.file, 100);
            zero(source.time, 24);
            zero(source.inputDevice, 32);
            zero(source.inputSerial, 32);

            memset(&film, 0xff, sizeof(Header::Film));
            zero(film.id, 2);
            zero(film.type, 2);
            zero(film.offset, 2);
            zero(film.prefix, 6);
            zero(film.count, 4);
            zero(film.format, 32);
            zero(film.frameId, 32);
            zero(film.slate, 100);

            memset(&tv, 0xff, sizeof(Header::TV));
        }

        void Header::convertEndian()
        {
            memory::endian(&file.imageOffset, 1, 4);
            memory::endian(&file.size, 1, 4);
            memory::endian(&file.dittoKey, 1, 4);
            memory::endian(&file.headerSize, 1, 4);
            memory::endian(&file.industryHeaderSize, 1, 4);
            memory::endian(&file.userHeaderSize, 1, 4);
            memory::endian(&file.encryptionKey, 1, 4);

            memory::endian(&image.orient, 1, 2);
            memory::endian(&image.elemSize, 1, 2);
            memory::endian(image.size, 2, 4);
            for (size_t i = 0; i < 8; ++i)
            {
                memory::endian(&image.elem[i].dataSign, 1, 4);
                memory::endian(&image.elem[i].lowData, 1, 4);
                memory::endian(&image.elem[i].lowQuantity, 1, 4);
                memory::endian(&image.elem[i].highData, 1, 4);
                memory::endian(&image.elem[i].highQuantity, 1, 4);
                memory::endian(&image.elem[i].packing, 1, 2);
                memory::endian(&image.elem[i].encoding, 1, 2);
                memory::endian(&image.elem[i].dataOffset, 1, 4);
                memory::endian(&image.elem[i].linePadding, 1, 4);
                memory::endian(&image.elem[i].elemPadding, 1, 4);
            }

            memory::endian(source.offset, 2, 4);
            memory::endian(source.center, 2, 4);
            memory::endian(source.size, 2, 4);
            memory::endian(source.border, 4, 2);
            memory::endian(source.pixelAspect, 2, 4);
            memory::endian(source.scanSize, 2, 4);

            memory::endian(&film.frame, 1, 4);
            memory::endian(&film.sequence, 1, 4);
            memory::endian(&film.hold, 1, 4);
            memory::endian(&film.frameRate, 1, 4);
            memory::endian(&film.shutter, 1, 4);

            memory::endian(&tv.timecode, 1, 4);
            memory::endian(&tv.userBits, 1, 4);
            memory::endian(tv.sampleRate, 2, 4);
            memory::endian(&tv.frameRate, 1, 4);
            memory::endian(&tv.timeOffset, 1, 4);
            memory::endian(&tv.gamma, 1, 4);
            memory::endian(&tv.blackLevel, 1, 4);
            memory::endian(&tv.blackGain, 1, 4);
            memory::endian(&tv.breakpoint, 1, 4);
            memory::endian(&tv.whiteLevel, 1, 4);
            memory::endian(&tv.integrationTimes, 1, 4);
        }

        namespace
        {
            //! These hard-coded values are meant to catch uninitialized values.
            const int32_t _intMax   = 1000000;
            const float   _floatMax = 1000000.F;
            const float   _minSpeed = .000001F;

            bool isValid(const uint8_t* in)
            {
                return *in != 0xff;
            }

            bool isValid(const uint16_t* in)
            {
                return *in != 0xffff;
            }

            bool isValid(const uint32_t* in)
            {
                return *in != 0xffffffff && *in < static_cast<uint32_t>(_intMax);
            }

            bool isValid(const float* in)
            {
                return
                    *(reinterpret_cast<const uint32_t*>(in)) != 0xffffffff &&
                    *in > -_floatMax &&
                    *in < _floatMax;
            }
        }

        Header Header::read(
            const std::shared_ptr<file::FileIO>& io,
            avio::Info& info,
            Transfer& transfer)
        {
            Header out;

            // Read the file section of the header.
            io->read(&out.file, sizeof(Header::File));

            // Check the magic number.
            memory::Endian fileEndian = memory::Endian::First;
            if (0 == memcmp(&out.file.magic, magic[0], 4))
            {
                fileEndian = memory::Endian::MSB;
            }
            else if (0 == memcmp(&out.file.magic, magic[1], 4))
            {
                fileEndian = memory::Endian::LSB;
            }
            else
            {
                throw std::runtime_error(string::Format("{0}: {1}").
                    arg(io->getFileName()).
                    arg("Bad magic number"));
            }

            // Read the reset of the header.
            io->read(&out.image, sizeof(Header::Image));
            io->read(&out.source, sizeof(Header::Source));
            io->read(&out.film, sizeof(Header::Film));
            io->read(&out.tv, sizeof(Header::TV));

            // Flip the endian of the data if necessary.
            imaging::Info imageInfo;
            if (fileEndian != memory::getEndian())
            {
                io->setEndianConversion(true);
                out.convertEndian();
                imageInfo.layout.endian = memory::opposite(memory::getEndian());
            }

            // Image information.
            if (out.image.elemSize != 1)
            {
                throw std::runtime_error(string::Format("{0}: {1}").
                    arg(io->getFileName()).
                    arg("Unsupported file"));
            }
            imageInfo.size.w = out.image.size[0];
            imageInfo.size.h = out.image.size[1];

            switch (static_cast<Orient>(out.image.orient))
            {
            case Orient::LeftRightBottomTop:
                imageInfo.layout.mirror.y = true;
                break;
            case Orient::RightLeftTopBottom:
                imageInfo.layout.mirror.x = true;
                break;
            case Orient::RightLeftBottomTop:
                imageInfo.layout.mirror.x = true;
                imageInfo.layout.mirror.y = true;
                break;
            default: break;
            }

            switch (static_cast<Components>(out.image.elem[0].packing))
            {
            case Components::Pack:
            {
                uint8_t channels = 0;
                switch (static_cast<Descriptor>(out.image.elem[0].descriptor))
                {
                case Descriptor::L:    channels = 1; break;
                case Descriptor::RGB:  channels = 3; break;
                case Descriptor::RGBA: channels = 4; break;
                default: break;
                }
                imageInfo.pixelType = imaging::getIntType(channels, out.image.elem[0].bitDepth);
            }
            break;
            case Components::TypeA:
                switch (out.image.elem[0].bitDepth)
                {
                case 10:
                    if (Descriptor::RGB == static_cast<Descriptor>(out.image.elem[0].descriptor))
                    {
                        imageInfo.pixelType = imaging::PixelType::RGB_U10;
                        imageInfo.layout.alignment = 4;
                    }
                    break;
                case 16:
                {
                    uint8_t channels = 0;
                    switch (static_cast<Descriptor>(out.image.elem[0].descriptor))
                    {
                    case Descriptor::L:    channels = 1; break;
                    case Descriptor::RGB:  channels = 3; break;
                    case Descriptor::RGBA: channels = 4; break;
                    default: break;
                    }
                    imageInfo.pixelType = imaging::getIntType(channels, out.image.elem[0].bitDepth);
                    break;
                }
                default: break;
                }
                break;
            default: break;
            }
            if (imaging::PixelType::None == imageInfo.pixelType)
            {
                throw std::runtime_error(string::Format("{0}: {1}").
                    arg(io->getFileName()).
                    arg("Unsupported file"));
            }
            const size_t dataByteCount = imaging::getDataByteCount(imageInfo);
            const size_t ioSize = io->getSize();
            if (dataByteCount > ioSize - out.file.imageOffset)
            {
                throw std::runtime_error(string::Format("{0}: {1}").
                    arg(io->getFileName()).
                    arg("Incomplete file"));
            }

            if (out.image.elem[0].encoding)
            {
                throw std::runtime_error(string::Format("{0}: {1}").
                    arg(io->getFileName()).
                    arg("Unsupported file"));
            }

            if (isValid(&out.image.elem[0].linePadding) && out.image.elem[0].linePadding)
            {
                throw std::runtime_error(string::Format("{0}: {1}").
                    arg(io->getFileName()).
                    arg("Unsupported file"));
            }

            if (Transfer::FilmPrint == static_cast<Transfer>(out.image.elem[0].transfer))
            {
                transfer = Transfer::FilmPrint;
            }

            info.video.push_back(imageInfo);

            // Tags.
            if (cineon::isValid(out.file.time, 24))
            {
                info.tags["Time"] = cineon::toString(out.file.time, 24);
            }
            if (cineon::isValid(out.file.creator, 100))
            {
                info.tags["Creator"] = cineon::toString(out.file.creator, 100);
            }
            if (cineon::isValid(out.file.project, 200))
            {
                info.tags["Project"] = cineon::toString(out.file.project, 200);
            }
            if (cineon::isValid(out.file.copyright, 200))
            {
                info.tags["Copyright"] = cineon::toString(out.file.copyright, 200);
            }

            if (isValid(&out.source.offset[0]) && isValid(&out.source.offset[1]))
            {
                std::stringstream ss;
                ss << out.source.offset[0] << " " << out.source.offset[1];
                info.tags["Source Offset"] = ss.str();
            }
            if (isValid(&out.source.center[0]) && isValid(&out.source.center[1]))
            {
                std::stringstream ss;
                ss << out.source.center[0] << " " << out.source.center[1];
                info.tags["Source Center"] = ss.str();
            }
            if (isValid(&out.source.size[0]) && isValid(&out.source.size[1]))
            {
                std::stringstream ss;
                ss << out.source.size[0] << " " << out.source.size[1];
                info.tags["Source Size"] = ss.str();
            }
            if (cineon::isValid(out.source.file, 100))
            {
                info.tags["Source File"] = cineon::toString(out.source.file, 100);
            }
            if (cineon::isValid(out.source.time, 24))
            {
                info.tags["Source Time"] = cineon::toString(out.source.time, 24);
            }
            if (cineon::isValid(out.source.inputDevice, 32))
            {
                info.tags["Source Input Device"] = cineon::toString(out.source.inputDevice, 32);
            }
            if (cineon::isValid(out.source.inputSerial, 32))
            {
                info.tags["Source Input Serial"] = cineon::toString(out.source.inputSerial, 32);
            }
            if (isValid(&out.source.border[0]) && isValid(&out.source.border[1]) &&
                isValid(&out.source.border[2]) && isValid(&out.source.border[3]))
            {
                std::stringstream ss;
                ss << out.source.border[0] << " ";
                ss << out.source.border[1] << " ";
                ss << out.source.border[2] << " ";
                ss << out.source.border[3];
                info.tags["Source Border"] = ss.str();
            }
            if (isValid(&out.source.pixelAspect[0]) && isValid(&out.source.pixelAspect[1]))
            {
                std::stringstream ss;
                ss << out.source.pixelAspect[0] << " " << out.source.pixelAspect[1];
                info.tags["Source Pixel Aspect"] = ss.str();
            }
            if (isValid(&out.source.scanSize[0]) && isValid(&out.source.scanSize[1]))
            {
                std::stringstream ss;
                ss << out.source.scanSize[0] << " " << out.source.scanSize[1];
                info.tags["Source Scan Size"] = ss.str();
            }

            if (cineon::isValid(out.film.id, 2) && cineon::isValid(out.film.type, 2) &&
                cineon::isValid(out.film.offset, 2) && cineon::isValid(out.film.prefix, 6) &&
                cineon::isValid(out.film.count, 4))
            {
                info.tags["Keycode"] = time::keycodeToString(
                    std::stoi(std::string(out.film.id, 2)),
                    std::stoi(std::string(out.film.type, 2)),
                    std::stoi(std::string(out.film.prefix, 6)),
                    std::stoi(std::string(out.film.count, 4)),
                    std::stoi(std::string(out.film.offset, 2)));
            }
            if (cineon::isValid(out.film.format, 32))
            {
                info.tags["Film Format"] = cineon::toString(out.film.format, 32);
            }
            if (isValid(&out.film.frame))
            {
                std::stringstream ss;
                ss << out.film.frame;
                info.tags["Film Frame"] = ss.str();
            }
            if (isValid(&out.film.sequence))
            {
                std::stringstream ss;
                ss << out.film.sequence;
                info.tags["Film Sequence"] = ss.str();
            }
            if (isValid(&out.film.hold))
            {
                std::stringstream ss;
                ss << out.film.hold;
                info.tags["Film Hold"] = ss.str();
            }
            if (isValid(&out.film.frameRate) && out.film.frameRate > _minSpeed)
            {
                info.videoDuration = otime::RationalTime(1.0, out.film.frameRate);
                std::stringstream ss;
                ss << out.film.frameRate;
                info.tags["Film Frame Rate"] = ss.str();
            }
            if (isValid(&out.film.shutter))
            {
                std::stringstream ss;
                ss << out.film.shutter;
                info.tags["Film Shutter"] = ss.str();
            }
            if (cineon::isValid(out.film.frameId, 32))
            {
                info.tags["Film Frame ID"] = cineon::toString(out.film.frameId, 32);
            }
            if (cineon::isValid(out.film.slate, 100))
            {
                info.tags["Film Slate"] = cineon::toString(out.film.slate, 100);
            }

            if (isValid(&out.tv.timecode))
            {
                std::stringstream ss;
                ss << out.tv.timecode;
                info.tags["Timecode"] = ss.str();
            }
            if (isValid(&out.tv.interlace))
            {
                std::stringstream ss;
                ss << static_cast<unsigned int>(out.tv.interlace);
                info.tags["TV Interlace"] = ss.str();
            }
            if (isValid(&out.tv.field))
            {
                std::stringstream ss;
                ss << static_cast<unsigned int>(out.tv.field);
                info.tags["TV Field"] = ss.str();
            }
            if (isValid(&out.tv.videoSignal))
            {
                std::stringstream ss;
                ss << static_cast<unsigned int>(out.tv.videoSignal);
                info.tags["TV Video Signal"] = ss.str();
            }
            if (isValid(&out.tv.sampleRate[0]) && isValid(&out.tv.sampleRate[1]))
            {
                std::stringstream ss;
                ss << out.tv.sampleRate[0] << " " << out.tv.sampleRate[1];
                info.tags["TV Sample Rate"] = ss.str();
            }
            if (isValid(&out.tv.frameRate) && out.tv.frameRate > _minSpeed)
            {
                info.videoDuration = otime::RationalTime(1.0, out.tv.frameRate);
                std::stringstream ss;
                ss << out.tv.frameRate;
                info.tags["TV Frame Rate"] = ss.str();
            }
            if (isValid(&out.tv.timeOffset))
            {
                std::stringstream ss;
                ss << out.tv.timeOffset;
                info.tags["TV Time Offset"] = ss.str();
            }
            if (isValid(&out.tv.gamma))
            {
                std::stringstream ss;
                ss << out.tv.gamma;
                info.tags["TV Gamma"] = ss.str();
            }
            if (isValid(&out.tv.blackLevel))
            {
                std::stringstream ss;
                ss << out.tv.blackLevel;
                info.tags["TV Black Level"] = ss.str();
            }
            if (isValid(&out.tv.blackGain))
            {
                std::stringstream ss;
                ss << out.tv.blackGain;
                info.tags["TV Black Gain"] = ss.str();
            }
            if (isValid(&out.tv.breakpoint))
            {
                std::stringstream ss;
                ss << out.tv.breakpoint;
                info.tags["TV Breakpoint"] = ss.str();
            }
            if (isValid(&out.tv.whiteLevel))
            {
                std::stringstream ss;
                ss << out.tv.whiteLevel;
                info.tags["TV White Level"] = ss.str();
            }
            if (isValid(&out.tv.integrationTimes))
            {
                std::stringstream ss;
                ss << out.tv.integrationTimes;
                info.tags["TV Integration Times"] = ss.str();
            }

            // Set the file position.
            if (out.file.imageOffset)
            {
                io->setPos(out.file.imageOffset);
            }

            return out;
        }

        void Header::write(
            const std::shared_ptr<file::FileIO>& io,
            const avio::Info& info,
            Version version,
            Endian endian,
            Transfer transfer)
        {
            Header header;

            switch (version)
            {
            case Version::_1_0: memcpy(header.file.version, "V1.0", 4); break;
            case Version::_2_0: memcpy(header.file.version, "V2.0", 4); break;
            default: break;
            }

            header.file.imageOffset = 2048;
            header.file.headerSize = 2048 - 384;
            header.file.industryHeaderSize = 384;
            header.file.userHeaderSize = 0;
            header.file.size = 0;
            header.file.dittoKey = 0;
            header.file.encryptionKey = 0;

            header.image.elemSize = 1;
            const auto& imageInfo = info.video[0];
            header.image.size[0] = imageInfo.size.w;
            header.image.size[1] = imageInfo.size.h;
            header.image.orient = static_cast<uint16_t>(Orient::LeftRightTopBottom);

            switch (imageInfo.pixelType)
            {
            case imaging::PixelType::L_U8:
            case imaging::PixelType::L_U16:
            case imaging::PixelType::L_F16:
            case imaging::PixelType::L_F32:
                header.image.elem[0].descriptor = static_cast<uint8_t>(Descriptor::L);
                break;
            case imaging::PixelType::RGB_U8:
            case imaging::PixelType::RGB_U10:
            case imaging::PixelType::RGB_U16:
            case imaging::PixelType::RGB_F16:
            case imaging::PixelType::RGB_F32:
                header.image.elem[0].descriptor = static_cast<uint8_t>(Descriptor::RGB);
                break;
            case imaging::PixelType::RGBA_U8:
            case imaging::PixelType::RGBA_U16:
            case imaging::PixelType::RGBA_F16:
            case imaging::PixelType::RGBA_F32:
                header.image.elem[0].descriptor = static_cast<uint8_t>(Descriptor::RGBA);
                break;
            default: break;
            }

            switch (imageInfo.pixelType)
            {
            case imaging::PixelType::RGB_U10:
                header.image.elem[0].packing = static_cast<uint16_t>(Components::TypeA);
                break;
            default: break;
            }

            const int bitDepth = imaging::getBitDepth(imageInfo.pixelType);
            header.image.elem[0].bitDepth = bitDepth;
            header.image.elem[0].dataSign = 0;
            header.image.elem[0].lowData = 0;
            switch (bitDepth)
            {
            case 8:  header.image.elem[0].highData = 255; break;
            case 10: header.image.elem[0].highData = 1023; break;
            case 12: header.image.elem[0].highData = 4095; break;
            case 16: header.image.elem[0].highData = 65535; break;
            default: break;
            }

            switch (transfer)
            {
            case Transfer::FilmPrint:
                header.image.elem[0].transfer = static_cast<uint8_t>(Transfer::FilmPrint);
                switch (version)
                {
                case Version::_1_0:
                    header.image.elem[0].colorimetric = static_cast<uint8_t>(Colorimetric_1_0::FilmPrint);
                    break;
                default:
                    header.image.elem[0].colorimetric = static_cast<uint8_t>(Colorimetric_1_0::User);
                    break;
                }
                break;
            default:
                header.image.elem[0].transfer = static_cast<uint8_t>(Transfer::Linear);
                switch (version)
                {
                case Version::_2_0:
                    header.image.elem[0].colorimetric = static_cast<uint8_t>(Colorimetric_2_0::FilmPrint);
                    break;
                default:
                    header.image.elem[0].colorimetric = static_cast<uint8_t>(Colorimetric_2_0::User);
                    break;
                }
                break;
            }

            header.image.elem[0].encoding = 0;
            header.image.elem[0].dataOffset = 2048;
            header.image.elem[0].linePadding = 0;
            header.image.elem[0].elemPadding = 0;

            auto i = info.tags.find("Time");
            if (i != info.tags.end())
            {
                cineon::fromString(i->second, header.file.time, 24, false);
            }
            i = info.tags.find("Creator");
            if (i != info.tags.end())
            {
                cineon::fromString(i->second, header.file.creator, 100, false);
            }
            i = info.tags.find("Project");
            if (i != info.tags.end())
            {
                cineon::fromString(i->second, header.file.project, 200, false);
            }
            i = info.tags.find("Copyright");
            if (i != info.tags.end())
            {
                cineon::fromString(i->second, header.file.copyright, 200, false);
            }

            i = info.tags.find("Source Offset");
            if (i != info.tags.end())
            {
                std::stringstream ss(i->second);
                ss >> header.source.offset[0];
                ss >> header.source.offset[1];
            }
            i = info.tags.find("Source Center");
            if (i != info.tags.end())
            {
                std::stringstream ss(i->second);
                ss >> header.source.center[0];
                ss >> header.source.center[1];
            }
            i = info.tags.find("Source Size");
            if (i != info.tags.end())
            {
                std::stringstream ss(i->second);
                ss >> header.source.size[0];
                ss >> header.source.size[1];
            }
            i = info.tags.find("Source File");
            if (i != info.tags.end())
            {
                cineon::fromString(i->second, header.source.file, 100, false);
            }
            i = info.tags.find("Source Time");
            if (i != info.tags.end())
            {
                cineon::fromString(i->second, header.source.time, 24, false);
            }
            i = info.tags.find("Source Input Device");
            if (i != info.tags.end())
            {
                cineon::fromString(i->second, header.source.inputDevice, 32, false);
            }
            i = info.tags.find("Source Input Serial");
            if (i != info.tags.end())
            {
                cineon::fromString(i->second, header.source.inputSerial, 32, false);
            }
            i = info.tags.find("Source Border");
            if (i != info.tags.end())
            {
                std::stringstream ss(i->second);
                ss >> header.source.border[0];
                ss >> header.source.border[1];
                ss >> header.source.border[2];
                ss >> header.source.border[3];
            }
            i = info.tags.find("Source Pixel Aspect");
            if (i != info.tags.end())
            {
                std::stringstream ss(i->second);
                ss >> header.source.pixelAspect[0];
                ss >> header.source.pixelAspect[1];
            }
            i = info.tags.find("Source Scan Size");
            if (i != info.tags.end())
            {
                std::stringstream ss(i->second);
                ss >> header.source.scanSize[0];
                ss >> header.source.scanSize[1];
            }

            i = info.tags.find("Keycode");
            if (i != info.tags.end())
            {
                int id = 0;
                int type = 0;
                int prefix = 0;
                int count = 0;
                int offset = 0;
                time::stringToKeycode(i->second, id, type, prefix, count, offset);
                snprintf(header.film.id, 2, "%d", id);
                snprintf(header.film.type, 2, "%d", type);
                snprintf(header.film.prefix, 6, "%d", prefix);
                snprintf(header.film.count, 4, "%d", count);
                snprintf(header.film.offset, 2, "%d", offset);
            }
            i = info.tags.find("Film Format");
            if (i != info.tags.end())
            {
                cineon::fromString(i->second, header.film.format, 32, false);
            }
            i = info.tags.find("Film Frame");
            if (i != info.tags.end())
            {
                header.film.frame = std::stoi(i->second);
            }
            i = info.tags.find("Film Sequence");
            if (i != info.tags.end())
            {
                header.film.sequence = std::stoi(i->second);
            }
            i = info.tags.find("Film Hold");
            if (i != info.tags.end())
            {
                header.film.hold = std::stoi(i->second);
            }
            i = info.tags.find("Film Frame Rate");
            if (i != info.tags.end())
            {
                header.film.frameRate = std::stof(i->second);
            }
            i = info.tags.find("Film Shutter");
            if (i != info.tags.end())
            {
                header.film.shutter = std::stof(i->second);
            }
            i = info.tags.find("Film Frame ID");
            if (i != info.tags.end())
            {
                cineon::fromString(i->second, header.film.frameId, 32, false);
            }
            i = info.tags.find("Film Slate");
            if (i != info.tags.end())
            {
                cineon::fromString(i->second, header.film.slate, 100, false);
            }

            i = info.tags.find("Timecode");
            if (i != info.tags.end())
            {
                time::stringToTimecode(i->second, header.tv.timecode);
            }
            i = info.tags.find("TV Interlace");
            if (i != info.tags.end())
            {
                header.tv.interlace = std::stoi(i->second);
            }
            i = info.tags.find("TV Field");
            if (i != info.tags.end())
            {
                header.tv.field = std::stoi(i->second);
            }
            i = info.tags.find("TV Video Signal");
            if (i != info.tags.end())
            {
                header.tv.videoSignal = std::stoi(i->second);
            }
            i = info.tags.find("TV Sample Rate");
            if (i != info.tags.end())
            {
                std::stringstream ss(i->second);
                ss >> header.tv.sampleRate[0];
                ss >> header.tv.sampleRate[1];
            }
            i = info.tags.find("TV Frame Rate");
            if (i != info.tags.end())
            {
                header.tv.frameRate = std::stof(i->second);
            }
            i = info.tags.find("TV Time Offset");
            if (i != info.tags.end())
            {
                header.tv.timeOffset = std::stof(i->second);
            }
            i = info.tags.find("TV Gamma");
            if (i != info.tags.end())
            {
                header.tv.gamma = std::stof(i->second);
            }
            i = info.tags.find("TV Black Level");
            if (i != info.tags.end())
            {
                header.tv.blackLevel = std::stof(i->second);
            }
            i = info.tags.find("TV Black Gain");
            if (i != info.tags.end())
            {
                header.tv.blackGain = std::stof(i->second);
            }
            i = info.tags.find("TV Breakpoint");
            if (i != info.tags.end())
            {
                header.tv.breakpoint = std::stof(i->second);
            }
            i = info.tags.find("TV White Level");
            if (i != info.tags.end())
            {
                header.tv.whiteLevel = std::stof(i->second);
            }
            i = info.tags.find("TV Integration Times");
            if (i != info.tags.end())
            {
                header.tv.integrationTimes = std::stof(i->second);
            }

            memory::Endian fileEndian = memory::getEndian();
            switch (endian)
            {
            case Endian::MSB: fileEndian = memory::Endian::MSB; break;
            case Endian::LSB: fileEndian = memory::Endian::LSB; break;
            default: break;
            }
            if (fileEndian != memory::getEndian())
            {
                io->setEndianConversion(true);
                header.convertEndian();
            }
            memcpy(
                &header.file.magic,
                memory::Endian::MSB == fileEndian ? magic[0] : magic[1],
                4);
            io->write(&header.file, sizeof(Header::File));
            io->write(&header.image, sizeof(Header::Image));
            io->write(&header.source, sizeof(Header::Source));
            io->write(&header.film, sizeof(Header::Film));
            io->write(&header.tv, sizeof(Header::TV));
        }

        void Header::finishWrite(const std::shared_ptr<file::FileIO>& io)
        {
            const uint32_t size = static_cast<uint32_t>(io->getPos());
            io->setPos(12);
            io->writeU32(size);
        }

        void Plugin::_init()
        {
            IPlugin::_init(
                "DPX",
                { ".dpx" });
        }

        Plugin::Plugin()
        {}
            
        std::shared_ptr<Plugin> Plugin::create()
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init();
            return out;
        }

        std::shared_ptr<avio::IRead> Plugin::read(
            const std::string& fileName,
            const avio::Options& options)
        {
            return Read::create(fileName, options);
        }

        std::vector<imaging::PixelType> Plugin::getWritePixelTypes() const
        {
            return
            {
                imaging::PixelType::RGB_U10
            };
        }

        uint8_t Plugin::getWriteAlignment(imaging::PixelType) const
        {
            return 4;
        }

        std::shared_ptr<avio::IWrite> Plugin::write(
            const std::string& fileName,
            const avio::Info& info,
            const avio::Options& options)
        {
            return !info.video.empty() && _isWriteCompatible(info.video[0]) ?
                Write::create(fileName, info, options) :
                nullptr;
        }
    }

    TLR_ENUM_SERIALIZE_IMPL(dpx, Version);
    TLR_ENUM_SERIALIZE_IMPL(dpx, Endian);
}
