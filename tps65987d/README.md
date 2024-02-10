# TPS65987D Test Board v3

<p align="center">
   <img src="https://i.imgur.com/e9K5JSS.png"/>
</p>

## What is this folder? 
This folder contains the test fixture schematic and board files (EAGLE) designed to test the functionality of the TPS65987D PD controller and TUSB546A superspeed mux, as well as other features that will be included on the main carrier board for the final build. Feel free to use it as a reference design for a USB-C DRP application in your designs. 
It also contains the project file that can be used to edit specific parameters in the TPS65987x GUI Application Tool, depending on your required use case. I used an RP2040 to flash the binary to the SPI module & final binary to be used in the CM5 will be uploaded when ready. 

Flash via command line:
```flashrom -p serprog:dev=/dev/ttyACM0:115200,spispeed=12M -c W25X05 -w .bin```




