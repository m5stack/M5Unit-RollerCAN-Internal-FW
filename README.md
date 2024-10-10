# M5Unit RollerCAN Internal Firmware

### SKU:U188

**RollerCAN Unit** is a motion execution kit integrated with multiple control functions, specifically designed for efficient motion control. This product supports DC power input of 6-16V (via the `CAN` (XT30) interface) or 5V input (via the Grove interface), automatically adjusting the power coefficient to ensure optimal performance. It features a built-in FOC (Field-Oriented Control) closed-loop drive system, utilizing a 3504 200KV brushless motor, with a maximum continuous phase current of 0.5A and a short-term current of 1A without forced cooling. The driver employs a magnetic encoder for feedback and supports current, speed, and position three-loop control, ensuring precise control. 

The device's axis can optionally include a **slip ring**, allowing the top Grove interface to remain connected to the bottom while supporting 360° rotation, enabling the extension of additional modules on the top while maintaining power and data transmission for the rotating part. 

Additionally, the device is equipped with a 0.66-inch OLED display on the back to provide real-time status updates, along with an RGB indicator light and function buttons for convenient human-computer interaction. The design features LEGO-compatible mounting holes and M3 screw holes at the top and base, facilitating quick assembly and integration. The hardware and software of the RollerCAN Unit are fully open source, supporting motion control and parameter adjustments via `CAN` or `I2C` buses, and providing SWD and SWO debugging interfaces to further enhance flexibility for developers. This product is widely used in robotic joints, motion control, industrial automation, and visual demonstration projects.

## Related Link

See also examples using conventional methods here.

- [Unit RollerCAN & Datasheet](https://docs.m5stack.com/en/unit/Unit-RollerCAN)

## License

- [M5Unit-RollerCAN-MIT](LICENSE)
