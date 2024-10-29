#! /bin/bash

SDK_PATH="$(pwd)"/sdk_install/install/
# SDK_PATH=/docker_data/acc_robotaxi/sdk_install
export SDK_INSTALL_PATH=${SDK_PATH}

mkdir build_rashypi
cd build_rashypi
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/linux_aarch64_rashypi.cmake -DCMAKE_INSTALL_PREFIX=../app ..
make -j8
make install

