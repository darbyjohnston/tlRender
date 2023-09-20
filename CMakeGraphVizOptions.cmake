# Example:
# > cmake --graphviz=tlRenderLibraries.dot -S ..\..\..\..\tlRender -DTLRENDER_TESTS=OFF
# > dot tlRenderLibraries.dot -Tpng > tlRenderLibraries.png

set(GRAPHVIZ_GRAPH_HEADER "node [ fontsize = 24 ];")
set(GRAPHVIZ_EXECUTABLES FALSE)
set(GRAPHVIZ_INTERFACE_LIBS FALSE)
set(GRAPHVIZ_EXTERNAL_LIBS FALSE)
set(GRAPHVIZ_IGNORE_TARGETS glad tlPlay tlApp tlGLApp tlBakeApp tlResourceApp tlPlayGLApp tlPlayQtApp)
