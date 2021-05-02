// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrRender/Timeline.h>

#include <tlrRender/Error.h>
#include <tlrRender/File.h>
#include <tlrRender/String.h>

#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/stackAlgorithm.h>
#include <opentimelineio/timeline.h>

#if defined(TLR_ENABLE_PYTHON)
#include <Python.h>
#endif

#include <array>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace tlr
{
    namespace timeline
    {
        TLR_ENUM_LABEL_IMPL(Playback, "Stop", "Forward", "Reverse");
        TLR_ENUM_LABEL_IMPL(Loop, "Loop", "Once", "Ping-Pong");

        math::BBox2f fitWindow(const imaging::Size& image, const imaging::Size& window)
        {
            math::BBox2f out;
            const float windowAspect = window.getAspect();
            const float imageAspect = image.getAspect();
            math::BBox2f bbox;
            if (windowAspect > imageAspect)
            {
                out = math::BBox2f(
                    window.w / 2.F - (window.h * imageAspect) / 2.F,
                    0.F,
                    window.h * imageAspect,
                    window.h);
            }
            else
            {
                out = math::BBox2f(
                    0.F,
                    window.h / 2.F - (window.w / imageAspect) / 2.F,
                    window.w,
                    window.w / imageAspect);
            }
            return out;
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

            otio::SerializableObject::Retainer<otio::Timeline> read(
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
                    const std::string fileNameNormalized = file::normalize(fileName);
                    auto pyReadFromFileArg = PyUnicode_FromStringAndSize(fileNameNormalized.c_str(), fileNameNormalized.size());
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

            std::string getFileName(const otio::ImageSequenceReference* ref)
            {
                std::stringstream ss;
                ss << ref->target_url_base() <<
                    ref->name_prefix() <<
                    std::setfill('0') << std::setw(ref->frame_zero_padding()) << ref->start_frame() <<
                    ref->name_suffix();
                return ss.str();
            }
        }

        void Timeline::_init(const std::string& fileName)
        {
            // Read the timeline.
            otio::ErrorStatus errorStatus;
            _timeline = read(fileName, &errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error(errorStatus.full_description);
            }

            // Get the timeline duration.
            _duration = _timeline.value->duration(&errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error(errorStatus.full_description);
            }

            // Flatten the timeline.
            _flattenedTimeline = otio::flatten_stack(_timeline.value->tracks(), &errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error(errorStatus.full_description);
            }

            // Create the I/O system.
            _ioSystem = io::System::create();

            // Change the working directory.
            std::string path;
            file::split(fileName, &path);
            file::changeDir(path);

            // The first clip defines the image information.
            for (const auto& child : _flattenedTimeline.value->children())
            {
                if (auto clip = dynamic_cast<otio::Clip*>(child.value))
                {
                    std::string fileName;
                    if (auto externalRef = dynamic_cast<otio::ExternalReference*>(clip->media_reference()))
                    {
                        fileName = externalRef->target_url();
                    }
                    else if (auto imageSequenceRef = dynamic_cast<otio::ImageSequenceReference*>(clip->media_reference()))
                    {
                        fileName = getFileName(imageSequenceRef);
                    }
                    if (auto read = _ioSystem->read(fileName))
                    {
                        const auto info = read->getInfo();
                        if (!info.video.empty())
                        {
                            _imageInfo = info.video[0].info;
                            break;
                        }
                    }
                }
            }
        }

        Timeline::Timeline()
        {}

        Timeline::~Timeline()
        {}

        std::shared_ptr<Timeline> Timeline::create(const std::string& fileName)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            out->_init(fileName);
            return out;
        }

        void Timeline::setPlayback(Playback value)
        {
            if (value == _playback)
                return;
            _playback = value;
            switch (_playback)
            {
            case Playback::Stop:
                break;
            case Playback::Forward:
                _startTime = std::chrono::steady_clock::now();
                _playbackStartTime = _currentTime;
                break;
            case Playback::Reverse:
                //! \todo Reverse playback.
                break;
            default:
                break;
            }
        }

        void Timeline::setLoop(Loop value)
        {
            _loop = value;
        }

        void Timeline::seek(const otime::RationalTime& value)
        {
            auto tmp = value;
            if (tmp.value() >= _duration.value())
            {
                tmp = otime::RationalTime(0, _duration.rate());
            }
            else if (tmp.value() < 0.0)
            {
                tmp = otime::RationalTime(_duration.value() - 1, _duration.rate());
            }
            if (tmp == _currentTime)
                return;

            _currentTime = tmp;

            for (const auto& i : _readers)
            {
                otio::ErrorStatus errorStatus;
                auto time = _flattenedTimeline.value->transformed_time(_currentTime, i.first, &errorStatus);
                if (errorStatus != otio::ErrorStatus::OK)
                {
                    throw std::runtime_error(errorStatus.full_description);
                }
                i.second->seek(time);
            }

            switch (_playback)
            {
            case Playback::Forward:
                _startTime = std::chrono::steady_clock::now();
                _playbackStartTime = _currentTime;
                break;
            default: break;
            }
        }

        void Timeline::tick()
        {
            switch (_playback)
            {
            case Playback::Forward:
            {
                // Calculate the current time.
                const auto now = std::chrono::steady_clock::now();
                const std::chrono::duration<float> diff = now - _startTime;
                _currentTime = _playbackStartTime + otime::RationalTime(diff.count() * _duration.rate(), _duration.rate());
                const otime::RationalTime maxTime = _duration - otime::RationalTime(1.0, _duration.rate());
                switch (_loop)
                {
                case Loop::Loop:
                    if (_currentTime > maxTime)
                    {
                        seek(otime::RationalTime(0, _duration.rate()));
                    }
                    break;
                case Loop::Once:
                    _currentTime = std::min(_currentTime, maxTime);
                    break;
                case Loop::PingPong:
                    //! \todo Ping-pong loop mode.
                    break;
                default: break;
                }
                break;
            }
            default: break;
            }

            // Create and destroy I/O readers.
            for (const auto& child : _flattenedTimeline.value->children())
            {
                if (auto clip = dynamic_cast<otio::Clip*>(child.value))
                {
                    std::string fileName;
                    if (auto externalRef = dynamic_cast<otio::ExternalReference*>(clip->media_reference()))
                    {
                        fileName = externalRef->target_url();
                    }
                    else if (auto imageSequenceRef = dynamic_cast<otio::ImageSequenceReference*>(clip->media_reference()))
                    {
                        fileName = getFileName(imageSequenceRef);
                    }

                    otio::ErrorStatus errorStatus;
                    auto range = clip->range_in_parent(&errorStatus);
                    if (errorStatus != otio::ErrorStatus::OK)
                    {
                        throw std::runtime_error(errorStatus.full_description);
                    }

                    // Find the I/O reader for this clip.
                    const auto i = std::find_if(
                        _readers.begin(),
                        _readers.end(),
                        [clip](const Reader& value)
                        {
                            return value.first == clip;
                        });

                    // Is the clip active?
                    if (_currentTime >= range.start_time() &&
                        _currentTime < range.start_time() + range.duration())
                    {
                        if (i == _readers.end())
                        {
                            const auto time = _flattenedTimeline.value->transformed_time(_currentTime, clip, &errorStatus);
                            if (errorStatus != otio::ErrorStatus::OK)
                            {
                                throw std::runtime_error(errorStatus.full_description);
                            }
                            // Create a new I/O reader.
                            if (auto read = _ioSystem->read(fileName, otime::RationalTime(0, time.rate())))
                            {
                                read->seek(time);
                                _readers.push_back(std::make_pair(clip, read));
                            }
                        }
                    }
                    else
                    {
                        if (i != _readers.end())
                        {
                            // Destroy the I/O reader.
                            _readers.erase(i);
                        }
                    }
                }
            }

            // Tick the readers.
            for (auto i : _readers)
            {
                i.second->tick();
            }

            // Update the current image.
            for (auto i : _readers)
            {
                otio::ErrorStatus errorStatus;
                auto range = i.first.value->trimmed_range_in_parent(&errorStatus);
                if (errorStatus != otio::ErrorStatus::OK)
                {
                    throw std::runtime_error(errorStatus.full_description);
                }
                if (range.has_value())
                {
                    // Is the clip active?
                    if (_currentTime >= range.value().start_time() &&
                        _currentTime < range.value().start_time() + range.value().duration())
                    {
                        auto& queue = i.second->getVideoQueue();
                        if (queue.size())
                        {
                            // Get the frame from the video queue, discarding out of date frames.
                            io::VideoFrame frame = queue.front();
                            auto time = i.first.value->transformed_time(frame.time, _flattenedTimeline, &errorStatus);
                            if (errorStatus != otio::ErrorStatus::OK)
                            {
                                throw std::runtime_error(errorStatus.full_description);
                            }
                            while (queue.size() > 1 && time < _currentTime)
                            {
                                queue.pop();
                                frame = queue.front();
                                time = i.first.value->transformed_time(frame.time, _flattenedTimeline, &errorStatus);
                                if (errorStatus != otio::ErrorStatus::OK)
                                {
                                    throw std::runtime_error(errorStatus.full_description);
                                }
                            }
                            if (const auto& image = frame.image)
                            {
                                _currentImage = image;
                            }
                        }
                    }
                }
            }
        }

        void Timeline::setVideoQueueSize(size_t value)
        {
            _ioSystem->setVideoQueueSize(value);
        }
    }

    TLR_ENUM_SERIALIZE_IMPL(timeline, Playback);
    TLR_ENUM_SERIALIZE_IMPL(timeline, Loop);
}
