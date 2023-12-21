# Retro-Lite-CM5
For regular updates/following progress on the build, please check out the forum post here:
https://bitbuilt.net/forums/index.php?threads/retro-lite-cm5.5815/

The Retro Lite CM5 is an upgrade to the Retro Lite CM4 handheld. The project goal is to create a more powerful handheld that can handle 3DS, PS2 and GC emulation (none of which the Pi can do). I also would like to be able to play some games via Box64/86.  

Powered by the RK3588s compute module from Radxa, which is an SBC SOM module with an ARMv8 64-bit processor. Features 8 cores/threads - 4 x ARM Cortex-A55/A76 cores with A55 cores clocked at 1.8GHz, and A76 cores clocked at 2.4GHz. 

The Retro Lite CM5 offers a more modern gaming experience with DisplayPort via USB Type-C, USB 3.0 & HDMI 2.1. Built in PCIe WiFI 6.0 & BT 5.2, as well as a new 720p IPS display @ 5.5" with touchscreen support & dual tact trigger buttons for GameCube support make this a much more viable low powered ARM SBC gaming PC than the Raspberry Pi 4. 

The operating system will be based on Armbian, with custom device tree for hardware interfacing & RetroPie pre-installed. This handheld should be able to play the entire PS2 library at least 1x native resolution for all games, as well as GameCube at 1x on Linux. Android is currently more performant due to Vulkan drivers however Linux Vulkan blobs should come in the near future. 

![Image of Retro Lite CM5](https://i.imgur.com/evUsqqW.png)
![Image of Retro Lite CM52](https://i.imgur.com/OTbErjX.png)
# Retro Lite CM5: Radxa RK3588s Handheld 

## Description
 
TBD

## Features

TBD

## Requirements to Build the Retro Lite CM5

### Code 

TBD

-------------------------------------------------------------------------------------------------------

### CAD
TBD

-------------------------------------------------------------------------------------------------------

### PCB
TBD

-------------------------------------------------------------------------------------------------------

### BOM (Bill of Materials) 
TBD

-------------------------------------------------------------------------------------------------------

### To-do
- [x] Hardware familiarization - Orange Pi 5/Radxa CM5/IO board schematics
- [x] Design v1 prototype left/right controller PCBs (ATMEGA32u4 based)
- [x] Design v1 DSI test display board
- [x] Design v1 Power Management/DisplayPort/USB-C PD test board
- [x] Order v1 Power Management/DisplayPort/USB-C PD test board
- [x] Design v2 prototype left/right controller PCBs (RP2040 based)
- [x] Test v1 Power Management/DisplayPort Alternate Mode/USB-C PD test board functionality - works!
- [ ] Design CM5 carrier main board that implements new hardware features, such as DisplayPort, USB3 and PCIe. Implement new boost regulator to handle potential max 15W of power 
- [x] Prepare an Armbian based OS image, including the BSP (bootloader, dtb and kernel) for the RK3588s Armbian rootfs. DTB should include the ILI9881C/WM8960 drivers
- [x] Integrate new 5.5 inch MIPI DSI display. This includes enabling the ILI9881C display driver, device tree customizations for the display
- [x] Test and verify display functionality, including proper backlight & 60 FPS. Verify display works with Armbian 
- [x] WM8960 codec integration via Orange Pi 5/CM5 IO for testing. Integrate the WM8960 i2c code driver including any device tree customizations for codec. Finally, test and verify audio works correctly
- [x] Integrate touch/verify via the GT911 controller 
- [x] Add RetroPie software package
- [x] Test and verify emulators work correctly with Mali blobs and can be launched via EmulationStation front end
- [x] Setup Mali Open GL blob drivers to work with PS2 (AetherSX2) & GC (Dolphin) emulators due to no Vulkan backend available
- [ ] Write HID controller code for RP2040, based on GP2040-CE
- [ ] Integrate an IMU into the controller PCB for gyro functionality (RP2040)
- [ ] Integrate brightness display toggle via GPIO PWM control 
- [ ] Redesign housing (increase 3mm all directions) in Solidworks. Improve trigger feel and add battery mounting bracket 
- [x] Redesign housing with injection molding in mind - include appropriate draft angles, ribs, gussets, radiused corners & bosses
- [x] Redesign housing to use original switch lite fan enclosure 
- [ ] Standby and wakeup RK3588s SoC implementation via ATtiny84 and button press
- [ ] Power on/power off via ATtiny84 and button press 
- [ ] MAX17055 integration - ATtiny84/CM5 to act as master (use LEDs for battery indication or OSD if possible to draw to screen FB) 
- [ ] Port any necessary C code base from Retro Lite CM4 to RK3588 (if appropriate) 
- [ ] Prepare reference implementation of Wayland image via Weston for launching games 
- [ ] Implement an on-screen indicator for volume control via GPIOs
- [ ] Implement an on-screen indicator for brightness control in fixed steps
- [ ] Create a headphone icon to indicate headphones hotplugged 
- [ ] Other optimizations/improvements/bug fixes 
- [ ] Final GitHub open source release & announcement

