// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <pxr/pxr.h>
#include <pxr/base/tf/diagnostic.h>
#include <pxr/base/tf/token.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usdImaging/usdAppUtils/api.h>
#include <pxr/usdImaging/usdAppUtils/camera.h>
#include <pxr/usdImaging/usdAppUtils/frameRecorder.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/imaging/hd/renderBuffer.h>
#include <pxr/imaging/hdSt/hioConversions.h>
#include <pxr/imaging/hdSt/textureUtils.h>
#include <pxr/imaging/hdx/tokens.h>
#include <pxr/imaging/hdx/types.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

using namespace PXR_NS;

namespace
{
    void glfwErrorCallback(int, const char* description)
    {
        std::cerr << "GLFW ERROR: " << description << std::endl;
    }
}

int main(int argc, char* argv[])
{
    int r = 0;
    try
    {
        // Command line arguments.
        if (argc != 3)
        {
            throw std::runtime_error("usage: usdrecord-glfw (input usd file) (output image file)");
        }
        const std::string usdFileName(argv[1]);
        const std::string imageFileName(argv[2]);
        
        // Open the USD file.
        auto stage = UsdStage::Open(usdFileName);
        auto camera = UsdAppUtilsGetCameraAtPath(stage, SdfPath("main_cam"));
        std::cout << "Camera: " << camera.GetPath().GetAsToken().GetText() << std::endl;
        
        // Initialize GLFW.
        glfwSetErrorCallback(glfwErrorCallback);
        int glfwMajor = 0;
        int glfwMinor = 0;
        int glfwRevision = 0;
        glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);
        std::cout << "GLFW version: " << glfwMajor << "." << glfwMinor << "." << glfwRevision << std::endl;
        if (!glfwInit())
        {
            throw std::runtime_error("Cannot initialize GLFW");
        }
        
        // Create the window.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
        GLFWwindow* glfwWindow = glfwCreateWindow(1, 1, "usdrecord-glfw", NULL, NULL);
        if (!glfwWindow)
        {
            throw std::runtime_error("Cannot create window");
        }
        glfwMakeContextCurrent(glfwWindow);
        const int glMajor = glfwGetWindowAttrib(glfwWindow, GLFW_CONTEXT_VERSION_MAJOR);
        const int glMinor = glfwGetWindowAttrib(glfwWindow, GLFW_CONTEXT_VERSION_MINOR);
        const int glRevision = glfwGetWindowAttrib(glfwWindow, GLFW_CONTEXT_REVISION);
        std::cout << "OpenGL version: " << glMajor << "." << glMinor << "." << glRevision << std::endl;
            
        // Record the frames.
        {
            const bool gpuEnabled = true;
            UsdAppUtilsFrameRecorder frameRecorder(TfToken(), gpuEnabled);
            frameRecorder.Record(stage, camera, stage->GetStartTimeCode(), imageFileName);
        }
        
        // Clean up.
        glfwDestroyWindow(glfwWindow);
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        r = 1;
    }
    return r;
}

