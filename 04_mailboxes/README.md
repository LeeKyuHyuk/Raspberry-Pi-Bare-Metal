Tutorial 04 - Mailboxes
=======================

UART0(PL011)을 구현하려면 Mailbox가 필요합니다. 이 튜토리얼에서는 Mailbox 인터페이스를 소개합니다.
Mailbox를 사용하여 보드의 일련번호를 요청하고 UART1(Auxilary mini UART)에 출력합니다.

참고: QEMU는 기본적으로 UART1을 터미널로 리다이렉션하지 않고 UART0만 리다이렉션 하므로, `-serial null -serial stdio`를 사용해야 합니다.

uart.h, uart.c
--------------

`uart_hex(d)` 2진수 값을 16진수 형식으로 출력합니다.

```c
void uart_hex(unsigned int d)
{
    unsigned int n;
    int c;
    for (c = 28; c >= 0; c -= 4)
    {
        n = (d >> c) & 0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n += n > 9 ? 0x37 : 0x30;
        uart_send(n);
    }
}
```

1. 인수로 `unsiged int`의 `d`를 받습니다. 만약 `0xD`를 `d`로 받는다면 `0x0000000D`가 `d`에 저장됩니다.
2. `d`를 `c`번 Right Shift한 뒤, `0xF`와 AND 비트연산을 하고 그 값을 `n`에 저장합니다.
3. `n`이 9보다 크면, '`A`'~'`F`'로 변환합니다. 변환하는 방법은 `n`에 `0x37`를 더합니다. 만약 9보다 작거나 같다면 `n`에 `0x30`을 더합니다.

위의 방법에서 `n`에 `0x30`이나 `0x37`를 더하는 게 이해하기 힘들 수 있습니다.  
아래의 `ASCII Table`를 보면 이해가 빠를 것입니다.

**< ASCII Table >**

| Dec | Hex | Char | Raw encoding |
|-----|-----|------|--------------|
| 48 | `0030` | 0 | `0x30` |
| 49 | `0031` | 1 | `0x31` |
| 50 | `0032` | 2 | `0x32` |
| 51 | `0033` | 3 | `0x33` |
| 52 | `0034` | 4 | `0x34` |
| 53 | `0035` | 5 | `0x35` |
| 54 | `0036` | 6 | `0x36` |
| 55 | `0037` | 7 | `0x37` |
| 56 | `0038` | 8 | `0x38` |
| 57 | `0039` | 9 | `0x39` |
| 58 | `003a` | : | `0x3A` |
| 59 | `003b` | ; | `0x3B` |
| 60 | `003c` | < | `0x3C` |
| 61 | `003d` | = | `0x3D` |
| 62 | `003e` | > | `0x3E` |
| 63 | `003f` | ? | `0x3F` |
| 64 | `0040` | @ | `0x40` |
| 65 | `0041` | A | `0x41` |
| 66 | `0042` | B | `0x42` |
| 67 | `0043` | C | `0x43` |
| 68 | `0044` | D | `0x44` |
| 69 | `0045` | D | `0x45` |
| 70 | `0046` | F | `0x46` |

만약 `n`이 `13`이라면, 위의 ASCII Table에서 `0x37`에서 아래로 13번 내려가면 Char가 'D'입니다. 이는 `13`의 16진수인 `0xD`와 같습니다.  
이 값을 `uart_send()`를 사용하여 출력되게 하는 원리입니다.

mbox.h, mbox.c
--------------

`mbox` 배열에 메시지를 채운 다음, Mailbox 채널을 지정한 뒤 `mbox_call(ch)`를 호출하여 GPU로 전달합니다.

Mailbox의 일반적인 절차:

- Mailbox에서 Read 할 때 :
    1. Empty flag가 설정되지 않을 때까지 Mailbox Status Register를 읽습니다.
    2. Mailbox Read Register에서 데이터를 읽어옵니다.
    3. 하위 4비트가 원하는 채널 번호와 일치하지 않으면 ①에서 다시 반복합니다.
    4. 상위 28 비트는 반환된 데이터입니다.

- Mailbox에 Write 할 때 :
    1. Full flag가 설정되지 않을 때까지 Mailbox Status Register를 읽습니다.
    2. 채널(하위 4비트)과 데이터(상위 28비트)가 합쳐진 것을 Mailbox Write Register에 기록합니다.

이 예시에서는 [Property Channel](https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface)을 사용하였습니다,
메시지 형식은 다음과 같습니다:

```
 0. 바이트 단위의 메시지 크기, (x+1)*4
 1. MBOX_REQUEST magic value, 요청 메시지를 나타냅니다
 2-x. 태그(Tags)
 x+1. MBOX_TAG_LAST magic value, 더 이상 태그가 없음을 나타냅니다
```

각 태그의 모양:

```
 n+0. 태그 식별자
 n+1. 바이트 단위의 버퍼 크기 값
 n+2. 무조건 0이어야 합니다.
 n+3. (선택적인) 버퍼의 값
```

Main
----

보드의 일련번호를 요청한 다음 Serial 콘솔에 출력합니다.
