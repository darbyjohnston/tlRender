// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimelinePrivate.h>

#include <tlTimeline/MemoryReference.h>
#include <tlTimeline/Util.h>

#include <tlIO/System.h>

#include <tlCore/FileInfo.h>

#include <dtk/core/Context.h>
#include <dtk/core/Format.h>
#include <dtk/core/String.h>

#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>

#include <mz.h>
#include <mz_strm.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>

#include <filesystem>

namespace tl
{
    namespace timeline
    {
        namespace
        {
            file::Path getAudioPath(
                const std::shared_ptr<dtk::Context>& context,
                const file::Path& path,
                const FileSequenceAudio& fileSequenceAudio,
                const std::string& fileSequenceAudioFileName,
                const std::string& fileSequenceAudioDirectory,
                const file::PathOptions& pathOptions)
            {
                file::Path out;
                auto ioSystem = context->getSystem<io::System>();
                const auto audioExtensions = ioSystem->getExtensions(static_cast<int>(io::FileType::Audio));
                switch (fileSequenceAudio)
                {
                case FileSequenceAudio::BaseName:
                {
                    std::vector<std::string> names;
                    names.push_back(path.getDirectory() + path.getBaseName());
                    std::string tmp = path.getBaseName();
                    if (!tmp.empty() && '.' == tmp[tmp.size() - 1])
                    {
                        tmp.pop_back();
                    }
                    names.push_back(path.getDirectory() + tmp);
                    for (const auto& name : names)
                    {
                        for (const auto& extension : audioExtensions)
                        {
                            const file::Path audioPath(name + extension, pathOptions);
                            if (std::filesystem::exists(std::filesystem::u8path(audioPath.get())))
                            {
                                out = audioPath;
                                break;
                            }
                        }
                    }
                    break;
                }
                case FileSequenceAudio::FileName:
                    out = file::Path(path.getDirectory() + fileSequenceAudioFileName, pathOptions);
                    break;
                case FileSequenceAudio::Directory:
                {
                    const file::Path directoryPath(path.getDirectory(), fileSequenceAudioDirectory, pathOptions);
                    file::ListOptions listOptions;
                    listOptions.maxNumberDigits = pathOptions.maxNumberDigits;
                    std::vector<file::FileInfo> list;
                    file::list(directoryPath.get(), list, listOptions);
                    for (const auto& fileInfo : list)
                    {
                        if (file::Type::File == fileInfo.getType())
                        {
                            for (const auto& extension : audioExtensions)
                            {
                                if (extension == fileInfo.getPath().getExtension())
                                {
                                    out = fileInfo.getPath();
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
                default: break;
                }
                return out;
            }
        }

        class ZipReader
        {
        public:
            ZipReader(const std::string& fileName)
            {
                mz_zip_reader_create(&reader);
                if (!reader)
                {
                    throw std::runtime_error(dtk::Format(
                        "{0}: Cannot create zip reader").arg(fileName));
                }
                int32_t err = mz_zip_reader_open_file(reader, fileName.c_str());
                if (err != MZ_OK)
                {
                    throw std::runtime_error(dtk::Format(
                        "{0}: Cannot open zip reader").arg(fileName));
                }
            }

            ~ZipReader()
            {
                mz_zip_reader_delete(&reader);
            }

            void* reader = nullptr;
        };

        class ZipReaderFile
        {
        public:
            ZipReaderFile(void* reader, const std::string& fileName) :
                reader(reader)
            {
                int32_t err = mz_zip_reader_entry_open(reader);
                if (err != MZ_OK)
                {
                    throw std::runtime_error(dtk::Format(
                        "{0}: Cannot open zip entry").arg(fileName));
                }
            }

            ~ZipReaderFile()
            {
                mz_zip_reader_entry_close(reader);
            }

            void* reader = nullptr;
        };

        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> readOTIO(
            const file::Path& path,
            OTIO_NS::ErrorStatus* errorStatus)
        {
            OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> out;
            const std::string fileName = path.get();
            const std::string extension = dtk::toLower(path.getExtension());
            if (".otio" == extension)
            {
                out = dynamic_cast<OTIO_NS::Timeline*>(
                    OTIO_NS::Timeline::from_json_file(fileName, errorStatus));
            }
            else if (".otioz" == extension)
            {
                {
                    ZipReader zipReader(fileName);

                    const std::string contentFileName = "content.otio";
                    int32_t err = mz_zip_reader_locate_entry(
                        zipReader.reader,
                        contentFileName.c_str(),
                        0);
                    if (err != MZ_OK)
                    {
                        throw std::runtime_error(dtk::Format(
                            "{0}: Cannot find zip entry").arg(contentFileName));
                    }
                    mz_zip_file* fileInfo = nullptr;
                    err = mz_zip_reader_entry_get_info(zipReader.reader, &fileInfo);
                    if (err != MZ_OK)
                    {
                        throw std::runtime_error(dtk::Format(
                            "{0}: Cannot get zip entry information").arg(contentFileName));
                    }
                    ZipReaderFile zipReaderFile(zipReader.reader, contentFileName);
                    std::vector<char> buf;
                    buf.resize(fileInfo->uncompressed_size + 1);
                    err = mz_zip_reader_entry_read(
                        zipReader.reader,
                        buf.data(),
                        fileInfo->uncompressed_size);
                    if (err != fileInfo->uncompressed_size)
                    {
                        throw std::runtime_error(dtk::Format(
                            "{0}: Cannot read zip entry").arg(contentFileName));
                    }
                    buf[fileInfo->uncompressed_size] = 0;

                    out = dynamic_cast<OTIO_NS::Timeline*>(
                        OTIO_NS::Timeline::from_json_string(buf.data(), errorStatus));

                    auto fileIO = dtk::FileIO::create(fileName, dtk::FileMode::Read);
                    for (auto clip : out->find_children<OTIO_NS::Clip>())
                    {
                        if (auto externalReference =
                            dynamic_cast<OTIO_NS::ExternalReference*>(clip->media_reference()))
                        {
                            const std::string mediaFileName = file::Path(
                                externalReference->target_url()).get();

                            int32_t err = mz_zip_reader_locate_entry(zipReader.reader, mediaFileName.c_str(), 0);
                            if (err != MZ_OK)
                            {
                                throw std::runtime_error(dtk::Format(
                                    "{0}: Cannot find zip entry").arg(mediaFileName));
                            }
                            err = mz_zip_reader_entry_get_info(zipReader.reader, &fileInfo);
                            if (err != MZ_OK)
                            {
                                throw std::runtime_error(dtk::Format(
                                    "{0}: Cannot get zip entry information").arg(mediaFileName));
                            }

                            const size_t headerSize =
                                30 +
                                fileInfo->filename_size +
                                fileInfo->extrafield_size;
                            auto memoryReference = new ZipMemoryReference(
                                fileIO,
                                externalReference->target_url(),
                                fileIO->getMemoryStart() +
                                fileInfo->disk_offset +
                                headerSize,
                                fileInfo->uncompressed_size,
                                externalReference->available_range(),
                                externalReference->metadata());
                            clip->set_media_reference(memoryReference);
                        }
                        else if (auto imageSequenceReference =
                            dynamic_cast<OTIO_NS::ImageSequenceReference*>(clip->media_reference()))
                        {
                            std::vector<const uint8_t*> memory;
                            std::vector<size_t> memory_sizes;
                            for (int number = 0;
                                number < imageSequenceReference->number_of_images_in_sequence();
                                ++number)
                            {
                                const std::string mediaFileName = file::Path(
                                    imageSequenceReference->target_url_for_image_number(number)).get();

                                int32_t err = mz_zip_reader_locate_entry(zipReader.reader, mediaFileName.c_str(), 0);
                                if (err != MZ_OK)
                                {
                                    throw std::runtime_error(dtk::Format(
                                        "{0}: Cannot find zip entry").arg(mediaFileName));
                                }
                                err = mz_zip_reader_entry_get_info(zipReader.reader, &fileInfo);
                                if (err != MZ_OK)
                                {
                                    throw std::runtime_error(dtk::Format(
                                        "{0}: Cannot get zip entry information").arg(mediaFileName));
                                }

                                const size_t headerSize =
                                    30 +
                                    fileInfo->filename_size +
                                    fileInfo->extrafield_size;
                                memory.push_back(
                                    fileIO->getMemoryStart() +
                                    fileInfo->disk_offset +
                                    headerSize);
                                memory_sizes.push_back(fileInfo->uncompressed_size);
                            }
                            auto memoryReference = new ZipMemorySequenceReference(
                                fileIO,
                                imageSequenceReference->target_url_for_image_number(0),
                                memory,
                                memory_sizes,
                                imageSequenceReference->available_range(),
                                imageSequenceReference->metadata());
                            clip->set_media_reference(memoryReference);
                        }
                    }
                }
            }
            return out;
        }

        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> create(
            const std::shared_ptr<dtk::Context>& context,
            const file::Path& path,
            const Options& options)
        {
            return create(context, path, file::Path(), options);
        }

        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> create(
            const std::shared_ptr<dtk::Context>& context,
            const file::Path& inputPath,
            const file::Path& inputAudioPath,
            const Options& options)
        {
            OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> out;
            std::string error;
            file::Path path = inputPath;
            file::Path audioPath = inputAudioPath;
            try
            {
                auto ioSystem = context->getSystem<io::System>();

                // Is the input a sequence?
                const bool isSequence =
                    io::FileType::Sequence == ioSystem->getFileType(path.getExtension()) &&
                    !path.getNumber().empty();
                if (isSequence)
                {
                    if (!path.isSequence())
                    {
                        // Check for other files in the sequence.
                        std::vector<file::FileInfo> list;
                        file::ListOptions listOptions;
                        listOptions.sequenceExtensions = { path.getExtension() };
                        listOptions.maxNumberDigits = options.pathOptions.maxNumberDigits;
                        file::list(path.getDirectory(), list, listOptions);
                        const auto i = std::find_if(
                            list.begin(),
                            list.end(),
                            [path](const file::FileInfo& value)
                            {
                                return value.getPath().sequence(path);
                            });
                        if (i != list.end())
                        {
                            path = i->getPath();
                        }
                    }
                    if (audioPath.isEmpty())
                    {
                        // Check for an associated audio file.
                        audioPath = getAudioPath(
                            context,
                            path,
                            options.fileSequenceAudio,
                            options.fileSequenceAudioFileName,
                            options.fileSequenceAudioDirectory,
                            options.pathOptions);
                    }
                }

                // Is the input a video or audio file?
                if (auto read = ioSystem->read(path, options.ioOptions))
                {
                    const auto info = read->getInfo().get();

                    OTIO_NS::RationalTime startTime = time::invalidTime;
                    OTIO_NS::Track* videoTrack = nullptr;
                    OTIO_NS::Track* audioTrack = nullptr;
                    OTIO_NS::ErrorStatus errorStatus;

                    // Read the video.
                    if (!info.video.empty())
                    {
                        startTime = info.videoTime.start_time();
                        auto videoClip = new OTIO_NS::Clip;
                        videoClip->set_source_range(info.videoTime);
                        if (isSequence)
                        {
                            auto mediaReference = new OTIO_NS::ImageSequenceReference(
                                path.getProtocol() + path.getDirectory(),
                                path.getBaseName(),
                                path.getExtension(),
                                info.videoTime.start_time().value(),
                                1,
                                info.videoTime.duration().rate(),
                                path.getPadding());
                            mediaReference->set_available_range(info.videoTime);
                            videoClip->set_media_reference(mediaReference);
                        }
                        else
                        {
                            videoClip->set_media_reference(new OTIO_NS::ExternalReference(
                                path.get(),
                                info.videoTime));
                        }
                        videoTrack = new OTIO_NS::Track("Video", std::nullopt, OTIO_NS::Track::Kind::video);
                        videoTrack->append_child(videoClip, &errorStatus);
                        if (OTIO_NS::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }

                    // Read the separate audio if provided.
                    if (!audioPath.isEmpty())
                    {
                        if (auto audioRead = ioSystem->read(audioPath, options.ioOptions))
                        {
                            const auto audioInfo = audioRead->getInfo().get();

                            auto audioClip = new OTIO_NS::Clip;
                            audioClip->set_source_range(audioInfo.audioTime);
                            audioClip->set_media_reference(new OTIO_NS::ExternalReference(
                                audioPath.get(),
                                audioInfo.audioTime));

                            audioTrack = new OTIO_NS::Track("Audio", std::nullopt, OTIO_NS::Track::Kind::audio);
                            audioTrack->append_child(audioClip, &errorStatus);
                            if (OTIO_NS::is_error(errorStatus))
                            {
                                throw std::runtime_error("Cannot append child");
                            }
                        }
                    }
                    else if (info.audio.isValid())
                    {
                        if (startTime.is_invalid_time())
                        {
                            startTime = info.audioTime.start_time();
                        }

                        auto audioClip = new OTIO_NS::Clip;
                        audioClip->set_source_range(info.audioTime);
                        audioClip->set_media_reference(new OTIO_NS::ExternalReference(
                            path.get(),
                            info.audioTime));

                        audioTrack = new OTIO_NS::Track("Audio", std::nullopt, OTIO_NS::Track::Kind::audio);
                        audioTrack->append_child(audioClip, &errorStatus);
                        if (OTIO_NS::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }

                    // Create the stack.
                    auto otioStack = new OTIO_NS::Stack;
                    if (videoTrack)
                    {
                        otioStack->append_child(videoTrack, &errorStatus);
                        if (OTIO_NS::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }
                    if (audioTrack)
                    {
                        otioStack->append_child(audioTrack, &errorStatus);
                        if (OTIO_NS::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }

                    // Create the timeline.
                    out = new OTIO_NS::Timeline(path.get());
                    out->set_tracks(otioStack);
                    if (time::isValid(startTime))
                    {
                        out->set_global_start_time(startTime);
                    }
                }
            }
            catch (const std::exception& e)
            {
                error = e.what();
            }

            auto logSystem = context->getLogSystem();
            logSystem->print(
                "tl::timeline::create",
                dtk::Format(
                    "\n"
                    "    Create from path: {0}\n"
                    "    Audio path: {1}").
                arg(path.get()).
                arg(audioPath.get()));

            // Is the input an OTIO file?
            if (!out)
            {
                OTIO_NS::ErrorStatus errorStatus;
                out = readOTIO(path, &errorStatus);
                if (OTIO_NS::is_error(errorStatus))
                {
                    out = nullptr;
                    error = errorStatus.full_description;
                }
                else if (!out)
                {
                    error = dtk::Format("{0}: Cannot read timeline").arg(path.get());
                }
            }
            if (!out)
            {
                throw std::runtime_error(error);
            }

            OTIO_NS::AnyDictionary dict;
            dict["path"] = path.get();
            dict["audioPath"] = audioPath.get();
            out->metadata()["tlRender"] = dict;

            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<dtk::Context>& context,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            out->_init(context, timeline, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::string& fileName,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            auto otioTimeline = timeline::create(
                context,
                file::Path(fileName, options.pathOptions),
                options);
            out->_init(context, otioTimeline, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<dtk::Context>& context,
            const file::Path& path,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            auto otioTimeline = timeline::create(context, path, options);
            out->_init(context, otioTimeline, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::string& fileName,
            const std::string& audioFileName,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            auto otioTimeline = timeline::create(
                context,
                file::Path(fileName, options.pathOptions),
                file::Path(audioFileName, options.pathOptions),
                options);
            out->_init(context, otioTimeline, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<dtk::Context>& context,
            const file::Path& path,
            const file::Path& audioPath,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            auto otioTimeline = timeline::create(
                context,
                path,
                audioPath,
                options);
            out->_init(context, otioTimeline, options);
            return out;
        }
    }
}
