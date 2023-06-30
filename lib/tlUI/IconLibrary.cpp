// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IconLibrary.h>

#include <tlIO/IOSystem.h>

#include <tlCore/StringFormat.h>
#include <tlCore/LRUCache.h>

namespace
{
#include "Resources/Audio_192.h"
#include "Resources/Audio_96.h"
#include "Resources/BellowsClosed_192.h"
#include "Resources/BellowsClosed_96.h"
#include "Resources/BellowsOpen_192.h"
#include "Resources/BellowsOpen_96.h"
#include "Resources/Clear_192.h"
#include "Resources/Clear_96.h"
#include "Resources/Color_192.h"
#include "Resources/Color_96.h"
#include "Resources/Compare_192.h"
#include "Resources/Compare_96.h"
#include "Resources/CompareA_192.h"
#include "Resources/CompareA_96.h"
#include "Resources/CompareB_192.h"
#include "Resources/CompareB_96.h"
#include "Resources/CompareDifference_192.h"
#include "Resources/CompareDifference_96.h"
#include "Resources/CompareHorizontal_192.h"
#include "Resources/CompareHorizontal_96.h"
#include "Resources/CompareOverlay_192.h"
#include "Resources/CompareOverlay_96.h"
#include "Resources/CompareTile_192.h"
#include "Resources/CompareTile_96.h"
#include "Resources/CompareVertical_192.h"
#include "Resources/CompareVertical_96.h"
#include "Resources/CompareWipe_192.h"
#include "Resources/CompareWipe_96.h"
#include "Resources/Copy_192.h"
#include "Resources/Copy_96.h"
#include "Resources/Decrement_192.h"
#include "Resources/Decrement_96.h"
#include "Resources/Devices_192.h"
#include "Resources/Devices_96.h"
#include "Resources/Directory_192.h"
#include "Resources/Directory_96.h"
#include "Resources/DirectoryUp_192.h"
#include "Resources/DirectoryUp_96.h"
#include "Resources/DockWidgetClose_192.h"
#include "Resources/DockWidgetClose_96.h"
#include "Resources/DockWidgetNormal_192.h"
#include "Resources/DockWidgetNormal_96.h"
#include "Resources/Empty_192.h"
#include "Resources/Empty_96.h"
#include "Resources/File_192.h"
#include "Resources/File_96.h"
#include "Resources/FileBrowser_192.h"
#include "Resources/FileBrowser_96.h"
#include "Resources/FileClose_192.h"
#include "Resources/FileClose_96.h"
#include "Resources/FileCloseAll_192.h"
#include "Resources/FileCloseAll_96.h"
#include "Resources/FileOpen_192.h"
#include "Resources/FileOpen_96.h"
#include "Resources/FileOpenSeparateAudio_192.h"
#include "Resources/FileOpenSeparateAudio_96.h"
#include "Resources/Files_192.h"
#include "Resources/Files_96.h"
#include "Resources/FrameNext_192.h"
#include "Resources/FrameNext_96.h"
#include "Resources/FramePrev_192.h"
#include "Resources/FramePrev_96.h"
#include "Resources/Increment_192.h"
#include "Resources/Increment_96.h"
#include "Resources/Info_192.h"
#include "Resources/Info_96.h"
#include "Resources/MenuArrow_192.h"
#include "Resources/MenuArrow_96.h"
#include "Resources/MenuChecked_192.h"
#include "Resources/MenuChecked_96.h"
#include "Resources/MenuUnchecked_192.h"
#include "Resources/MenuUnchecked_96.h"
#include "Resources/Messages_192.h"
#include "Resources/Messages_96.h"
#include "Resources/Mute_192.h"
#include "Resources/Mute_96.h"
#include "Resources/Next_192.h"
#include "Resources/Next_96.h"
#include "Resources/PlaybackForward_192.h"
#include "Resources/PlaybackForward_96.h"
#include "Resources/PlaybackReverse_192.h"
#include "Resources/PlaybackReverse_96.h"
#include "Resources/PlaybackStop_192.h"
#include "Resources/PlaybackStop_96.h"
#include "Resources/Prev_192.h"
#include "Resources/Prev_96.h"
#include "Resources/Reset_192.h"
#include "Resources/Reset_96.h"
#include "Resources/Settings_192.h"
#include "Resources/Settings_96.h"
#include "Resources/SubMenuArrow_192.h"
#include "Resources/SubMenuArrow_96.h"
#include "Resources/TimeEnd_192.h"
#include "Resources/TimeEnd_96.h"
#include "Resources/TimeStart_192.h"
#include "Resources/TimeStart_96.h"
#include "Resources/ViewFrame_192.h"
#include "Resources/ViewFrame_96.h"
#include "Resources/ViewZoom1To1_192.h"
#include "Resources/ViewZoom1To1_96.h"
#include "Resources/Volume_192.h"
#include "Resources/Volume_96.h"
#include "Resources/WindowFullScreen_192.h"
#include "Resources/WindowFullScreen_96.h"
#include "Resources/WindowSecondary_192.h"
#include "Resources/WindowSecondary_96.h"
}

#include <atomic>
#include <condition_variable>
#include <list>
#include <mutex>
#include <thread>

namespace tl
{
    namespace ui
    {
        struct IconLibrary::Private
        {
            std::weak_ptr<system::Context> context;
            
            std::map<std::string, std::vector<uint8_t> > iconData;
            
            struct Request
            {
                std::string name;
                float displayScale = 1.F;
                std::promise<std::shared_ptr<imaging::Image> > promise;
            };
            const size_t requestTimeout = 5;

            typedef std::pair<std::string, float> CacheKey;

            struct Mutex
            {
                std::list<std::shared_ptr<Request> > requests;
                bool stopped = false;
                memory::LRUCache<CacheKey, std::shared_ptr<imaging::Image> > cache;
                std::mutex mutex;
            };
            Mutex mutex;

            struct Thread
            {
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };
        
        void IconLibrary::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            p.context = context;
            
            p.iconData["Audio_96.png"] = Audio_96_png;
            p.iconData["BellowsClosed_96.png"] = BellowsClosed_96_png;
            p.iconData["BellowsOpen_96.png"] = BellowsOpen_96_png;
            p.iconData["Clear_96.png"] = Clear_96_png;
            p.iconData["Color_96.png"] = Color_96_png;
            p.iconData["Compare_96.png"] = Compare_96_png;
            p.iconData["CompareA_96.png"] = CompareA_96_png;
            p.iconData["CompareB_96.png"] = CompareB_96_png;
            p.iconData["CompareDifference_96.png"] = CompareDifference_96_png;
            p.iconData["CompareHorizontal_96.png"] = CompareHorizontal_96_png;
            p.iconData["CompareOverlay_96.png"] = CompareOverlay_96_png;
            p.iconData["CompareTile_96.png"] = CompareTile_96_png;
            p.iconData["CompareVertical_96.png"] = CompareVertical_96_png;
            p.iconData["CompareWipe_96.png"] = CompareWipe_96_png;
            p.iconData["Copy_96.png"] = Copy_96_png;
            p.iconData["Decrement_96.png"] = Decrement_96_png;
            p.iconData["Devices_96.png"] = Devices_96_png;
            p.iconData["Directory_96.png"] = Directory_96_png;
            p.iconData["DirectoryUp_96.png"] = DirectoryUp_96_png;
            p.iconData["DockWidgetClose_96.png"] = DockWidgetClose_96_png;
            p.iconData["DockWidgetNormal_96.png"] = DockWidgetNormal_96_png;
            p.iconData["Empty_96.png"] = Empty_96_png;
            p.iconData["File_96.png"] = File_96_png;
            p.iconData["FileBrowser_96.png"] = FileBrowser_96_png;
            p.iconData["FileClose_96.png"] = FileClose_96_png;
            p.iconData["FileCloseAll_96.png"] = FileCloseAll_96_png;
            p.iconData["FileOpen_96.png"] = FileOpen_96_png;
            p.iconData["FileOpenSeparateAudio_96.png"] = FileOpenSeparateAudio_96_png;
            p.iconData["Files_96.png"] = Files_96_png;
            p.iconData["FrameNext_96.png"] = FrameNext_96_png;
            p.iconData["FramePrev_96.png"] = FramePrev_96_png;
            p.iconData["Increment_96.png"] = Increment_96_png;
            p.iconData["Info_96.png"] = Info_96_png;
            p.iconData["MenuArrow_96.png"] = MenuArrow_96_png;
            p.iconData["MenuChecked_96.png"] = MenuChecked_96_png;
            p.iconData["MenuUnchecked_96.png"] = MenuUnchecked_96_png;
            p.iconData["Messages_96.png"] = Messages_96_png;
            p.iconData["Mute_96.png"] = Mute_96_png;
            p.iconData["Next_96.png"] = Next_96_png;
            p.iconData["PlaybackForward_96.png"] = PlaybackForward_96_png;
            p.iconData["PlaybackReverse_96.png"] = PlaybackReverse_96_png;
            p.iconData["PlaybackStop_96.png"] = PlaybackStop_96_png;
            p.iconData["Prev_96.png"] = Prev_96_png;
            p.iconData["Reset_96.png"] = Reset_96_png;
            p.iconData["Settings_96.png"] = Settings_96_png;
            p.iconData["SubMenuArrow_96.png"] = SubMenuArrow_96_png;
            p.iconData["TimeEnd_96.png"] = TimeEnd_96_png;
            p.iconData["TimeStart_96.png"] = TimeStart_96_png;
            p.iconData["ViewFrame_96.png"] = ViewFrame_96_png;
            p.iconData["ViewZoom1To1_96.png"] = ViewZoom1To1_96_png;
            p.iconData["Volume_96.png"] = Volume_96_png;
            p.iconData["WindowFullScreen_96.png"] = WindowFullScreen_96_png;
            p.iconData["WindowSecondary_96.png"] = WindowSecondary_96_png;            

            p.iconData["Audio_192.png"] = Audio_192_png;
            p.iconData["BellowsClosed_192.png"] = BellowsClosed_192_png;
            p.iconData["BellowsOpen_192.png"] = BellowsOpen_192_png;
            p.iconData["Clear_192.png"] = Clear_192_png;
            p.iconData["Color_192.png"] = Color_192_png;
            p.iconData["Compare_192.png"] = Compare_192_png;
            p.iconData["CompareA_192.png"] = CompareA_192_png;
            p.iconData["CompareB_192.png"] = CompareB_192_png;
            p.iconData["CompareDifference_192.png"] = CompareDifference_192_png;
            p.iconData["CompareHorizontal_192.png"] = CompareHorizontal_192_png;
            p.iconData["CompareOverlay_192.png"] = CompareOverlay_192_png;
            p.iconData["CompareTile_192.png"] = CompareTile_192_png;
            p.iconData["CompareVertical_192.png"] = CompareVertical_192_png;
            p.iconData["CompareWipe_192.png"] = CompareWipe_192_png;
            p.iconData["Copy_192.png"] = Copy_192_png;
            p.iconData["Decrement_192.png"] = Decrement_192_png;
            p.iconData["Devices_192.png"] = Devices_192_png;
            p.iconData["Directory_192.png"] = Directory_192_png;
            p.iconData["DirectoryUp_192.png"] = DirectoryUp_192_png;
            p.iconData["DockWidgetClose_192.png"] = DockWidgetClose_192_png;
            p.iconData["DockWidgetNormal_192.png"] = DockWidgetNormal_192_png;
            p.iconData["Empty_192.png"] = Empty_192_png;
            p.iconData["File_192.png"] = File_192_png;
            p.iconData["FileBrowser_192.png"] = FileBrowser_192_png;
            p.iconData["FileClose_192.png"] = FileClose_192_png;
            p.iconData["FileCloseAll_192.png"] = FileCloseAll_192_png;
            p.iconData["FileOpen_192.png"] = FileOpen_192_png;
            p.iconData["FileOpenSeparateAudio_192.png"] = FileOpenSeparateAudio_192_png;
            p.iconData["Files_192.png"] = Files_192_png;
            p.iconData["FrameNext_192.png"] = FrameNext_192_png;
            p.iconData["FramePrev_192.png"] = FramePrev_192_png;
            p.iconData["Increment_192.png"] = Increment_192_png;
            p.iconData["Info_192.png"] = Info_192_png;
            p.iconData["MenuArrow_192.png"] = MenuArrow_192_png;
            p.iconData["MenuChecked_192.png"] = MenuChecked_192_png;
            p.iconData["MenuUnchecked_192.png"] = MenuUnchecked_192_png;
            p.iconData["Messages_192.png"] = Messages_192_png;
            p.iconData["Mute_192.png"] = Mute_192_png;
            p.iconData["Next_192.png"] = Next_192_png;
            p.iconData["PlaybackForward_192.png"] = PlaybackForward_192_png;
            p.iconData["PlaybackReverse_192.png"] = PlaybackReverse_192_png;
            p.iconData["PlaybackStop_192.png"] = PlaybackStop_192_png;
            p.iconData["Prev_192.png"] = Prev_192_png;
            p.iconData["Reset_192.png"] = Reset_192_png;
            p.iconData["Settings_192.png"] = Settings_192_png;
            p.iconData["SubMenuArrow_192.png"] = SubMenuArrow_192_png;
            p.iconData["TimeEnd_192.png"] = TimeEnd_192_png;
            p.iconData["TimeStart_192.png"] = TimeStart_192_png;
            p.iconData["ViewFrame_192.png"] = ViewFrame_192_png;
            p.iconData["ViewZoom1To1_192.png"] = ViewZoom1To1_192_png;
            p.iconData["Volume_192.png"] = Volume_192_png;
            p.iconData["WindowFullScreen_192.png"] = WindowFullScreen_192_png;
            p.iconData["WindowSecondary_192.png"] = WindowSecondary_192_png;            

            p.mutex.cache.setMax(100);

            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
                    while (p.thread.running)
                    {
                        std::list<std::shared_ptr<Private::Request> > requests;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex.mutex);
                            if (p.thread.cv.wait_for(
                                lock,
                                std::chrono::milliseconds(p.requestTimeout),
                                [this]
                                {
                                    return !_p->mutex.requests.empty();
                                }))
                            {
                                std::swap(requests, p.mutex.requests);
                            }
                        }
                        for (const auto& request : requests)
                        {
                            std::shared_ptr<imaging::Image> image;
                            int dpi = 96;
                            if (request->displayScale >= 2.F)
                            {
                                dpi = 192;
                            }
                            const std::string name = string::Format("{0}_{1}.png").
                                arg(request->name).
                                arg(dpi);
                            const auto i = p.iconData.find(name);
                            if (i != p.iconData.end())
                            {
                                try
                                {
                                    if (auto context = p.context.lock())
                                    {
                                        auto io = context->getSystem<io::System>();
                                        auto reader = io->read(
                                            file::Path(name),
                                            { file::MemoryRead(i->second.data(), i->second.size()) });
                                        if (reader)
                                        {
                                            const auto ioInfo = reader->getInfo().get();
                                            const auto videoData = reader->readVideo(ioInfo.videoTime.start_time()).get();
                                            image = videoData.image;
                                        }
                                    }
                                }
                                catch (const std::exception&)
                                {}
                            }
                            request->promise.set_value(image);
                            {
                                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                                p.mutex.cache.add(
                                    std::make_pair(request->name, request->displayScale),
                                    image);
                            }
                        }
                    }

                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.stopped = true;
                    }
                    cancelRequests();
                });
        }

        IconLibrary::IconLibrary() :
            _p(new Private)
        {}

        IconLibrary::~IconLibrary()
        {
            TLRENDER_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<IconLibrary> IconLibrary::create(
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<IconLibrary>(new IconLibrary);
            out->_init(context);
            return out;
        }

        std::future<std::shared_ptr<imaging::Image> > IconLibrary::request(
            const std::string& name,
            float displayScale)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::Request>();
            request->name = name;
            request->displayScale = displayScale;
            auto future = request->promise.get_future();
            bool valid = false;
            std::shared_ptr<imaging::Image> cached;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.cache.get(std::make_pair(name, displayScale), cached))
                {
                    if (!p.mutex.stopped)
                    {
                        valid = true;
                        p.mutex.requests.push_back(request);
                    }
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else if (cached)
            {
                request->promise.set_value(cached);
            }
            else
            {
                request->promise.set_value(nullptr);
            }
            return future;
        }
        
        void IconLibrary::cancelRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::Request> > requests;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                requests = std::move(p.mutex.requests);
            }
            for (auto& request : requests)
            {
                request->promise.set_value(nullptr);
            }
        }
    }
}

