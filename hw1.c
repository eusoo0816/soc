/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "xparameters.h"
#include "xgpio.h"

// 簡單延遲（小延遲單位，模擬 PWM 頻率）
void short_delay() {
    volatile int i;
    for (i = 0; i < 800; i++) {
        ;
    }
}

int main()
{
    XGpio LED_XGpio;
    int brightness = 0;     // 亮度 0~100（占空比百分比）
    int direction = 1;      // 變亮(+1) 或變暗(-1)
    int i;

    XGpio_Initialize(&LED_XGpio, XPAR_AXI_GPIO_0_DEVICE_ID);
    XGpio_SetDataDirection(&LED_XGpio, 1, 0);

    printf("Start Software PWM Breathing LED...\n");

    while (1) {
        // 一次 PWM cycle 模擬：100 steps
        for (i = 0; i < 300; i++) {
            if (i < brightness)
                XGpio_DiscreteWrite(&LED_XGpio, 1, 0b00000001); // LED ON
            else
                XGpio_DiscreteWrite(&LED_XGpio, 1, 0b00000000); // LED OFF

            short_delay(); // 控制頻率
        }

        // 調整亮度
        brightness += direction;

        // 到最大或最小亮度就反轉
        if (brightness >= 300 || brightness <= 0) {
            direction = -direction;
        }
    }

    return 0;
}




