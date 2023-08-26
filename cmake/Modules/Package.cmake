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
    endif()
    
    install(
        FILES ${INSTALL_DLLS}
        DESTINATION bin)
endif()
