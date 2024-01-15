// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

#include <tlCore/Path.h>

namespace tl
{
    namespace ui
    {
        //! File edit widget.
        class FileEdit : public IWidget
        {
            TLRENDER_NON_COPYABLE(FileEdit);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            FileEdit();

        public:
            virtual ~FileEdit();

            //! Create a new widget.
            static std::shared_ptr<FileEdit> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the path.
            void setPath(const file::Path&);

            //! Get the path.
            const file::Path& getPath() const;

            //! Set the callback.
            void setCallback(const std::function<void(const file::Path&)>&);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            void _openDialog();

            TLRENDER_PRIVATE();
        };
    }
}
