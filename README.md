<div align="center">
  <h1>Retro Lite CM5: RK3588s Gaming Handheld</h1>
</div>

<div align="center">
  <img src="https://i.imgur.com/v9KPeW5.png" alt="Image of Retro Lite CM5">
</div>

<div align="center">
  <img src="https://i.imgur.com/daHOwZD.png" alt="Image of Retro Lite CM5">
</div>

The Retro Lite CM5 is an upgrade to the Retro Lite CM4 handheld. The project goal is to create a more powerful handheld that can handle 3DS, PS2 and GC emulation (none of which the Pi can do). I also would like to be able to play some games via Box64/86. It has a relatively large battery so run time should be excellent, considering it is ARM based. 

Powered by the RK3588s compute module from Radxa, which is an SBC SOM module with an ARMv8 64-bit processor. Features 8 cores/threads - 4 x ARM Cortex-A55/A76 cores with A55 cores clocked at 1.8GHz, and A76 cores clocked at 2.4GHz. It also has a built-in Mali G610 GPU. 

The Retro Lite CM5 offers a more modern gaming experience with DisplayPort via USB Type-C, USB 3.0 & HDMI 2.1. Built-in PCIe WiFI 6.0 & BT 5.2, as well as a new 720p IPS display @ 5.5" with touchscreen support & dual tact trigger buttons for GameCube support, make this a much more viable low-powered ARM SBC gaming PC than the Raspberry Pi 4. 

The operating system will be based on Armbian, with a custom device tree for hardware interfacing & RetroPie pre-installed. However, the user can use it however they would like, either in a Linux desktop environment or with other Linux based frontends, such as ES-DE. This handheld should be able to play the entire PS2 library at least 1x native resolution for all games, as well as GameCube at 1x on Linux with OpenGLES drivers. Android is currently more performant due to Vulkan drivers, however Linux Vulkan blobs should come soon (or someday, who knows). We also now have succesfully compiled ROCKNIX as well, so that will be an option to use as well. 

For regular updates/following progress on the build, please check out the forum post here:
https://bitbuilt.net/forums/index.php?threads/retro-lite-cm5.5815/

# Retro Lite CM5: Radxa RK3588s Handheld 

## Features

Hardware specifications:

- 3D printable housing in PLA. Comfort grips for added ergonomics
- Radxa CM5 Compute Module (8GB RAM/64GB eMMC)
- SoC – Rockchip RK3588S octa-core processor with 4x Cortex‑A76 cores @ up to 2.4GHz, 4x Cortex‑A55 core @ 1.8GHz
Arm Mali-G610 MP4 “Odin” GPU Video decoder – 8Kp60 H.265, VP9, AVS2, 8Kp30 H.264 AVC/MVC, 4Kp60 AV1, 1080p60 MPEG-2/-1, VC-1, VP8 Video encoder – 8Kp30 H.265/H.264 video encoder
- WiFi 6/Bluetooth 5.2 via PCIe E-key slot (Intel AX210) - https://www.intel.com/content/www/us/en/products/sku/204836/intel-wifi-6e-ax210-gig/specifications.html
- 6 layer carrier board with 3 B2B mezzanine connectors to interface with any Radxa CM5 module 
- 5v boost rated at 3.5A continuous current
- RP2040 gamepad - complete with SDL mappings and evdev gyro support/mouse control via MPU6050
- 1280 x 720 (5.5" DSI IPS LCD): DSI video output on internal display
- Up to 4k HDMI video output via HDMI output
- Up to 4k DisplayPort Alternate Mode via Type-C
- USB 2.0/3.1 capable USB-C data transfer
- USB-C dual role port functionality (sink/source)
- Brightness and volume HUD adjustment. Brightness is adjusted by holding plus hotkey + down/up. Volume controlled either by volume buttons or plus hotkey + left right DPAD
- USB-C PD charging support via sink profiles supporting 5V/3A, 9V/3A, 12V/2A & 15V/2.6A (switch charger) via TPS65987D PD controller (see binary in TPS65987D folder). Recommended to use <12V for best charge and play performance 
- Stereo Audio Output via i2s. Dual stereo speakers with ported chambers
- Headphone jack, with automatic switching
- 5000mAh lipo, providing around 1.5-5 hours of gameplay depending on load (to be upgraded soon to >8000mAh)
- Dual stacked shoulder buttons (L, R, LZ, RZ) with dual tact buttons for GameCube functionality (LR analog/LR digital)
- 2x hall effect analog sticks running at 3v3
- Resin casted ABXY, DPAD, start+select, shoulder buttons
- Silicone membranes for nice button feel 

Software specifications:

- Armbian GNOME desktop (Kernel 6.1.57 as of this post)
- Android 13 support (beta)
- Full upstream Rocknix support
- Wayland windowing system (xorg capable)
- Hardware graphics support via Panfrost/OpenGLES (no Vulkan support… for now)
- Safe software/hardware shutdown (either from software or via button)
- Low power sleep mode - in my tests, 24 hours in standby mode draws around 6% of total battery life
- Range adjustable joysticks in software by pressing plus hotkey plus R3 --> A --> rotate sticks --> A combination
-------------------------------------------------------------------------------------------------------

### CAD
Enclosure files have been uploaded in STL and STEP format [here.](https://github.com/StonedEdge/Retro-Lite-CM5/tree/main/enclosure)
Make sure to use v2 as these contain all the latest and greatest changes. 

-------------------------------------------------------------------------------------------------------

### PCB
v1 Controller PCBs have been uploaded [here.](https://github.com/StonedEdge/Retro-Lite-CM5/tree/main/rp2040_gamepad/PCB)
Please note that you will not be able to assemble the unit with the UART/serial debug components soldered onto the boards, so make sure these aren't populated. 

v1.1 CM5 carrier boards have been uploaded [here.](https://github.com/StonedEdge/Retro-Lite-CM5/tree/main/cm5_carrier_pcb)

-------------------------------------------------------------------------------------------------------

### BOM (Bill of Materials) 
v1 Right Controller PCB BOM have been uploaded [here.](https://github.com/StonedEdge/Retro-Lite-CM5/blob/main/rp2040_gamepad/PCB/Retro_Lite_CM5_Right_PCB_v1%20-%20BOM.csv)

v1 Left Controller PCB BOM is coming soon. I want to upload both a top & bottom DPAD version. 

v1 V2.2 Carrier PCB BOM has been uploaded [here.](https://github.com/StonedEdge/Retro-Lite-CM5/blob/main/cm5_carrier_pcb/Retro_Lite_CM5_IO_X2.2_BOM.csv) 
The board has been verified and fully functional with Radxa V2.2 CM5 modules! I have not tested V2.21 with success yet, however I will check soon. 

-------------------------------------------------------------------------------------------------------

### To-do
- [x] Hardware familiarization - Orange Pi 5/Radxa CM5/IO board schematics
- [x] Design v1 prototype left/right controller PCBs (ATMEGA32u4 based)
- [x] Design v1 DSI test display board
- [x] Design v1 Power Management/DisplayPort/USB-C PD test board
- [x] Order v1 Power Management/DisplayPort/USB-C PD test board
- [x] Design v2 prototype left/right controller PCBs (RP2040 based)
- [x] Test v1 Power Management/DisplayPort Alternate Mode/USB-C PD test board functionality - works!
- [x] Design CM5 carrier main board (v1.0) that implements new hardware features, such as DisplayPort, USB3 and PCIe
- [x] Order CM5 carrier board (v1.0) with desired fab house. (It's in production!)
- [x] Test CM5 carrier board (v1.0) hardware for a basic functioning prototype
- [x] Redesign CM5 carrier board (v1.1) with fixes for 3rd connector polarity & other small fixes
- [x] Reorder v1.1 carrier board
- [x] Investigate RK806 PMIC issue
- [x] Prepare an Armbian based OS image, including the BSP (bootloader, dtb and kernel) for the RK3588s Armbian rootfs. DTB should include the ILI9881C/WM8960 drivers
- [x] Integrate new 5.5 inch MIPI DSI display. This includes enabling the ILI9881C display driver, device tree customizations for the display
- [x] Test and verify display functionality, including proper backlight & 60 FPS. Verify display works with Armbian 
- [x] WM8960 codec integration via Orange Pi 5/CM5 IO for testing. Integrate the WM8960 i2c code driver including any device tree customizations for codec. Finally, test and verify audio works correctly
- [x] Integrate touch/verify via the GT911 controller 
- [x] Test and verify emulators work correctly with Mali blobs and can be launched via EmulationStation front end
- [x] Setup Mali Open GL blob drivers to work with PS2 (AetherSX2) & GC (Dolphin) emulators due to no Vulkan backend available (yet!)
- [x] Write HID controller code for RP2040. Verify basic gamepad functionality works as intended
- [x] Integrate an IMU into the controller PCB for gyro functionality. Include EKF algorithm (MPU-6050 6-axis IMU) 
- [x] Redesign housing (increase 3mm in all directions) in Solidworks. Improve trigger feel and add battery mounting bracket
- [x] Redesign housing v2 (increase thickness to accomodate 6Ah battery cell at 3.8v, mount battery and fan to front housing)
- [x] Redesign housing with injection molding in mind - include appropriate draft angles, ribs, gussets, radiused corners & bosses
- [x] Redesign housing to use original switch lite fan enclosure 
- [x] Power on/power off/sleep mode via ATtiny84 (Power control)
- [x] Put together a fully functional dts for the CM5 IO Board (Device Tree Source) to understand hardware configuration 
- [x] MAX17055 integration into device tree 
- [x] Prepare reference implementation for launching games via EmulationStation/some form of frontend (likely ES-DE/RetroPie)
- [x] Implement an on-screen indicator for volume, brightness control via GPIOs
- [x] Rocknix upstream support
- [x] Android 13 support 
- [ ] Adjust BQ24292i registers to charge to 4.35v max instead of 4.2v due to using LiHv cell
- [x] Other optimizations/improvements/bug fixes
- [x] Create prototype showcase video for YouTube
- [x] Final GitHub open source release & announcement!

-------------------------------------------------------------------------------------------------------

### Frequently Asked Questions (FAQ) 
Q: How can I get my hand on a Retro Lite CM5? Do you sell them or will you sell them?

A: Radxa have advised that **this device will not be mass produced**. There is no method to get your hands on one of these at this time. 

Q: Why dont you use an OLED screen? 

A: We may eventually, however the panel we chose was one that we knew we could get working in Linux for now. An upgrade may come in the future, if we can find a good panel that works with Linux. 

Q: Why do this when you can get an equivalent handheld from Aliexpress for $150? This is stupid and DOA! Makes no sense! Non-sensical! I'm going to buy myself a better handheld. 

A: Because it's fun to build something awesome. The End. Thanks for visiting & farewell, this project isn't for you. 

Q: Is Vulkan working yet on Linux?

A: Not yet, but eventually it will, thanks to the efforts from Collabora and ARM. We'll be sure to integrate it into our Linux image once it's available. 

Q: Can you please make me one for exposure on YouTube? I promise to give incredible reviews!

A: Nope. I simply don't have time. 

### License

[![CC BY-NC-SA 4.0][cc-by-nc-sa-shield]][cc-by-nc-sa]

This work is licensed under a
[Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License][cc-by-nc-sa].

[![CC BY-NC-SA 4.0][cc-by-nc-sa-image]][cc-by-nc-sa]

[cc-by-nc-sa]: http://creativecommons.org/licenses/by-nc-sa/4.0/
[cc-by-nc-sa-image]: https://licensebuttons.net/l/by-nc-sa/4.0/88x31.png
[cc-by-nc-sa-shield]: https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg


