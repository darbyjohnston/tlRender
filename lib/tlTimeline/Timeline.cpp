// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/Timeline.h>

#include <tlTimeline/Util.h>

#include <tlIO/IOSystem.h>

#include <tlCore/Assert.h>
#include <tlCore/Error.h>
#include <tlCore/File.h>
#include <tlCore/FileInfo.h>
#include <tlCore/LogSystem.h>
#include <tlCore/Math.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/stackAlgorithm.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/track.h>
#include <opentimelineio/transition.h>

#if defined(TLRENDER_ENABLE_PYTHON)
#include <Python.h>
#endif

#include <atomic>
#include <array>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

namespace tl
{
    namespace timeline
    {
        std::vector<std::string> getExtensions(
            int types,
            const std::shared_ptr<system::Context>& context)
        {
            std::vector<std::string> out;
            //! \todo Get extensions for the Python adapters.
            if (types & static_cast<int>(io::FileType::Movie))
            {
                out.push_back(".otio");
            }
            if (auto ioSystem = context->getSystem<io::System>())
            {
                for (const auto& plugin : ioSystem->getPlugins())
                {
                    const auto& extensions = plugin->getExtensions(types);
                    out.insert(out.end(), extensions.begin(), extensions.end());
                }
            }
            return out;
        }

        TLRENDER_ENUM_IMPL(
            FileSequenceAudio,
            "None",
            "BaseName",
            "FileName",
            "Directory");
        TLRENDER_ENUM_SERIALIZE_IMPL(FileSequenceAudio);

        bool Options::operator == (const Options& other) const
        {
            return fileSequenceAudio == other.fileSequenceAudio &&
                fileSequenceAudioFileName == other.fileSequenceAudioFileName &&
                fileSequenceAudioDirectory == other.fileSequenceAudioDirectory &&
                videoRequestCount == other.videoRequestCount &&
                audioRequestCount == other.audioRequestCount &&
                requestTimeout == other.requestTimeout &&
                ioOptions == other.ioOptions &&
                pathOptions == other.pathOptions;
        }

        bool Options::operator != (const Options& other) const
        {
            return !(*this == other);
        }

        namespace
        {
#if defined(TLRENDER_ENABLE_PYTHON)
            class PyObjectRef
            {
            public:
                PyObjectRef(PyObject* o) :
                    o(o)
                {
                    if (!o)
                    {
                        throw std::runtime_error("Python error");
                    }
                }

                ~PyObjectRef()
                {
                    Py_XDECREF(o);
                }

                PyObject* o = nullptr;

                operator PyObject* () const { return o; }
            };
#endif

            otio::SerializableObject::Retainer<otio::Timeline> readTimeline(
                const std::string& fileName,
                otio::ErrorStatus* errorStatus)
            {
                otio::SerializableObject::Retainer<otio::Timeline> out;
#if defined(TLRENDER_ENABLE_PYTHON)
                Py_Initialize();
                try
                {
                    auto pyModule = PyObjectRef(PyImport_ImportModule("opentimelineio.adapters"));

                    auto pyReadFromFile = PyObjectRef(PyObject_GetAttrString(pyModule, "read_from_file"));
                    auto pyReadFromFileArgs = PyObjectRef(PyTuple_New(1));
                    auto pyReadFromFileArg = PyUnicode_FromStringAndSize(fileName.c_str(), fileName.size());
                    if (!pyReadFromFileArg)
                    {
                        throw std::runtime_error("Cannot create arg");
                    }
                    PyTuple_SetItem(pyReadFromFileArgs, 0, pyReadFromFileArg);
                    auto pyTimeline = PyObjectRef(PyObject_CallObject(pyReadFromFile, pyReadFromFileArgs));

                    auto pyToJSONString = PyObjectRef(PyObject_GetAttrString(pyTimeline, "to_json_string"));
                    auto pyJSONString = PyObjectRef(PyObject_CallObject(pyToJSONString, NULL));
                    out = otio::SerializableObject::Retainer<otio::Timeline>(
                        dynamic_cast<otio::Timeline*>(otio::Timeline::from_json_string(
                            PyUnicode_AsUTF8AndSize(pyJSONString, NULL),
                            errorStatus)));
                }
                catch (const std::exception& e)
                {
                    errorStatus->outcome = otio::ErrorStatus::Outcome::FILE_OPEN_FAILED;
                    errorStatus->details = e.what();
                }
                if (PyErr_Occurred())
                {
                    PyErr_Print();
                }
                Py_Finalize();
#else
                out = dynamic_cast<otio::Timeline*>(otio::Timeline::from_json_file(fileName, errorStatus));
#endif
                return out;
            }

            const std::string urlFilePrefix = "file://";

            std::string removeURLFilePrefix(const std::string& value)
            {
                std::string out = value;
                if (0 == out.compare(0, urlFilePrefix.size(), urlFilePrefix))
                {
                    out.replace(0, urlFilePrefix.size(), "");
                }
                return out;
            }

            file::Path _getAudioPath(
                const file::Path& path,
                const FileSequenceAudio& fileSequenceAudio,
                const std::string& fileSequenceAudioFileName,
                const std::string& fileSequenceAudioDirectory,
                const file::PathOptions& pathOptions,
                const std::shared_ptr<system::Context>& context)
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
                            if (file::exists(audioPath))
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
                    for (const auto& fileInfo : file::dirList(directoryPath.get(), pathOptions))
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

        struct Timeline::Private
        {
            file::Path fixPath(const file::Path&) const;
            file::Path getPath(const otio::MediaReference*) const;

            bool getVideoInfo(const otio::Composable*);
            bool getAudioInfo(const otio::Composable*);

            float transitionValue(double frame, double in, double out) const;

            void tick();
            void requests();
            struct Reader
            {
                std::shared_ptr<io::IRead> read;
                io::Info info;
                otime::TimeRange range;
            };
            Reader createReader(
                const otio::Track*,
                const otio::Clip*,
                const io::Options&);
            std::future<io::VideoData> readVideo(
                const otio::Track*,
                const otio::Clip*,
                const otime::RationalTime&,
                uint16_t videoLayer);
            std::future<io::AudioData> readAudio(
                const otio::Track*,
                const otio::Clip*,
                const otime::TimeRange&);
            void stopReaders();
            void delReaders();

            std::weak_ptr<system::Context> context;
            otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;
            file::Path path;
            file::Path audioPath;
            Options options;
            otime::RationalTime duration = time::invalidTime;
            otime::RationalTime globalStartTime = time::invalidTime;
            io::Info ioInfo;
            std::vector<otime::TimeRange> activeRanges;
            std::shared_ptr<PrimitiveFactory> primitiveFactory;
            std::vector<std::shared_ptr<IPrimitive> > primitives;

            struct VideoLayerData
            {
                VideoLayerData() {};
                VideoLayerData(VideoLayerData&&) = default;

                std::future<io::VideoData> image;
                std::vector<std::shared_ptr<IPrimitive> > primitives;

                std::future<io::VideoData> imageB;
                std::vector<std::shared_ptr<IPrimitive> > primitivesB;

                Transition transition = Transition::None;
                float transitionValue = 0.F;
            };
            struct VideoRequest
            {
                VideoRequest() {};
                VideoRequest(VideoRequest&&) = default;

                otime::RationalTime time = time::invalidTime;
                uint16_t videoLayer = 0;
                std::promise<VideoData> promise;

                std::vector<VideoLayerData> layerData;
            };
            std::list<std::shared_ptr<VideoRequest> > videoRequests;
            std::list<std::shared_ptr<VideoRequest> > videoRequestsInProgress;

            struct AudioLayerData
            {
                AudioLayerData() {};
                AudioLayerData(AudioLayerData&&) = default;

                std::future<io::AudioData> audio;
                std::future<io::AudioData> audioB;
                float transitionValue = 0.F;
            };
            struct AudioRequest
            {
                AudioRequest() {};
                AudioRequest(AudioRequest&&) = default;

                int64_t seconds = -1;
                std::promise<AudioData> promise;

                std::vector<AudioLayerData> layerData;
            };
            std::list<std::shared_ptr<AudioRequest> > audioRequests;
            std::list<std::shared_ptr<AudioRequest> > audioRequestsInProgress;

            std::condition_variable requestCV;

            std::map<const otio::Clip*, Reader> readers;
            std::list<std::shared_ptr<io::IRead> > stoppedReaders;

            std::thread thread;
            std::mutex mutex;
            bool stopped = false;
            std::atomic<bool> running;

            std::chrono::steady_clock::time_point logTimer;
        };

        void Timeline::_init(
            const otio::SerializableObject::Retainer<otio::Timeline>& otioTimeline,
            const std::shared_ptr<system::Context>& context,
            const Options& options)
        {
            TLRENDER_P();

            auto logSystem = context->getLogSystem();
            {
                std::vector<std::string> lines;
                lines.push_back(std::string());
                lines.push_back(string::Format("    File sequence audio: {0}").
                    arg(options.fileSequenceAudio));
                lines.push_back(string::Format("    File sequence audio file name: {0}").
                    arg(options.fileSequenceAudioFileName));
                lines.push_back(string::Format("    File sequence audio directory: {0}").
                    arg(options.fileSequenceAudioDirectory));
                lines.push_back(string::Format("    Video request count: {0}").
                    arg(options.videoRequestCount));
                lines.push_back(string::Format("    Audio request count: {0}").
                    arg(options.audioRequestCount));
                lines.push_back(string::Format("    Request timeout: {0}ms").
                    arg(options.requestTimeout.count()));
                for (const auto& i : options.ioOptions)
                {
                    lines.push_back(string::Format("    AV I/O {0}: {1}").
                        arg(i.first).
                        arg(i.second));
                }
                lines.push_back(string::Format("    Path max number digits: {0}").
                    arg(options.pathOptions.maxNumberDigits));
                logSystem->print(
                    string::Format("tl::timeline::Timeline {0}").arg(this),
                    string::join(lines, "\n"));
            }

            p.context = context;
            p.options = options;
            p.otioTimeline = otioTimeline;

            // Get information about the timeline.
            otio::ErrorStatus errorStatus;
            auto duration = timeline::getDuration(p.otioTimeline.value, otio::Track::Kind::video);
            if (!duration.has_value())
            {
                duration = timeline::getDuration(p.otioTimeline.value, otio::Track::Kind::audio);
            }
            if (duration.has_value())
            {
                p.duration = duration.value();
            }
            if (p.otioTimeline.value->global_start_time().has_value())
            {
                p.globalStartTime = p.otioTimeline.value->global_start_time().value();
            }
            else
            {
                p.globalStartTime = otime::RationalTime(0, p.duration.rate());
            }
            for (const auto& i : p.otioTimeline.value->tracks()->children())
            {
                if (auto otioTrack = dynamic_cast<const otio::Track*>(i.value))
                {
                    if (otio::Track::Kind::video == otioTrack->kind())
                    {
                        if (p.getVideoInfo(otioTrack))
                        {
                            break;
                        }
                    }
                }
            }
            for (const auto& i : p.otioTimeline.value->tracks()->children())
            {
                if (auto otioTrack = dynamic_cast<const otio::Track*>(i.value))
                {
                    if (otio::Track::Kind::audio == otioTrack->kind())
                    {
                        if (p.getAudioInfo(otioTrack))
                        {
                            break;
                        }
                    }
                }
            }
            p.primitiveFactory = PrimitiveFactory::create(context);
            p.primitiveFactory->read(p.otioTimeline->metadata(), p.primitives);

            logSystem->print(
                string::Format("tl::timeline::Timeline {0}").arg(this),
                string::Format(
                    "\n"
                    "    Duration: {0}\n"
                    "    Global start time: {1}\n"
                    "    Video: {2} {3}\n"
                    "    Audio: {4} {5} {6}").
                arg(p.duration).
                arg(p.globalStartTime).
                arg(!p.ioInfo.video.empty() ? p.ioInfo.video[0].size : imaging::Size()).
                arg(!p.ioInfo.video.empty() ? p.ioInfo.video[0].pixelType : imaging::PixelType::None).
                arg(p.ioInfo.audio.channelCount).
                arg(p.ioInfo.audio.dataType).
                arg(p.ioInfo.audio.sampleRate));

            // Create a new thread.
            p.running = true;
            p.thread = std::thread(
                [this]
                {
                    TLRENDER_P();

                    p.logTimer = std::chrono::steady_clock::now();

                    while (p.running)
                    {
                        p.tick();
                    }

                    {
                        std::list<std::shared_ptr<Private::VideoRequest> > videoRequestsCleanup;
                        std::list<std::shared_ptr<Private::AudioRequest> > audioRequestsCleanup;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex);
                            p.stopped = true;
                            while (!p.videoRequests.empty())
                            {
                                videoRequestsCleanup.push_back(p.videoRequests.front());
                                p.videoRequests.pop_front();
                            }
                            while (!p.audioRequests.empty())
                            {
                                audioRequestsCleanup.push_back(p.audioRequests.front());
                                p.audioRequests.pop_front();
                            }
                        }
                        while (!p.videoRequestsInProgress.empty())
                        {
                            videoRequestsCleanup.push_back(p.videoRequestsInProgress.front());
                            p.videoRequestsInProgress.pop_front();
                        }
                        while (!p.audioRequestsInProgress.empty())
                        {
                            audioRequestsCleanup.push_back(p.audioRequestsInProgress.front());
                            p.audioRequestsInProgress.pop_front();
                        }
                        for (auto& request : videoRequestsCleanup)
                        {
                            VideoData data;
                            data.time = request->time;
                            for (auto& i : request->layerData)
                            {
                                VideoLayer layer;
                                if (i.image.valid())
                                {
                                    layer.image = i.image.get().image;
                                }
                                if (i.imageB.valid())
                                {
                                    layer.imageB = i.imageB.get().image;
                                }
                                layer.transition = i.transition;
                                layer.transitionValue = i.transitionValue;
                                data.layers.push_back(layer);
                            }
                            request->promise.set_value(data);
                        }
                        for (auto& request : audioRequestsCleanup)
                        {
                            AudioData data;
                            data.seconds = request->seconds;
                            for (auto& i : request->layerData)
                            {
                                AudioLayer layer;
                                if (i.audio.valid())
                                {
                                    layer.audio = i.audio.get().audio;
                                }
                                data.layers.push_back(layer);
                            }
                            request->promise.set_value(data);
                        }
                    }
                });
        }

        Timeline::Timeline() :
            _p(new Private)
        {}

        Timeline::~Timeline()
        {
            TLRENDER_P();
            p.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

        std::shared_ptr<Timeline> Timeline::create(
            const otio::SerializableObject::Retainer<otio::Timeline>& timeline,
            const std::shared_ptr<system::Context>& context,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            out->_init(timeline, context, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::string& fileName,
            const std::shared_ptr<system::Context>& context,
            const Options& options)
        {
            otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;
            bool isSequence = false;
            const file::Path path(fileName, options.pathOptions);
            file::Path audioPath;
            std::string error;
            try
            {
                auto ioSystem = context->getSystem<io::System>();
                if (auto read = ioSystem->read(path, options.ioOptions))
                {
                    const auto info = read->getInfo().get();
                    otime::RationalTime globalStartTime = time::invalidTime;
                    otio::Track* videoTrack = nullptr;
                    otio::Track* audioTrack = nullptr;
                    otio::ErrorStatus errorStatus;
                    if (!info.video.empty())
                    {
                        globalStartTime = info.videoTime.start_time();
                        auto videoClip = new otio::Clip;
                        videoClip->set_source_range(info.videoTime);
                        isSequence = io::FileType::Sequence == ioSystem->getFileType(path.getExtension()) &&
                            !path.getNumber().empty();
                        if (isSequence)
                        {
                            videoClip->set_media_reference(new otio::ImageSequenceReference(
                                path.getDirectory(),
                                path.getBaseName(),
                                path.getExtension(),
                                info.videoTime.start_time().value(),
                                1,
                                info.videoTime.duration().rate(),
                                path.getPadding()));
                        }
                        else
                        {
                            videoClip->set_media_reference(new otio::ExternalReference(path.get()));
                        }
                        videoTrack = new otio::Track("Video", otio::nullopt, otio::Track::Kind::video);
                        videoTrack->append_child(videoClip, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }

                        if (isSequence)
                        {
                            audioPath = _getAudioPath(
                                path,
                                options.fileSequenceAudio,
                                options.fileSequenceAudioFileName,
                                options.fileSequenceAudioDirectory,
                                options.pathOptions,
                                context);
                            if (!audioPath.isEmpty())
                            {
                                if (auto audioRead = ioSystem->read(audioPath, options.ioOptions))
                                {
                                    const auto audioInfo = audioRead->getInfo().get();

                                    auto audioClip = new otio::Clip;
                                    audioClip->set_source_range(audioInfo.audioTime);
                                    audioClip->set_media_reference(new otio::ExternalReference(audioPath.get()));

                                    audioTrack = new otio::Track("Audio", otio::nullopt, otio::Track::Kind::audio);
                                    audioTrack->append_child(audioClip, &errorStatus);
                                    if (otio::is_error(errorStatus))
                                    {
                                        throw std::runtime_error("Cannot append child");
                                    }
                                }
                            }
                        }
                    }

                    if (!audioTrack && info.audio.isValid())
                    {
                        auto audioClip = new otio::Clip;
                        audioClip->set_source_range(info.audioTime);
                        audioClip->set_media_reference(new otio::ExternalReference(path.get()));

                        audioTrack = new otio::Track("Audio", otio::nullopt, otio::Track::Kind::audio);
                        audioTrack->append_child(audioClip, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }

                    auto otioStack = new otio::Stack;
                    if (videoTrack)
                    {
                        otioStack->append_child(videoTrack, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }
                    if (audioTrack)
                    {
                        otioStack->append_child(audioTrack, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }

                    otioTimeline = new otio::Timeline(path.get());
                    otioTimeline->set_tracks(otioStack);
                    if (globalStartTime != time::invalidTime)
                    {
                        otioTimeline->set_global_start_time(globalStartTime);
                    }
                }
            }
            catch (const std::exception& e)
            {
                error = e.what();
            }

            auto logSystem = context->getLogSystem();
            logSystem->print(
                "tl::timeline::Timeline",
                string::Format(
                    "\n"
                    "    Create from path: {0}\n"
                    "    Audio path: {1}").
                arg(path.get()).
                arg(audioPath.get()));

            if (!otioTimeline)
            {
                otio::ErrorStatus errorStatus;
                otioTimeline = readTimeline(path.get(), &errorStatus);
                if (otio::is_error(errorStatus))
                {
                    otioTimeline = nullptr;
                    error = errorStatus.full_description;
                }
                else if (!otioTimeline)
                {
                    error = "Cannot read timeline";
                }
            }
            if (!otioTimeline)
            {
                throw std::runtime_error(error);
            }

            auto out = std::shared_ptr<Timeline>(new Timeline);
            out->_p->path = path;
            out->_p->audioPath = audioPath;
            out->_init(otioTimeline, context, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::string& fileName,
            const std::string& audioFileName,
            const std::shared_ptr<system::Context>& context,
            const Options& options)
        {
            otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;
            bool isSequence = false;
            std::string error;
            const file::Path path(fileName, options.pathOptions);
            const file::Path audioPath(audioFileName, options.pathOptions);
            try
            {
                auto ioSystem = context->getSystem<io::System>();
                if (auto read = ioSystem->read(path, options.ioOptions))
                {
                    const auto info = read->getInfo().get();
                    otime::RationalTime globalStartTime = time::invalidTime;
                    otio::Track* videoTrack = nullptr;
                    otio::Track* audioTrack = nullptr;
                    otio::ErrorStatus errorStatus;
                    if (!info.video.empty())
                    {
                        globalStartTime = otime::RationalTime(0.0, info.videoTime.duration().rate());
                        auto videoClip = new otio::Clip;
                        videoClip->set_source_range(info.videoTime);
                        isSequence = io::FileType::Sequence == ioSystem->getFileType(path.getExtension()) &&
                            !path.getNumber().empty();
                        if (isSequence)
                        {
                            globalStartTime = info.videoTime.start_time();
                            videoClip->set_media_reference(new otio::ImageSequenceReference(
                                path.getDirectory(),
                                path.getBaseName(),
                                path.getExtension(),
                                info.videoTime.start_time().value(),
                                1,
                                info.videoTime.duration().rate(),
                                path.getPadding()));
                        }
                        else
                        {
                            videoClip->set_media_reference(new otio::ExternalReference(path.get()));
                        }
                        videoTrack = new otio::Track("Video", otio::nullopt, otio::Track::Kind::video);
                        videoTrack->append_child(videoClip, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }

                    if (auto audioRead = ioSystem->read(audioPath, options.ioOptions))
                    {
                        const auto audioInfo = audioRead->getInfo().get();

                        auto audioClip = new otio::Clip;
                        audioClip->set_source_range(audioInfo.audioTime);
                        audioClip->set_media_reference(new otio::ExternalReference(audioPath.get()));

                        audioTrack = new otio::Track("Audio", otio::nullopt, otio::Track::Kind::audio);
                        audioTrack->append_child(audioClip, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }

                    auto otioStack = new otio::Stack;
                    if (videoTrack)
                    {
                        otioStack->append_child(videoTrack, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }
                    if (audioTrack)
                    {
                        otioStack->append_child(audioTrack, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }

                    otioTimeline = new otio::Timeline(path.get());
                    otioTimeline->set_tracks(otioStack);
                    if (globalStartTime != time::invalidTime)
                    {
                        otioTimeline->set_global_start_time(globalStartTime);
                    }
                }
            }
            catch (const std::exception& e)
            {
                error = e.what();
            }

            auto logSystem = context->getLogSystem();
            logSystem->print(
                "tl::timeline::Timeline",
                string::Format(
                    "\n"
                    "    Create from path: {0}\n"
                    "    Audio path: {1}").
                arg(path.get()).
                arg(audioPath.get()));

            if (!otioTimeline)
            {
                otio::ErrorStatus errorStatus;
                otioTimeline = readTimeline(path.get(), &errorStatus);
                if (otio::is_error(errorStatus))
                {
                    otioTimeline = nullptr;
                    error = errorStatus.full_description;
                }
                else if (!otioTimeline)
                {
                    error = "Cannot read timeline";
                }
            }
            if (!otioTimeline)
            {
                throw std::runtime_error(error);
            }

            auto out = std::shared_ptr<Timeline>(new Timeline);
            out->_p->path = path;
            out->_p->audioPath = audioPath;
            out->_init(otioTimeline, context, options);
            return out;
        }

        const std::weak_ptr<system::Context>& Timeline::getContext() const
        {
            return _p->context;
        }
        
        const otio::SerializableObject::Retainer<otio::Timeline>& Timeline::getTimeline() const
        {
            return _p->otioTimeline;
        }

        const file::Path& Timeline::getPath() const
        {
            return _p->path;
        }

        const file::Path& Timeline::getAudioPath() const
        {
            return _p->audioPath;
        }

        const Options& Timeline::getOptions() const
        {
            return _p->options;
        }

        const otime::RationalTime& Timeline::getGlobalStartTime() const
        {
            return _p->globalStartTime;
        }

        const otime::RationalTime& Timeline::getDuration() const
        {
            return _p->duration;
        }

        const io::Info& Timeline::getIOInfo() const
        {
            return _p->ioInfo;
        }

        void Timeline::setActiveRanges(const std::vector<otime::TimeRange>& ranges)
        {
            _p->activeRanges = ranges;
        }

        std::future<VideoData> Timeline::getVideo(const otime::RationalTime& time, uint16_t videoLayer)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->time = time;
            request->videoLayer = videoLayer;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                if (!p.stopped)
                {
                    valid = true;
                    p.videoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.requestCV.notify_one();
            }
            else
            {
                request->promise.set_value(VideoData());
            }
            return future;
        }

        std::future<AudioData> Timeline::getAudio(int64_t seconds)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::AudioRequest>();
            request->seconds = seconds;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                if (!p.stopped)
                {
                    valid = true;
                    p.audioRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.requestCV.notify_one();
            }
            else
            {
                request->promise.set_value(AudioData());
            }
            return future;
        }

        void Timeline::cancelRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::VideoRequest> > videoRequestsCleanup;
            std::list<std::shared_ptr<Private::AudioRequest> > audioRequestsCleanup;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                videoRequestsCleanup = std::move(p.videoRequests);
                audioRequestsCleanup = std::move(p.audioRequests);
            }
            for (auto& request : videoRequestsCleanup)
            {
                request->promise.set_value(VideoData());
            }
            for (auto& request : audioRequestsCleanup)
            {
                request->promise.set_value(AudioData());
            }
            for (auto& i : p.readers)
            {
                i.second.read->cancelRequests();
            }
        }

        file::Path Timeline::Private::fixPath(const file::Path& path) const
        {
            std::string directory;
            if (!path.isAbsolute())
            {
                directory = this->path.getDirectory();
            }
            return file::Path(directory, path.get(), options.pathOptions);
        }

        file::Path Timeline::Private::getPath(const otio::MediaReference* ref) const
        {
            file::Path out;
            if (auto externalRef = dynamic_cast<const otio::ExternalReference*>(ref))
            {
                const std::string url = removeURLFilePrefix(externalRef->target_url());
                out = file::Path(url, options.pathOptions);
            }
            else if (auto imageSequenceRef = dynamic_cast<const otio::ImageSequenceReference*>(ref))
            {
                const std::string urlBase = removeURLFilePrefix(imageSequenceRef->target_url_base());
                std::stringstream ss;
                ss << urlBase <<
                    imageSequenceRef->name_prefix() <<
                    std::setfill('0') << std::setw(imageSequenceRef->frame_zero_padding()) << imageSequenceRef->start_frame() <<
                    imageSequenceRef->name_suffix();
                out = file::Path(ss.str(), options.pathOptions);
            }
            return fixPath(out);
        }

        bool Timeline::Private::getVideoInfo(const otio::Composable* composable)
        {
            if (auto clip = dynamic_cast<const otio::Clip*>(composable))
            {
                if (auto context = this->context.lock())
                {
                    // The first video clip defines the video information for the timeline.
                    io::Options ioOptions = options.ioOptions;
                    otio::ErrorStatus errorStatus;
                    ioOptions["SequenceIO/DefaultSpeed"] = string::Format("{0}").arg(clip->duration(&errorStatus).rate());
                    const file::Path path(getPath(clip->media_reference()));
                    if (auto read = context->getSystem<io::System>()->read(path, ioOptions))
                    {
                        const auto ioInfo = read->getInfo().get();
                        this->ioInfo.video = ioInfo.video;
                        this->ioInfo.videoTime = ioInfo.videoTime;
                        this->ioInfo.tags.insert(ioInfo.tags.begin(), ioInfo.tags.end());
                        return true;
                    }
                }
            }
            if (auto composition = dynamic_cast<const otio::Composition*>(composable))
            {
                for (const auto& child : composition->children())
                {
                    if (getVideoInfo(child))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        bool Timeline::Private::getAudioInfo(const otio::Composable* composable)
        {
            if (auto clip = dynamic_cast<const otio::Clip*>(composable))
            {
                if (auto context = this->context.lock())
                {
                    // The first audio clip defines the audio information for the timeline.
                    io::Options ioOptions = options.ioOptions;
                    if (auto read = context->getSystem<io::System>()->read(getPath(clip->media_reference()), ioOptions))
                    {
                        const auto ioInfo = read->getInfo().get();
                        this->ioInfo.audio = ioInfo.audio;
                        this->ioInfo.audioTime = ioInfo.audioTime;
                        this->ioInfo.tags.insert(ioInfo.tags.begin(), ioInfo.tags.end());
                        return true;
                    }
                }
            }
            if (auto composition = dynamic_cast<const otio::Composition*>(composable))
            {
                for (const auto& child : composition->children())
                {
                    if (getAudioInfo(child))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        float Timeline::Private::transitionValue(double frame, double in, double out) const
        {
            return (frame - in) / (out - in);
        }

        void Timeline::Private::tick()
        {
            requests();
            stopReaders();
            delReaders();

            // Logging.
            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = now - logTimer;
            if (diff.count() > 10.F)
            {
                logTimer = now;
                if (auto context = this->context.lock())
                {
                    size_t videoRequestsSize = 0;
                    size_t audioRequestsSize = 0;
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        videoRequestsSize = videoRequests.size();
                        audioRequestsSize = audioRequests.size();
                    }
                    auto logSystem = context->getLogSystem();
                    logSystem->print(
                        string::Format("tl::timeline::Timeline {0}").arg(this),
                        string::Format(
                        "\n"
                        "    Path: {0}\n"
                        "    Video requests: {1}, {2} in-progress, {3} max\n"
                        "    Audio requests: {4}, {5} in-progress, {6} max\n"
                        "    Readers: {7}").
                        arg(path.get()).
                        arg(videoRequestsSize).
                        arg(videoRequestsInProgress.size()).
                        arg(options.videoRequestCount).
                        arg(audioRequestsSize).
                        arg(audioRequestsInProgress.size()).
                        arg(options.audioRequestCount).
                        arg(readers.size()));
                }
            }

            // Sleep for a bit...
            time::sleep(std::chrono::milliseconds(1));
        }

        void Timeline::Private::requests()
        {
            // Gather requests.
            std::list<std::shared_ptr<VideoRequest> > newVideoRequests;
            std::list<std::shared_ptr<AudioRequest> > newAudioRequests;
            {
                std::unique_lock<std::mutex> lock(mutex);
                requestCV.wait_for(
                    lock,
                    options.requestTimeout,
                    [this]
                    {
                        return !videoRequests.empty() ||
                            !videoRequestsInProgress.empty() ||
                            !audioRequests.empty() ||
                            !audioRequestsInProgress.empty();
                    });
                while (!videoRequests.empty() &&
                    (videoRequestsInProgress.size() + newVideoRequests.size()) < options.videoRequestCount)
                {
                    newVideoRequests.push_back(videoRequests.front());
                    videoRequests.pop_front();
                }
                while (!audioRequests.empty() &&
                    (audioRequestsInProgress.size() + newAudioRequests.size()) < options.audioRequestCount)
                {
                    newAudioRequests.push_back(audioRequests.front());
                    audioRequests.pop_front();
                }
            }

            // Traverse the timeline for new video requests.
            for (auto& request : newVideoRequests)
            {
                try
                {
                    for (const auto& otioTrack : otioTimeline->video_tracks())
                    {
                        for (const auto& otioChild : otioTrack->children())
                        {
                            if (auto otioItem = dynamic_cast<otio::Item*>(otioChild.value))
                            {
                                const auto time = request->time - globalStartTime;
                                otio::ErrorStatus errorStatus;
                                const auto range = otioItem->trimmed_range_in_parent(&errorStatus);
                                if (range.has_value() && range.value().contains(time))
                                {
                                    VideoLayerData videoData;
                                    if (auto otioClip = dynamic_cast<const otio::Clip*>(otioItem))
                                    {
                                        if (readers.find(otioClip) == readers.end())
                                        {
                                            auto reader = createReader(otioTrack, otioClip, options.ioOptions);
                                            if (reader.read)
                                            {
                                                readers[otioClip] = reader;
                                            }
                                        }
                                        videoData.image = readVideo(otioTrack, otioClip, time, request->videoLayer);
                                        videoData.primitives = primitives;
                                        primitiveFactory->read(otioClip->metadata(), videoData.primitives);
                                    }
                                    const auto neighbors = otioTrack->neighbors_of(otioItem, &errorStatus);
                                    if (auto otioTransition = dynamic_cast<otio::Transition*>(neighbors.second.value))
                                    {
                                        if (time > range.value().end_time_inclusive() - otioTransition->in_offset())
                                        {
                                            videoData.transition = toTransition(otioTransition->transition_type());
                                            videoData.transitionValue = transitionValue(
                                                time.value(),
                                                range.value().end_time_inclusive().value() - otioTransition->in_offset().value(),
                                                range.value().end_time_inclusive().value() + otioTransition->out_offset().value() + 1.0);
                                            const auto transitionNeighbors = otioTrack->neighbors_of(otioTransition, &errorStatus);
                                            if (const auto otioClipB = dynamic_cast<otio::Clip*>(transitionNeighbors.second.value))
                                            {
                                                if (readers.find(otioClipB) == readers.end())
                                                {
                                                    auto reader = createReader(otioTrack, otioClipB, options.ioOptions);
                                                    if (reader.read)
                                                    {
                                                        readers[otioClipB] = reader;
                                                    }
                                                }
                                                videoData.imageB = readVideo(otioTrack, otioClipB, time, request->videoLayer);
                                                videoData.primitivesB = primitives;
                                                primitiveFactory->read(otioClipB->metadata(), videoData.primitivesB);
                                            }
                                        }
                                    }
                                    if (auto otioTransition = dynamic_cast<otio::Transition*>(neighbors.first.value))
                                    {
                                        if (time < range.value().start_time() + otioTransition->out_offset())
                                        {
                                            std::swap(videoData.image, videoData.imageB);
                                            std::swap(videoData.primitives, videoData.primitivesB);
                                            videoData.transition = toTransition(otioTransition->transition_type());
                                            videoData.transitionValue = transitionValue(
                                                time.value(),
                                                range.value().start_time().value() - otioTransition->in_offset().value() - 1.0,
                                                range.value().start_time().value() + otioTransition->out_offset().value());
                                            const auto transitionNeighbors = otioTrack->neighbors_of(otioTransition, &errorStatus);
                                            if (const auto otioClipB = dynamic_cast<otio::Clip*>(transitionNeighbors.first.value))
                                            {
                                                if (readers.find(otioClipB) == readers.end())
                                                {
                                                    auto reader = createReader(otioTrack, otioClipB, options.ioOptions);
                                                    if (reader.read)
                                                    {
                                                        readers[otioClipB] = reader;
                                                    }
                                                }
                                                videoData.image = readVideo(otioTrack, otioClipB, time, request->videoLayer);
                                                videoData.primitives = primitives;
                                                primitiveFactory->read(otioClipB->metadata(), videoData.primitives);
                                            }
                                        }
                                    }
                                    request->layerData.push_back(std::move(videoData));
                                }
                            }
                        }
                    }
                }
                catch (const std::exception&)
                {
                    //! \todo How should this be handled?
                }

                videoRequestsInProgress.push_back(request);
            }

            // Traverse the timeline for new audio requests.
            for (auto& request : newAudioRequests)
            {
                try
                {
                    for (const auto& otioTrack : otioTimeline->audio_tracks())
                    {
                        for (const auto& otioChild : otioTrack->children())
                        {
                            if (auto otioItem = dynamic_cast<otio::Item*>(otioChild.value))
                            {
                                const otime::RationalTime time = time::floor(
                                    otime::RationalTime(request->seconds, 1.0) - globalStartTime.rescaled_to(1.0));
                                const otime::TimeRange timeRange = otime::TimeRange(time, otime::RationalTime(1.0, 1.0));
                                otio::ErrorStatus errorStatus;
                                const auto range = otioItem->trimmed_range_in_parent(&errorStatus);
                                if (range.has_value() && range.value().intersects(timeRange))
                                {
                                    AudioLayerData audioData;
                                    if (auto otioClip = dynamic_cast<const otio::Clip*>(otioItem))
                                    {
                                        if (readers.find(otioClip) == readers.end())
                                        {
                                            auto reader = createReader(otioTrack, otioClip, options.ioOptions);
                                            if (reader.read)
                                            {
                                                readers[otioClip] = reader;
                                            }
                                        }
                                        audioData.audio = readAudio(otioTrack, otioClip, timeRange);
                                    }
                                    request->layerData.push_back(std::move(audioData));
                                }
                            }
                        }
                    }
                }
                catch (const std::exception&)
                {
                    //! \todo How should this be handled?
                }

                audioRequestsInProgress.push_back(request);
            }

            // Check for finished video requests.
            auto videoRequestIt = videoRequestsInProgress.begin();
            while (videoRequestIt != videoRequestsInProgress.end())
            {
                bool valid = true;
                for (auto& i : (*videoRequestIt)->layerData)
                {
                    if (i.image.valid())
                    {
                        valid &= i.image.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                    }
                    if (i.imageB.valid())
                    {
                        valid &= i.imageB.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                    }
                }
                if (valid)
                {
                    VideoData data;
                    data.time = (*videoRequestIt)->time;
                    try
                    {
                        for (auto& j : (*videoRequestIt)->layerData)
                        {
                            VideoLayer layer;
                            if (j.image.valid())
                            {
                                layer.image = j.image.get().image;
                            }
                            layer.primitives = j.primitives;
                            if (j.imageB.valid())
                            {
                                layer.imageB = j.imageB.get().image;
                            }
                            layer.primitivesB = j.primitivesB;
                            layer.transition = j.transition;
                            layer.transitionValue = j.transitionValue;
                            data.layers.push_back(layer);
                        }
                    }
                    catch (const std::exception&)
                    {
                        //! \todo How should this be handled?
                    }
                    (*videoRequestIt)->promise.set_value(data);
                    videoRequestIt = videoRequestsInProgress.erase(videoRequestIt);
                    continue;
                }
                ++videoRequestIt;
            }

            // Check for finished audio requests.
            auto audioRequestIt = audioRequestsInProgress.begin();
            while (audioRequestIt != audioRequestsInProgress.end())
            {
                bool valid = true;
                for (auto& i : (*audioRequestIt)->layerData)
                {
                    if (i.audio.valid())
                    {
                        valid &= i.audio.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                    }
                    if (i.audioB.valid())
                    {
                        valid &= i.audioB.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                    }
                }
                if (valid)
                {
                    AudioData data;
                    data.seconds = (*audioRequestIt)->seconds;
                    try
                    {
                        for (auto& j : (*audioRequestIt)->layerData)
                        {
                            AudioLayer layer;
                            if (j.audio.valid())
                            {
                                layer.audio = j.audio.get().audio;
                            }
                            data.layers.push_back(layer);
                        }
                    }
                    catch (const std::exception&)
                    {
                        //! \todo How should this be handled?
                    }
                    (*audioRequestIt)->promise.set_value(data);
                    audioRequestIt = audioRequestsInProgress.erase(audioRequestIt);
                    continue;
                }
                ++audioRequestIt;
            }
        }

        Timeline::Private::Reader Timeline::Private::createReader(
            const otio::Track* track,
            const otio::Clip* clip,
            const io::Options& ioOptions)
        {
            Reader out;
            if (auto context = this->context.lock())
            {
                const file::Path path = getPath(clip->media_reference());
                io::Options options = ioOptions;
                options["SequenceIO/DefaultSpeed"] = string::Format("{0}").arg(duration.rate());
                const auto ioSystem = context->getSystem<io::System>();
                auto read = ioSystem->read(path, options);
                io::Info info;
                if (read)
                {
                    info = read->getInfo().get();
                }
                if (read)
                {
                    context->log(
                        string::Format("tl::timeline::Timeline {0}").arg(this),
                        string::Format(
                            "\n"
                            "    Read: {0}\n"
                            "    Video: {1} {2}\n"
                            "    Video time: {3}\n"
                            "    Audio: {4} {5} {6}\n"
                            "    Audio time: {7}").
                        arg(path.get()).
                        arg(!info.video.empty() ? info.video[0].size : imaging::Size()).
                        arg(!info.video.empty() ? info.video[0].pixelType : imaging::PixelType::None).
                        arg(info.videoTime).
                        arg(info.audio.channelCount).
                        arg(info.audio.dataType).
                        arg(info.audio.sampleRate).
                        arg(info.audioTime));

                    // Get the clip start and end time taking transitions into account.
                    otio::ErrorStatus errorStatus;
                    const auto range = clip->trimmed_range(&errorStatus);
                    otime::RationalTime startTime = range.start_time();
                    auto endTime = startTime + range.duration();
                    const auto neighbors = track->neighbors_of(clip, &errorStatus);
                    if (auto transition = dynamic_cast<const otio::Transition*>(neighbors.first.value))
                    {
                        startTime -= transition->in_offset();
                    }
                    if (auto transition = dynamic_cast<const otio::Transition*>(neighbors.second.value))
                    {
                        endTime += transition->out_offset();
                    }

                    out.read = read;
                    out.info = info;
                    const auto ancestor = dynamic_cast<const otio::Item*>(getRoot(clip));
                    out.range = clip->transformed_time_range(
                        otime::TimeRange::range_from_start_end_time(globalStartTime + startTime, globalStartTime + endTime),
                        ancestor,
                        &errorStatus);
                }
            }
            return out;
        }

        std::future<io::VideoData> Timeline::Private::readVideo(
            const otio::Track* track,
            const otio::Clip* clip,
            const otime::RationalTime& time,
            uint16_t videoLayer)
        {
            std::future<io::VideoData> out;
            if (auto context = this->context.lock())
            {
                const auto j = readers.find(clip);
                if (j != readers.end())
                {
                    otio::ErrorStatus errorStatus;
                    const auto clipTime = track->transformed_time(time, clip, &errorStatus);
                    const auto readTime = time::floor(clipTime.rescaled_to(j->second.info.videoTime.duration().rate()));
                    out = j->second.read->readVideo(readTime, videoLayer);
                }
            }
            return out;
        }

        std::future<io::AudioData> Timeline::Private::readAudio(
            const otio::Track* track,
            const otio::Clip* clip,
            const otime::TimeRange& timeRange)
        {
            std::future<io::AudioData> out;
            if (auto context = this->context.lock())
            {
                const auto j = readers.find(clip);
                if (j != readers.end())
                {
                    otio::ErrorStatus errorStatus;
                    const auto clipRange = track->transformed_time_range(timeRange, clip, &errorStatus);
                    const auto readRange = otime::TimeRange(
                        time::floor(clipRange.start_time().rescaled_to(ioInfo.audio.sampleRate)),
                        time::ceil(clipRange.duration().rescaled_to(ioInfo.audio.sampleRate)));
                    out = j->second.read->readAudio(readRange);
                }
            }
            return out;
        }

        void Timeline::Private::stopReaders()
        {
            auto i = readers.begin();
            while (i != readers.end())
            {
                bool del = true;
                for (const auto& activeRange : activeRanges)
                {
                    if (i->second.range.intersects(activeRange))
                    {
                        del = false;
                        break;
                    }
                }
                if (del && !i->second.read->hasRequests())
                {
                    if (auto context = this->context.lock())
                    {
                        context->log("tl::timeline::Timeline", path.get() + ": Stop: " + i->second.read->getPath().get());
                    }
                    auto read = i->second.read;
                    read->stop();
                    stoppedReaders.push_back(read);
                    i = readers.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }

        void Timeline::Private::delReaders()
        {
            auto i = stoppedReaders.begin();
            while (i != stoppedReaders.end())
            {
                if ((*i)->hasStopped())
                {
                    if (auto context = this->context.lock())
                    {
                        context->log("tl::timeline::Timeline", path.get() + ": Delete: " + (*i)->getPath().get());
                    }
                    i = stoppedReaders.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }
    }
}
