#!/bin/bash
module="device_led"
major="202"

echo `sudo mknod /dev/${module} c ${major} 0` #디바이스 파일 추가
echo `sudo chmod 777 /dev/${module}` #디바이스 파일에 권한 추가
echo `ls /dev/${module} -al` #추가한 디바이스 파일 확인하기
