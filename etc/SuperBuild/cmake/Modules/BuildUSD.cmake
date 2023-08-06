# \todo The Windows build currently only works in "Release" and "RelWithDebInfo"
# configurations. Building in "Debug" gives these error messages:
#
# LINK : fatal error LNK1104: cannot open file 'tbb_debug.lib'
#
# However both "tbb_debug.lib" and "tbb_debug.dll" exist in the install directory.

include(ExternalProject)

set(USD_DEPS)

set(USD_ARGS)
if(CMAKE_OSX_ARCHITECTURES)
    list(APPEND USD_ARGS --build-target ${CMAKE_OSX_ARCHITECTURES})
endif()
if(CMAKE_OSX_DEPLOYMENT_TARGET)
    list(APPEND USD_ARGS --build-args)
    list(APPEND USD_ARGS USD,"-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    list(APPEND USD_ARGS OpenSubdiv,"-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    list(APPEND USD_ARGS MaterialX,"-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    #list(APPEND USD_ARGS TBB,"CFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} CXXFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()
list(APPEND USD_ARGS --no-python)

set(USD_INSTALL_COMMAND)
if(WIN32)
    # \todo On Windows the USD cmake build system installs the "*.dll" files
    # and "usd" directory into "lib", however it seems like they need to be
    # in "bin" instead.
    set(USD_INSTALL_COMMAND
        ${CMAKE_COMMAND} -E copy_directory ${CMAKE_INSTALL_PREFIX}/lib/usd  ${CMAKE_INSTALL_PREFIX}/bin/usd
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/boost_atomic-vc143-mt-x64-1_78.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/boost_regex-vc143-mt-x64-1_78.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_ar.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_arch.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_cameraUtil.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_garch.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_geomUtil.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_gf.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_glf.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_hd.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_hdGp.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_hdMtlx.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_hdSt.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_hdar.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_hdsi.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_hdx.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_hf.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_hgi.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_hgiGL.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_hgiInterop.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_hio.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_js.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_kind.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_ndr.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_pcp.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_plug.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_pxOsd.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_sdf.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_sdr.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_tf.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_trace.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usd.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdAppUtils.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdBakeMtlx.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdGeom.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdHydra.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdImaging.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdImagingGL.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdLux.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdMedia.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdMtlx.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdPhysics.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdProc.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdProcImaging.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdRender.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdRi.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdRiImaging.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdShade.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdSkel.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdSkelImaging.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdUI.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdUtils.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdVol.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_usdVolImaging.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_vt.dll ${CMAKE_INSTALL_PREFIX}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/usd_work.dll ${CMAKE_INSTALL_PREFIX}/bin)
endif()

ExternalProject_Add(
    USD
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/USD
    DEPENDS ${USD_DEPS}
    URL https://github.com/PixarAnimationStudios/OpenUSD/archive/refs/tags/v23.08.tar.gz
    CONFIGURE_COMMAND ""
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/USD-patch/build_usd.py
        ${CMAKE_CURRENT_BINARY_DIR}/USD/src/USD/build_scripts/build_usd.py
    BUILD_COMMAND ${TLRENDER_USD_PYTHON} build_scripts/build_usd.py ${USD_ARGS} ${CMAKE_INSTALL_PREFIX}
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND "${USD_INSTALL_COMMAND}")
