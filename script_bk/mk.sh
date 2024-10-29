#! /bin/bash

# SDK_PATH=/home/qiliang/docker_data/acc_robotaxi/sdk_install/install/
# SDK_PATH=/docker_data/acc_robotaxi/sdk_install/install
SDK_PATH="$(pwd)"/sdk/sdk_install/
export SDK_INSTALL_PATH=${SDK_PATH}


# 指定cmake， 默认x86编译
TOOLCHAIN_FILE=./cmake/linux_x86_toolchain.cmake
if [ -n "$1" ]; then
    if [ "$1" == "j5" ]; then
        TOOLCHAIN_FILE=./cmake/linux_aarch64_j5.cmake
    elif [ "$1" == "qnx" ]; then
        TOOLCHAIN_FILE=./cmake/qnx_aarch64_toolchain.cmake
        source ~/qnx710/qnxsdp-env.sh  # source qnx env
    elif [ "$1" == "s32g" ]; then
        TOOLCHAIN_FILE=./cmake/linux_s32g_toolchain.cmake
    elif [ "$1" == "ti" ]; then
        TOOLCHAIN_FILE=./cmake/linux_aarch64_ti.cmake
    elif [ "$1" == "pi" ]; then  # rashypi
        TOOLCHAIN_FILE=./cmake/linux_aarch64_rashypi.cmake
    else
        echo "invalid input: $1"
        echo "Usage: $0 [j5|qnx|s32g|ti|pi]"
        exit 1
    fi
fi


mkdir build_$1
cd build_$1
cmake -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE} -DCMAKE_INSTALL_PREFIX=../app  ..
make -j8 
make install

