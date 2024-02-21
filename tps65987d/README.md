# TPS65987D Test Board v3

<p align="center">
   <img src="https://i.imgur.com/e9K5JSS.png"/>
</p>

## What is this folder? 
This folder contains the test fixture schematic and board files (EAGLE) designed to test the functionality of the TPS65987D PD controller and TUSB546A superspeed mux, as well as other features that will be included on the main carrier board for the final build. Feel free to use it as a reference design for a USB-C DRP application in your designs. 
It also contains the project file that can be used to edit specific parameters in the TPS65987x GUI Application Tool, depending on your required use case. I used an RP2040 to flash the binary to the SPI module & final binary to be used in the CM5 will be uploaded when ready. 


## Purpose of PCB
To provide an environment to test out the TPS6598x GUI Application Tool, change parameters and flash a binary for a USB-C DRP application. The current CM5 is programmed with the parameters found in the .pjt file here. You can use this tool if you want to play around with it or change certain parameters to suit your specific application: https://www.ti.com/tool/TPS6598X-CONFIG

The flash module is an W25X05 and communicates with the PD device over the SPI bus. It loads this flash either via 3V3 power or dead battery mode via VBUS power. You can also program the TPS65987D (the USB-PD controller) via the I2C bus. 

The bottom USB-C connector is for plugging in sink/source device to charge the battery or power a USB device (like a keyboard, mouse or thumb drive). This will be the port on the final handheld itself. 

The top two USB-C ports are for high speed data buses, namely DisplayPort & USB 3.1.

The ATtiny is connected to the button, which is used to control power, sleep & regulator enable. We use this to safely tell the CM5 to shutdown before the enable pin is driven low. The ATtiny is also connected to the BQ24292i over I2C & sets the specific registers required for monitoring the battery - charging current, termination current, and other parameters like charge cutoff voltage. 

Flash the SPI module via the command line, for example, using serprog. 
```flashrom -p serprog:dev=/dev/ttyACM0:115200,spispeed=12M -c W25X05 -w .bin```




