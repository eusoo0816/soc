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

#include "xgpio.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"

// 對應你在 system.h / xparameters.h 裡的設定
#define GPIO_DEVICE_ID      XPAR_AXI_GPIO_0_DEVICE_ID
#define INTC_DEVICE_ID      XPAR_SCUGIC_SINGLE_DEVICE_ID
#define GPIO_INTERRUPT_ID   XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR

#define BTN_CHANNEL         1  // channel 1 → 按鈕（輸入）
#define LED_CHANNEL         2  // channel 2 → LED（輸出）

XGpio Gpio;
XScuGic Intc;

volatile int interrupt_flag = 0;

// 中斷服務函數（ISR）
void GpioIsr(void *InstancePtr) {
    // 清除中斷旗標
    XGpio_InterruptClear(&Gpio, BTN_CHANNEL);

    // 記錄事件（也可直接點亮 LED）
    interrupt_flag = 1;

    xil_printf(">> Button Interrupt Triggered!\n");
}

int main() {
    int status;

    xil_printf(">>> Start GPIO Interrupt Test <<<\n");

    // 初始化 GPIO
    status = XGpio_Initialize(&Gpio, GPIO_DEVICE_ID);
    if (status != XST_SUCCESS) {
        xil_printf("GPIO Init Failed\n");
        return XST_FAILURE;
    }

    // Channel 1 為輸入（按鈕）
    XGpio_SetDataDirection(&Gpio, BTN_CHANNEL, 0xFFFFFFFF);
    // Channel 2 為輸出（LED）
    XGpio_SetDataDirection(&Gpio, LED_CHANNEL, 0x00000000);

    // 開啟中斷
    XGpio_InterruptEnable(&Gpio, BTN_CHANNEL);
    XGpio_InterruptGlobalEnable(&Gpio);

    // 初始化中斷控制器
    XScuGic_Config *IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
    XScuGic_CfgInitialize(&Intc, IntcConfig, IntcConfig->CpuBaseAddress);

    // 註冊中斷處理函式
    XScuGic_Connect(&Intc, GPIO_INTERRUPT_ID, (Xil_ExceptionHandler)GpioIsr, &Gpio);
    XScuGic_Enable(&Intc, GPIO_INTERRUPT_ID);

    // 啟用 CPU 端中斷
    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                                 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
                                 &Intc);
    Xil_ExceptionEnable();

    xil_printf("System initialized, waiting for button...\n");

    while (1) {
        if (interrupt_flag) {
            interrupt_flag = 0;

            // LED 全亮
            XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0xFF);
            xil_printf("LEDs ON\n");

            // 延遲後關閉 LED
            for (volatile int i = 0; i < 900000000; i++); // 簡單延遲
            XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x00);
            xil_printf("LEDs OFF\n");
        }
    }
}
