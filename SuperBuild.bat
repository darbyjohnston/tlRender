set BUILD_TYPE=%1
IF "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

set JOBS=4

cmake ^
    -S tlRender\etc\SuperBuild ^
    -B superbuild-%BUILD_TYPE% ^
    -DCMAKE_INSTALL_PREFIX=%CD%\install-%BUILD_TYPE% ^
    -DCMAKE_PREFIX_PATH=%CD%\install-%BUILD_TYPE% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
cmake --build superbuild-%BUILD_TYPE% -j %JOBS% --config %BUILD_TYPE%

cmake ^
    -S tlRender ^
    -B build-%BUILD_TYPE% ^
    -DCMAKE_INSTALL_PREFIX=%CD%\install-%BUILD_TYPE% ^
    -DCMAKE_PREFIX_PATH=%CD%\install-%BUILD_TYPE% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
cmake --build build-%BUILD_TYPE% -j %JOBS% --config %BUILD_TYPE%
cmake --build build-%BUILD_TYPE% --config %BUILD_TYPE% --target INSTALL

