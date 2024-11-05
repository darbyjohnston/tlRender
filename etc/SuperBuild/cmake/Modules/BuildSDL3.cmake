include(ExternalProject)

set(SDL3_GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git")
set(SDL3_GIT_TAG "preview-3.1.6")

set(SDL3_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DSDL_STATIC=ON
    -DSDL_DUMMYVIDEO=OFF
    -DSDL_OPENGL=OFF
    -DSDL_OPENGLES=OFF
    -DSDL_ALSA=ON
    -DSDL_OSS=OFF
    -DSDL_JACK=OFF
    -DSDL_PIPEWIRE=OFF
    -DSDL_PULSEAUDIO=ON
    -DSDL_SNDIO=OFF
    -DSDL_X11=OFF
    -DSDL_WAYLAND=OFF
    -DSDL_RPI=OFF
    -DSDL_COCOA=OFF
    -DSDL_DIRECTX=OFF
    -DSDL_XINPUT=OFF
    -DSDL_RENDER_D3D=OFF
    -DSDL_RENDER_METAL=OFF
    -DSDL_VIVANTE=OFF
    -DSDL_VULKAN=OFF
    -DSDL_METAL=OFF
    -DSDL_KMSDRM=OFF
    -DSDL_VIDEO=OFF
    -DSDL_RENDER=OFF
    -DSDL_JOYSTICK=OFF
    -DSDL_HAPTIC=OFF
    #-DSDL_HIDAPI=OFF
    -DSDL_POWER=OFF
    -DSDL_SENSOR=OFF
    -DSDL_DIALOG=OFF)

ExternalProject_Add(
    SDL3
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/SDL3
    GIT_REPOSITORY ${SDL3_GIT_REPOSITORY}
    GIT_TAG ${SDL3_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${SDL3_ARGS})
