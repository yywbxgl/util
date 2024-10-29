#!/usr/bin/env bash

# 错误码
ERR_MSG_DOCKER_NOT_INSTALL="Docker not installed."
ERR_MSG_DOCKER_SOCKET_PERMISSION="Docker socket permission deny."
ERR_MSG_NVIDIA_DOCKER_NOT_INSTALL="Nvidia docker not installed."
ERR_MSG_DOCKER_NOT_RUNNING="Docker not running."
GPUexist="Flase"
TAG="runtime_V_1_0_2"

DOCKER_USER=None
IMAGE_NAME=robotaxi

# 运行环境检测失败，打印错误码并且退出
function envCheckFailedAndExit() {
   echo "images environment error."
   echo "Error msg: $1"
   exit 1
}

# 检测Docker是否安装
function checkDockerInstall() {
   docker --version | grep "Docker version" 1>/dev/null 2>&1
   # shellcheck disable=SC2181
   if [ $? != 0 ]; then
      envCheckFailedAndExit "$ERR_MSG_DOCKER_NOT_INSTALL"
   fi
}

# 检测nvidia-docker是否安装
function checkNvidiaDocker() {
   nvidia-docker -v | grep 'Docker version' 1>/dev/null 2>&1
   # shellcheck disable=SC2181
   if [ $? != 0 ]; then
      envCheckFailedAndExit "$ERR_MSG_NVIDIA_DOCKER_NOT_INSTALL"
   fi
}

# 检测nvidia-driver是否安装
function checkNvidiaDocker() {
   nvidia-smi | grep 'Driver Version' 1>/dev/null 2>&1
   # shellcheck disable=SC2181
   if [ $? != 0 ]; then
      echo "GPU driver is not installed, Please check the installation of driver   OK"
      GPUexist="False"
   else
      echo "GPU driver is detected   OK"
      GPUexist="True"
   fi
}

# 检验docker是否在运行
function checkDockerIsRunning() {
   checkResult=$(docker info --format '{{json .}}' | grep "Is the docker daemon running")
   if [ -n "$checkResult" ]; then
      envCheckFailedAndExit "$ERR_MSG_DOCKER_NOT_RUNNING"
   fi
}

# 检测docker socket权限
function checkDockerPermission() {
   checkResult=$(docker info --format '{{json .}}' | grep "Got permission denied while trying to connect to the Docker daemon socket")
   if [ -n "$checkResult" ]; then
      envCheckFailedAndExit "$ERR_MSG_DOCKER_SOCKET_PERMISSION"
   fi
}

# 检测运行环境
function checkRuntimeEnvironment() {
   echo "Begin check images environment..."
   checkDockerInstall
   checkDockerPermission
   checkDockerIsRunning
   checkNvidiaDocker
   echo "images environment OK"
   echo ""
}

# 在docker容器中创建对应的非root用户
function createDockerUser() {
   if [ "${USER}" != "root" ] || [ "$1" == "car" ]; then
      echo ""
      echo "Current user is not root, begin to create docker user..."
      docker exec "${CONTAINER_NAME}" bash -c 'userdel jiahangzhu; /scripts/add_user.sh'
      # shellcheck disable=SC2181
      if [ $? == 0 ]; then
         echo "Create docker user success."
         echo ""
      else
         echo "Create docker user failed."
         exit 1
      fi
   fi
}

function main() {
   if [ "$1" == "car" ]; then
      GRP_ID=1000
      GRP_NAME=joyson
      USER_ID=1000
      USER_NAME=joyson
   else
      GRP_ID=$(id -g)
      GRP_NAME=$(id -g -n)
      USER_ID=$(id -u)
      USER_NAME=$(whoami)
   fi

   echo "Group id=$GRP_ID name=$GRP_NAME"
   echo "User id=$USER_ID name=$USER_NAME"
   echo ""
   EASY_PATH=/home/$USER_NAME/docker_data

   FULL_IMAGE_NAME=$IMAGE_NAME:$TAG
   checkRuntimeEnvironment

   if [ ! -d "$EASY_PATH" ]; then
      echo "easy_path not exist, create dir ${EASY_PATH}"
      mkdir "$EASY_PATH"
   fi

   CONTAINER_NAME="${IMAGE_NAME}"_runtime_"${USER_NAME}"
   docker ps -a --format "{{.Names}}" | grep "$CONTAINER_NAME" 1>/dev/null
   # shellcheck disable=SC2181
   if [ $? == 0 ]; then
      echo "${CONTAINER_NAME} is running, stop and remove..."
      docker stop "$CONTAINER_NAME" 1>/dev/null
      docker rm -v -f "$CONTAINER_NAME" 1>/dev/null
      echo "${CONTAINER_NAME} stop and remove success"
      echo ""
   fi

   echo "Starting docker container ${CONTAINER_NAME} ..."

   xhost + 

   if [ $GPUexist == "False" ]; then 
      echo "=========  NO GPU VERSION  ========="
      docker run -it --shm-size="8g" -d --privileged --name "$CONTAINER_NAME" \
         --network host \
         -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix \
         -e DOCKER_IMG="$IMAGE_NAME" \
         -e DOCKER_USER="$USER_NAME" \
         -e DOCKER_USER_ID="$USER_ID" \
         -e DOCKER_GRP="$GRP_NAME" \
         -e DOCKER_GRP_ID="$GRP_ID" \
	 --device /det/ttyUSB0 \
         -v "${EASY_PATH}":/docker_data \
         "$FULL_IMAGE_NAME" \
         /bin/bash
    else           
      echo "=========  GPU VERSION  ========="
      docker run -it --shm-size="16g" --gpus=all -d --privileged \
         --name "$CONTAINER_NAME" \
         --network host -p 6000:6000 -p 7000:7000 \
         -v /dev/video7:/dev/video7 --device=/dev/video7 \
         -v /dev/video6:/dev/video6 --device=/dev/video6 \
         -v /dev/video5:/dev/video5 --device=/dev/video5 \
         -v /dev/video4:/dev/video4 --device=/dev/video4 \
         -v /dev/video3:/dev/video3 --device=/dev/video3 \
         -v /dev/video2:/dev/video2 --device=/dev/video2 \
         -v /dev/video1:/dev/video1 --device=/dev/video1 \
         -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix \
         -e DOCKER_IMG="$IMAGE_NAME" \
         -e DOCKER_USER="$USER_NAME" \
         -e DOCKER_USER_ID="$USER_ID" \
         -e DOCKER_GRP="$GRP_NAME" \
         -e DOCKER_GRP_ID="$GRP_ID" \
         -v "${EASY_PATH}":/docker_data \
         "$FULL_IMAGE_NAME" \
         /bin/bash
   fi

   # shellcheck disable=SC2181
   if [ $? -ne 0 ]; then
      echo "Failed to start docker container \"${CONTAINER_NAME}\" based on image: $FULL_IMAGE_NAME"
      exit 1
   fi

   createDockerUser $1

   echo "Finished setting up docker environment."
   echo "Now you can enter with: bash robotaxi_runtime_into.sh"
}

main "$1"
