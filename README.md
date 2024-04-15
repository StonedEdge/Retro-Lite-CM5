<div align="center">
  <h1>Retro Lite CM5: RK3588s Gaming Handheld</h1>
</div>

<div align="center">
  <img src="https://i.imgur.com/VhgvOEU.png" alt="Image of Retro Lite CM5">
</div>


The Retro Lite CM5 is an upgrade to the Retro Lite CM4 handheld. The project goal is to create a more powerful handheld that can handle 3DS, PS2 and GC emulation (none of which the Pi can do). I also would like to be able to play some games via Box64/86.  

Powered by the RK3588s compute module from Radxa, which is an SBC SOM module with an ARMv8 64-bit processor. Features 8 cores/threads - 4 x ARM Cortex-A55/A76 cores with A55 cores clocked at 1.8GHz, and A76 cores clocked at 2.4GHz. 

The Retro Lite CM5 offers a more modern gaming experience with DisplayPort via USB Type-C, USB 3.0 & HDMI 2.1. Built-in PCIe WiFI 6.0 & BT 5.2, as well as a new 720p IPS display @ 5.5" with touchscreen support & dual tact trigger buttons for GameCube support, make this a much more viable low-powered ARM SBC gaming PC than the Raspberry Pi 4. 

The operating system will be based on Armbian, with a custom device tree for hardware interfacing & RetroPie pre-installed. This handheld should be able to play the entire PS2 library at least 1x native resolution for all games, as well as GameCube at 1x on Linux. Android is currently more performant due to Vulkan drivers however Linux Vulkan blobs should come soon (or someday, who knows). 

For regular updates/following progress on the build, please check out the forum post here:
https://bitbuilt.net/forums/index.php?threads/retro-lite-cm5.5815/

# Retro Lite CM5: Radxa RK3588s Handheld 

## Description
 
TBD

## Features

TBD

A hint: it plays video games. Who knew?

## Requirements to Build the Retro Lite CM5

### Code 

TBD

-------------------------------------------------------------------------------------------------------

### CAD
Enclosure files have been uploaded in STL and STEP format [here.](https://github.com/StonedEdge/Retro-Lite-CM5/tree/main/enclosure)

-------------------------------------------------------------------------------------------------------

### PCB
v1 Controller PCBs have been uploaded [here.](https://github.com/StonedEdge/Retro-Lite-CM5/tree/main/rp2040_gamepad/PCB)
Please note that you will not be able to assemble the unit with the UART/serial debug components soldered onto the boards, so make sure these aren't populated. 

v1 V2.2 Carrier PCB gerber files have been **removed** due to discovering major flaws in the v1 iteration. 
I won't upload these until the design is fully tested and completed.  

-------------------------------------------------------------------------------------------------------

### BOM (Bill of Materials) 
v1 Right Controller PCB BOM have been uploaded [here.](https://github.com/StonedEdge/Retro-Lite-CM5/blob/main/rp2040_gamepad/PCB/Retro_Lite_CM5_Right_PCB_v1%20-%20BOM.csv)

v1 Left Controller PCB BOM is coming soon. I want to upload both a top & bottom DPAD version. 

v1 V2.2 Carrier PCB BOM has been uploaded [here.](https://github.com/StonedEdge/Retro-Lite-CM5/blob/main/cm5_carrier_pcb/Retro_Lite_CM5_IO_X2.2_BOM.csv) 
Note that this has not yet been physically tested, however is on order with the fab house currently, so order at your own risk. You have been warned! 

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
- [ ] Redesign CM5 carrier board (v1.1) with fixes for 3rd connector polarity & other small fixes
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
- [x] Redesign housing with injection molding in mind - include appropriate draft angles, ribs, gussets, radiused corners & bosses
- [x] Redesign housing to use original switch lite fan enclosure 
- [x] Power on/power off/sleep mode via ATtiny84 (Power control)
- [x] Put together a fully functional dts for the CM5 IO Board (Device Tree Source) to understand hardware configuration 
- [x] MAX17055 integration into device tree 
- [ ] Prepare reference implementation for launching games via EmulationStation/some form of frontend (likely ES-DE/RetroPie)
- [ ] Implement an on-screen indicator for volume, brightness control via GPIOs
- [ ] Create a headphone icon to indicate headphones hotplugged 
- [ ] Main Boss: The Ultimate Menu Mode! Toggle gyro, adjust brightness, adjust volume, and some other cool stuff. Courtesy of GinKage. 
- [ ] Other optimizations/improvements/bug fixes
- [ ] Create prototype showcase video for YouTube
- [ ] Final GitHub open source release & announcement!

-------------------------------------------------------------------------------------------------------

### Frequently Asked Questions (FAQ) 
Q: How can I get my hand on a Retro Lite CM5? Do you sell them or will you sell them?

A: The handheld is still in the design phase and is not ready to be sold. I won't rule out selling these in the future as they are a lot easier to make than the previous CM4 version, especially with 3D printing becoming an option now.
We'll see where it takes us! And besides, the Radxa CM5 SOM still isn't even available to the general public yet for sale. Besides, the benefit of open source is to learn to DIY!

Q: Why dont you use an OLED screen? 

A: We may eventually, however the panel we chose was one that we knew we could get working in Linux for now. An upgrade may come in the future, if we can find a good panel that works with Linux. 

Q: Why do this when you can get an equivalent handheld from Aliexpress for $150? This is stupid and DOA! Makes no sense! Non-sensical! I'm going to buy myself a better handheld. 

A: Because it's fun to build something awesome. The End. Thanks for visiting & farewell, this project isn't for you. 

Q: Is Vulkan working yet on Linux?

A: Not yet, but eventually it will, thanks to the efforts from Collabora and ARM. We'll be sure to integrate it into our Linux image once it's available. 

Q: Will you be using a BSP kernel for this or bleeding edge?

A: Probably not, we want a newer kernel that will give us the latest packages. Eventually the RK3588s will be mainlined, it's just a matter of when, not if. 

Q: Can you please make me one for exposure on YouTube? I promise to give incredible reviews!

A: Nope. 


