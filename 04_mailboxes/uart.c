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

/* Auxilary mini UART 레지스터 */
#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE+0x00215068))

/**
 * UART 초기화 :
 * 전송속도와 특성(115200 8N1)을 설정하고 GPIO에 매핑
 */
void uart_init()
{
    register unsigned int r;

    /* UART 초기화 */
    *AUX_ENABLE |= 1; // UART1(AUX mini UART) 활성화
    *AUX_MU_CNTL = 0; // Tx, Rx 비활성화
    *AUX_MU_LCR = 3; // 8 bits
    *AUX_MU_MCR = 0;
    *AUX_MU_IER = 0;
    *AUX_MU_IIR = 0xc6; // Interrupt 비활성화
    *AUX_MU_BAUD = 270; // 전송속도를 115200로 설정
    /* UART1를 GPIO핀에 매핑 */
    r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15)); // GPIO14, GPIO15
    r |= (2 << 12) | (2 << 15);    // Alternative 5
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
    *GPPUDCLK0 = 0;   // GPIO 설정 Flush
    *AUX_MU_CNTL = 3; // UART Tx, Rx 활성화
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
    } while (!(*AUX_MU_LSR & 0x20));
    /* 문자를 버퍼에 씁니다 */
    *AUX_MU_IO = c;
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
    } while (!(*AUX_MU_LSR & 0x01));
    /* 버퍼를 읽고 반환합니다 */
    r = (char)(*AUX_MU_IO);
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
