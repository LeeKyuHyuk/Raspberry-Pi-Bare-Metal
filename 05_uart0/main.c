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

#include "uart.h"
#include "mbox.h"

void main()
{
    // UART 장치 초기화
    uart_init();

    // 보드의 고유한 Serial Number를 Mailbox 호출로 얻습니다.
    mbox[0] = 8 * 4;        // Message의 길이
    mbox[1] = MBOX_REQUEST; // Request message임을 표기합니다.

    mbox[2] = MBOX_TAG_GETSERIAL; // Get Serial Number Command
    mbox[3] = 8;                  // Buffer Size
    mbox[4] = 8;
    // Output Buffer(mbox[5], mbox[6])를 초기화 합니다.
    mbox[5] = 0;
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;

    // Message를 GPU로 보내고, 응답을 받습니다.
    if (mbox_call(MBOX_CH_PROP))
    {
        uart_puts("My serial number is: ");
        uart_hex(mbox[6]);
        uart_hex(mbox[5]);
        uart_puts("\n");
    }
    else
    {
        uart_puts("Unable to query serial!\n");
    }

    // 모든 것을 다시 출력(Echo)
    while (1)
    {
        uart_send(uart_getc());
    }
}
