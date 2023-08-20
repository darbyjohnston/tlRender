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
                "precision mediump int;\n"
                "precision mediump float;\n"
                "\n"
                "attribute vec3 vPos;\n"
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
                "}\n";
        }

        std::string meshFragmentSource()
        {
            return
                "precision mediump int;\n"
                "precision mediump float;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
                "}\n";
        }

        std::string colorMeshVertexSource()
        {
            return
                "precision mediump int;\n"
                "precision mediump float;\n"
                "\n"
                "attribute vec3 vPos;\n"
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
                "}\n";
        }

        std::string colorMeshFragmentSource()
        {
            return
                "precision mediump int;\n"
                "precision mediump float;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
                "}\n";
        }

        std::string textFragmentSource()
        {
            return
                "precision mediump int;\n"
                "precision mediump float;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
                "}\n";
        }

        std::string textureFragmentSource()
        {
            return
                "precision mediump int;\n"
                "precision mediump float;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
                "}\n";
        }

        std::string imageFragmentSource()
        {
            return
                "precision mediump int;\n"
                "precision mediump float;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
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
                "precision mediump int;\n"
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
                "precision mediump int;\n"
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

