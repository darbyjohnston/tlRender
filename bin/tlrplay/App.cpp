// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlrTimeline/IO.h>

#include <tlrCore/Assert.h>
#include <tlrCore/File.h>
#include <tlrCore/StringFormat.h>
#include <tlrCore/Time.h>

#include <glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/stackAlgorithm.h>

#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

namespace tlr
{
    void App::_init(int argc, char* argv[])
    {
        IApp::_init(
            argc,
            argv,
            "tlrplay",
            "Play an editorial timeline.",
            {
                app::CmdLineValueArg<std::string>::create(
                    _input,
                    "Input",
                    "The input timeline.")
            },
            {
                app::CmdLineValueOption<float>::create(
                    _options.windowScale,
                    { "-windowScale", "-ws" },
                    string::Format("Set the window size scale factor. Default: {0}").
                        arg(_options.windowScale),
                    "(value)"),
                app::CmdLineFlagOption::create(
                    _options.fullScreen,
                    { "-fullScreen", "-fs" },
                    "Enable full screen mode."),
                app::CmdLineValueOption<bool>::create(
                    _options.hud,
                    { "-hud" },
                    string::Format("Enable the HUD (heads up display). Default: {0}").
                        arg(_options.hud),
                    "(value)"),
                app::CmdLineValueOption<bool>::create(
                    _options.startPlayback,
                    { "-startPlayback", "-sp" },
                    string::Format("Automatically start playback. Default: {0}").
                        arg(_options.startPlayback),
                    "(value)"),
                app::CmdLineValueOption<bool>::create(
                    _options.loopPlayback,
                    { "-loopPlayback", "-lp" },
                    string::Format("Loop playback. Default: {0}").
                        arg(_options.loopPlayback),
                    "(value)")
            });
    }

    App::App()
    {}

    App::~App()
    {}

    std::shared_ptr<App> App::create(int argc, char* argv[])
    {
        auto out = std::shared_ptr<App>(new App);
        out->_init(argc, argv);
        return out;
    }

    void App::run()
    {
        if (_exit != 0)
        {
            return;
        }
        
        // Read the timeline.
        _readTimeline();

        // Create the window.
        GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* glfwVidmode = glfwGetVideoMode(glfwMonitor);
        _createWindow(imaging::Size(
            std::min(static_cast<int>(_info.size.w * _options.windowScale), glfwVidmode->width),
            std::min(static_cast<int>(_info.size.h * _options.windowScale), glfwVidmode->height)));
        glfwSetFramebufferSizeCallback(_glfwWindow, _frameBufferSizeCallback);
        glfwSetWindowContentScaleCallback(_glfwWindow, _windowContentScaleCallback);
        if (_options.fullScreen)
        {
            _fullscreenWindow();
        }
        glfwSetKeyCallback(_glfwWindow, _keyCallback);
        glfwShowWindow(_glfwWindow);

        // Print the shortcuts help.
        _printShortcutsHelp();

        // Start the main loop.
        if (_options.startPlayback)
        {
            _forwardPlayback();
        }
        while (_running && !glfwWindowShouldClose(_glfwWindow))
        {
            glfwPollEvents();
            _tick();
        }
    }

    void App::exit()
    {
        _running = false;
    }

    namespace
    {
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

    void App::_readTimeline()
    {
        // Read the timeline.
        otio::ErrorStatus errorStatus;
        _timeline = timeline::read(_input, &errorStatus);
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
        {
            std::stringstream ss;
            ss << "Timeline duration: " << _duration;
            _printVerbose(ss.str());
        }

        // Flatten the timeline.
        _flattenedTimeline = otio::flatten_stack(_timeline.value->tracks(), &errorStatus);
        if (errorStatus != otio::ErrorStatus::OK)
        {
            throw std::runtime_error(errorStatus.full_description);
        }

        // Change the working directory.
        std::string path;
        file::split(_input, &path);
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
                        _info = info.video[0].info;
                        {
                            std::stringstream ss;
                            ss << "First clip info: " << _info;
                            _printVerbose(ss.str());
                        }
                        break;
                    }
                }
            }
        }
    }

    void App::_tick()
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
                if (_options.loopPlayback)
                {
                    if (_currentTime > maxTime)
                    {
                        _seek(otime::RationalTime(0, _duration.rate()));
                    }
                }
                else
                {
                    _currentTime = std::min(_currentTime, maxTime);
                }
                /*{
                    std::stringstream ss;
                    ss << "Current time: " << _currentTime;
                    _printVerbose(ss.str());
                }*/
                break;
            }
            default: break;
        }

        // Update.
        _updateReaders();
        _updateHUD();

        // Render this frame.
        if (_renderDirty)
        {
            _render->begin(imaging::Info(_frameBufferSize.w, _frameBufferSize.h, _info.pixelType));
            _renderVideo();
            if (_options.hud)
            {
                _renderHUD();
            }
            _render->end();

            // Copy the render buffer to the window.
            glViewport(0, 0, _frameBufferSize.w, _frameBufferSize.h);
            glClearColor(0.F, 0.F, 0.F, 0.F);
            glClear(GL_COLOR_BUFFER_BIT);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, _render->getID());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(
                0, 0, _frameBufferSize.w, _frameBufferSize.h,
                0, 0, _frameBufferSize.w, _frameBufferSize.h,
                GL_COLOR_BUFFER_BIT,
                GL_NEAREST);
            glfwSwapBuffers(_glfwWindow);

            _renderDirty = false;
        }
        else
        {
            time::sleep(std::chrono::microseconds(1000));
        }
    }
    
    void App::_updateReaders()
    {
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
                            {
                                std::stringstream ss;
                                ss << _currentTime << ": Create " << fileName;
                                _printVerbose(ss.str());
                            }
                        }
                    }
                }
                else
                {
                    if (i != _readers.end())
                    {
                        // Destroy the I/O reader.
                        {
                            std::stringstream ss;
                            ss << _currentTime << ": Destroy " << i->second->getFileName();
                            _printVerbose(ss.str());
                        }
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
                        av::io::VideoFrame frame = queue.front();
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
                            if (image != _currentImage)
                            {
                                _currentImage = image;
                                _renderDirty = true;
                            }
                        }
                    }
                }
            }
        }
    }

    void App::_updateHUD()
    {
        std::map<app::HUDElement, std::string> hudLabels;

        // Input file name.
        hudLabels[app::HUDElement::UpperLeft] = "Input: " + _input;

        // Current time.
        otime::ErrorStatus errorStatus;
        const std::string label = _currentTime.to_timecode(&errorStatus);
        if (errorStatus != otio::ErrorStatus::OK)
        {
            throw std::runtime_error(errorStatus.details);
        }
        hudLabels[app::HUDElement::LowerLeft] = "Time: " + label;

        // Speed.
        std::stringstream ss;
        ss.precision(2);
        ss << "Speed: " << std::fixed << _duration.rate();
        hudLabels[app::HUDElement::LowerRight] = ss.str();

        if (hudLabels != _hudLabels)
        {
            _hudLabels = hudLabels;
            _renderDirty = true;
        }
    }

    void App::_forwardPlayback()
    {
        _playback = Playback::Forward;
        _startTime = std::chrono::steady_clock::now();
        _playbackStartTime = _currentTime;
    }

    void App::_stopPlayback()
    {
        _playback = Playback::Stop;
    }

    void App::_playbackCallback(Playback value)
    {
        switch (value)
        {
        case Playback::Stop:
            _stopPlayback();
            break;
        case Playback::Forward:
            _forwardPlayback();
            break;
        default: break;
        }
        {
            std::stringstream ss;
            ss << "Playback: " << static_cast<int>(_playback);
            _print(ss.str());
        }
    }

    void App::_loopPlaybackCallback(bool value)
    {
        _options.loopPlayback = value;
        {
            std::stringstream ss;
            ss << "Loop playback: " << _options.loopPlayback;
            _print(ss.str());
        }
    }

    void App::_seek(const otime::RationalTime& value)
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

        /*{
            std::stringstream ss;
            otime::ErrorStatus errorStatus;
            std::string timecode = _currentTime.to_timecode(&errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error(errorStatus.details);
            }
            ss << "Seek: " << timecode;
            _print(ss.str());
        }*/
    }

    void App::_seekCallback(const otime::RationalTime& value)
    {
        _stopPlayback();
        _seek(value);
    }
}
