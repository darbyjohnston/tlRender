// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/FileBrowserPrivate.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/ThumbnailSystem.h>

#include <tlCore/StringFormat.h>

#include <ctime>

namespace tl
{
    namespace ui
    {
        struct Button::Private
        {
            file::FileInfo fileInfo;
            FileBrowserOptions options;
            std::vector<std::string> labels;
            std::vector<int> columns;
            std::weak_ptr<ThumbnailSystem> thumbnailSystem;
            InfoRequest infoRequest;
            std::unique_ptr<io::Info> ioInfo;
            ThumbnailRequest thumbnailRequest;
            std::shared_ptr<image::Image> thumbnail;
            std::chrono::steady_clock::time_point thumbnailTime;

            struct SizeData
            {
                int margin = 0;
                int spacing = 0;
                int border = 0;
                image::FontInfo fontInfo;
                image::FontMetrics fontMetrics;
                bool textInit = true;
                std::vector<int> textWidths;
            };
            SizeData size;

            struct DrawData
            {
                std::vector< std::vector<std::shared_ptr<image::Glyph> > > glyphs;
            };
            DrawData draw;
        };

        void Button::_init(
            const file::FileInfo& fileInfo,
            const FileBrowserOptions& options,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IButton::_init("tl::ui::ListButton", context, parent);
            TLRENDER_P();

            setButtonRole(ColorRole::None);
            setAcceptsKeyFocus(true);

            p.fileInfo = fileInfo;
            p.options = options;

            p.labels.push_back(fileInfo.getPath().get(-1, false));

            switch (fileInfo.getType())
            {
            case file::Type::File:
                setIcon("File");
                p.labels.push_back(fileInfo.getPath().getExtension());
                break;
            case file::Type::Directory:
                setIcon("Directory");
                p.labels.push_back(std::string());
                break;
            default: break;
            }

            std::string label;
            const uint64_t size = fileInfo.getSize();
            if (size < memory::megabyte)
            {
                label = string::Format("{0}KB").
                    arg(size / static_cast<float>(memory::kilobyte), 2);
            }
            else if (size < memory::gigabyte)
            {
                label = string::Format("{0}MB").
                    arg(size / static_cast<float>(memory::megabyte), 2);
            }
            else
            {
                label = string::Format("{0}GB").
                    arg(size / static_cast<float>(memory::gigabyte), 2);
            }
            p.labels.push_back(label);

            const std::time_t time = fileInfo.getTime();
            std::tm* localtime = std::localtime(&time);
            char buffer[32];
            std::strftime(buffer, 32, "%a %d/%m/%Y %H:%M:%S", localtime);
            p.labels.push_back(buffer);

            p.thumbnailSystem = context->getSystem<ThumbnailSystem>();
        }

        Button::Button() :
            _p(new Private)
        {}

        Button::~Button()
        {}

        std::shared_ptr<Button> Button::create(
            const file::FileInfo& fileInfo,
            const FileBrowserOptions& options,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Button>(new Button);
            out->_init(fileInfo, options, context, parent);
            return out;
        }

        const std::vector<int>& Button::getTextWidths() const
        {
            return _p->size.textWidths;
        }

        void Button::setColumns(const std::vector<int>& value)
        {
            _p->columns = value;
        }

        void Button::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const TickEvent& event)
        {
            IButton::tickEvent(parentsVisible, parentsEnabled, event);
            TLRENDER_P();
            if (p.infoRequest.future.valid() &&
                p.infoRequest.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                p.ioInfo = std::make_unique<io::Info>(p.infoRequest.future.get());
                _updates |= ui::Update::Size;
                _updates |= ui::Update::Draw;
            }
            const auto now = std::chrono::steady_clock::now();
            if (p.thumbnailRequest.future.valid() &&
                p.thumbnailRequest.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                p.thumbnail = p.thumbnailRequest.future.get();
                p.thumbnailTime = now;
                _updates |= ui::Update::Size;
                _updates |= ui::Update::Draw;
            }
        }

        void Button::sizeHintEvent(const SizeHintEvent& event)
        {
            IButton::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(SizeRole::MarginInside, event.displayScale);
            p.size.spacing = event.style->getSizeRole(SizeRole::Spacing, event.displayScale);
            p.size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);

            _sizeHint = math::Size2i();
            if (!p.labels.empty())
            {
                p.size.fontMetrics = event.getFontMetrics(_fontRole);
                const auto fontInfo = event.style->getFontRole(_fontRole, event.displayScale);
                if (fontInfo != p.size.fontInfo || p.size.textInit)
                {
                    p.size.fontInfo = fontInfo;
                    p.size.textInit = false;
                    p.size.textWidths.clear();
                    for (const auto& label : p.labels)
                    {
                        p.size.textWidths.push_back(
                            event.fontSystem->getSize(label, fontInfo).w);
                    }
                    p.draw.glyphs.clear();
                }
                _sizeHint.h = p.size.fontMetrics.lineHeight;
            }
            if (p.thumbnail)
            {
                _sizeHint.w += p.thumbnail->getWidth();
                _sizeHint.h = std::max(_sizeHint.h, p.thumbnail->getHeight());
            }
            else if (_iconImage)
            {
                _sizeHint.w += _iconImage->getWidth();
                if (!p.labels.empty())
                {
                    _sizeHint.w += p.size.spacing;
                }
                _sizeHint.h = std::max(_sizeHint.h, _iconImage->getHeight());
            }
            _sizeHint.w +=
                p.size.margin * 2 +
                p.size.border * 4;
            _sizeHint.h +=
                p.size.margin * 2 +
                p.size.border * 4;
        }

        void Button::clipEvent(
            const math::Box2i& clipRect,
            bool clipped,
            const ClipEvent& event)
        {
            IWidget::clipEvent(clipRect, clipped, event);
            TLRENDER_P();
            if (!clipped)
            {
                if (p.options.thumbnails)
                {
                    if (auto thumbnailSystem = p.thumbnailSystem.lock())
                    {
                        p.infoRequest = thumbnailSystem->getInfo(p.fileInfo.getPath());
                        p.thumbnailRequest = thumbnailSystem->getThumbnail(
                            p.options.thumbnailHeight,
                            p.fileInfo.getPath());
                    }
                }
            }
            else
            {
                p.infoRequest = InfoRequest();
                p.thumbnailRequest = ThumbnailRequest();
                p.thumbnail.reset();
                p.draw.glyphs.clear();
            }
        }

        void Button::drawEvent(
            const math::Box2i& drawRect,
            const DrawEvent& event)
        {
            IButton::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::Box2i& g = _geometry;
            const bool enabled = isEnabled();

            // Draw the key focus.
            if (_keyFocus)
            {
                event.render->drawMesh(
                    border(g, p.size.border * 2),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::KeyFocus));
            }

            // Draw the background and checked state.
            const ColorRole colorRole = _checked ?
                ColorRole::Checked :
                _buttonRole;
            if (colorRole != ColorRole::None)
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(colorRole));
            }

            // Draw the pressed and hover states.
            if (_mouse.press && _geometry.contains(_mouse.pos))
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (_mouse.inside)
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ColorRole::Hover));
            }

            // Draw the thumbnail or icon.
            const math::Box2i g2 = g.margin(-p.size.border * 2);
            int x = g2.x() + p.size.margin;
            if (p.thumbnail)
            {
                const image::Size& size = p.thumbnail->getSize();
                event.render->drawImage(
                    p.thumbnail,
                    math::Box2i(
                        x,
                        g2.y() + g2.h() / 2 - size.h / 2,
                        size.w,
                        size.h),
                    event.style->getColorRole(enabled ?
                        ColorRole::Text :
                        ColorRole::TextDisabled));
                x += size.w + p.size.spacing;
            }
            else if (_iconImage)
            {
                const image::Size& size = _iconImage->getSize();
                event.render->drawImage(
                    _iconImage,
                    math::Box2i(
                        x,
                        g2.y() + g2.h() / 2 - size.h / 2,
                        size.w,
                        size.h),
                    event.style->getColorRole(enabled ?
                        ColorRole::Text :
                        ColorRole::TextDisabled));
                x += size.w + p.size.spacing;
            }

            // Draw the text.
            int rightColumnsWidth = 0;
            for (size_t i = 1; i < p.columns.size(); ++i)
            {
                rightColumnsWidth += p.columns[i];
            }
            const bool glyphsInit = p.draw.glyphs.empty();
            for (size_t i = 0; i < p.labels.size() && i < p.columns.size(); ++i)
            {
                if (glyphsInit)
                {
                    p.draw.glyphs.push_back(
                        event.fontSystem->getGlyphs(p.labels[i], p.size.fontInfo));
                }
                const math::Vector2i pos(
                    x,
                    g2.y() + g2.h() / 2 - p.size.fontMetrics.lineHeight / 2 +
                    p.size.fontMetrics.ascender);
                event.render->drawText(
                    p.draw.glyphs[i],
                    pos,
                    event.style->getColorRole(enabled ?
                        ColorRole::Text :
                        ColorRole::TextDisabled));
                if (0 == i)
                {
                    x = g2.max.x - p.size.margin - rightColumnsWidth;
                }
                else
                {
                    x += p.columns[i];
                }
            }
        }

        void Button::keyPressEvent(KeyEvent& event)
        {
            if (0 == event.modifiers)
            {
                switch (event.key)
                {
                case Key::Enter:
                    event.accept = true;
                    takeKeyFocus();
                    if (_pressedCallback)
                    {
                        _pressedCallback();
                    }
                    _click();
                    break;
                case Key::Escape:
                    if (hasKeyFocus())
                    {
                        event.accept = true;
                        releaseKeyFocus();
                    }
                    break;
                default: break;
                }
            }
        }

        void Button::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }
    }
}
