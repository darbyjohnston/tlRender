set(HEADERS
    Assert.h
    BBox.h
    BBoxInline.h
    Cache.h
    CacheInline.h
    Error.h
    File.h
    Math.h
    MathInline.h
    Matrix.h
    MatrixInline.h
    Range.h
    RangeInline.h
    String.h
    StringFormat.h
    Util.h
    Vector.h
    VectorInline.h)
set(SOURCE
    Assert.cpp
    File.cpp
    String.cpp
    StringFormat.cpp
    Util.cpp)

add_library(tlrCore ${HEADERS} ${SOURCE})
target_link_libraries(tlrCore FSeq)
set_target_properties(tlrCore PROPERTIES FOLDER lib)
