#! /bin/bash



source ~/qnx710/qnxsdp-env.sh

# SDK_PATH=/home/qiliang/docker_data/acc_robotaxi/sdk_install/install/
SDK_PATH="$(pwd)"/sdk_install/install/
export SDK_INSTALL_PATH=${SDK_PATH}

mkdir build_qnx
cd build_qnx 
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/qnx_aarch64_toolchain.cmake -DCMAKE_INSTALL_PREFIX=../app  ..
make -j8 
make install

