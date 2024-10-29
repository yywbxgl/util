#! /bin/bash

CYBER_WORK_DIR="/mnt/docker_data/cyber"
CYBER_LOG_DIR="${CYBER_WORK_DIR}/log"
CYBER_CONFIG_DIR="${CYBER_WORK_DIR}/conf"

if [ ! -d ${CYBER_WORK_DIR} ];then
	mkdir -p ${CYBER_WORK_DIR}
fi

if [ ! -d ${CYBER_LOG_DIR} ];then
	mkdir -p ${CYBER_LOG_DIR}
fi

if [ ! -d ${CYBER_CONFIG_DIR} ];then
	mkdir -p ${CYBER_CONFIG_DIR}
fi

# copy files to target directory
current_source_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
current_source_dir="$(pwd)"

cp ${current_source_dir}/app/etc/*.conf ${CYBER_CONFIG_DIR}
#sudo cp ${current_source_dir}/app/etc/*  /etc


export CYBER_PATH=${CYBER_WORK_DIR} 

export GLOG_log_dir=${CYBER_LOG_DIR}
export GLOG_alsologtostderr=0
export GLOG_colorlogtostderr=1
export GLOG_minloglevel=0

export sysmo_start=0

# for DEBUG log
export GLOG_v=4

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/mnt/docker_data/acc_robotaxi/sdk_install/install/aarch64/lib:/mnt/docker_data/acc_robotaxi/app/lib
