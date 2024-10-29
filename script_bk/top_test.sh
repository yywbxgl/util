#!/bin/bash

while true; do
  # 获取当前时间
  timestamp=$(date +"%Y-%m-%d %H:%M:%S")

  # 获取CPU占用最高的5个进程
  top_processes=$(ps -eo pid,%cpu,rss,comm --sort=-%cpu | head -n 11)
  #top_processes=$(ps aux --sort=-%cpu | head -n 11)

  # 将结果写入top.txt文件
  echo "[$timestamp] CPU占用最高的10个进程：" >> ./top.txt
  echo "$top_processes" >> ./top.txt
  echo "" >> ./top.txt

  # 等待一分钟
  sleep 10
done
