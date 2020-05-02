Tutorial 01 - Bare Minimum
==========================

우리는 여기서 아무것도 하지 않을것입니다. 우리가 빌드한 툴체인을 테스트 할 것입니다. 결과는 아마도 `kernel8.img`가 Raspberry Pi에서 부팅되고 무한루프에서 CPU코어를 중지할것입니다. 여러분은 아래의 명령을 실행하여 확인할 수 있습니다.

```sh
$ qemu-system-aarch64 -M raspi3 -kernel kernel8.img -d in_asm
                         ... 생략 ...
0x0000000000080004:  17ffffff      b #-0x4 (addr 0x80000)
```

시작
-----

아직 환경은 C에 대한 준비가 되어있지 않으므로, 어셈블리에서 작은 프로그램을 구현해야합니다. 이 첫 번째 튜토리얼은 매우 간단하므로 지금까지는 C가 아닙니다.

CPU에는 4개의 코어가 있습니다. 4개의 코어 모두 다 같은 무한 루프를 실행하고 있습니다.

Makefile
--------

우리가 작성한 `Makefile`은 매우 간단합니다. `start.S` 파일이 유일한 소스이기 때문에 `start.S`를 컴파일합니다. 그런 다음 링커 단계에서 `linker.ld` 스크립트를 사용하여 링킹합니다. 마지막으로 ELF 실행 파일을 Raw Image로 변환합니다.

Linker script
-------------

`kernel8.img`가 로드 될 기본 주소를 설정하고 우리가 가지고 있는 유일한 섹션을 넣습니다. AArch64의 경우 로드 주소는 `0x80000`이며, AArch32는 `0x8000`입니다.