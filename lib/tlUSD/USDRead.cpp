// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUSD/USDPrivate.h>

namespace tl
{
    namespace usd
    {        
        struct Read::Private
        {
            int64_t id = -1;
            std::shared_ptr<Renderer> renderer;
        };
                
        void Read::_init(
            int64_t id,
            const std::shared_ptr<Renderer>& renderer,
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            IRead::_init(path, memory, options, logSystem);
            TLRENDER_P();
            p.id = id;
            p.renderer = renderer;
        }

        Read::Read() :
            _p(new Private)
        {}

        Read::~Read()
        {}

        std::shared_ptr<Read> Read::create(
            int64_t id,
            const std::shared_ptr<Renderer>& renderer,
            const file::Path& path,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(id, renderer, path, {}, options, logSystem);
            return out;
        }

        std::future<io::Info> Read::getInfo()
        {
            TLRENDER_P();
            return p.renderer->getInfo(p.id, _path);
        }
        
        std::future<io::VideoData> Read::readVideo(const otime::RationalTime& time, uint16_t layer)
        {
            TLRENDER_P();
            return p.renderer->render(p.id, _path, time, layer);
        }
        
        void Read::cancelRequests()
        {
            TLRENDER_P();
            p.renderer->cancelRequests(p.id);
        }
    }
}

