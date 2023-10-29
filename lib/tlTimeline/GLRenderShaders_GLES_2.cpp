// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimeline/GLRenderPrivate.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace timeline
    {
        std::string vertexSource()
        {
            return
                "precision mediump float;\n"
                "\n"
                "attribute vec3 vPos;\n"
                "attribute vec2 vTexture;\n"
                "varying vec2 fTexture;\n"
                "\n"
                "struct Transform\n"
                "{\n"
                "    mat4 mvp;\n"
                "};\n"
                "\n"
                "uniform Transform transform;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    gl_Position = transform.mvp * vec4(vPos, 1.0);\n"
                "    fTexture = vTexture;\n"
                "}\n";
        }

        std::string meshFragmentSource()
        {
            return
                "precision mediump float;\n"
                "\n"
                "uniform vec4 color;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    gl_FragColor = color;\n"
                "}\n";
        }

        std::string colorMeshVertexSource()
        {
            return
                "precision mediump float;\n"
                "\n"
                "attribute vec3 vPos;\n"
                "attribute vec4 vColor;\n"
                "varying vec4 fColor;\n"
                "\n"
                "struct Transform\n"
                "{\n"
                "    mat4 mvp;\n"
                "};\n"
                "\n"
                "uniform Transform transform;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    gl_Position = transform.mvp * vec4(vPos, 1.0);\n"
                "    fColor = vColor;\n"
                "}\n";
        }

        std::string colorMeshFragmentSource()
        {
            return
                "precision mediump float;\n"
                "\n"
                "varying vec4 fColor;\n"
                "\n"
                "uniform vec4 color;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    gl_FragColor = fColor * color;\n"
                "}\n";
        }

        std::string textFragmentSource()
        {
            return
                "precision mediump float;\n"
                "\n"
                "varying vec2 fTexture;\n"
                "\n"
                "uniform vec4 color;\n"
                "uniform sampler2D textureSampler;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    gl_FragColor.r = color.r;\n"
                "    gl_FragColor.g = color.g;\n"
                "    gl_FragColor.b = color.b;\n"
                "    gl_FragColor.a = color.a * texture2D(textureSampler, fTexture).r;\n"
                "}\n";
        }

        std::string textureFragmentSource()
        {
            return
                "precision mediump float;\n"
                "\n"
                "varying vec2 fTexture;\n"
                "\n"
                "uniform vec4 color;\n"
                "uniform sampler2D textureSampler;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    gl_FragColor = texture2D(textureSampler, fTexture) * color;\n"
                "}\n";
        }

        std::string imageFragmentSource()
        {
            return
                "precision mediump float;\n"
                "\n"
                "varying vec2 fTexture;\n"
                "\n"
                "uniform vec4 color;\n"
                "uniform sampler2D textureSampler;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    gl_FragColor = texture2D(textureSampler, fTexture) * color;\n"
                "}\n";
        }

        std::string displayFragmentSource(
            const std::string& colorConfigDef,
            const std::string& colorConfig,
            const std::string& lutDef,
            const std::string& lut,
            LUTOrder lutOrder)
        {
             return
                "precision mediump float;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
                "}\n";
        }

        std::string differenceFragmentSource()
        {
            return
                "precision mediump float;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
                "}\n";
        }
    }
}

