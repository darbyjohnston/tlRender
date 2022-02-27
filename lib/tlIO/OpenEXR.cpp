// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIO/OpenEXR.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <ImfChannelList.h>
#include <ImfDoubleAttribute.h>
#include <ImfFloatVectorAttribute.h>
#include <ImfFramesPerSecond.h>
#include <ImfIntAttribute.h>
#include <ImfStandardAttributes.h>
#include <ImfThreading.h>

#include <array>

using namespace tl::core;

namespace tl
{
    namespace io
    {
        namespace exr
        {
            TLRENDER_ENUM_IMPL(
                ChannelGrouping,
                "None",
                "Known",
                "All");
            TLRENDER_ENUM_SERIALIZE_IMPL(ChannelGrouping);

            TLRENDER_ENUM_IMPL(
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
            TLRENDER_ENUM_SERIALIZE_IMPL(Compression);

            Channel::Channel()
            {}

            Channel::Channel(
                const std::string&    name,
                Imf::PixelType        pixelType,
                const math::Vector2i& sampling) :
                name(name),
                pixelType(pixelType),
                sampling(sampling)
            {}

            Layer::Layer(
                const std::vector<Channel>& channels,
                bool                        luminanceChroma) :
                channels(channels),
                luminanceChroma(luminanceChroma)
            {
                std::vector<std::string> names;
                for (const auto& i : channels)
                {
                    names.push_back(i.name);
                }
                name = getLayerName(names);
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

            std::string getLayerName(const std::vector<std::string>& value)
            {
                std::string out;

                std::set<std::string> prefixes;
                std::vector<std::string> suffixes;
                for (const auto& i : value)
                {
                    size_t index = i.find_last_of('.');
                    if (index != std::string::npos)
                    {
                        prefixes.insert(i.substr(0, index));
                        suffixes.push_back(i.substr(index + 1));
                    }
                    else
                    {
                        prefixes.insert(i);
                    }
                }

                out = string::join(std::vector<std::string>(prefixes.begin(), prefixes.end()), ',');
                if (!suffixes.empty())
                {
                    out += '.';
                    out += string::join(suffixes, ',');
                }

                return out;
            }

            Imf::ChannelList getDefaultLayer(const Imf::ChannelList& in)
            {
                Imf::ChannelList out;
                for (auto i = in.begin(); i != in.end(); ++i)
                {
                    const std::string tmp(i.name());
                    const size_t index = tmp.find_first_of('.');
                    if (index != std::string::npos)
                    {
                        if (index != 0 || index != tmp.size() - 1)
                        {
                            continue;
                        }
                    }
                    out.insert(i.name(), i.channel());
                }
                return out;
            }

            const Imf::Channel* find(const Imf::ChannelList& in, std::string& channel)
            {
                const std::string channelLower = string::toLower(channel);
                for (auto i = in.begin(); i != in.end(); ++i)
                {
                    const std::string inName(i.name());
                    const size_t index = inName.find_last_of('.');
                    const std::string tmp =
                        (index != std::string::npos) ?
                        inName.substr(index + 1, inName.size() - index - 1) :
                        inName;
                    if (channelLower == string::toLower(tmp))
                    {
                        channel = inName;
                        return &i.channel();
                    }
                }
                return nullptr;
            }

            namespace
            {
                bool compare(const std::vector<Imf::Channel>& in)
                {
                    for (size_t i = 1; i < in.size(); ++i)
                    {
                        if (!(in[0] == in[i]))
                        {
                            return false;
                        }
                    }
                    return true;
                }

                std::vector<Layer> _getLayers(const Imf::ChannelList& in, ChannelGrouping channelGrouping)
                {
                    std::vector<Layer> out;
                    std::vector<const Imf::Channel*> reserved;
                    if (channelGrouping != ChannelGrouping::None)
                    {
                        // Look for known channel configurations then convert the remainder.

                        // RGB / RGBA.
                        std::string rName = "r";
                        std::string gName = "g";
                        std::string bName = "b";
                        std::string aName = "a";
                        const Imf::Channel* r = find(in, rName);
                        const Imf::Channel* g = find(in, gName);
                        const Imf::Channel* b = find(in, bName);
                        const Imf::Channel* a = find(in, aName);
                        if (!r)
                        {
                            rName = "red";
                            r = find(in, rName);
                        }
                        if (!g)
                        {
                            gName = "green";
                            g = find(in, gName);
                        }
                        if (!b)
                        {
                            bName = "blue";
                            b = find(in, bName);
                        }
                        if (!a)
                        {
                            aName = "alpha";
                            a = find(in, aName);
                        }
                        if (r && g && b && a && compare({ *r, *g, *b, *a }))
                        {
                            out.push_back(Layer({
                                fromImf(rName, *r),
                                fromImf(gName, *g),
                                fromImf(bName, *b),
                                fromImf(aName, *a) }));
                            reserved.push_back(r);
                            reserved.push_back(g);
                            reserved.push_back(b);
                            reserved.push_back(a);
                        }
                        else if (r && g && b && compare({ *r, *g, *b }))
                        {
                            out.push_back(Layer({
                                fromImf(rName, *r),
                                fromImf(gName, *g),
                                fromImf(bName, *b) }));
                            reserved.push_back(r);
                            reserved.push_back(g);
                            reserved.push_back(b);
                        }

                        // Luminance, XYZ.
                        std::string yName = "y";
                        std::string ryName = "ry";
                        std::string byName = "by";
                        std::string xName = "x";
                        std::string zName = "z";
                        const Imf::Channel* y = find(in, yName);
                        const Imf::Channel* ry = find(in, ryName);
                        const Imf::Channel* by = find(in, byName);
                        const Imf::Channel* x = find(in, xName);
                        const Imf::Channel* z = find(in, zName);
                        if (y && a && compare({ *y, *a }))
                        {
                            out.push_back(Layer({
                                fromImf(yName, *y),
                                fromImf(aName, *a) }));
                            reserved.push_back(y);
                            reserved.push_back(a);
                        }
                        else if (y && ry && by &&
                            1 == y->xSampling &&
                            1 == y->ySampling &&
                            2 == ry->xSampling &&
                            2 == ry->ySampling &&
                            2 == by->xSampling &&
                            2 == by->ySampling)
                        {
                            out.push_back(Layer({
                                fromImf(yName, *y),
                                fromImf(ryName, *ry),
                                fromImf(byName, *by) },
                                true));
                            reserved.push_back(y);
                            reserved.push_back(ry);
                            reserved.push_back(by);
                        }
                        else if (x && y && z && compare({ *x, *y, *z }))
                        {
                            out.push_back(Layer({
                                fromImf(xName, *x),
                                fromImf(yName, *y),
                                fromImf(zName, *z) }));
                            reserved.push_back(x);
                            reserved.push_back(y);
                            reserved.push_back(z);
                        }
                        else if (x && y && compare({ *x, *y }))
                        {
                            out.push_back(Layer({
                                fromImf(xName, *x),
                                fromImf(yName, *y) }));
                            reserved.push_back(x);
                            reserved.push_back(y);
                        }
                        else if (x)
                        {
                            out.push_back(Layer({ fromImf(xName, *x) }));
                            reserved.push_back(x);
                        }
                        else if (y)
                        {
                            out.push_back(Layer({ fromImf(yName, *y) }));
                            reserved.push_back(y);
                        }
                        else if (z)
                        {
                            out.push_back(Layer({ fromImf(zName, *z) }));
                            reserved.push_back(z);
                        }

                        // Colored mattes.
                        std::string arName = "ar";
                        std::string agName = "ag";
                        std::string abName = "ab";
                        const Imf::Channel* ar = find(in, arName);
                        const Imf::Channel* ag = find(in, agName);
                        const Imf::Channel* ab = find(in, abName);
                        if (ar && ag && ab && compare({ *ar, *ag, *ab }))
                        {
                            out.push_back(Layer({
                                fromImf(arName, *ar),
                                fromImf(agName, *ag),
                                fromImf(abName, *ab) }));
                            reserved.push_back(ar);
                            reserved.push_back(ag);
                            reserved.push_back(ab);
                        }
                    }

                    // Convert the remainder.
                    for (auto i = in.begin(); i != in.end();)
                    {
                        std::vector<Channel> list;

                        // Add the first channel.
                        const std::string& name = i.name();
                        const Imf::Channel& channel = i.channel();
                        ++i;
                        if (std::find(reserved.begin(), reserved.end(), &channel) != reserved.end())
                        {
                            continue;
                        }
                        list.push_back(fromImf(name, channel));
                        if (ChannelGrouping::All == channelGrouping)
                        {
                            // Group as many additional channels as possible.
                            for (;
                                i != in.end() &&
                                i.channel() == channel;
                                ++i)
                            {
                                if (std::find(reserved.begin(), reserved.end(), &i.channel()) != reserved.end())
                                {
                                    continue;
                                }
                                list.push_back(fromImf(i.name(), i.channel()));
                            }
                        }

                        // Add the layer.
                        out.push_back(Layer(list));
                    }

                    return out;
                }

            } // namespace

            std::vector<Layer> getLayers(const Imf::ChannelList& in, ChannelGrouping channelGrouping)
            {
                std::vector<Layer> out;

                // Get the default layer.
                for (const auto& layer : _getLayers(getDefaultLayer(in), channelGrouping))
                {
                    out.push_back(layer);
                }

                // Get the additional layers.
                std::set<std::string> layers;
                in.layers(layers);
                for (auto i = layers.begin(); i != layers.end(); ++i)
                {
                    Imf::ChannelList list;
                    Imf::ChannelList::ConstIterator f, l;
                    in.channelsInLayer(*i, f, l);
                    for (auto j = f; j != l; ++j)
                    {
                        list.insert(j.name(), j.channel());
                    }
                    for (const auto& layer : _getLayers(list, channelGrouping))
                    {
                        out.push_back(layer);
                    }
                }

                return out;
            }

            namespace
            {
                const std::vector<std::string> knownAttributes =
                {
                    // Predefined attributes.
                    "displayWindow",
                    "dataWindow",
                    "pixelAspectRatio",
                    "screenWindowCenter",
                    "screenWindowWidth",
                    "channels",
                    "lineOrder",
                    "compression",

                    // Multipart attributes.
                    "name",
                    "type",
                    "version",
                    "chunkCount",
                    "view",

                    // Tile description.
                    "tileDescription",

                    // Standard attributes.
                    "chromaticities",
                    "whiteLuminance",
                    "adoptedNeutral",
                    "renderingTransform",
                    "lookModTransform",
                    "xDensity",
                    "owner",
                    "comments",
                    "capDate",
                    "utcOffset",
                    "longitude",
                    "latitude",
                    "altitude",
                    "focus",
                    "expTime",
                    "aperture",
                    "isoSpeed",
                    "envMap",
                    "keyCode",
                    "timeCode",
                    "wrapModes",
                    "framesPerSecond",
                    "multiView",
                    "worldToCamera",
                    "worldToNDC",
                    "deepImageState",
                    "originalDataWindow",
                    "dwaCompressionLevel"
                };

                template<typename T>
                std::string serialize(const T& value)
                {
                    std::stringstream ss;
                    ss << value;
                    return ss.str();
                }

                template<typename T>
                std::string serialize(const std::vector<T>& value)
                {
                    std::vector<std::string> list;
                    for (const auto& i : value)
                    {
                        std::stringstream ss;
                        ss << serialize(i);
                        list.push_back(ss.str());
                    }
                    return string::join(list, " ");
                }

                template<typename T>
                std::string serialize(const Imath::Vec2<T>& value)
                {
                    std::stringstream ss;
                    ss << value.x << " " << value.y;
                    return ss.str();
                }

                template<typename T>
                std::string serialize(const Imath::Vec3<T>& value)
                {
                    std::stringstream ss;
                    ss << value.x << " " << value.y << " " << value.z;
                    return ss.str();
                }

                template<typename T>
                std::string serialize(const Imath::Box<Imath::Vec2<T> >& value)
                {
                    std::stringstream ss;
                    ss << value.min.x << " " << value.min.y << " " <<
                        value.max.x << " " << value.max.y;
                    return ss.str();
                }

                template<typename T>
                std::string serialize(const Imath::Box<Imath::Vec3<T> >& value)
                {
                    std::stringstream ss;
                    ss << value.min.x << " " << value.min.y << " " << value.min.z << " " <<
                        value.max.x << " " << value.max.y << " " << value.max.z;
                    return ss.str();
                }

                template<>
                std::string serialize(const Imf::Compression& value)
                {
                    const std::vector<std::string> text =
                    {
                        "None",
                        "RLE",
                        "ZIPS",
                        "ZIP",
                        "PIZ",
                        "PXR24",
                        "B44",
                        "B44A",
                        "DWAA",
                        "DWAB"
                    };
                    return text[value];
                }

                template<>
                std::string serialize(const Imf::LineOrder& value)
                {
                    const std::vector<std::string> text =
                    {
                        "Increasing Y",
                        "Decreasing Y",
                        "Random Y"
                    };
                    return text[value];
                }

                template<>
                std::string serialize(const Imf::LevelMode& value)
                {
                    const std::vector<std::string> text =
                    {
                        "One Level",
                        "Mipmap Levels",
                        "Ripmap Levels"
                    };
                    return text[value];
                }

                template<>
                std::string serialize(const Imf::LevelRoundingMode& value)
                {
                    const std::vector<std::string> text =
                    {
                        "Round Down",
                        "Round Up"
                    };
                    return text[value];
                }

                template<>
                std::string serialize(const Imf::DeepImageState& value)
                {
                    const std::vector<std::string> text =
                    {
                        "Messy",
                        "Sorted",
                        "Non Overlapping",
                        "Tidy"
                    };
                    return text[value];
                }

                template<>
                std::string serialize(const Imf::TimeCode& value)
                {
                    return time::timecodeToString(value.timeAndFlags());
                }

                template<>
                std::string serialize(const Imf::KeyCode& value)
                {
                    return time::keycodeToString(
                        value.filmMfcCode(),
                        value.filmType(),
                        value.prefix(),
                        value.count(),
                        value.perfOffset());
                }

                template<>
                std::string serialize(const Imf::Chromaticities& value)
                {
                    std::stringstream ss;
                    ss << serialize(value.red) << " ";
                    ss << serialize(value.green) << " ";
                    ss << serialize(value.blue) << " ";
                    ss << serialize(value.white);
                    return ss.str();
                }

                template<>
                std::string serialize(const Imf::Rational& value)
                {
                    std::stringstream ss;
                    ss << value.n << " " << value.d;
                    return ss.str();
                }

            } // namespace

            void readTags(const Imf::Header& header, std::map<std::string, std::string>& tags)
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
                    tags["Channels"] = string::join(values, " ");
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
                    const auto& value = header.tileDescription();
                    {
                        std::stringstream ss;
                        ss << value.xSize << " " << value.ySize;
                        tags["Tile Size"] = ss.str();
                    }
                    tags["Tile Level Mode"] = serialize(value.mode);
                    tags["Tile Level Rounding Mode"] = serialize(value.roundingMode);
                }

                // Standard attributes.
                if (hasChromaticities(header))
                {
                    tags["Chromaticities"] = serialize(chromaticitiesAttribute(header).value());
                }
                if (hasWhiteLuminance(header))
                {
                    tags["White Luminance"] = serialize(whiteLuminanceAttribute(header).value());
                }
                if (hasAdoptedNeutral(header))
                {
                    tags["Adopted Neutral"] = serialize(adoptedNeutralAttribute(header).value());
                }
                if (hasRenderingTransform(header))
                {
                    tags["Rendering Transform"] = renderingTransformAttribute(header).value();
                }
                if (hasLookModTransform(header))
                {
                    tags["Look Modification Transform"] = lookModTransformAttribute(header).value();
                }
                if (hasXDensity(header))
                {
                    tags["X Density"] = serialize(xDensityAttribute(header).value());
                }
                if (hasOwner(header))
                {
                    tags["Owner"] = ownerAttribute(header).value();
                }
                if (hasComments(header))
                {
                    tags["Comments"] = commentsAttribute(header).value();
                }
                if (hasCapDate(header))
                {
                    tags["Capture Date"] = capDateAttribute(header).value();
                }
                if (hasUtcOffset(header))
                {
                    tags["UTC Offset"] = serialize(utcOffsetAttribute(header).value());
                }
                if (hasLongitude(header))
                {
                    tags["Longitude"] = serialize(longitudeAttribute(header).value());
                }
                if (hasLatitude(header))
                {
                    tags["Latitude"] = serialize(latitudeAttribute(header).value());
                }
                if (hasAltitude(header))
                {
                    tags["Altitude"] = serialize(altitudeAttribute(header).value());
                }
                if (hasFocus(header))
                {
                    tags["Focus"] = serialize(focusAttribute(header).value());
                }
                if (hasExpTime(header))
                {
                    tags["Exposure Time"] = serialize(expTimeAttribute(header).value());
                }
                if (hasAperture(header))
                {
                    tags["Aperture"] = serialize(apertureAttribute(header).value());
                }
                if (hasIsoSpeed(header))
                {
                    tags["ISO Speed"] = serialize(isoSpeedAttribute(header).value());
                }
                if (hasEnvmap(header))
                {
                    tags["Environment Map"] = serialize(envmapAttribute(header).value());
                }
                if (hasKeyCode(header))
                {
                    tags["Keycode"] = serialize(keyCodeAttribute(header).value());
                }
                if (hasTimeCode(header))
                {
                    tags["Timecode"] = serialize(timeCodeAttribute(header).value());
                }
                if (hasWrapmodes(header))
                {
                    tags["Wrap Modes"] = wrapmodesAttribute(header).value();
                }
                if (hasFramesPerSecond(header))
                {
                    const Imf::Rational data = framesPerSecondAttribute(header).value();
                    tags["Frame Per Second"] = string::Format("{0}").arg(data.n / static_cast<double>(data.d));
                }
                if (hasMultiView(header))
                {
                    tags["Multi-View"] = serialize(multiViewAttribute(header).value());
                }
                if (hasWorldToCamera(header))
                {
                    tags["World To Camera"] = serialize(worldToCameraAttribute(header).value());
                }
                if (hasWorldToNDC(header))
                {
                    tags["World To NDC"] = serialize(worldToNDCAttribute(header).value());
                }
                if (hasDeepImageState(header))
                {
                    tags["Deep Image State"] = serialize(deepImageStateAttribute(header).value());
                }
                if (hasOriginalDataWindow(header))
                {
                    tags["Original Data Window"] = serialize(originalDataWindowAttribute(header).value());
                }
                if (hasDwaCompressionLevel(header))
                {
                    tags["DWA Compression Level"] = serialize(dwaCompressionLevelAttribute(header).value());
                }

                // Other attributes.
                for (auto i = header.begin(); i != header.end(); ++i)
                {
                    const auto j = std::find(knownAttributes.begin(), knownAttributes.end(), i.name());
                    if (j == knownAttributes.end())
                    {
                        if ("string" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::StringAttribute>(i.name()))
                            {
                                tags[i.name()] =  ta->value();
                            }
                        }
                        else if ("stringVector" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::StringVectorAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("int" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::IntAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("float" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::FloatAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("floatVector" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::FloatVectorAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("double" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::DoubleAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("v2i" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::V2iAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("v2f" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::V2fAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("v2d" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::V2dAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("v3i" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::V3iAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("v3f" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::V3fAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("v3d" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::V3dAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("box2i" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::Box2iAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("box2f" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::Box2fAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("m33f" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::M33fAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("m33d" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::M33dAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("m44f" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::M44fAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("m44d" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::M44dAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                        else if ("rational" == std::string(i.attribute().typeName()))
                        {
                            if (const auto ta = header.findTypedAttribute<Imf::RationalAttribute>(i.name()))
                            {
                                tags[i.name()] = serialize(ta->value());
                            }
                        }
                    }
                }
            }

            void writeTags(const std::map<std::string, std::string>& tags, double speed, Imf::Header& header)
            {
                auto i = tags.find("Chromaticities");
                if (i != tags.end())
                {
                    std::stringstream ss(i->second);
                    std::vector<Imath::V2f> chromaticities;
                    chromaticities.resize(8);
                    for (size_t i = 0; i < 4; ++i)
                    {
                        ss >> chromaticities[i].x;
                        ss >> chromaticities[i].y;
                    }
                    addChromaticities(header,
                        Imf::Chromaticities(
                            chromaticities[0],
                            chromaticities[1],
                            chromaticities[2],
                            chromaticities[3]));
                }
                i = tags.find("White Luminance");
                if (i != tags.end())
                {
                    addWhiteLuminance(header, std::stof(i->second));
                }
                i = tags.find("X Density");
                if (i != tags.end())
                {
                    addXDensity(header, std::stof(i->second));
                }
                i = tags.find("Owner");
                if (i != tags.end())
                {
                    addOwner(header, i->second);
                }
                i = tags.find("Comments");
                if (i != tags.end())
                {
                    addComments(header, i->second);
                }
                i = tags.find("Capture Date");
                if (i != tags.end())
                {
                    addCapDate(header, i->second);
                }
                i = tags.find("UTC Offset");
                if (i != tags.end())
                {
                    addUtcOffset(header, std::stof(i->second));
                }
                i = tags.find("Longitude");
                if (i != tags.end())
                {
                    addLongitude(header, std::stof(i->second));
                }
                i = tags.find("Latitude");
                if (i != tags.end())
                {
                    addLatitude(header, std::stof(i->second));
                }
                i = tags.find("Altitude");
                if (i != tags.end())
                {
                    addAltitude(header, std::stof(i->second));
                }
                i = tags.find("Focus");
                if (i != tags.end())
                {
                    addFocus(header, std::stof(i->second));
                }
                i = tags.find("Exposure Time");
                if (i != tags.end())
                {
                    addExpTime(header, std::stof(i->second));
                }
                i = tags.find("Aperture");
                if (i != tags.end())
                {
                    addAperture(header, std::stof(i->second));
                }
                i = tags.find("ISO Speed");
                if (i != tags.end())
                {
                    addIsoSpeed(header, std::stof(i->second));
                }
                i = tags.find("Keycode");
                if (i != tags.end())
                {
                    int id = 0;
                    int type = 0;
                    int prefix = 0;
                    int count = 0;
                    int offset = 0;
                    time::stringToKeycode(i->second, id, type, prefix, count, offset);
                    addKeyCode(header, Imf::KeyCode(id, type, prefix, count, offset));
                }
                i = tags.find("Timecode");
                if (i != tags.end())
                {
                    uint32_t timecode = 0;
                    time::stringToTimecode(i->second, timecode);
                    addTimeCode(header, timecode);
                }
                const auto speedRational = time::toRational(speed);
                addFramesPerSecond(
                    header,
                    Imf::Rational(speedRational.first, speedRational.second));
            }

            math::BBox2i fromImath(const Imath::Box2i& value)
            {
                return math::BBox2i(math::Vector2i(value.min.x, value.min.y), math::Vector2i(value.max.x, value.max.y));
            }

            Channel fromImf(const std::string& name, const Imf::Channel& channel)
            {
                return Channel(
                    name,
                    channel.type,
                    math::Vector2i(channel.xSampling, channel.ySampling));
            }

            void Plugin::_init(const std::weak_ptr<log::System>& logSystem)
            {
                IPlugin::_init(
                    "OpenEXR",
                    { { ".exr", FileExtensionType::VideoOnly } },
                    logSystem);

                Imf::setGlobalThreadCount(0);
            }

            Plugin::Plugin()
            {}
            
            std::shared_ptr<Plugin> Plugin::create(const std::weak_ptr<log::System>& logSystem)
            {
                auto out = std::shared_ptr<Plugin>(new Plugin);
                out->_init(logSystem);
                return out;
            }

            std::shared_ptr<IRead> Plugin::read(
                const file::Path& path,
                const Options& options)
            {
                return Read::create(path, merge(options, _options), _logSystem);
            }

            imaging::Info Plugin::getWriteInfo(
                const imaging::Info& info,
                const Options& options) const
            {
                imaging::Info out;
                out.size = info.size;
                switch (info.pixelType)
                {
                case imaging::PixelType::RGBA_F16:
                    out.pixelType = info.pixelType;
                    break;
                default: break;
                }
                out.layout.mirror.y = true;
                return out;
            }

            std::shared_ptr<IWrite> Plugin::write(
                const file::Path& path,
                const Info& info,
                const Options& options)
            {
                if (info.video.empty() || (!info.video.empty() && !_isWriteCompatible(info.video[0], options)))
                    throw std::runtime_error(string::Format("{0}: {1}").
                        arg(path.get()).
                        arg("Unsupported video"));
                return Write::create(path, info, merge(options, _options), _logSystem);
            }
        }
    }
}
