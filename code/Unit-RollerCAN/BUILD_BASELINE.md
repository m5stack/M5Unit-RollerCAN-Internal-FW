# ROLLERCAN 构建与静态检查基线

## 1. 适用范围

本文档记录改版第 2 步的构建门槛。此阶段只调整构建检查，不修改电机控制、CAN/I2C 协议、引脚、PID 参数和 Bootloader 源码。

## 2. 固定构建环境

- MCU：STM32G431CB
- 编译器：Arm GNU Toolchain 15.2.Rel1（GCC 15.2.1）
- CMake：4.3.1
- Ninja：1.13.2
- 配置：`Debug`、`Release`
- 构建命令：`cmake --preset Debug` / `cmake --build --preset Debug`
- 构建命令：`cmake --preset Release` / `cmake --build --preset Release`
- 产物：`ROLLERCAN.elf`、`ROLLERCAN.bin`、`ROLLERCAN.map`

## 3. 地址与镜像门槛

- Bootloader：`0x08000000..0x08001FFF`，共 8 KiB。
- 应用 FLASH：起始地址 `0x08002000`，长度 `0x1B7FC`（112636 字节）。
- RAM：起始地址 `0x20000000`，长度 32 KiB。
- 最后 4 字节不属于应用链接分区，保持现有工程布局不变。
- Debug：FLASH 镜像不得超过应用分区。
- Release：除不得越界外，至少保留 8192 字节供后续协议和位置模型扩展。

构建后通过 `arm-none-eabi-objdump -h` 读取加载地址，以最高 FLASH 段结束地址减去应用起始地址计算占用。该算法包含段间对齐填充；`.data` 虽在 RAM 中运行，其初始化镜像存放在 FLASH，因此也计入；DWARF 调试段不计入。

## 4. 启动地址核对

- ELF 向量表符号 `g_pfnVectors` 位于 `0x08002000`。
- 应用链接脚本 FLASH 起始地址为 `0x08002000`。
- Bootloader 的 `APPLICATION_ADDRESS` 为 `0x08002000`。
- Bootloader 跳转前执行 `SCB->VTOR = APPLICATION_ADDRESS`，随后设置 MSP 并跳转。

因此经 Bootloader 启动时向量表配置一致。发布固件启用应用 `main.c` 中的 `IAP_Set()`，确保中断向量表使用 `0x08002000` 的应用向量。

## 5. 当前警告基线

以下是旧代码已有警告，均作为后续确定性修复项，本阶段不改源码：

| 模块 | 类型 | 后续处理 |
| --- | --- | --- |
| `myadc.c` | 两条可能返回未初始化值 | 为非法通道提供确定默认值 |
| `u8g2_disp_fun.c` | `snprintf` 参数类型与格式串不匹配；无效局部变量 | 修正定宽类型格式并清理确认无用变量 |
| `button.c`、`tle5012b.c`、`main.c`、`fdcan.c` | 无效局部变量 | 逐项确认后清理 |
| `motordriver.c` | 未使用静态函数 | 确认调用关系后处理 |
| `flash.c` | 多行注释警告 | 在独立格式/警告提交中修正 |

后续要求：改动不得新增未解释的编译警告。警告修复必须与目录迁移、协议扩展分别提交。

## 6. 验收记录

第 2 步全量构建结果：

| 配置 | FLASH | 剩余 FLASH | RAM | 旧警告 |
| --- | ---: | ---: | ---: | ---: |
| Debug | 105400 / 112636 B（93.58%） | 7236 B | 6288 / 32768 B（19.19%） | 37 |
| Release | 93336 / 112636 B（82.87%） | 19300 B | 6272 / 32768 B（19.14%） | 37 |

- Debug 和 Release 均成功生成 `.elf`、`.bin`、`.map`。
- 自动检查输出与链接器 `--print-memory-usage` 的 FLASH 数值一致。
- 使用临时 20000 字节 Release 余量验证时检查按预期失败；正式门槛 8192 字节通过。
- 37 条警告均来自第 1 步冻结的旧源码，本阶段没有新增源码警告。
- Bootloader 不进入本阶段提交。
