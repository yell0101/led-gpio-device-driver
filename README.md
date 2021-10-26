# 과제 준비

## 과제 개요



# 과제 수행 과정
## 1. 준비물 빛 개발환경

- Host PC : Oracle NM VirtualBox / Ubuntu 16.04
- Target Board : Raspberry pi 4 (kernel version : 5.10.63-vl7+)
- raspberry pi kernel source (rpi-5.10.y)


## 4. HelloLed
- sudo insmod hello.ko 를 실행하면 GPIO 18번에 연결된 LED, ACT LED 5번 blink.
- blink: mdelay 함수 이용

## 5. ActPhoto_Module
-  ActPhoto_Module.c를 Makefile로 크로스 컴파일해서  ActPhoto_Module_app.ko를 만듦. 라즈베리파이에 옮김.
- 디바이스 파일 추가. 라즈베리 파이에서 쉘 스크립트 실행 `./ActPhoto_Module_dev.sh`

```bash
#!/bin/bash
module= "ActPhoto_Module"
major="202"

echo `sudo mknod /dev/${module} c ${major} 0` // 디바이스 파일 추가
echo `sudo chmod 777 /dev/${module}` // 디바이스 파일에 권한 추가
echo `ls /dev/${module} -al` // 추가한 디바이스 파일 확인하기

```

-  ActPhoto_Module_app.c를 gcc -o a gpiosw-test.c로 컴파일 해서 실행파일 a를 만듦
- a를 실행시키면, 인터럽트 동작이 일어날 때 BCM 18 번에 연결된 500ms 간격으로 LED on/off 5번 실행

## 7. ActPhoto_sysfs
- 인터럽트가 발생하면 ACT와 GPIO 18번에 연결 된 LED blink 시작. 다시 인터럽트 발생하면 LED blink 끝. 위 과정을 반복
- blink : timer 이용. sysfs 파일 값을 변경하여 period 변경.
- period 수정 방법
```
cd /sys/class/LedBlink2/LedBlink2
sudo su
echo 100 > period
```
