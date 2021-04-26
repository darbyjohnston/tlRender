// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlrTimeline/IO.h>

#include <tlrCore/Assert.h>
#include <tlrCore/File.h>

#include <glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <opentimelineio/externalReference.h>
#include <opentimelineio/stackAlgorithm.h>

#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <sstream>

namespace tlr
{
    App::App()
    {}

    App::~App()
    {
        _render.reset();
        _fontSystem.reset();
        _destroyWindow();
    }

    std::shared_ptr<App> App::create(int argc, char* argv[])
    {
        auto out = std::shared_ptr<App>(new App);
        out->_init(argc, argv);
        return out;
    }

    int App::run()
    {
        // Parse the command line.
        int r = _parseCmdLine();
        if (r != 0)
        {
            return r;
        }

        // Create the I/O system.
        _ioSystem = av::io::System::create();
        _ioSystem->setVideoQueueSize(_options.ioVideoQueueSize);

        // Read the timeline.
        _readTimeline();

        // Create the window.
        _createWindow();
        _shortcutsHelp();
        
        // Create the renderer.
        _fontSystem = render::FontSystem::create();
        _render = render::Render::create();

        // Start the main loop.
        if (_options.startPlayback)
        {
            _forwardPlayback();
        }
        while (_running && !glfwWindowShouldClose(_glfwWindow))
        {
            glfwPollEvents();
            _tick();
            glfwSwapBuffers(_glfwWindow);
        }

        return r;
    }

    void App::exit()
    {
        _running = false;
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
        file::change_dir(path);

        // The first clip defines the image information.
        for (const auto& child : _flattenedTimeline.value->children())
        {
            if (auto clip = dynamic_cast<otio::Clip*>(child.value))
            {
                if (auto externalRef = dynamic_cast<otio::ExternalReference*>(clip->media_reference()))
                {
                    if (_ioSystem->canRead(externalRef->target_url()))
                    {
                        if (auto read = _ioSystem->read(externalRef->target_url()))
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

        // Update the I/O readers.
        for (auto i : _readers)
        {
            i.second->tick();
        }
        _updateReaders();

        // Update frame buffer size.
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(_glfwWindow, &width, &height);
        _frameBufferSize.w = width;
        _frameBufferSize.h = height;
        glfwGetWindowContentScale(_glfwWindow, &_contentScale.x, &_contentScale.y);

        // Render this frame.
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
    }
    
    void App::_updateReaders()
    {
        // Create and destroy I/O readers.
        for (const auto& child : _flattenedTimeline.value->children())
        {
            if (auto clip = dynamic_cast<otio::Clip*>(child.value))
            {
                if (auto externalRef = dynamic_cast<otio::ExternalReference*>(clip->media_reference()))
                {
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
                            // Create a new I/O reader.
                            if (_ioSystem->canRead(externalRef->target_url()))
                            {
                                const std::string& target_url = externalRef->target_url();
                                if (auto read = _ioSystem->read(target_url))
                                {
                                    const auto clipRange = clip->trimmed_range(&errorStatus);
                                    read->seek(clipRange.start_time());
                                    if (errorStatus != otio::ErrorStatus::OK)
                                    {
                                        throw std::runtime_error(errorStatus.full_description);
                                    }
                                    _readers.push_back(std::make_pair(clip, read));
                                    {
                                        std::stringstream ss;
                                        ss << _currentTime << ": Create " << target_url;
                                        _printVerbose(ss.str());
                                    }
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
        const auto tmp = otime::RationalTime(
            std::min(std::max(value.value(), 0.0), _duration.value() - 1.0),
            _duration.rate());
        if (tmp == _currentTime)
            return;
        _currentTime = tmp;
        for (const auto& i : _readers)
        {
            i.second->seek(_currentTime);
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

    void App::_print(const std::string& value)
    {
        std::cout << value << std::endl;
    }

    void App::_printVerbose(const std::string& value)
    {
        if (_options.verbose)
        {
            std::cout << value << std::endl;
        }
    }

    void App::_printError(const std::string& value)
    {
        std::cerr << "ERROR: " << value << std::endl;
    }
}
