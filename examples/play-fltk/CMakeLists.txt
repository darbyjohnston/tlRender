if ( FLTK_FOUND )

  include_directories( ${FLTK_INCLUDE_DIRS} )

  set(HEADERS
      App.h
      Util.h)
  set(SOURCE
      App.cpp
      Util.cpp
      main.cpp)

  add_executable(play-fltk ${SOURCE} ${HEADERS})
  target_link_libraries(play-fltk PRIVATE tlApp tlGL GLFW fltk fltk_gl )
  set_target_properties(play-fltk PROPERTIES FOLDER examples)

endif()
