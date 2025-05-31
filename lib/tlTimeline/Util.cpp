// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/Util.h>

#include <tlTimeline/MemoryReference.h>

#include <tlIO/System.h>

#include <tlCore/FileInfo.h>
#include <tlCore/URL.h>

#include <dtk/core/Assert.h>
#include <dtk/core/Context.h>
#include <dtk/core/Error.h>
#include <dtk/core/Format.h>
#include <dtk/core/String.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>

#include <ctime>

#include <mz.h>
#include <mz_os.h>
#include <mz_strm.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>

namespace tl
{
    namespace timeline
    {
        std::vector<std::string> getExtensions(
            const std::shared_ptr<dtk::Context>& context,
            int types)
        {
            std::vector<std::string> out;
            if (types & static_cast<int>(io::FileType::Media))
            {
                out.push_back(".otio");
                out.push_back(".otioz");
            }
            if (auto ioSystem = context->getSystem<io::ReadSystem>())
            {
                for (const auto& plugin : ioSystem->getPlugins())
                {
                    const auto& extensions = plugin->getExtensions(types);
                    out.insert(out.end(), extensions.begin(), extensions.end());
                }
            }
            return out;
        }

        std::vector<OTIO_NS::TimeRange> toRanges(std::vector<OTIO_NS::RationalTime> frames)
        {
            std::vector<OTIO_NS::TimeRange> out;
            if (!frames.empty())
            {
                std::sort(frames.begin(), frames.end());
                auto i = frames.begin();
                auto j = i;
                do
                {
                    auto k = j + 1;
                    if (k != frames.end() && (*k - *j).value() > 1)
                    {
                        out.push_back(OTIO_NS::TimeRange::range_from_start_end_time_inclusive(*i, *j));
                        i = k;
                        j = k;
                    }
                    else if (k == frames.end())
                    {
                        out.push_back(OTIO_NS::TimeRange::range_from_start_end_time_inclusive(*i, *j));
                        i = k;
                        j = k;
                    }
                    else
                    {
                        ++j;
                    }
                } while (j != frames.end());
            }
            return out;
        }

        OTIO_NS::RationalTime loop(
            const OTIO_NS::RationalTime& value,
            const OTIO_NS::TimeRange& range,
            bool* looped)
        {
            auto out = value;
            const OTIO_NS::RationalTime duration = range.duration();
            if (duration.value() > 0.0)
            {
                while (out < range.start_time())
                {
                    if (looped)
                    {
                        *looped = true;
                    }
                    out += range.duration();
                }
                while (out > range.end_time_inclusive())
                {
                    if (looped)
                    {
                        *looped = true;
                    }
                    out -= range.duration();
                }
            }
            return out;
        }

        int64_t loop(
            int64_t value,
            const OTIO_NS::TimeRange& range,
            bool* looped)
        {
            return loop(
                OTIO_NS::RationalTime(value, 1.0),
                OTIO_NS::TimeRange(
                    range.start_time().rescaled_to(1.0),
                    range.duration().rescaled_to(1.0)),
                looped).value();
        }

        DTK_ENUM_IMPL(
            CacheDirection,
            "Forward",
            "Reverse");

        const OTIO_NS::Composable* getRoot(const OTIO_NS::Composable* composable)
        {
            const OTIO_NS::Composable* out = composable;
            for (; out->parent(); out = out->parent())
                ;
            return out;
        }

        std::optional<OTIO_NS::RationalTime> getDuration(
            const OTIO_NS::Timeline* otioTimeline,
            const std::string& kind)
        {
            std::optional<OTIO_NS::RationalTime> out;
            OTIO_NS::ErrorStatus errorStatus;
            for (auto track : otioTimeline->find_children<OTIO_NS::Track>(&errorStatus))
            {
                if (kind == track->kind())
                {
                    const OTIO_NS::RationalTime duration = track->duration(&errorStatus);
                    if (out.has_value())
                    {
                        out = std::max(out.value(), duration);
                    }
                    else
                    {
                        out = duration;
                    }
                }
            }
            return out;
        }

        OTIO_NS::TimeRange getTimeRange(const OTIO_NS::Timeline* otioTimeline)
        {
            OTIO_NS::TimeRange out = time::invalidTimeRange;
            auto duration = timeline::getDuration(otioTimeline, OTIO_NS::Track::Kind::video);
            if (!duration.has_value())
            {
                duration = timeline::getDuration(otioTimeline, OTIO_NS::Track::Kind::audio);
            }
            if (duration.has_value())
            {
                const OTIO_NS::RationalTime startTime = otioTimeline->global_start_time().has_value() ?
                    otioTimeline->global_start_time().value().rescaled_to(duration->rate()) :
                    OTIO_NS::RationalTime(0, duration->rate());
                out = OTIO_NS::TimeRange(startTime, duration.value());
            }
            return out;
        }

        std::vector<file::Path> getPaths(
            const std::shared_ptr<dtk::Context>& context,
            const file::Path& path,
            const file::PathOptions& pathOptions)
        {
            std::vector<file::Path> out;
            const auto fileInfo = file::FileInfo(path);
            switch (fileInfo.getType())
            {
            case file::Type::Directory:
            {
                auto ioSystem = context->getSystem<io::ReadSystem>();
                file::ListOptions listOptions;
                listOptions.maxNumberDigits = pathOptions.maxNumberDigits;
                std::vector<file::FileInfo> list;
                file::list(path.get(-1, file::PathType::Path), list, listOptions);
                for (const auto& fileInfo : list)
                {
                    const file::Path& path = fileInfo.getPath();
                    const std::string extension = dtk::toLower(path.getExtension());
                    switch (ioSystem->getFileType(extension))
                    {
                    case io::FileType::Media:
                    case io::FileType::Sequence:
                        out.push_back(path);
                        break;
                    default:
                        //! \todo Get extensions for the Python adapters?
                        if (".otio" == extension ||
                            ".otioz" == extension)
                        {
                            out.push_back(path);
                        }
                        break;
                    }
                }
                break;
            }
            default:
                out.push_back(path);
                break;
            }
            return out;
        }

        file::Path getPath(
            const std::string& url,
            const std::string& directory,
            const file::PathOptions& pathOptions)
        {
            file::Path out(url::decode(url), pathOptions);
            if (out.isFileProtocol() && !out.isAbsolute())
            {
                out.setDirectory(file::appendSeparator(directory) + out.getDirectory());
            }
            return out;
        }

        file::Path getPath(
            const OTIO_NS::MediaReference* ref,
            const std::string& directory,
            file::PathOptions pathOptions)
        {
            std::string url;
            dtk::RangeI sequence;
            if (auto externalRef = dynamic_cast<const OTIO_NS::ExternalReference*>(ref))
            {
                url = externalRef->target_url();
                pathOptions.maxNumberDigits = 0;
            }
            else if (auto imageSequenceRef = dynamic_cast<const OTIO_NS::ImageSequenceReference*>(ref))
            {
                std::stringstream ss;
                ss << imageSequenceRef->target_url_base() <<
                    imageSequenceRef->name_prefix() <<
                    std::setfill('0') << std::setw(imageSequenceRef->frame_zero_padding()) <<
                    imageSequenceRef->start_frame() <<
                    imageSequenceRef->name_suffix();
                url = ss.str();
                sequence = dtk::RangeI(
                    imageSequenceRef->start_frame(),
                    imageSequenceRef->end_frame());
            }
            else if (auto rawMemoryRef = dynamic_cast<const RawMemoryReference*>(ref))
            {
                url = rawMemoryRef->target_url();
                pathOptions.maxNumberDigits = 0;
            }
            else if (auto sharedMemoryRef = dynamic_cast<const SharedMemoryReference*>(ref))
            {
                url = sharedMemoryRef->target_url();
                pathOptions.maxNumberDigits = 0;
            }
            else if (auto rawMemorySequenceRef = dynamic_cast<const RawMemorySequenceReference*>(ref))
            {
                url = rawMemorySequenceRef->target_url();
            }
            else if (auto sharedMemorySequenceRef = dynamic_cast<const SharedMemorySequenceReference*>(ref))
            {
                url = sharedMemorySequenceRef->target_url();
            }
            file::Path out = timeline::getPath(url, directory, pathOptions);
            if (sequence.min() != sequence.max())
            {
                out.setSequence(sequence);
            }
            return out;
        }

        std::vector<dtk::InMemoryFile> getMemoryRead(
            const OTIO_NS::MediaReference* ref)
        {
            std::vector<dtk::InMemoryFile> out;
            if (auto rawMemoryReference =
                dynamic_cast<const RawMemoryReference*>(ref))
            {
                out.push_back(dtk::InMemoryFile(
                    rawMemoryReference->memory(),
                    rawMemoryReference->memory_size()));
            }
            else if (auto sharedMemoryReference =
                dynamic_cast<const SharedMemoryReference*>(ref))
            {
                if (const auto& memory = sharedMemoryReference->memory())
                {
                    out.push_back(dtk::InMemoryFile(
                        memory->data(),
                        memory->size()));
                }
            }
            else if (auto rawMemorySequenceReference =
                dynamic_cast<const RawMemorySequenceReference*>(ref))
            {
                const auto& memory = rawMemorySequenceReference->memory();
                const size_t memory_size = memory.size();
                const auto& memory_sizes = rawMemorySequenceReference->memory_sizes();
                const size_t memory_sizes_size = memory_sizes.size();
                for (size_t i = 0; i < memory_size && i < memory_sizes_size; ++i)
                {
                    out.push_back(dtk::InMemoryFile(memory[i], memory_sizes[i]));
                }
            }
            else if (auto sharedMemorySequenceReference =
                dynamic_cast<const SharedMemorySequenceReference*>(ref))
            {
                for (const auto& memory : sharedMemorySequenceReference->memory())
                {
                    if (memory)
                    {
                        out.push_back(dtk::InMemoryFile(memory->data(), memory->size()));
                    }
                }
            }
            return out;
        }

        DTK_ENUM_IMPL(
            ToMemoryReference,
            "Shared",
            "Raw");

        void toMemoryReferences(
            OTIO_NS::Timeline* otioTimeline,
            const std::string& directory,
            ToMemoryReference toMemoryReference,
            const file::PathOptions& pathOptions)
        {
            // Recursively iterate over all clips in the timeline.
            for (auto clip : otioTimeline->find_children<OTIO_NS::Clip>())
            {
                if (auto ref = dynamic_cast<OTIO_NS::ExternalReference*>(clip->media_reference()))
                {
                    // Get the external reference path.
                    const auto path = getPath(ref->target_url(), directory, pathOptions);

                    // Read the external reference into memory.
                    auto fileIO = dtk::FileIO::create(path.get(), dtk::FileMode::Read);
                    const size_t size = fileIO->getSize();

                    // Replace the external reference with a memory reference.
                    switch (toMemoryReference)
                    {
                    case ToMemoryReference::Shared:
                    {
                        auto memory = std::make_shared<MemoryReferenceData>();
                        memory->resize(size);
                        fileIO->read(memory->data(), size);
                        clip->set_media_reference(new SharedMemoryReference(
                            ref->target_url(),
                            memory,
                            clip->available_range(),
                            ref->metadata()));
                        break;
                    }
                    case ToMemoryReference::Raw:
                    {
                        uint8_t* memory = new uint8_t [size];
                        fileIO->read(memory, size);
                        clip->set_media_reference(new RawMemoryReference(
                            ref->target_url(),
                            memory,
                            size,
                            clip->available_range(),
                            ref->metadata()));
                        break;
                    }
                    default: break;
                    }
                }
                else if (auto ref = dynamic_cast<OTIO_NS::ImageSequenceReference*>(
                    clip->media_reference()))
                {
                    // Get the image sequence reference path.
                    const int padding = ref->frame_zero_padding();
                    std::stringstream ss;
                    ss << ref->target_url_base() <<
                        ref->name_prefix() <<
                        std::setfill('0') << std::setw(padding) << ref->start_frame() <<
                        ref->name_suffix();
                    const auto path = getPath(ss.str(), directory, pathOptions);

                    // Read the image sequence reference into memory.
                    std::vector<std::shared_ptr<MemoryReferenceData> > sharedMemoryList;
                    std::vector<const uint8_t*> rawMemoryList;
                    std::vector<size_t> rawMemorySizeList;
                    const auto range = clip->trimmed_range();
                    for (
                        int64_t frame = ref->start_frame();
                        frame < ref->start_frame() + range.duration().value();
                        ++frame)
                    {
                        const auto fileName = path.get(frame);
                        auto fileIO = dtk::FileIO::create(fileName, dtk::FileMode::Read);
                        const size_t size = fileIO->getSize();
                        switch (toMemoryReference)
                        {
                        case ToMemoryReference::Shared:
                        {
                            auto memory = std::make_shared<MemoryReferenceData>();
                            memory->resize(size);
                            fileIO->read(memory->data(), size);
                            sharedMemoryList.push_back(memory);
                            break;
                        }
                        case ToMemoryReference::Raw:
                        {
                            auto memory = new uint8_t [size];
                            fileIO->read(memory, size);
                            rawMemoryList.push_back(memory);
                            rawMemorySizeList.push_back(size);
                            break;
                        }
                        default: break;
                        }
                    }

                    // Replace the image sequence reference with a memory
                    // sequence reference.
                    switch (toMemoryReference)
                    {
                    case ToMemoryReference::Shared:
                        clip->set_media_reference(new SharedMemorySequenceReference(
                            path.get(),
                            sharedMemoryList,
                            clip->available_range(),
                            ref->metadata()));
                        break;
                    case ToMemoryReference::Raw:
                        clip->set_media_reference(new RawMemorySequenceReference(
                            path.get(),
                            rawMemoryList,
                            rawMemorySizeList,
                            clip->available_range(),
                            ref->metadata()));
                        break;
                    default: break;
                    }
                }
            }
        }

        OTIO_NS::RationalTime toVideoMediaTime(
            const OTIO_NS::RationalTime& time,
            const OTIO_NS::TimeRange& trimmedRangeInParent,
            const OTIO_NS::TimeRange& trimmedRange,
            double rate)
        {
            OTIO_NS::RationalTime out =
                time - trimmedRangeInParent.start_time() + trimmedRange.start_time();
            out = out.rescaled_to(rate).round();
            return out;
        }

        OTIO_NS::TimeRange toAudioMediaTime(
            const OTIO_NS::TimeRange& timeRange,
            const OTIO_NS::TimeRange& trimmedRangeInParent,
            const OTIO_NS::TimeRange& trimmedRange,
            double sampleRate)
        {
            OTIO_NS::TimeRange out = OTIO_NS::TimeRange(
                timeRange.start_time() - trimmedRangeInParent.start_time() + trimmedRange.start_time(),
                timeRange.duration());
            out = OTIO_NS::TimeRange(
                out.start_time().rescaled_to(sampleRate).round(),
                out.duration().rescaled_to(sampleRate).round());
            return out;
        }

        std::vector<std::shared_ptr<audio::Audio> > audioCopy(
            const audio::Info& info,
            const std::vector<AudioData>& data,
            Playback playback,
            int64_t frame,
            int64_t size)
        {
            std::vector<std::shared_ptr<audio::Audio> > out;

            // Adjust the frame for reverse playback.
            if (Playback::Reverse == playback)
            {
                frame -= size;
            }

            // Find the first chunk of audio data.
            const int64_t seconds = std::floor(frame / static_cast<double>(info.sampleRate));
            auto secondsIt = std::find_if(
                data.begin(),
                data.end(),
                [seconds](const AudioData& data)
                {
                    return seconds == data.seconds;
                });

            // Find the second chunk of audio data.
            const int64_t secondsPlusOne = seconds + 1;
            auto secondsPlusOneIt = std::find_if(
                data.begin(),
                data.end(),
                [secondsPlusOne](const AudioData& data)
                {
                    return secondsPlusOne == data.seconds;
                });

            if (secondsIt != data.end())
            {
                // Adjust the size if necessary.
                const int64_t offset = frame - seconds * info.sampleRate;
                int64_t outSize = size;
                if ((offset + outSize) > info.sampleRate && secondsPlusOneIt == data.end())
                {
                    outSize = info.sampleRate - offset;
                }

                // Create the output audio.
                for (size_t i = 0; i < secondsIt->layers.size(); ++i)
                {
                    auto audio = audio::Audio::create(info, outSize);
                    audio->zero();
                    out.push_back(audio);
                }

                // Copy audio from the first chunk.
                const int64_t sizeTmp = std::min(outSize, static_cast<int64_t>(info.sampleRate) - offset);
                for (size_t i = 0; i < secondsIt->layers.size(); ++i)
                {
                    if (secondsIt->layers[i].audio &&
                        secondsIt->layers[i].audio->getInfo() == info)
                    {
                        memcpy(
                            out[i]->getData(),
                            secondsIt->layers[i].audio->getData() + offset * info.getByteCount(),
                            sizeTmp * info.getByteCount());
                    }
                }

                if (sizeTmp < outSize && secondsPlusOneIt != data.end())
                {
                    // Copy audio from the second chunk.
                    for (size_t i = 0; i < secondsIt->layers.size() && i < secondsPlusOneIt->layers.size(); ++i)
                    {
                        if (secondsPlusOneIt->layers[i].audio &&
                            secondsPlusOneIt->layers[i].audio->getInfo() == info)
                        {
                            memcpy(
                                out[i]->getData() + sizeTmp * info.getByteCount(),
                                secondsPlusOneIt->layers[i].audio->getData(),
                                (outSize - sizeTmp) * info.getByteCount());
                        }
                    }
                }
            }

            return out;
        }

        namespace
        {
            class OTIOZWriter
            {
            public:
                OTIOZWriter(
                    const std::string& fileName,
                    const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&,
                    const std::string& directory = std::string());

                ~OTIOZWriter();

            private:
                void _addCompressed(
                    const std::string& content,
                    const std::string& fileNameInZip);
                void _addUncompressed(
                    const std::string& fileName,
                    const std::string& fileNameInZip);

                static std::string _getMediaFileName(
                    const std::string& url,
                    const std::string& directory);
                static std::string _getFileNameInZip(const std::string& url);
                static std::string _normzalizePathSeparators(const std::string&);
                static bool _isFileNameAbsolute(const std::string&);

                std::string _fileName;
                void* _writer = nullptr;
            };

            OTIOZWriter::OTIOZWriter(
                const std::string& fileName,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
                const std::string& directory)
            {
                _fileName = fileName;

                // Copy the timeline.
                OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> timelineCopy(
                    dynamic_cast<OTIO_NS::Timeline*>(
                        OTIO_NS::Timeline::from_json_string(timeline->to_json_string())));

                // Find the media references.
                std::map<std::string, std::string> mediaFilesNames;
                std::string directoryTmp = _normzalizePathSeparators(directory);
                if (!directoryTmp.empty() && directoryTmp.back() != '/')
                {
                    directoryTmp += '/';
                }
                for (const auto& clip : timelineCopy->find_clips())
                {
                    if (auto ref = dynamic_cast<OTIO_NS::ExternalReference*>(clip->media_reference()))
                    {
                        const std::string& url = ref->target_url();
                        const std::string mediaFileName = _getMediaFileName(url, directoryTmp);
                        const std::string fileNameInZip = _getFileNameInZip(url);
                        mediaFilesNames[mediaFileName] = fileNameInZip;
                        ref->set_target_url(fileNameInZip);
                    }
                    else if (auto ref = dynamic_cast<OTIO_NS::ImageSequenceReference*>(clip->media_reference()))
                    {
                        const int padding = ref->frame_zero_padding();
                        std::stringstream ss;
                        ss << ref->target_url_base() <<
                            ref->name_prefix() <<
                            std::setfill('0') << std::setw(padding) << ref->start_frame() <<
                            ref->name_suffix();
                        const file::Path path(_getMediaFileName(ss.str(), directoryTmp));
                        const auto range = clip->trimmed_range();
                        for (
                            int64_t frame = ref->start_frame();
                            frame < ref->start_frame() + range.duration().value();
                            ++frame)
                        {
                            const std::string mediaFileName = path.get(frame);
                            const std::string fileNameInZip = _getFileNameInZip(mediaFileName);
                            mediaFilesNames[mediaFileName] = fileNameInZip;
                        }
                        ref->set_target_url_base(_getFileNameInZip(ref->target_url_base()));
                    }
                }

                // Open the output file.
                mz_zip_writer_create(&_writer);
                if (!_writer)
                {
                    throw std::runtime_error(dtk::Format("Cannot create writer: \"{0}\"").arg(fileName));
                }
                int32_t err = mz_zip_writer_open_file(_writer, fileName.c_str(), 0, 0);
                if (err != MZ_OK)
                {
                    throw std::runtime_error(dtk::Format("Cannot open output file: \"{0}\"").arg(fileName));
                }

                // Add the content and version files.
                _addCompressed("1.0.0", "version.txt");
                _addCompressed(timelineCopy->to_json_string(), "content.otio");

                // Add the media files.
                for (const auto& i : mediaFilesNames)
                {
                    _addUncompressed(i.first, i.second);
                }

                // Close the file.
                err = mz_zip_writer_close(_writer);
                if (err != MZ_OK)
                {
                    throw std::runtime_error(dtk::Format("Cannot close output file: \"{0}\"").arg(fileName));
                }
            }

            OTIOZWriter::~OTIOZWriter()
            {
                if (_writer)
                {
                    mz_zip_writer_delete(&_writer);
                }
            }

            void OTIOZWriter::_addCompressed(
                const std::string& content,
                const std::string& fileNameInZip)
            {
                mz_zip_file fileInfo;
                memset(&fileInfo, 0, sizeof(mz_zip_file));
                mz_zip_writer_set_compress_level(_writer, MZ_COMPRESS_LEVEL_NORMAL);
                fileInfo.version_madeby = MZ_VERSION_MADEBY;
                fileInfo.flag = MZ_ZIP_FLAG_UTF8;
                fileInfo.modified_date = std::time(nullptr);
                fileInfo.compression_method = MZ_COMPRESS_METHOD_DEFLATE;
                fileInfo.filename = fileNameInZip.c_str();
                int32_t err = mz_zip_writer_add_buffer(
                    _writer,
                    (void*)content.c_str(),
                    content.size(),
                    &fileInfo);
                if (err != MZ_OK)
                {
                    throw std::runtime_error(dtk::Format("Cannot add file: \"{0}\"").arg(_fileName));
                }
            }

            void OTIOZWriter::_addUncompressed(
                const std::string& fileName,
                const std::string& fileNameInZip)
            {
                /*auto fileIO = dtk::FileIO::create(fileName, dtk::FileMode::Read);
                std::vector<uint8_t> buf(fileIO->getSize());
                fileIO->read(buf.data(), buf.size());
                mz_zip_file fileInfo;
                memset(&fileInfo, 0, sizeof(mz_zip_file));
                fileInfo.version_madeby = MZ_VERSION_MADEBY;
                fileInfo.modified_date = std::time(nullptr);
                fileInfo.compression_method = MZ_COMPRESS_METHOD_STORE;
                fileInfo.filename = fileNameInZip.c_str();
                int32_t err = mz_zip_writer_add_buffer(
                    _writer,
                    (void*)buf.data(),
                    buf.size(),
                    &fileInfo);
                if (err != MZ_OK)
                {
                    throw std::runtime_error(dtk::Format("Cannot add file: \"{0}\"").arg(_fileName));
                }*/
                mz_zip_writer_set_compress_method(
                    _writer,
                    MZ_COMPRESS_METHOD_STORE);
                int32_t err = mz_zip_writer_add_file(
                    _writer,
                    fileName.c_str(),
                    fileNameInZip.c_str());
                if (err != MZ_OK)
                {
                    throw std::runtime_error(dtk::Format("Cannot add file: \"{0}\"").arg(fileName));
                }
            }

            std::string OTIOZWriter::_getFileNameInZip(const std::string& url)
            {                
                std::string::size_type r = url.rfind('/');
                if (std::string::npos == r)
                {
                    r = url.rfind('\\');
                }
                const std::string fileName =
                    std::string::npos == r ?
                    url :
                    url.substr(r + 1);
                return "media/" + fileName;
            }

            std::string OTIOZWriter::_getMediaFileName(
                const std::string& url,
                const std::string& directory)
            {
                std::string fileName = url;
                if ("file://" == fileName.substr(7))
                {
                    fileName.erase(0, 7);
                }
                if (!_isFileNameAbsolute(fileName))
                {
                    fileName = directory + fileName;
                }
                return fileName;
            }

            std::string OTIOZWriter::_normzalizePathSeparators(const std::string& fileName)
            {
                std::string out = fileName;
                for (size_t i = 0; i < out.size(); ++i)
                {
                    if ('\\' == out[i])
                    {
                        out[i] = '/';
                    }
                }
                return out;
            }

            bool OTIOZWriter::_isFileNameAbsolute(const std::string& fileName)
            {
                bool out = false;
                if (!fileName.empty() && '/' == fileName[0])
                {
                    out = true;
                }
                else if (!fileName.empty() && '\\' == fileName[0])
                {
                    out = true;
                }
                else if (fileName.size() >= 2 &&
                    (fileName[0] >= 'A' && fileName[0] <= 'Z' ||
                        fileName[0] >= 'a' && fileName[0] <= 'z') &&
                    ':' == fileName[1])
                {
                    out = true;
                }
                return out;
            }
        }

        bool writeOTIOZ(
            const std::string& fileName,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
            const std::string& directory)
        {
            bool out = false;
            try
            {
                OTIOZWriter(fileName, timeline, directory);
                out = true;
            }
            catch (const std::exception&)
            {}
            return out;
        }
    }
}
