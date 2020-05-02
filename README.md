Bare Metal Programming on Raspberry Pi 3
========================================

안녕하세요! 이 튜토리얼은 Raspberry Pi Bear Metal 응용 프로그램을 컴파일 하려는 사용자를 위해 만들어졌습니다.

대상 독자는 이 하트웨어를 처음 사용하는 취미 OS 개발자입니다. 이 튜토리얼에서는 기본적인 예시들을 제공합니다, 예를 들면 UART을 사용하여 콘솔에 쓰기, 키 입력 읽기, 화면 해상도 설정 및 Frame Buffer을 이용한 선 그리기와 같은 예시를 제공합니다. 또한 하드웨어의 일련번호, 하드웨어를 지원한 난수 및 부팅 파티션에서 파일을 읽는 방법을 알려드릴 것입니다.

이것은 OS 작성 방법에 대한 튜토리얼이 *아닙니다*. 메모리 관리 및 가상 파일 시스템(VFS) 또는 멀티태스킹 구현 방법과 같은 주제는 다루지 않습니다. Raspberry Pi 용 OS를 직접 만들려는 경우에는 계속하기 전에 몇 가지 조사를 하는 것이 좋습니다. 이 튜토리얼은 OS 이론이 아니라 하드웨어와의 인터페이스에 대한 내용들입니다. Raspberry Pi OS에 대해서는 [raspberry-pi-os](https://github.com/s-matyukevich/raspberry-pi-os)를 권장합니다.

왜 Raspberry Pi 3 인가요?
-------------------

여러 가지 이유로 이 보드를 선택했습니다. 첫 번째로 싸고 쉽게 얻을 수 있습니다. 두 번째로 64비트 시스템입니다. 필자는 오래전에 32비트 프로그래밍을 포기했습니다. 주소 공간이 엄청나게 커서 64비트는 훨씬 더 흥미롭습니다. 이로 인해 흥미로운 새로운 솔루션을 사용할 수 있습니다. 세 번째로 쉽게 프로그래밍 할 수 있는 MMIO(Memory-mapped I/O)만 사용합니다.

32비트 튜토리얼의 경우 다음을 권장합니다:

- [Cambridge tutorials](http://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/) (ASM 및 32 bit 만)

- [David Welch's tutorials](https://github.com/dwelch67/raspberrypi) (주로 C, 약간의 64 bit 예제를 포함)

- [Peter Lemon's tutorials](https://github.com/PeterLemon/RaspberryPi) (ASM 만, 64 bit)

- [Leon de Boer's tutorials](https://github.com/LdB-ECM/Raspberry-Pi) (C와 ASM, 64 bit, USB 및 OpenGL과 같은 더 복잡한 예시)

왜 C++가 아닌가요?
------------

C언어의 "freestanding" 모드를 사용하면 하드웨어에 직접적으로 접근하여 개발할 수 있습니다. C++에서는 런타임 라이브러리가 필요하므로 불가능합니다. 만약 관심이 있다면 [Circle C++](https://github.com/rsta2/circle) 라이브러리를 살펴보세요. 필수 C++ 런타임을 포함할 뿐만 아니라, 이 튜토리얼에서 논의할 모든 Raspberry Pi 기능도 구현합니다.

준비물
-------------

시작하기 전에 크로스 컴파일러 (자세한 내용은 `00_crosscompiler` 디렉토리를 참조하세요)와 [펌웨어 파일](https://github.com/raspberrypi/firmware/tree/master/boot)이 있는 FAT 파일시스템의 Micro SD카드가 필요합니다.

[Micro SD카드 USB 어댑터](http://media.kingston.com/images/products/prodReader-FCR-MRG2-img.jpg)를 구입하는 것이 좋습니다. (많은 제조업체에서 이러한 어댑터와 함께 SD카드를 배송합니다), USB 메모리와 같이 컴퓨터에 SD카드를 연결할 수 있으며, 특별한 카드 리더 인터페이스가 필요하지 않습니다.

SD카드에 FAT32 파티션을 만들고(MBR 유형), 포맷을 한 후 `bootcode.bin`, `start.elf` 및 `fixup.dat`를 복사할 수 있습니다. 또는 Raspbian 이미지를 다운로드하여 SD카드에 `dd`하고 마운트 한 다음 불필요한 `.img` 파일을 삭제할 수 있습니다. 여러분이 편한 것으로 진행하면 됩니다. 중요한 것은 이 튜토리얼을 통하여 `kernel8.img`를 만들고 SD카드의 루트 디렉토리에 복사해야 하며 다른 `.img` 파일은 없어야 한다는 것입니다.

또한 [USB Serial Debug Cable](https://www.adafruit.com/product/954)를 구입하는 것이 좋습니다. GPIO 핀 14/15에 연결한 뒤, 컴퓨터에서 다음과 같이 minicom을 실행합니다:

```sh
minicom -b 115200 -D /dev/ttyUSB0
```

가상화
---------

아쉽게도 QEMU 바이너리는 아직 Raspberry Pi 3를 지원하지 않습니다. 하지만 좋은 소식은 이를 구현하였으며 곧 업데이트될 예정입니다. (업데이트: [QEMU 2.12](https://wiki.qemu.org/ChangeLog/2.12#ARM)에서 가능합니다). 그때까지는 최신 소스에서 QEMU를 컴파일해야 합니다. 일단 컴파일 되면 다음과 같이 사용할 수 있습니다.

```sh
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
```

또는 (파일 시스템 튜토리얼에서 사용)

```sh
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -drive file=$(이미지파일),if=sd,format=raw -serial stdio
```

**-M raspi3**
첫 번째 인수는 QEMU에게 Raspberry Pi 3 하드웨어를 가상화하도록 지시합니다.

**-kernel kernel8.img**
두 번째는 커널 파일 이름을 알려줍니다.

**-drive file=$(yourimagefile),if=sd,format=raw**
두 번째의 경우, 이 인수는 SD카드 이미지를 알려줍니다. 이것은 Raspbian 이미지 일 수도 있습니다.

**-serial stdio**

**-serial null -serial stdio**

마지막 인수는 가상화된 UART0를 QEMU를 실행하는 터미널의 표준 입/출력으로 리다이렉션하여 시리얼로 전송된 모든 것이 표시되고, 터미널에 입력된 모든 키가 VM(Virtual Machine)에 의해 수신됩니다. UART1은 기본적으로 리다이렉션되지 않으므로 튜토리얼 05 이상에서만 작동합니다. 이를 위해서는 다음과 같은 추가하거나 `-chardev socket,host=localhost,port=1111,id=aux -serial chardev:aux` ([@godmar](https://github.com/godmar)님 알려주셔서 감사합니다), 또는 간단하게 두 개의 `-serial` 인수를 사용하면 됩니다 ([@cirosantilli](https://github.com/cirosantilli)님 감사합니다).

**!!!주의!!!** QEMU 가상화는 기초적이며 가장 일반적인 주변 장치만 가상화됩니다! **!!!주의!!!**

하드웨어에 대하여
------------------

인터넷에는 Raspberry Pi 3 하드웨어에 대해 자세히 설명하는 많은 페이지가 있으므로 간략하게 기본 사항만 다룰 것입니다.

보드에는 [BCM2837 SoC](https://github.com/raspberrypi/documentation/tree/master/hardware/raspberrypi/bcm2837)가 함께 제공됩니다.
BCM2837에는 다음과 같은 것을 포함하고 있습니다:

 - VideoCore GPU
 - ARM-Cortex-A53 CPU (ARMv8)
 - 일부 MMIO 매핑 주변 장치.

흥미롭게도 CPU는 보드의 메인 프로세서가 아닙니다. 전원이 켜지면 첫 번째 GPU가 실행됩니다. `bootcode.bin`에서 코드를 실행하여 초기화가 완료되면, `start.elf`을 실행합니다. `start.elf`는 ARM 실행 파일은 아니지만 GPU 용으로 컴파일 되었습니다. 우리가 관심을 갖는 것은 `kernel`로 시작하고 `.img`로 끝나는 ARM 실행 파일을 찾는 것입니다. Aarch64 모드에서 CPU를 프로그래밍 할 때는 `kernel8.img`만 있으면 됩니다. 일단 로드되면 GPU는 ARM 프로세서에 리셋 라인을 트리거 하여 주소 `0x80000`(정확하게는 `0`이지만, GPU는 먼저 [ARM 초기화 및 점프 코드](https://github.com/raspberrypi/tools/blob/master/armstubs/armstub8.S)를 넣습니다)에서 코드 실행을 시작합니다.

RAM(Raspberry Pi3의 경우 1GB)은 CPU와 GPU 간에 공유됩니다. 이 의미는 각자 메모리에 쓴 것을 읽을 수 있음을 의미합니다. 혼동을 피하기 위해 [Mailbox 인터페이스](https://github.com/raspberrypi/firmware/wiki/Mailboxes)를 정의하였습니다. CPU는 Mailbox에 메시지를 쓰고 GPU에게 메시지를 읽도록 지시합니다. GPU(메시지는 메모리에 있음을 알고 있어야 합니다)는 메시지를 해석하고 동일한 주소에 응답 메시지를 배치합니다. CPU는 GPU가 완료된 시점을 알기 위해 메모리를 폴링 해야 하며, 이를 통해 응답을 읽을 수 있습니다.

마찬가지로 모든 주변 장치는 메모리와 CPU를 통해 통신합니다. 각각의 메모리 주소는 `0x3F000000`부터 시작하지만 실제 RAM(Memory Mapped IO라고 불립니다)에는 없습니다. 주변 장치용 Mailbox는 없으며 각 장치마다 고유 한 프로토콜이 있습니다. 이 장치의 일반적인 메모리는 4바이트로 정렬된 주소(Word라고 합니다)에서 32비트 단위로 메모리를 읽고 써야 하며, 각 장치에는 제어/상태 및 데이터 Word가 있습니다. 아쉽게도 Broadcom(SoC 제조업체)는 자사 제품을 문서화하는데 있어 최악입니다. 우리가 가진 BCM2835 문서가 제일 좋은 문서이므로 가까이합시다.

CPU에는 가상 주소 공간을 만들 수 있는 메모리 관리 장치가 있습니다. 이는 특정 CPU 레지스터로 프로그래밍 할 수 있으며 이러한 MMIO 주소를 가상 주소 공간에 매핑할 때는 주의해야 합니다.

우리가 흥미롭게 보는 MMIO 주소 중 일부는 다음과 같습니다:
```
0x3F003000 - System Timer
0x3F00B000 - Interrupt controller
0x3F00B880 - VideoCore mailbox
0x3F100000 - Power management
0x3F104000 - Random Number Generator
0x3F200000 - General Purpose IO controller
0x3F201000 - UART0 (serial port, PL011)
0x3F215000 - UART1 (serial port, AUX mini UART)
0x3F300000 - External Mass Media Controller (SD card reader)
0x3F980000 - Universal Serial Bus controller
```
더 자세한 내용은, Raspberry Pi Firmware wiki 및 Github에 있는 설명서를 참조하세요.

https://github.com/raspberrypi

이 문서를 통하여 즐거운 Raspberry Pi 프로그래밍이 되길! :-)
