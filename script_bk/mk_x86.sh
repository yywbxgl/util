#! /bin/bash

# SDK_PATH=/home/qiliang/docker_data/acc_robotaxi/sdk_install/install/
# SDK_PATH=/docker_data/acc_robotaxi/sdk_install/install
SDK_PATH="$(pwd)"/sdk_install/install/
export SDK_INSTALL_PATH=${SDK_PATH}

mkdir build
cd build 
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/linux_x86_toolchain.cmake -DCMAKE_INSTALL_PREFIX=../app  ..
make -j8 
make install

