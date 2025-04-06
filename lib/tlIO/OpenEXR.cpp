// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/OpenEXRPrivate.h>

#include <dtk/core/Error.h>
#include <dtk/core/Format.h>
#include <dtk/core/String.h>

#include <ImfChannelList.h>
#include <ImfDoubleAttribute.h>
#include <ImfFloatVectorAttribute.h>
#include <ImfFramesPerSecond.h>
#include <ImfIntAttribute.h>
#include <ImfStandardAttributes.h>
#include <ImfStdIO.h>
#include <ImfThreading.h>

#include <array>

namespace tl
{
    namespace exr
    {
        DTK_ENUM_IMPL(
            Compression,
            "None",
            "RLE",
            "ZIPS",
            "ZIP",
            "PIZ",
            "PXR24",
            "B44",
            "B44A",
            "DWAA",
            "DWAB");

        std::set<std::string> getDefaultChannels(const std::set<std::string>& value)
        {
            std::set<std::string> out;
            for (const auto& i : value)
            {
                const auto j = i.find('.');
                if (j == std::string::npos)
                {
                    out.insert(i);
                }
                else
                {
                    const auto k = i.find('.', j + 1);
                    if (k == std::string::npos)
                    {
                        if (j == 0 || (j > 0 && j == i.size() - 1))
                        {
                            out.insert(i);
                        }
                    }
                }
            }
            return out;
        }

        namespace
        {
            std::list<std::string>::iterator find(std::list<std::string>& list, char c)
            {
                std::list<std::string>::iterator out = list.end();
                for (auto i = list.begin(); i != list.end(); ++i)
                {
                    const std::string s = dtk::toLower(*i);
                    const size_t size = s.size();
                    if (1 == size && s[0] == c)
                    {
                        out = i;
                        break;
                    }
                    else if (size > 1 && s[size - 1] == c && s[size - 2] == '.')
                    {
                        out = i;
                        break;
                    }
                }
                return out;
            }
        }

        void reorderChannels(std::vector<std::string>& channels)
        {
            std::list tmp(channels.begin(), channels.end());

            auto r = find(tmp, 'r');
            auto g = find(tmp, 'g');
            auto b = find(tmp, 'b');
            auto a = find(tmp, 'a');
            if (r != tmp.end() &&
                g != tmp.end() &&
                b != tmp.end() &&
                a != tmp.end())
            {
                tmp.push_front(*a);
                tmp.push_front(*b);
                tmp.push_front(*g);
                tmp.push_front(*r);
                tmp.erase(r);
                tmp.erase(g);
                tmp.erase(b);
                tmp.erase(a);
            }
            else if (r != tmp.end() &&
                g != tmp.end() &&
                b != tmp.end())
            {
                tmp.push_front(*b);
                tmp.push_front(*g);
                tmp.push_front(*r);
                tmp.erase(r);
                tmp.erase(g);
                tmp.erase(b);
            }

            channels.clear();
            channels.insert(channels.end(), tmp.begin(), tmp.end());
        }

        namespace
        {
            template<typename T>
            std::string serialize(const T& value)
            {
                std::stringstream ss;
                ss << value;
                return ss.str();
            }

            template<typename T>
            void deserialize(const std::string& s, T& value)
            {
                std::stringstream ss(s);
                ss >> value;
            }

            template<typename T>
            std::string serialize(const Imath::Vec2<T>& value)
            {
                std::stringstream ss;
                ss << value.x << " " << value.y;
                return ss.str();
            }

            template<>
            void deserialize(const std::string& s, Imath::V2f& value)
            {
                std::stringstream ss(s);
                ss >> value.x;
                ss >> value.y;
            }

            template<typename T>
            std::string serialize(const Imath::Box<Imath::Vec2<T> >& value)
            {
                std::stringstream ss;
                ss << value.min.x << " " <<
                    value.min.y << " " <<
                    value.max.x << " " <<
                    value.max.y;
                return ss.str();
            }

            template<>
            void deserialize(const std::string& s, Imath::Box2i& value)
            {
                std::stringstream ss(s);
                ss >> value.min.x;
                ss >> value.min.y;
                ss >> value.max.x;
                ss >> value.max.y;
            }

            template<typename T>
            std::string serialize(const Imath::Matrix44<T>& value)
            {
                std::vector<std::string> s;
                for (size_t j = 0; j < 4; ++j)
                {
                    for (size_t i = 0; i < 4; ++i)
                    {
                        s.push_back(dtk::Format("{0}").arg(value.x[j][i]));
                    }
                }
                return dtk::join(s, ' ');
            }

            template<>
            void deserialize(const std::string& s, Imath::M44f& value)
            {
                std::stringstream ss(s);
                for (size_t j = 0; j < 4; ++j)
                {
                    for (size_t i = 0; i < 4; ++i)
                    {
                        ss >> value.x[j][i];
                    }
                }
            }

            template<>
            std::string serialize(const Imf::TileDescription& value)
            {
                std::stringstream ss;
                ss << value.xSize << " " <<
                    value.ySize << " " <<
                    value.mode << " " <<
                    value.roundingMode;
                return ss.str();
            }

            template<>
            std::string serialize(const Imf::Chromaticities& value)
            {
                std::stringstream ss;
                ss << value.red.x << " " << value.red.y << " " <<
                    value.green.x << " " << value.green.y << " " <<
                    value.blue.x << " " << value.blue.y << " " <<
                    value.white.x << " " << value.white.y;
                return ss.str();
            }

            template<>
            void deserialize(const std::string& s, Imf::Chromaticities& value)
            {
                std::stringstream ss(s);
                ss >> value.red.x;
                ss >> value.red.y;
                ss >> value.green.x;
                ss >> value.green.y;
                ss >> value.blue.x;
                ss >> value.blue.y;
                ss >> value.white.x;
                ss >> value.white.y;
            }

            template<>
            std::string serialize(const Imf::Rational& value)
            {
                std::stringstream ss;
                ss << value.n << " " << value.d;
                return ss.str();
            }

            template<>
            void deserialize(const std::string& s, Imf::Rational& value)
            {
                std::stringstream ss(s);
                ss >> value.n;
                ss >> value.d;
            }

            template<>
            std::string serialize(const Imf::KeyCode& value)
            {
                return dtk::Format("{0}:{1}:{2}:{3}:{4}:{5}:{6}").
                    arg(value.filmMfcCode()).
                    arg(value.filmType()).
                    arg(value.prefix()).
                    arg(value.count()).
                    arg(value.perfOffset()).
                    arg(value.perfsPerFrame()).
                    arg(value.perfsPerCount());
            }

            template<>
            void deserialize(const std::string& s, Imf::KeyCode& value)
            {
                auto split = dtk::split(s, ':');
                if (split.size() != 7)
                {
                    throw dtk::ParseError();
                }
                value.setFilmMfcCode(std::atoi(split[0].c_str()));
                value.setFilmType(std::atoi(split[1].c_str()));
                value.setPrefix(std::atoi(split[2].c_str()));
                value.setCount(std::atoi(split[3].c_str()));
                value.setPerfOffset(std::atoi(split[4].c_str()));
                value.setPerfsPerFrame(std::atoi(split[5].c_str()));
                value.setPerfsPerCount(std::atoi(split[6].c_str()));
            }

            template<>
            std::string serialize(const Imf::TimeCode& value)
            {
                return dtk::Format("{0}:{1}:{2}:{3}").
                    arg(value.hours(), 2, '0').
                    arg(value.minutes(), 2, '0').
                    arg(value.seconds(), 2, '0').
                    arg(value.frame(), 2, '0');
            }

            template<>
            void deserialize(const std::string& s, Imf::TimeCode& value)
            {
                auto split = dtk::split(s, ':');
                if (split.size() != 4)
                {
                    throw dtk::ParseError();
                }
                value.setHours(std::atoi(split[0].c_str()));
                value.setMinutes(std::atoi(split[1].c_str()));
                value.setSeconds(std::atoi(split[2].c_str()));
                value.setFrame(std::atoi(split[3].c_str()));
            }

            template<>
            std::string serialize(const Imf::Envmap& value)
            {
                std::stringstream ss;
                ss << value;
                return ss.str();
            }

            template<>
            void deserialize(const std::string& s, Imf::Envmap& value)
            {
                std::stringstream ss(s);
                int tmp = 0;
                ss >> tmp;
                value = static_cast<Imf::Envmap>(tmp);
            }

            template<>
            std::string serialize(const Imf::StringVector& value)
            {
                std::stringstream ss;
                for (const auto& i : value)
                {
                    ss << std::string(dtk::Format("{0}:{1}").arg(i.size()).arg(i));
                }
                return ss.str();
            }

            template<>
            void deserialize(const std::string& s, Imf::StringVector& value)
            {
                std::string tmp = s;
                while (!tmp.empty())
                {
                    auto split = dtk::split(tmp, ':');
                    if (split.size() < 2)
                    {
                        throw dtk::ParseError();
                    }
                    int size = std::atoi(split[0].c_str());
                    split.erase(split.begin());
                    tmp = dtk::join(split, ':');
                    value.push_back(tmp.substr(0, size));
                    tmp.erase(0, size);
                }
            }

            template<>
            std::string serialize(const Imf::DeepImageState& value)
            {
                std::stringstream ss;
                ss << value;
                return ss.str();
            }

            template<>
            void deserialize(const std::string& s, Imf::DeepImageState& value)
            {
                std::stringstream ss(s);
                int tmp = 0;
                ss >> tmp;
                value = static_cast<Imf::DeepImageState>(tmp);
            }

#define TLRENDER_SERIALIZE_STD_ATTR(NAME, NAME_LOWER) \
    if (has##NAME(header)) \
    { \
        tags[#NAME] = serialize(NAME_LOWER##Attribute(header).value()); \
    }

#define TLRENDER_DESERIALIZE_STD_ATTR(NAME, TYPE) \
    { \
        auto i = tags.find(#NAME); \
        if (i != tags.end()) \
        { \
            TYPE v; \
            deserialize(i->second, v); \
            add##NAME(header, v); \
        } \
    }

        } // namespace

        void readTags(const Imf::Header& header, dtk::ImageTags& tags)
        {
            // Predefined attributes.
            tags["Display Window"] = serialize(header.displayWindow());
            tags["Data Window"] = serialize(header.dataWindow());
            tags["Pixel Aspect Ratio"] = serialize(header.pixelAspectRatio());
            tags["Screen Window Center"] = serialize(header.screenWindowCenter());
            tags["Screen Window Width"] = serialize(header.screenWindowWidth());
            {
                std::vector<std::string> values;
                for (auto i = header.channels().begin(); i != header.channels().end(); ++i)
                {
                    values.push_back(i.name());
                }
                tags["Channels"] = dtk::join(values, " ");
            }
            tags["Line Order"] = serialize(header.lineOrder());
            tags["Compression"] = serialize(header.compression());

            // Multipart attributes.
            if (header.hasName())
            {
                tags["Name"] = header.name();
            }
            if (header.hasType())
            {
                tags["Type"] = header.type();
            }
            if (header.hasVersion())
            {
                tags["Version"] = serialize(header.version());
            }
            if (header.hasChunkCount())
            {
                tags["Chunk Count"] = serialize(header.chunkCount());
            }
            if (header.hasView())
            {
                tags["View"] = header.view();
            }

            // Tile description.
            if (header.hasTileDescription())
            {
                tags["Tile"] = serialize(header.tileDescription());
            }

            // Standard attributes.
            TLRENDER_SERIALIZE_STD_ATTR(AdoptedNeutral, adoptedNeutral);
            TLRENDER_SERIALIZE_STD_ATTR(Altitude, altitude);
            TLRENDER_SERIALIZE_STD_ATTR(Aperture, aperture);
            TLRENDER_SERIALIZE_STD_ATTR(AscFramingDecisionList, ascFramingDecisionList);
            TLRENDER_SERIALIZE_STD_ATTR(CameraCCTSetting, cameraCCTSetting);
            TLRENDER_SERIALIZE_STD_ATTR(CameraColorBalance, cameraColorBalance);
            TLRENDER_SERIALIZE_STD_ATTR(CameraFirmwareVersion, cameraFirmwareVersion);
            TLRENDER_SERIALIZE_STD_ATTR(CameraLabel, cameraLabel);
            TLRENDER_SERIALIZE_STD_ATTR(CameraMake, cameraMake);
            TLRENDER_SERIALIZE_STD_ATTR(CameraModel, cameraModel);
            TLRENDER_SERIALIZE_STD_ATTR(CameraSerialNumber, cameraSerialNumber);
            TLRENDER_SERIALIZE_STD_ATTR(CameraTintSetting, cameraTintSetting);
            TLRENDER_SERIALIZE_STD_ATTR(CameraUuid, cameraUuid);
            TLRENDER_SERIALIZE_STD_ATTR(CapDate, capDate);
            TLRENDER_SERIALIZE_STD_ATTR(CaptureRate, captureRate);
            TLRENDER_SERIALIZE_STD_ATTR(Chromaticities, chromaticities);
            TLRENDER_SERIALIZE_STD_ATTR(Comments, comments);
            TLRENDER_SERIALIZE_STD_ATTR(DeepImageState, deepImageState);
            TLRENDER_SERIALIZE_STD_ATTR(EffectiveFocalLength, effectiveFocalLength);
            TLRENDER_SERIALIZE_STD_ATTR(Envmap, envmap);
            TLRENDER_SERIALIZE_STD_ATTR(EntrancePupilOffset, entrancePupilOffset);
            TLRENDER_SERIALIZE_STD_ATTR(ExpTime, expTime);
            TLRENDER_SERIALIZE_STD_ATTR(Focus, focus);
            TLRENDER_SERIALIZE_STD_ATTR(FramesPerSecond, framesPerSecond);
            TLRENDER_SERIALIZE_STD_ATTR(ImageCounter, imageCounter);
            TLRENDER_SERIALIZE_STD_ATTR(IsoSpeed, isoSpeed);
            TLRENDER_SERIALIZE_STD_ATTR(KeyCode, keyCode);
            TLRENDER_SERIALIZE_STD_ATTR(Latitude, latitude);
            TLRENDER_SERIALIZE_STD_ATTR(LensFirmwareVersion, lensFirmwareVersion);
            TLRENDER_SERIALIZE_STD_ATTR(LensMake, lensMake);
            TLRENDER_SERIALIZE_STD_ATTR(LensModel, lensModel);
            TLRENDER_SERIALIZE_STD_ATTR(LensSerialNumber, lensSerialNumber);
            TLRENDER_SERIALIZE_STD_ATTR(Longitude, longitude);
            TLRENDER_SERIALIZE_STD_ATTR(MultiView, multiView);
            TLRENDER_SERIALIZE_STD_ATTR(NominalFocalLength, nominalFocalLength);
            TLRENDER_SERIALIZE_STD_ATTR(OriginalDataWindow, originalDataWindow);
            TLRENDER_SERIALIZE_STD_ATTR(Owner, owner);
            TLRENDER_SERIALIZE_STD_ATTR(PinholeFocalLength, pinholeFocalLength);
            TLRENDER_SERIALIZE_STD_ATTR(ReelName, reelName);
            TLRENDER_SERIALIZE_STD_ATTR(SensorAcquisitionRectangle, sensorAcquisitionRectangle);
            TLRENDER_SERIALIZE_STD_ATTR(SensorOverallDimensions, sensorOverallDimensions);
            TLRENDER_SERIALIZE_STD_ATTR(SensorPhotositePitch, sensorPhotositePitch);
            TLRENDER_SERIALIZE_STD_ATTR(ShutterAngle, shutterAngle);
            TLRENDER_SERIALIZE_STD_ATTR(TStop, tStop);
            TLRENDER_SERIALIZE_STD_ATTR(TimeCode, timeCode);
            TLRENDER_SERIALIZE_STD_ATTR(UtcOffset, utcOffset);
            TLRENDER_SERIALIZE_STD_ATTR(WhiteLuminance, whiteLuminance);
            TLRENDER_SERIALIZE_STD_ATTR(WorldToCamera, worldToCamera);
            TLRENDER_SERIALIZE_STD_ATTR(WorldToNDC, worldToNDC);
            TLRENDER_SERIALIZE_STD_ATTR(Wrapmodes, wrapmodes);
            TLRENDER_SERIALIZE_STD_ATTR(XDensity, xDensity);
        }

        void writeTags(const dtk::ImageTags& tags, double speed, Imf::Header& header)
        {
            // Standard attributes.
            TLRENDER_DESERIALIZE_STD_ATTR(AdoptedNeutral, Imath::V2f);
            TLRENDER_DESERIALIZE_STD_ATTR(Altitude, float);
            TLRENDER_DESERIALIZE_STD_ATTR(Aperture, float);
            TLRENDER_DESERIALIZE_STD_ATTR(AscFramingDecisionList, std::string);
            TLRENDER_DESERIALIZE_STD_ATTR(CameraCCTSetting, float);
            TLRENDER_DESERIALIZE_STD_ATTR(CameraColorBalance, Imath::V2f);
            TLRENDER_DESERIALIZE_STD_ATTR(CameraFirmwareVersion, std::string);
            TLRENDER_DESERIALIZE_STD_ATTR(CameraLabel, std::string);
            TLRENDER_DESERIALIZE_STD_ATTR(CameraMake, std::string);
            TLRENDER_DESERIALIZE_STD_ATTR(CameraModel, std::string);
            TLRENDER_DESERIALIZE_STD_ATTR(CameraSerialNumber, std::string);
            TLRENDER_DESERIALIZE_STD_ATTR(CameraTintSetting, float);
            TLRENDER_DESERIALIZE_STD_ATTR(CameraUuid, std::string);
            TLRENDER_DESERIALIZE_STD_ATTR(CapDate, std::string);
            TLRENDER_DESERIALIZE_STD_ATTR(CaptureRate, Imf::Rational);
            TLRENDER_DESERIALIZE_STD_ATTR(Chromaticities, Imf::Chromaticities);
            TLRENDER_DESERIALIZE_STD_ATTR(Comments, std::string);
            TLRENDER_DESERIALIZE_STD_ATTR(DeepImageState, Imf::DeepImageState);
            TLRENDER_DESERIALIZE_STD_ATTR(EffectiveFocalLength, float);
            TLRENDER_DESERIALIZE_STD_ATTR(EntrancePupilOffset, float);
            TLRENDER_DESERIALIZE_STD_ATTR(Envmap, Imf::Envmap);
            TLRENDER_DESERIALIZE_STD_ATTR(ExpTime, float);
            TLRENDER_DESERIALIZE_STD_ATTR(Focus, float);
            TLRENDER_DESERIALIZE_STD_ATTR(FramesPerSecond, Imf::Rational);
            TLRENDER_DESERIALIZE_STD_ATTR(ImageCounter, int);
            TLRENDER_DESERIALIZE_STD_ATTR(IsoSpeed, float);
            TLRENDER_DESERIALIZE_STD_ATTR(KeyCode, Imf::KeyCode);
            TLRENDER_DESERIALIZE_STD_ATTR(Latitude, float);
            TLRENDER_DESERIALIZE_STD_ATTR(LensFirmwareVersion, std::string);
            TLRENDER_DESERIALIZE_STD_ATTR(LensMake, std::string);
            TLRENDER_DESERIALIZE_STD_ATTR(LensModel, std::string);
            TLRENDER_DESERIALIZE_STD_ATTR(LensSerialNumber, std::string);
            TLRENDER_DESERIALIZE_STD_ATTR(Longitude, float);
            TLRENDER_DESERIALIZE_STD_ATTR(MultiView, Imf::StringVector);
            TLRENDER_DESERIALIZE_STD_ATTR(NominalFocalLength, float);
            TLRENDER_DESERIALIZE_STD_ATTR(OriginalDataWindow, Imath::Box2i);
            TLRENDER_DESERIALIZE_STD_ATTR(Owner, std::string);
            TLRENDER_DESERIALIZE_STD_ATTR(PinholeFocalLength, float);
            TLRENDER_DESERIALIZE_STD_ATTR(ReelName, std::string);
            TLRENDER_DESERIALIZE_STD_ATTR(SensorAcquisitionRectangle, Imath::Box2i);
            TLRENDER_DESERIALIZE_STD_ATTR(SensorCenterOffset, Imath::V2f);
            TLRENDER_DESERIALIZE_STD_ATTR(SensorPhotositePitch, float);
            TLRENDER_DESERIALIZE_STD_ATTR(ShutterAngle, float);
            TLRENDER_DESERIALIZE_STD_ATTR(TStop, float);
            TLRENDER_DESERIALIZE_STD_ATTR(TimeCode, Imf::TimeCode);
            TLRENDER_DESERIALIZE_STD_ATTR(UtcOffset, float);
            TLRENDER_DESERIALIZE_STD_ATTR(WhiteLuminance, float);
            TLRENDER_DESERIALIZE_STD_ATTR(WorldToCamera, Imath::M44f);
            TLRENDER_DESERIALIZE_STD_ATTR(WorldToNDC, Imath::M44f);
            TLRENDER_DESERIALIZE_STD_ATTR(Wrapmodes, std::string);
            TLRENDER_DESERIALIZE_STD_ATTR(XDensity, float);
            {
                const auto speedRational = time::toRational(speed);
                addFramesPerSecond(
                    header,
                    Imf::Rational(speedRational.first, speedRational.second));
            }
        }

        Imf::Compression toImf(Compression value)
        {
            const std::array<Imf::Compression, 10> data =
            {
                Imf::NO_COMPRESSION,
                Imf::RLE_COMPRESSION,
                Imf::ZIPS_COMPRESSION,
                Imf::ZIP_COMPRESSION,
                Imf::PIZ_COMPRESSION,
                Imf::PXR24_COMPRESSION,
                Imf::B44_COMPRESSION,
                Imf::B44A_COMPRESSION,
                Imf::DWAA_COMPRESSION,
                Imf::DWAB_COMPRESSION
            };
            return data[static_cast<size_t>(value)];
        }

        dtk::Box2I fromImath(const Imath::Box2i& value)
        {
            return dtk::Box2I(dtk::V2I(value.min.x, value.min.y), dtk::V2I(value.max.x, value.max.y));
        }

        void ReadPlugin::_init(const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            IReadPlugin::_init(
                "OpenEXR",
                { { ".exr", io::FileType::Sequence } },
                logSystem);

            Imf::setGlobalThreadCount(0);
        }

        ReadPlugin::ReadPlugin()
        {}
            
        std::shared_ptr<ReadPlugin> ReadPlugin::create(
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<ReadPlugin>(new ReadPlugin);
            out->_init(logSystem);
            return out;
        }

        std::shared_ptr<io::IRead> ReadPlugin::read(
            const file::Path& path,
            const io::Options& options)
        {
            return Read::create(path, options, _logSystem.lock());
        }

        std::shared_ptr<io::IRead> ReadPlugin::read(
            const file::Path& path,
            const std::vector<dtk::InMemoryFile>& memory,
            const io::Options& options)
        {
            return Read::create(path, memory, options, _logSystem.lock());
        }

        void WritePlugin::_init(
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            IWritePlugin::_init(
                "OpenEXR",
                { { ".exr", io::FileType::Sequence } },
                logSystem);

            Imf::setGlobalThreadCount(0);
        }

        WritePlugin::WritePlugin()
        {}

        std::shared_ptr<WritePlugin> WritePlugin::create(
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<WritePlugin>(new WritePlugin);
            out->_init(logSystem);
            return out;
        }

        dtk::ImageInfo WritePlugin::getInfo(
            const dtk::ImageInfo& info,
            const io::Options& options) const
        {
            dtk::ImageInfo out;
            out.size = info.size;
            switch (info.type)
            {
            case dtk::ImageType::RGBA_F16:
                out.type = info.type;
                break;
            default: break;
            }
            out.layout.mirror.y = true;
            return out;
        }

        std::shared_ptr<io::IWrite> WritePlugin::write(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options)
        {
            if (info.video.empty() || (!info.video.empty() && !_isCompatible(info.video[0], options)))
                throw std::runtime_error(dtk::Format("Unsupported video: \"{0}\"").
                    arg(path.get()));
            return Write::create(path, info, options, _logSystem.lock());
        }
    }
}
