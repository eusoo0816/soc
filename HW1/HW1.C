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
#include "xil_io.h"
#include <unistd.h>

#define LIMIT_VALUE 100  // 要寫入的限制值
#define IP_BASE_ADDR XPAR_MYIP_0_S00_AXI_BASEADDR

int main()
{
    // Step 0: 初始化 AXI GPIO 並寫入 LIMIT_VALUE 到 PL
    XGpio Gpio_Limit;
    if (XGpio_Initialize(&Gpio_Limit, XPAR_AXI_GPIO_0_DEVICE_ID) != XST_SUCCESS) {
        printf("GPIO fail！\n");
        return -1;
    }

    XGpio_SetDataDirection(&Gpio_Limit, 1, 0x00);  //output
    XGpio_DiscreteWrite(&Gpio_Limit, 1, LIMIT_VALUE);
    printf("已寫入 GPIO 限制值: %d\n", LIMIT_VALUE);

    // Step 1: write in
    unsigned int count_max = 20;
    printf("寫入 count_max: %u 到 slv_reg0 (offset 0x00)\n", count_max);
    Xil_Out32(IP_BASE_ADDR + 0x00, count_max);

    printf("go!...\n");

    // Step 2: return runing
    unsigned int count_value = 0;
    unsigned int done = 0;

    printf("start\n");
    printf("time %.2f s \n", count_max * 0.335);

    while (done == 0) {
        count_value = Xil_In32(IP_BASE_ADDR + 0x04); // slv_reg1
        done = Xil_In32(IP_BASE_ADDR + 0x08);        // slv_reg2
        printf("[state] count_value: %u, done: %u\n", count_value, done);
        usleep(500000);  // 每 0.5 秒讀一次
    }

    // Step 3: result
    count_value = Xil_In32(IP_BASE_ADDR + 0x04);
    printf("計數完成！\n");
    printf("最終 count_value: %u\n", count_value);
    printf("Done 旗標: %u\n", done);

    printf("done\n");
    while (1); //read
    return 0;
}

