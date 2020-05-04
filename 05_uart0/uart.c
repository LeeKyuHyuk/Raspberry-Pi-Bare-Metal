/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "gpio.h"
#include "mbox.h"

/* PL011 UART 레지스터 */
#define UART0_DR        ((volatile unsigned int*)(MMIO_BASE+0x00201000))
#define UART0_FR        ((volatile unsigned int*)(MMIO_BASE+0x00201018))
#define UART0_IBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201024))
#define UART0_FBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201028))
#define UART0_LCRH      ((volatile unsigned int*)(MMIO_BASE+0x0020102C))
#define UART0_CR        ((volatile unsigned int*)(MMIO_BASE+0x00201030))
#define UART0_IMSC      ((volatile unsigned int*)(MMIO_BASE+0x00201038))
#define UART0_ICR       ((volatile unsigned int*)(MMIO_BASE+0x00201044))

/**
 * UART 초기화 :
 * 전송속도와 특성(115200 8N1)을 설정하고 GPIO에 매핑
 */
void uart_init()
{
    register unsigned int r;

    /* UART 초기화 */
    *UART0_CR = 0; // UART0를 끕니다

    /* 이관된 Divisor 값을 위해 UART Clock을 설정합니다 */
    mbox[0] = 9 * 4;
    mbox[1] = MBOX_REQUEST;
    mbox[2] = MBOX_TAG_SETCLKRATE; // Clock 속도를 설정
    mbox[3] = 12;
    mbox[4] = 8;
    mbox[5] = 2;       // UART Clock
    mbox[6] = 4000000; // 4Mhz
    mbox[7] = 0;       // Turbo를 0으로 설정합니다
    mbox[8] = MBOX_TAG_LAST;
    mbox_call(MBOX_CH_PROP);

    /* UART0를 GPIO핀에 매핑 */
    r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15)); // GPIO14, GPIO15
    r |= (4 << 12) | (4 << 15);    // Alternative 0
    *GPFSEL1 = r;
    *GPPUD = 0; // GPIO 14, 15핀 활성화
    r = 150;
    while (r--)
    {
        asm volatile("nop");
    }
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    r = 150;
    while (r--)
    {
        asm volatile("nop");
    }
    *GPPUDCLK0 = 0; // GPIO 설정 Flush

    *UART0_ICR = 0x7FF; // Interrupt를 Clear합니다
    *UART0_IBRD = 2;    // Baudrate를 115200로 설정합니다
    *UART0_FBRD = 0xB;
    *UART0_LCRH = 0b11 << 5; // 8n1로 설정합니다.
    *UART0_CR = 0x301;       // Tx, Rx, FIFO를 활성화 합니다.
}

/**
 * 문자 보내기
 */
void uart_send(unsigned int c)
{
    /* 문자를 보낼 수 있을 때까지 기다립니다 */
    do
    {
        asm volatile("nop");
    } while (*UART0_FR & 0x20);
    /* 문자를 버퍼에 씁니다 */
    *UART0_DR = c;
}

/**
 * 문자 받기
 */
char uart_getc()
{
    char r;
    /* 버퍼에 무언가가 올 때까지 기다립니다 */
    do
    {
        asm volatile("nop");
    } while (*UART0_FR & 0x10);
    /* 버퍼를 읽고 반환합니다 */
    r = (char)(*UART0_DR);
    /* 캐리지 리턴을 개행 문자로 변환합니다 */
    return r == '\r' ? '\n' : r;
}

/**
 * 문자열 출력
 */
void uart_puts(char *s)
{
    while (*s)
    {
        /* 개행 문자를 캐리지 리턴 + 개행으로 변환 */
        if (*s == '\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

/**
 * 2진수 값을 16진수 형식으로 출력
 */
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
