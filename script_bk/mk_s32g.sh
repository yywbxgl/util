#! /bin/bash

SDK_PATH="$(pwd)"/sdk_install/install/
# SDK_PATH=/docker_data/acc_robotaxi/sdk_install
export SDK_INSTALL_PATH=${SDK_PATH}

mkdir build_s32g
cd build_s32g
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/linux_s32g_toolchain.cmake -DCMAKE_INSTALL_PREFIX=../app ..
make -j8
make install

