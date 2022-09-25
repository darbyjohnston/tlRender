// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimelinePrivate.h>

#include <tlIO/IOSystem.h>

#include <tlCore/File.h>
#include <tlCore/FileInfo.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>

#if defined(TLRENDER_ENABLE_PYTHON)
#include <Python.h>
#endif

namespace tl
{
    namespace timeline
    {
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

        otio::SerializableObject::Retainer<otio::Timeline> create(
            const std::string& fileName,
            const std::shared_ptr<system::Context>& context,
            const Options& options)
        {
            otio::SerializableObject::Retainer<otio::Timeline> out;
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

                    out = new otio::Timeline(path.get());
                    out->set_tracks(otioStack);
                    if (globalStartTime != time::invalidTime)
                    {
                        out->set_global_start_time(globalStartTime);
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
                string::Format(
                    "\n"
                    "    Create from path: {0}\n"
                    "    Audio path: {1}").
                arg(path.get()).
                arg(audioPath.get()));

            if (!out)
            {
                otio::ErrorStatus errorStatus;
                out = readTimeline(path.get(), &errorStatus);
                if (otio::is_error(errorStatus))
                {
                    out = nullptr;
                    error = errorStatus.full_description;
                }
                else if (!out)
                {
                    error = "Cannot read timeline";
                }
            }
            if (!out)
            {
                throw std::runtime_error(error);
            }

            otio::AnyDictionary dict;
            dict["path"] = path;
            dict["audioPath"] = audioPath;
            out->metadata()["tl::timeline"] = dict;

            return out;
        }

        otio::SerializableObject::Retainer<otio::Timeline> create(
            const std::string& fileName,
            const std::string& audioFileName,
            const std::shared_ptr<system::Context>& context,
            const Options& options)
        {
            otio::SerializableObject::Retainer<otio::Timeline> out;
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

                    out = new otio::Timeline(path.get());
                    out->set_tracks(otioStack);
                    if (globalStartTime != time::invalidTime)
                    {
                        out->set_global_start_time(globalStartTime);
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
                string::Format(
                    "\n"
                    "    Create from path: {0}\n"
                    "    Audio path: {1}").
                arg(path.get()).
                arg(audioPath.get()));

            if (!out)
            {
                otio::ErrorStatus errorStatus;
                out = readTimeline(path.get(), &errorStatus);
                if (otio::is_error(errorStatus))
                {
                    out = nullptr;
                    error = errorStatus.full_description;
                }
                else if (!out)
                {
                    error = "Cannot read timeline";
                }
            }
            if (!out)
            {
                throw std::runtime_error(error);
            }

            otio::AnyDictionary dict;
            dict["path"] = path;
            dict["audioPath"] = audioPath;
            out->metadata()["tl::timeline"] = dict;

            return out;
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
            auto out = std::shared_ptr<Timeline>(new Timeline);
            out->_init(
                timeline::create(fileName, context, options),
                context,
                options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::string& fileName,
            const std::string& audioFileName,
            const std::shared_ptr<system::Context>& context,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            out->_init(
                timeline::create(fileName, audioFileName, context, options),
                context,
                options);
            return out;
        }
    }
}
