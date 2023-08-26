if(WIN32)
    set(CPACK_GENERATOR ZIP)
    
    set(INSTALL_DLLS)
    
    if(TLRENDER_FFMPEG)
        set(FFMPEG_DLLS
            ${CMAKE_INSTALL_PREFIX}/bin/avcodec-60.dll
            ${CMAKE_INSTALL_PREFIX}/bin/avdevice-60.dll
            ${CMAKE_INSTALL_PREFIX}/bin/avfilter-9.dll
            ${CMAKE_INSTALL_PREFIX}/bin/avformat-60.dll
            ${CMAKE_INSTALL_PREFIX}/bin/avutil-58.dll
            ${CMAKE_INSTALL_PREFIX}/bin/swresample-4.dll
            ${CMAKE_INSTALL_PREFIX}/bin/swscale-7.dll)
        list(APPEND INSTALL_DLLS ${FFMPEG_DLLS})
    endif()
    
    if(TLRENDER_USD)
        set(BOOST_DLLS
            ${CMAKE_INSTALL_PREFIX}/bin/boost_atomic-vc143-mt-x64-1_78.dll
            ${CMAKE_INSTALL_PREFIX}/bin/boost_regex-vc143-mt-x64-1_78.dll)
        set(MATERIALX_DLLS
            ${CMAKE_INSTALL_PREFIX}/bin/MaterialXCore.dll
            ${CMAKE_INSTALL_PREFIX}/bin/MaterialXFormat.dll
            ${CMAKE_INSTALL_PREFIX}/bin/MaterialXGenGlsl.dll
            ${CMAKE_INSTALL_PREFIX}/bin/MaterialXGenMdl.dll
            ${CMAKE_INSTALL_PREFIX}/bin/MaterialXGenMsl.dll
            ${CMAKE_INSTALL_PREFIX}/bin/MaterialXGenOsl.dll
            ${CMAKE_INSTALL_PREFIX}/bin/MaterialXGenShader.dll
            ${CMAKE_INSTALL_PREFIX}/bin/MaterialXRender.dll
            ${CMAKE_INSTALL_PREFIX}/bin/MaterialXRenderGlsl.dll
            ${CMAKE_INSTALL_PREFIX}/bin/MaterialXRenderHw.dll
            ${CMAKE_INSTALL_PREFIX}/bin/MaterialXRenderOsl.dll)
        set(TBB_DLLS
            ${CMAKE_INSTALL_PREFIX}/bin/tbb.dll)
        set(USD_DLLS
            ${CMAKE_INSTALL_PREFIX}/bin/usd_ar.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_arch.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_cameraUtil.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_garch.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_geomUtil.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_gf.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_glf.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_hd.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_hdGp.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_hdMtlx.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_hdSt.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_hdar.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_hdsi.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_hdx.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_hf.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_hgi.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_hgiGL.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_hgiInterop.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_hio.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_js.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_kind.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_ndr.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_pcp.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_plug.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_pxOsd.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_sdf.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_sdr.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_tf.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_trace.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usd.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdAppUtils.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdBakeMtlx.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdGeom.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdHydra.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdImaging.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdImagingGL.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdLux.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdMedia.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdMtlx.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdPhysics.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdProc.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdProcImaging.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdRender.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdRi.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdRiImaging.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdShade.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdSkel.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdSkelImaging.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdUI.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdUtils.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdVol.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_usdVolImaging.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_vt.dll
            ${CMAKE_INSTALL_PREFIX}/bin/usd_work.dll)
        list(APPEND INSTALL_DLLS ${BOOST_DLLS} ${MATERIALX_DLLS} ${TBB_DLLS} ${USD_DLLS})

        install(
            DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin/usd
            DESTINATION bin)
        install(
            DIRECTORY ${CMAKE_INSTALL_PREFIX}/plugin
            DESTINATION ".")
    endif()
    
    install(
        FILES ${INSTALL_DLLS}
        DESTINATION bin)

elseif(APPLE)

    set(CPACK_GENERATOR TGZ)

    list(APPEND CMAKE_INSTALL_RPATH
        "@executable_path/../lib")
    #set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    set(INSTALL_DYLIBS)
    
    if(TLRENDER_FFMPEG)
        set(FFMPEG_DYLIBS
            ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.60.3.100.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.60.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libavdevice.60.1.100.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libavdevice.60.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libavdevice.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libavfilter.9.3.100.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libavfilter.9.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libavfilter.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libavformat.60.3.100.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libavformat.60.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libavformat.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libavutil.58.2.100.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libavutil.58.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libavutil.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libswresample.4.10.100.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libswresample.4.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libswresample.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libswscale.7.1.100.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libswscale.7.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libswscale.dylib)
        list(APPEND INSTALL_DYLIBS ${FFMPEG_DYLIBS})
    endif()
    
    if(TLRENDER_USD)
        set(BOOST_DYLIBS
            ${CMAKE_INSTALL_PREFIX}/lib/libboost_atomic.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libboost_regex.dylib)
        set(MATERIALX_DYLIBS
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXCore.1.38.7.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXCore.1.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXCore.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXFormat.1.38.7.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXFormat.1.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXFormat.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXGenGlsl.1.38.7.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXGenGlsl.1.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXGenGlsl.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXGenMdl.1.38.7.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXGenMdl.1.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXGenMdl.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXGenMsl.1.38.7.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXGenMsl.1.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXGenMsl.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXGenOsl.1.38.7.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXGenOsl.1.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXGenOsl.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXGenShader.1.38.7.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXGenShader.1.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXGenShader.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXRender.1.38.7.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXRender.1.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXRender.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXRenderGlsl.1.38.7.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXRenderGlsl.1.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXRenderGlsl.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXRenderHw.1.38.7.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXRenderHw.1.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXRenderHw.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXRenderMsl.1.38.7.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXRenderMsl.1.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXRenderMsl.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXRenderOsl.1.38.7.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXRenderOsl.1.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libMaterialXRenderOsl.dylib)
        set(TBB_DYLIBS
            ${CMAKE_INSTALL_PREFIX}/lib/libtbb.dylib)
            #${CMAKE_INSTALL_PREFIX}/lib/libtbb_debug.dylib
            #${CMAKE_INSTALL_PREFIX}/lib/libtbbmalloc.dylib
            #${CMAKE_INSTALL_PREFIX}/lib/libtbbmalloc_debug.dylib
            #${CMAKE_INSTALL_PREFIX}/lib/libtbbmalloc_proxy.dylib
            #${CMAKE_INSTALL_PREFIX}/lib/libtbbmalloc_proxy_debug.dylib)
        set(OSD_DYLIBS
            ${CMAKE_INSTALL_PREFIX}/lib/libosdCPU.3.5.0.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libosdCPU.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libosdGPU.3.5.0.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libosdGPU.dylib)
        set(USD_DYLIBS
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_ar.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_arch.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_cameraUtil.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_garch.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_geomUtil.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_gf.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_glf.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_hd.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_hdGp.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_hdMtlx.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_hdSt.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_hdar.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_hdsi.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_hdx.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_hf.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_hgi.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_hgiGL.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_hgiInterop.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_hgiMetal.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_hio.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_js.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_kind.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_ndr.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_pcp.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_plug.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_pxOsd.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_sdf.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_sdr.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_tf.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_trace.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usd.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdAppUtils.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdBakeMtlx.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdGeom.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdHydra.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdImaging.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdImagingGL.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdLux.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdMedia.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdMtlx.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdPhysics.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdProc.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdProcImaging.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdRender.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdRi.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdRiImaging.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdShade.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdSkel.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdSkelImaging.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdUI.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdUtils.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdVol.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_usdVolImaging.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_vt.dylib
            ${CMAKE_INSTALL_PREFIX}/lib/libusd_work.dylib)
        list(APPEND INSTALL_DYLIBS ${BOOST_DYLIBS} ${MATERIALX_DYLIBS} ${TBB_DYLIBS} ${OSD_DYLIBS} ${USD_DYLIBS})

        install(
            DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib/usd
            DESTINATION lib)
        install(
            DIRECTORY ${CMAKE_INSTALL_PREFIX}/plugin
            DESTINATION ".")
    endif()
    
    install(
        FILES ${INSTALL_DYLIBS}
        DESTINATION lib)

endif()
