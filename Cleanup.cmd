@pushd "%~dp0" || exit /b 1
rmdir /S /Q bin
rmdir /S /Q obj
rmdir /S /Q libMmkDebugStack\build\native\lib
