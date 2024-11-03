rem Build a Visual Studio 2022 solution.
@echo off
pushd %~dp0
cmake -DCMAKE_INSTALL_PREFIX=./out/install/windows-msvc2022-debug -DDEIMOS_OS=windows -DDEIMOS_GEN=msvc2022 --no-warn-unused-cli -BC:./out/build/
popd