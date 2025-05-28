# 使用 SDK 的 C 程式與 VHDL 燒錄 FPGA 板子的流程

本文件說明如何使用 **Xilinx Vivado + SDK（或 Vitis）**，透過 VHDL + C 程式設計並燒錄到 FPGA 開發板。

---

## 🛠️ 工具準備

- Vivado Design Suite（含 SDK 或 Vitis）
- FPGA 開發板（如 Basys 3, Nexys A7）
- USB-JTAG 下載線
- VHDL 或 Verilog 硬體描述語言程式
- C 語言應用程式（搭配 Soft processor 或 Zynq）

---

## 🔄 開發流程總覽


---

## 🔧 Step 1：撰寫 VHDL 並建立硬體專案（Vivado）

1. 建立 Vivado 專案。
2. 撰寫 VHDL 模組（如 LED 控制器、計數器等）。
3. 如需處理器，可建立 Block Design，加入 MicroBlaze 或 Zynq。
4. 設定 IO Constraints（`.xdc`）。
5. 執行合成與實作，產生 `.bit` 檔案。

---

## 🔁 Step 2：產生硬體平台檔案

1. `File > Export > Export Hardware`
   - 選擇 **Include Bitstream**
   - 匯出為 `.hdf`（Vivado 2019.2 以下）或 `.xsa`（Vitis）

---

## 💻 Step 3：撰寫 C 程式（Xilinx SDK 或 Vitis）

1. 開啟 SDK 或 Vitis，匯入 `.hdf` / `.xsa`。
2. 建立新的應用程式專案。
3. 撰寫 C 程式控制硬體模組，例如控制 GPIO：

```c
#include "xgpio.h"
#include "xparameters.h"

int main() {
    XGpio led;
    XGpio_Initialize(&led, XPAR_GPIO_0_DEVICE_ID);
    XGpio_SetDataDirection(&led, 1, 0x0);  // Output

    while(1) {
        XGpio_DiscreteWrite(&led, 1, 0xFF); // 所有 LED 亮
    }
    return 0;
}
 Step 4：下載至 FPGA 板子
回到 Vivado：

Open Hardware Manager > Program Device 選擇 .bit 燒錄。

在 SDK/Vitis 中：

Run As > Launch on Hardware 將 .elf 執行在 FPGA 上。

✅ 成果
成功執行後：

VHDL 控制硬體邏輯（如計數、輸出 LED）

C 程式控制輸入輸出（如按鈕、動態調整）

