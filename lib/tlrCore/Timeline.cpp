// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/Timeline.h>

#include <tlrCore/Error.h>
#include <tlrCore/File.h>
#include <tlrCore/IO.h>
#include <tlrCore/String.h>

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

        void Timeline::_init(const std::string& fileName)
        {
            _fileName = fileName;

            // Read the timeline.
            otio::ErrorStatus errorStatus;
            _timeline = read(_fileName, &errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error(errorStatus.full_description);
            }
            _duration = _timeline.value->duration(&errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error(errorStatus.full_description);
            }
            if (_timeline.value->global_start_time().has_value())
            {
                _globalStartTime = _timeline.value->global_start_time().value();
            }
            else
            {
                _globalStartTime = otime::RationalTime(0, _duration.rate());
            }

            // Flatten the timeline.
            _flattenedTimeline = otio::flatten_stack(_timeline.value->tracks(), &errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error(errorStatus.full_description);
            }

            // Create the I/O system.
            _ioSystem = io::System::create();

            // Get information about the timeline.
            bool imageInfo = false;
            for (const auto& child : _flattenedTimeline.value->children())
            {
                if (auto clip = dynamic_cast<otio::Clip*>(child.value))
                {
                    // The first clip with video defines the image information
                    // for the timeline.
                    if (!imageInfo)
                    {
                        if (auto read = _ioSystem->read(_getFileName(clip->media_reference())))
                        {
                            const auto info = read->getInfo().get();
                            if (!info.video.empty())
                            {
                                imageInfo = true;
                                _imageInfo = info.video[0].info;
                            }
                        }
                    }

                    _clips.push_back(clip);
                    _clipRanges.push_back(_getRange(clip));
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

        std::future<io::VideoFrame> Timeline::render(const otime::RationalTime& time)
        {
            std::future<io::VideoFrame> out;
            const auto now = std::chrono::steady_clock::now();
            for (size_t i = 0; i < _clips.size(); ++i)
            {
                const auto& clip = _clips[i];
                const auto& range = _clipRanges[i];
                if (range.contains(time))
                {
                    otio::ErrorStatus errorStatus;
                    const auto clipTime = _flattenedTimeline.value->transformed_time(time - _globalStartTime, clip, &errorStatus);
                    if (errorStatus != otio::ErrorStatus::OK)
                    {
                        throw std::runtime_error(errorStatus.full_description);
                    }

                    const auto j = _readers.find(clip);
                    if (j != _readers.end())
                    {
                        const auto readTime = clipTime.rescaled_to(j->second.info.video[0].duration);
                        out = j->second.read->getVideoFrame(otime::RationalTime(floor(readTime.value()), readTime.rate()));
                    }
                    else
                    {
                        const std::string fileName = _getFileName(clip->media_reference());
                        auto read = _ioSystem->read(fileName, otime::RationalTime(0, _duration.rate()));
                        io::Info info;
                        if (read)
                        {
                            info = read->getInfo().get();
                        }
                        if (read && !info.video.empty())
                        {
                            //std::cout << "read: " << fileName << std::endl;
                            Reader reader;
                            reader.read = read;
                            reader.info = info;
                            const auto readTime = clipTime.rescaled_to(info.video[0].duration);
                            out = read->getVideoFrame(otime::RationalTime(floor(readTime.value()), readTime.rate()));
                            _readers[clip] = std::move(reader);
                        }
                        else
                        {
                            //! \todo How should this be handled?
                        }
                    }
                }
            }
            return out;
        }

        void Timeline::setActiveRanges(const std::vector<otime::TimeRange>& ranges)
        {
            _activeRanges = ranges;
        }

        void Timeline::cancelRenders()
        {
            for (auto& i : _readers)
            {
                i.second.read->cancelVideoFrames();
            }
        }

        void Timeline::tick()
        {
            for (size_t i = 0; i < _clips.size(); ++i)
            {
                const auto& clip = _clips[i];
                const auto& range = _clipRanges[i];
                bool del = true;
                for (const auto& activeRange : _activeRanges)
                {
                    if (range.intersects(activeRange))
                    {
                        del = false;
                        break;
                    }
                }
                if (del)
                {
                    auto j = _readers.find(clip);
                    if (j != _readers.end() && !j->second.read->hasVideoFrames())
                    {
                        //std::cout << "destroy: " << j->second.read->getFileName() << std::endl;
                        _readers.erase(j);
                    }
                }
            }
        }

        std::string Timeline::_fixFileName(const std::string& fileName) const
        {
            std::string absolute;
            if (!file::isAbsolute(fileName))
            {
                file::split(_fileName, &absolute);
            }
            std::string path;
            std::string baseName;
            std::string number;
            std::string extension;
            file::split(file::normalize(fileName), &path, &baseName, &number, &extension);
            return absolute + path + baseName + number + extension;
        }

        std::string Timeline::_getFileName(const otio::ImageSequenceReference* ref) const
        {
            std::stringstream ss;
            ss << ref->target_url_base() <<
                ref->name_prefix() <<
                std::setfill('0') << std::setw(ref->frame_zero_padding()) << ref->start_frame() <<
                ref->name_suffix();
            return ss.str();
        }

        std::string Timeline::_getFileName(const otio::MediaReference* ref) const
        {
            std::string out;
            if (auto externalRef = dynamic_cast<const otio::ExternalReference*>(ref))
            {
                out = externalRef->target_url();
            }
            else if (auto imageSequenceRef = dynamic_cast<const otio::ImageSequenceReference*>(ref))
            {
                out = _getFileName(imageSequenceRef);
            }
            return _fixFileName(out);
        }

        otime::TimeRange Timeline::_getRange(const otio::SerializableObject::Retainer<otio::Clip>& clip) const
        {
            otime::TimeRange out;
            otio::ErrorStatus errorStatus;
            const auto& optional = clip.value->trimmed_range_in_parent(&errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error(errorStatus.full_description);
            }
            bool outOfRange = true;
            if (optional.has_value())
            {
                out = otime::TimeRange(
                    _globalStartTime + optional.value().start_time(),
                    optional.value().duration());
            }
            return out;
        }
    }
}
