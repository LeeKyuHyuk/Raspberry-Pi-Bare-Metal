Tutorial 05 - UART0, PL011
==========================

이 튜토리얼은 `04_mailboxes`와 동일하지만 UART0(PL011)을 사용하여 일련번호를 출력합니다. 따라서 아래와 같이 QEMU로 실행할 수 있습니다.

```sh
$ qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
My serial number is: 0000000000000000
```

uart.h, uart.c
--------------

Rate Divisor 값을 사용하기 전에 PL011에 유효한 클럭 속도를 설정해야 합니다. 이전에 사용한 것과 동일한 Property Channel을 사용하여 Mailbox를 통해 수행됩니다. 그것 말고는 인터페이스는 UART1 인터페이스와 동일합니다.  
구현 방법은 [`uart.c`](https://github.com/LeeKyuHyuk/Raspberry-Pi-Bare-Metal/blob/raspberry-pi-3/05_uart0/uart.c)와 [`BCM2837-ARM-Peripherals-Revised-V2-1.pdf`](https://raw.githubusercontent.com/LeeKyuHyuk/Raspberry-Pi-Bare-Metal/raspberry-pi-3/Documentation/BCM2837-ARM-Peripherals-Revised-V2-1.pdf)의 175~195페이지를 참고해보세요.

Main
----

보드의 일련번호를 요청한 다음 Serial 콘솔에 출력합니다.
