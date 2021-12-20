// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrApp/CmdLine.h>

#include <tlrCore/Context.h>
#include <tlrCore/SequenceIO.h>
#if defined(OpenEXR_FOUND)
#include <tlrCore/OpenEXR.h>
#endif
#if defined(FFmpeg_FOUND)
#include <tlrCore/FFmpeg.h>
#endif

#include <memory>
#include <string>
#include <vector>

namespace tlr
{
    //! Applications.
    namespace app
    {
        class ICmdLineArg;
        class ICmdLineOption;

        //! Application options.
        struct Options
        {
            float sequenceDefaultSpeed = avio::sequenceDefaultSpeed;
            int sequenceThreadCount = avio::sequenceThreadCount;
#if defined(OpenEXR_FOUND)
            exr::Compression exrCompression = exr::Compression::ZIP;
            float exrDWACompressionLevel = 45.F;
#endif
#if defined(FFmpeg_FOUND)
            std::string ffmpegWriteProfile;
            int ffmpegThreadCount = ffmpeg::threadCount;
#endif
            bool log = false;
            bool help = false;
        };

        //! Base class for applications.
        class IApp : public std::enable_shared_from_this<IApp>
        {
            TLR_NON_COPYABLE(IApp);

        protected:
            void _init(
                int argc,
                char* argv[],
                const std::string& cmdLineName,
                const std::string& cmdLineSummary,
                const std::vector<std::shared_ptr<ICmdLineArg> >& = {},
                const std::vector<std::shared_ptr<ICmdLineOption> >& = {});
            IApp();

        public:
            virtual ~IApp() = 0;

            //! Get the exit code.
            int getExit() const;

        protected:
            void _log(const std::string&, core::LogType = core::LogType::Message);

            void _print(const std::string&);
            void _printNewline();
            void _printError(const std::string&);

            std::shared_ptr<core::Context> _context;
            Options _options;
            int _exit = 0;

        private:
            int _parseCmdLine();
            void _printCmdLineHelp();

            TLR_PRIVATE();
        };
    }
}
