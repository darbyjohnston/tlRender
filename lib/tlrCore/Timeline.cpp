// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/Timeline.h>

#include <tlrCore/AVIOSystem.h>
#include <tlrCore/Assert.h>
#include <tlrCore/Error.h>
#include <tlrCore/File.h>
#include <tlrCore/LogSystem.h>
#include <tlrCore/Math.h>
#include <tlrCore/String.h>
#include <tlrCore/StringFormat.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/stackAlgorithm.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/track.h>
#include <opentimelineio/transition.h>

#if defined(TLR_ENABLE_PYTHON)
#include <Python.h>
#endif

#include <atomic>
#include <array>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

namespace tlr
{
    namespace timeline
    {
        std::vector<std::string> getExtensions()
        {
            //! \todo Get extensions for the Python adapters.
            return { ".otio" };
        }

        std::vector<otime::TimeRange> toRanges(std::vector<otime::RationalTime> frames)
        {
            std::vector<otime::TimeRange> out;
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
                        out.push_back(otime::TimeRange::range_from_start_end_time_inclusive(*i, *j));
                        i = k;
                        j = k;
                    }
                    else if (k == frames.end())
                    {
                        out.push_back(otime::TimeRange::range_from_start_end_time_inclusive(*i, *j));
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

        const otio::Composable* getRoot(const otio::Composable* composable)
        {
            const otio::Composable* out = composable;
            for (; out->parent(); out = out->parent())
                ;
            return out;
        }

        bool Options::operator == (const Options& other) const
        {
            return videoRequestCount == other.videoRequestCount &&
                audioRequestCount == other.audioRequestCount &&
                requestTimeout == other.requestTimeout &&
                avioOptions == other.avioOptions;
        }

        bool Options::operator != (const Options& other) const
        {
            return !(*this == other);
        }

        TLR_ENUM_IMPL(
            Transition,
            "None",
            "Dissolve");
        TLR_ENUM_SERIALIZE_IMPL(Transition);

        Transition toTransition(const std::string& value)
        {
            Transition out = Transition::None;
            if (otio::Transition::Type::SMPTE_Dissolve == value)
            {
                out = Transition::Dissolve;
            }
            return out;
        }

        bool VideoLayer::operator == (const VideoLayer& other) const
        {
            return image == other.image &&
                imageB == other.imageB &&
                transition == other.transition &&
                transitionValue == other.transitionValue;
        }

        bool VideoLayer::operator != (const VideoLayer& other) const
        {
            return !(*this == other);
        }

        bool VideoData::operator == (const VideoData& other) const
        {
            return time == other.time &&
                layers == other.layers;
        }

        bool VideoData::operator != (const VideoData& other) const
        {
            return !(*this == other);
        }

        bool AudioLayer::operator == (const AudioLayer& other) const
        {
            return audio == other.audio;
        }

        bool AudioLayer::operator != (const AudioLayer& other) const
        {
            return !(*this == other);
        }

        bool AudioData::operator == (const AudioData& other) const
        {
            return seconds == other.seconds &&
                layers == other.layers;
        }

        bool AudioData::operator != (const AudioData& other) const
        {
            return !(*this == other);
        }

        namespace
        {
#if defined(TLR_ENABLE_PYTHON)
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
#if defined(TLR_ENABLE_PYTHON)
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
        }

        struct Timeline::Private
        {
            file::Path fixPath(const file::Path&) const;
            file::Path getPath(const otio::ImageSequenceReference*) const;
            file::Path getPath(const otio::MediaReference*) const;

            bool getAVInfo(const otio::Composable*);

            float transitionValue(double frame, double in, double out) const;

            void init();
            void tick();
            void requests();
            void createReader(
                const otio::Track*,
                const otio::Clip*,
                const avio::Options&);
            std::future<avio::VideoData> readVideo(
                const otio::Track*,
                const otio::Clip*,
                const otime::RationalTime&,
                uint16_t videoLayer);
            std::future<avio::AudioData> readAudio(
                const otio::Track*,
                const otio::Clip*,
                const otime::TimeRange&);
            void stopReaders();
            void delReaders();

            std::weak_ptr<core::Context> context;
            otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;
            file::Path path;
            Options options;
            otime::RationalTime duration = time::invalidTime;
            otime::RationalTime globalStartTime = time::invalidTime;
            avio::Info avInfo;
            std::vector<otime::TimeRange> activeRanges;

            struct Item
            {
                const otio::Track* track = nullptr;
                const otio::Item* item = nullptr;
                otime::TimeRange range;
            };
            std::vector<Item> items;

            struct VideoLayerData
            {
                VideoLayerData() {};
                VideoLayerData(VideoLayerData&&) = default;

                std::future<avio::VideoData> image;
                std::future<avio::VideoData> imageB;
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
            std::list<VideoRequest> videoRequests;
            std::list<VideoRequest> videoRequestsInProgress;

            struct AudioLayerData
            {
                AudioLayerData() {};
                AudioLayerData(AudioLayerData&&) = default;

                std::future<avio::AudioData> audio;
                std::future<avio::AudioData> audioB;
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
            std::list<AudioRequest> audioRequests;
            std::list<AudioRequest> audioRequestsInProgress;

            std::condition_variable requestCV;

            struct Reader
            {
                std::shared_ptr<avio::IRead> read;
                avio::Info info;
                otime::TimeRange range;
            };
            std::map<const otio::Clip*, Reader> readers;
            std::list<std::shared_ptr<avio::IRead> > stoppedReaders;

            std::thread thread;
            std::mutex mutex;
            bool stopped = false;
            std::atomic<bool> running;

            std::chrono::steady_clock::time_point logTimer;
        };

        void Timeline::_init(
            const otio::SerializableObject::Retainer<otio::Timeline>& otioTimeline,
            const std::shared_ptr<core::Context>& context,
            const Options& options)
        {
            TLR_PRIVATE_P();

            p.context = context;
            p.options = options;
            p.otioTimeline = otioTimeline;

            // Get information about the timeline.
            otio::ErrorStatus errorStatus;
            p.duration = p.otioTimeline.value->duration(&errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error(errorStatus.full_description);
            }
            if (p.otioTimeline.value->global_start_time().has_value())
            {
                p.globalStartTime = p.otioTimeline.value->global_start_time().value();
            }
            else
            {
                p.globalStartTime = otime::RationalTime(0, p.duration.rate());
            }
            p.getAVInfo(p.otioTimeline.value->tracks());

            // Create a new thread.
            p.running = true;
            p.thread = std::thread(
                [this]
                {
                    TLR_PRIVATE_P();

                    p.logTimer = std::chrono::steady_clock::now();
                    p.init();
                    while (p.running)
                    {
                        p.tick();
                    }

                    std::list<Private::VideoRequest> videoRequestsCleanup;
                    std::list<Private::AudioRequest> audioRequestsCleanup;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        p.stopped = true;
                        while (!p.videoRequests.empty())
                        {
                            videoRequestsCleanup.push_back(std::move(p.videoRequests.front()));
                            p.videoRequests.pop_front();
                        }
                        while (!p.audioRequests.empty())
                        {
                            audioRequestsCleanup.push_back(std::move(p.audioRequests.front()));
                            p.audioRequests.pop_front();
                        }
                    }
                    while (!p.videoRequestsInProgress.empty())
                    {
                        videoRequestsCleanup.push_back(std::move(p.videoRequestsInProgress.front()));
                        p.videoRequestsInProgress.pop_front();
                    }
                    while (!p.audioRequestsInProgress.empty())
                    {
                        audioRequestsCleanup.push_back(std::move(p.audioRequestsInProgress.front()));
                        p.audioRequestsInProgress.pop_front();
                    }
                    for (auto& request : videoRequestsCleanup)
                    {
                        VideoData data;
                        data.time = request.time;
                        for (auto& i : request.layerData)
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
                        request.promise.set_value(data);
                    }
                    for (auto& request : audioRequestsCleanup)
                    {
                        AudioData data;
                        data.seconds = request.seconds;
                        for (auto& i : request.layerData)
                        {
                            AudioLayer layer;
                            if (i.audio.valid())
                            {
                                layer.audio = i.audio.get().audio;
                            }
                            data.layers.push_back(layer);
                        }
                        request.promise.set_value(data);
                    }
                });
        }

        Timeline::Timeline() :
            _p(new Private)
        {}

        Timeline::~Timeline()
        {
            TLR_PRIVATE_P();
            p.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

        std::shared_ptr<Timeline> Timeline::create(
            const otio::SerializableObject::Retainer<otio::Timeline>& timeline,
            const std::shared_ptr<core::Context>& context,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            out->_init(timeline, context, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const file::Path& path,
            const std::shared_ptr<core::Context>& context,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;
            std::string error;
            try
            {
                if (auto read = context->getSystem<avio::System>()->read(path, options.avioOptions))
                {
                    const auto info = read->getInfo().get();
                    otime::RationalTime globalStartTime(0.0, info.videoTime.duration().rate());
                    auto otioClip = new otio::Clip;
                    otioClip->set_source_range(info.videoTime);
                    if (avio::VideoType::Sequence == info.videoType && !path.getNumber().empty())
                    {
                        globalStartTime = info.videoTime.start_time();
                        otioClip->set_media_reference(new otio::ImageSequenceReference(
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
                        otioClip->set_media_reference(new otio::ExternalReference(path.get()));
                    }
                    auto otioTrack = new otio::Track();
                    otio::ErrorStatus errorStatus;
                    otioTrack->append_child(otioClip, &errorStatus);
                    if (errorStatus != otio::ErrorStatus::OK)
                    {
                        throw std::runtime_error("Cannot append child");
                    }
                    auto otioStack = new otio::Stack;
                    otioStack->append_child(otioTrack, &errorStatus);
                    if (errorStatus != otio::ErrorStatus::OK)
                    {
                        throw std::runtime_error("Cannot append child");
                    }
                    otioTimeline = new otio::Timeline;
                    otioTimeline->set_tracks(otioStack);
                    otioTimeline->set_global_start_time(globalStartTime);
                }
            }
            catch (const std::exception& e)
            {
                error = e.what();
            }
            if (!otioTimeline)
            {
                otio::ErrorStatus errorStatus;
                otioTimeline = readTimeline(path.get(), &errorStatus);
                if (errorStatus != otio::ErrorStatus::OK)
                {
                    throw std::runtime_error(errorStatus.full_description);
                }
            }
            if (!otioTimeline)
            {
                throw std::runtime_error(error);
            }
            out->_p->path = path;
            out->_init(otioTimeline, context, options);
            return out;
        }

        const std::weak_ptr<core::Context>& Timeline::getContext() const
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

        const avio::Info& Timeline::getAVInfo() const
        {
            return _p->avInfo;
        }

        std::future<VideoData> Timeline::getVideo(const otime::RationalTime& time, uint16_t videoLayer)
        {
            TLR_PRIVATE_P();
            Private::VideoRequest request;
            request.time = time;
            request.videoLayer = videoLayer;
            auto future = request.promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                if (!p.stopped)
                {
                    valid = true;
                    p.videoRequests.push_back(std::move(request));
                }
            }
            if (valid)
            {
                p.requestCV.notify_one();
            }
            else
            {
                request.promise.set_value(VideoData());
            }
            return future;
        }

        std::future<AudioData> Timeline::getAudio(int64_t seconds)
        {
            TLR_PRIVATE_P();
            Private::AudioRequest request;
            request.seconds = seconds;
            auto future = request.promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                if (!p.stopped)
                {
                    valid = true;
                    p.audioRequests.push_back(std::move(request));
                }
            }
            if (valid)
            {
                p.requestCV.notify_one();
            }
            else
            {
                request.promise.set_value(AudioData());
            }
            return future;
        }

        void Timeline::setActiveRanges(const std::vector<otime::TimeRange>& ranges)
        {
            _p->activeRanges = ranges;
        }

        void Timeline::cancelRequests()
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.videoRequests.clear();
            p.audioRequests.clear();
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
            return file::Path(directory, path.get());
        }

        file::Path Timeline::Private::getPath(const otio::ImageSequenceReference* ref) const
        {
            std::stringstream ss;
            ss << ref->target_url_base() <<
                ref->name_prefix() <<
                std::setfill('0') << std::setw(ref->frame_zero_padding()) << ref->start_frame() <<
                ref->name_suffix();
            return file::Path(ss.str());
        }

        file::Path Timeline::Private::getPath(const otio::MediaReference* ref) const
        {
            file::Path out;
            if (auto externalRef = dynamic_cast<const otio::ExternalReference*>(ref))
            {
                //! \bug Handle URL parsing.
                out = file::Path(externalRef->target_url());
            }
            else if (auto imageSequenceRef = dynamic_cast<const otio::ImageSequenceReference*>(ref))
            {
                out = getPath(imageSequenceRef);
            }
            return fixPath(out);
        }

        bool Timeline::Private::getAVInfo(const otio::Composable* composable)
        {
            if (auto clip = dynamic_cast<const otio::Clip*>(composable))
            {
                if (auto context = this->context.lock())
                {
                    // The first clip with defines the information for the timeline.
                    avio::Options avioOptions = options.avioOptions;
                    otio::ErrorStatus errorStatus;
                    avioOptions["SequenceIO/DefaultSpeed"] = string::Format("{0}").arg(clip->duration(&errorStatus).rate());
                    if (auto read = context->getSystem<avio::System>()->read(getPath(clip->media_reference()), avioOptions))
                    {
                        avInfo = read->getInfo().get();
                        return true;
                    }
                }
            }
            if (auto composition = dynamic_cast<const otio::Composition*>(composable))
            {
                for (const auto& child : composition->children())
                {
                    if (getAVInfo(child))
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

        void Timeline::Private::init()
        {
            for (const auto& j : otioTimeline->tracks()->children())
            {
                if (auto otioTrack = dynamic_cast<const otio::Track*>(j.value))
                {
                    for (const auto& k : otioTrack->children())
                    {
                        if (auto otioItem = dynamic_cast<const otio::Item*>(k.value))
                        {
                            otio::ErrorStatus errorStatus;
                            const auto rangeOpt = otioItem->trimmed_range_in_parent(&errorStatus);
                            if (rangeOpt.has_value())
                            {
                                Item item;
                                item.track = otioTrack;
                                item.item = otioItem;
                                item.range = rangeOpt.value();
                                items.emplace_back(item);
                            }
                        }
                    }
                }
            }
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
                        string::Format("tlr::timeline::Timeline {0}").arg(this),
                        string::Format(
                        "\n"
                        "    path: {0}\n"
                        "    video requests: {1}/{2}/{3} (size/in-progress/max)\n"
                        "    audio requests: {4}/{5}/{6} (size/in-progress/max)\n"
                        "    readers: {7}").
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
        }

        void Timeline::Private::requests()
        {
            // Gather requests.
            std::list<VideoRequest> newVideoRequests;
            std::list<AudioRequest> newAudioRequests;
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
                    newVideoRequests.push_back(std::move(videoRequests.front()));
                    videoRequests.pop_front();
                }
                while (!audioRequests.empty() &&
                    (audioRequestsInProgress.size() + newAudioRequests.size()) < options.audioRequestCount)
                {
                    newAudioRequests.push_back(std::move(audioRequests.front()));
                    audioRequests.pop_front();
                }
            }

            // Traverse the timeline for new video requests.
            for (auto& request : newVideoRequests)
            {
                try
                {
                    for (const auto& i : items)
                    {
                        const auto time = request.time - globalStartTime;
                        if (i.range.contains(time))
                        {
                            VideoLayerData videoData;
                            if ( auto otioClip = dynamic_cast<const otio::Clip*>(i.item))
                            {
                                createReader(i.track, otioClip, options.avioOptions);
                                videoData.image = readVideo(i.track, otioClip, time, request.videoLayer);
                            }
                            otio::ErrorStatus errorStatus;
                            const auto neighbors = i.track->neighbors_of(i.item, &errorStatus);
                            if (auto otioTransition = dynamic_cast<otio::Transition*>(neighbors.second.value))
                            {
                                if (time > i.range.end_time_inclusive() - otioTransition->in_offset())
                                {
                                    videoData.transition = toTransition(otioTransition->transition_type());
                                    videoData.transitionValue = transitionValue(
                                        time.value(),
                                        i.range.end_time_inclusive().value() - otioTransition->in_offset().value(),
                                        i.range.end_time_inclusive().value() + otioTransition->out_offset().value() + 1.0);
                                    const auto transitionNeighbors = i.track->neighbors_of(otioTransition, &errorStatus);
                                    if (const auto otioClipB = dynamic_cast<otio::Clip*>(transitionNeighbors.second.value))
                                    {
                                        createReader(i.track, otioClipB, options.avioOptions);
                                        videoData.imageB = readVideo(i.track, otioClipB, time, request.videoLayer);
                                    }
                                }
                            }
                            if (auto otioTransition = dynamic_cast<otio::Transition*>(neighbors.first.value))
                            {
                                if (time < i.range.start_time() + otioTransition->out_offset())
                                {
                                    std::swap(videoData.image, videoData.imageB);
                                    videoData.transition = toTransition(otioTransition->transition_type());
                                    videoData.transitionValue = transitionValue(
                                        time.value(),
                                        i.range.start_time().value() - otioTransition->in_offset().value() - 1.0,
                                        i.range.start_time().value() + otioTransition->out_offset().value());
                                    const auto transitionNeighbors = i.track->neighbors_of(otioTransition, &errorStatus);
                                    if (const auto otioClipB = dynamic_cast<otio::Clip*>(transitionNeighbors.first.value))
                                    {
                                        createReader(i.track, otioClipB, options.avioOptions);
                                        videoData.image = readVideo(i.track, otioClipB, time, request.videoLayer);
                                    }
                                }
                            }
                            request.layerData.push_back(std::move(videoData));
                        }
                    }
                }
                catch (const std::exception&)
                {
                    //! \todo How should this be handled?
                }

                videoRequestsInProgress.push_back(std::move(request));
            }

            // Traverse the timeline for new audio requests.
            for (auto& request : newAudioRequests)
            {
                try
                {
                    for (const auto& i : items)
                    {
                        const otime::TimeRange timeRange(
                            otime::RationalTime(request.seconds, 1.0) - globalStartTime.rescaled_to(1.0),
                            otime::RationalTime(1.0, 1.0));
                        if (i.range.intersects(timeRange))
                        {
                            AudioLayerData audioData;
                            if (auto otioClip = dynamic_cast<const otio::Clip*>(i.item))
                            {
                                createReader(i.track, otioClip, options.avioOptions);
                                audioData.audio = readAudio(i.track, otioClip, timeRange);
                            }
                            request.layerData.push_back(std::move(audioData));
                        }
                    }
                }
                catch (const std::exception&)
                {
                    //! \todo How should this be handled?
                }

                audioRequestsInProgress.push_back(std::move(request));
            }

            // Check for finished video requests.
            auto videoRequestIt = videoRequestsInProgress.begin();
            while (videoRequestIt != videoRequestsInProgress.end())
            {
                bool valid = true;
                for (auto& i : videoRequestIt->layerData)
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
                    data.time = videoRequestIt->time;
                    try
                    {
                        for (auto& j : videoRequestIt->layerData)
                        {
                            VideoLayer layer;
                            if (j.image.valid())
                            {
                                layer.image = j.image.get().image;
                            }
                            if (j.imageB.valid())
                            {
                                layer.imageB = j.imageB.get().image;
                            }
                            layer.transition = j.transition;
                            layer.transitionValue = j.transitionValue;
                            data.layers.push_back(layer);
                        }
                    }
                    catch (const std::exception&)
                    {
                        //! \todo How should this be handled?
                    }
                    videoRequestIt->promise.set_value(data);
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
                for (auto& i : audioRequestIt->layerData)
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
                    data.seconds = audioRequestIt->seconds;
                    try
                    {
                        for (auto& j : audioRequestIt->layerData)
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
                    audioRequestIt->promise.set_value(data);
                    audioRequestIt = audioRequestsInProgress.erase(audioRequestIt);
                    continue;
                }
                ++audioRequestIt;
            }
        }

        void Timeline::Private::createReader(
            const otio::Track* track,
            const otio::Clip* clip,
            const avio::Options& ioOptions)
        {
            if (readers.find(clip) == readers.end())
            {
                if (auto context = this->context.lock())
                {
                    const file::Path path = getPath(clip->media_reference());
                    avio::Options options = ioOptions;
                    options["SequenceIO/DefaultSpeed"] = string::Format("{0}").arg(duration.rate());
                    const auto ioSystem = context->getSystem<avio::System>();
                    auto read = ioSystem->read(path, options);
                    avio::Info info;
                    if (read)
                    {
                        info = read->getInfo().get();
                    }
                    if (read && !info.video.empty())
                    {
                        context->log(
                            string::Format("tlr::timeline::Timeline {0}").arg(this),
                            string::Format(
                                "\n"
                                "    read: {0}\n"
                                "    video: {1}\n"
                                "    video type: {2}\n"
                                "    video time: {3}\n"
                                "    audio: {4}\n"
                                "    audio time: {5}").
                            arg(path.get()).
                            arg(info.video[0]).
                            arg(info.videoType).
                            arg(info.videoTime).
                            arg(info.audio).
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

                        Reader reader;
                        reader.read = read;
                        reader.info = info;
                        const auto ancestor = dynamic_cast<const otio::Item*>(getRoot(clip));
                        reader.range = clip->transformed_time_range(
                            otime::TimeRange::range_from_start_end_time(globalStartTime + startTime, globalStartTime + endTime),
                            ancestor,
                            &errorStatus);
                        readers[clip] = std::move(reader);
                    }
                }
            }
        }

        std::future<avio::VideoData> Timeline::Private::readVideo(
            const otio::Track* track,
            const otio::Clip* clip,
            const otime::RationalTime& time,
            uint16_t videoLayer)
        {
            std::future<avio::VideoData> out;
            if (auto context = this->context.lock())
            {
                const auto j = readers.find(clip);
                if (j != readers.end())
                {
                    otio::ErrorStatus errorStatus;
                    const auto clipTime = track->transformed_time(time, clip, &errorStatus);
                    const auto readTime = clipTime.rescaled_to(j->second.info.videoTime.duration().rate());
                    const auto floorTime = time::floor(readTime);
                    out = j->second.read->readVideo(floorTime, videoLayer);
                }
            }
            return out;
        }

        std::future<avio::AudioData> Timeline::Private::readAudio(
            const otio::Track* track,
            const otio::Clip* clip,
            const otime::TimeRange& timeRange)
        {
            std::future<avio::AudioData> out;
            if (auto context = this->context.lock())
            {
                const auto j = readers.find(clip);
                if (j != readers.end())
                {
                    otio::ErrorStatus errorStatus;
                    const auto clipRange = track->transformed_time_range(timeRange, clip, &errorStatus);
                    const auto floorRange = otime::TimeRange(
                        time::floor(clipRange.start_time().rescaled_to(avInfo.audio.sampleRate)),
                        time::floor(clipRange.duration().rescaled_to(avInfo.audio.sampleRate)));
                    out = j->second.read->readAudio(floorRange);
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
                        context->log("tlr::timeline::Timeline", path.get() + ": Stop: " + i->second.read->getPath().get());
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
                        context->log("tlr::timeline::Timeline", path.get() + ": Delete: " + (*i)->getPath().get());
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
