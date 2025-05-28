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
    {    
    // Step 1: Write count_max to slv_reg0
    unsigned int count_max = 20;
    printf("Writing count_max: %u to slv_reg0 (offset 0x00)\n", count_max);
    Xil_Out32(IP_BASE_ADDR, count_max);
    printf("Write completed. Please press the button to start counting...\n");

    // Step 2: Poll slv_reg1 (count_value) and slv_reg2 (done)
    unsigned int count_value = 0;
    unsigned int done = 0;
    printf("Starting to poll counter status...\n");
    // [MODIFIED] Updated expected counting time for count_max = 10
    printf("Expected counting time: ~3.35 seconds (10 counts, ~335ms per count)\n");

    while (done == 0) {
        count_value = Xil_In32(IP_BASE_ADDR + 0x04); // Read slv_reg1
        done = Xil_In32(IP_BASE_ADDR + 0x08);       // Read slv_reg2
        printf("[Status] count_value: %u, done: %u\n", count_value, done);
        usleep(500000); // 500ms delay to match ~335ms counting period
    }

    // Step 3: Verify final state
    count_value = Xil_In32(IP_BASE_ADDR + 0x04);
    printf("Counting completed!\n");
    printf("Final count_value (should be 0): %u\n", count_value);
    printf("Done flag (should be 1): %u\n", done);

    printf("=== Test Finished ===\n");
    while (1);
    return 0;
    }
    XGpio Gpio_Limit;

    XGpio_Initialize(&Gpio_Limit, XPAR_AXI_GPIO_0_DEVICE_ID);

    // 設定 GPIO 為輸出方向（由 PS 輸出數值）
    XGpio_SetDataDirection(&Gpio_Limit, 1, 0x00);  // Channel 1, 全部為輸出

    // 寫入數值到 PL 模組 (HW1) 的 i_limit
    XGpio_DiscreteWrite(&Gpio_Limit, 1, LIMIT_VALUE);

    printf("已寫入限制值: %d\n", LIMIT_VALUE);

    return 0;
}

