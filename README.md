# M5Unit RollerCAN Internal Firmware

### SKU:U188

**RollerCAN Unit** is a motion execution kit integrated with multiple control functions, specifically designed for efficient motion control. This product supports DC power input of 6-16V (via the `CAN` (XT30) interface) or 5V input (via the Grove interface), automatically adjusting the power coefficient to ensure optimal performance. It features a built-in FOC (Field-Oriented Control) closed-loop drive system, utilizing a 3504 200KV brushless motor, with a maximum continuous phase current of 0.5A and a short-term current of 1A without forced cooling. The driver employs a magnetic encoder for feedback and supports current, speed, and position three-loop control, ensuring precise control. 

The device's axis can optionally include a **slip ring**, allowing the top Grove interface to remain connected to the bottom while supporting 360° rotation, enabling the extension of additional modules on the top while maintaining power and data transmission for the rotating part. 

Additionally, the device is equipped with a 0.66-inch OLED display on the back to provide real-time status updates, along with an RGB indicator light and function buttons for convenient human-computer interaction. The design features LEGO-compatible mounting holes and M3 screw holes at the top and base, facilitating quick assembly and integration. The hardware and software of the RollerCAN Unit are fully open source, supporting motion control and parameter adjustments via `CAN` or `I2C` buses, and providing SWD and SWO debugging interfaces to further enhance flexibility for developers. This product is widely used in robotic joints, motion control, industrial automation, and visual demonstration projects.

## Related Link

See also examples using conventional methods here.

- [Unit RollerCAN & Datasheet](https://docs.m5stack.com/en/unit/Unit-RollerCAN)

## Related Project

This project references the following open source projects.

- [smartknob](https://github.com/scottbez1/smartknob)
- [PID_Controller](https://github.com/tcleg/PID_Controller)
- [u8g2](https://github.com/olikraus/u8g2)

## Build

Install CMake, Ninja, and Arm GNU Toolchain, then add their executable
directories to `PATH`. As an alternative, set `ARM_GNU_TOOLCHAIN_PATH` to the
Arm GNU Toolchain installation directory or its `bin` directory.

Run the following commands from `code/Unit-RollerCAN` or
`code/Unit-RollerCAN-Bootloader`:

```shell
cmake --preset Debug
cmake --build --preset Debug
```

For J-Link flashing/debugging, install the SEGGER software normally or set
`JLINK_PATH`/`JLINK_HOME` to its installation directory. The application also
accepts `-DJLINK_COMMANDER=<executable>` as a local CMake override. Keep such
machine-specific values in the ignored `CMakeUserPresets.json`, not in tracked
project files.

VS Code Cortex-Debug additionally requires `arm-none-eabi-gdb` and
`JLinkGDBServerCL.exe` on `PATH`. Alternatively, configure the user-level
`cortex-debug.armToolchainPath.windows` and
`cortex-debug.JLinkGDBServerPath.windows` settings.

## Version Changes: V2 -> V3

- Migrated the application and bootloader from MDK-ARM to CMake with unified Debug/Release, J-Link, `.bin`, and `.hex` workflows.
- Reorganized the source tree, standardized formatting, and removed legacy Keil project files and generated artifacts.
- Extended the I2C and CAN protocols with position readback, UID, product ID, firmware information, parameter readback, calibration, and protection functions.
- Fixed control and communication issues involving encoder configuration, CAN response handling, PID limiting, calibration persistence, and I2C compatibility.
- Improved Bootloader/IAP update support and release packaging. The current RollerCAN application firmware is V3.

## License

- [smartknob][] Copyright (c) 2022 Scott Bezek and licensed under Apache License, Version 2.0 License.
- [PID_Controller][] Copyright (c) 2013-2014 tcleg and licensed under GPLv3 License.
- [u8g2][] Copyright (c) 2016 olikraus and licensed under BSD License.

[smartknob]: https://github.com/scottbez1/smartknob
[PID_Controller]: https://github.com/tcleg/PID_Controller
[u8g2]: https://github.com/olikraus/u8g2
