# CCLoader-Linux
This code is for loading firmware to your CC2541 chip. But instead of being connected to your typical ESP or Arduino, this code works for an embedded Linux platform utilising libgpiod.

For some reason unknown to me this does not work for CC2540. Only CC2541.

Also in contrast to the usual setup there is no serial communication between the Bitbanger (connected to CC2541) and the data provider (reading .bin file). 
Rather we use tcpip sockets to communicate between two programs. This can be done much more elegantly, i know. but for me this suffices for now.


## Compilation
I Usually compile this using the cross compiler that buildroot provides to me after i build the embedded platform.

```
export PATH=$PATH:~/tmp/buildroot/output/host/bin/

./build.sh
```

## Running
This code needs to be ran on an embedded Linux platform such as a Raspberry Pi or even better a custom built SoC like Allwinner (V3S/T113-S3, and the like).

Invocation is done two way. First run the TCPIP listener (a.k.a bitbanger part). The GPIO pins used for CC/CD and RESET you have to change in the source code analogue to your board/schematic design.

`./main &`

Now the 'bitbanger' code is ready to receive TCPIP client for sending data. 

`./CCLoader_Firmware/main ./CCLoader_Firmware/CC2531hm10v709.bin`

This invocation starts the file reader program to sends its content to the bitbanger part over tcpip.
A counter will show what code block is being sent over the line to the CC chip. When it reaches 512, all is done.

There is not much beauty here. Just glueing together what is already available online.  It did however not find any conversion of the CCLoader to be ran on embedded Linux instead of ESP/Arduino. So here is what i made of it. 

## Using CC2541 
After uploading the firmware you can connect to the BLE chip using AT commands over a serial line.
Don't forget to setup the Linux DTS (device tree source) for the right uart instance and gpio pins.

For instance:
```
&uart4 {
        pinctrl-0 = <&uart4_pb2_pins>;
        pinctrl-names = "default";
        status = "okay";
};
```

and .....
```
&pio{
        uart4_pb2_pins: uart4-pb2-pins {
                pins = "PB2", "PB3";
                function = "uart4";
        };
};
```

I usually include the minicom application on my embedded devices in order to test. 
Open it up with the right device and baudrate and you should be good. 

`minicom -b 115200 -D /dev/ttyS1` 

Please mind you that the latest firmware versions of the CC2541 don't require any linefeeds or carriage returns. 
Just copy the 'AT' words and paste these into the minicom window using a mouse. 
Typing AT commands won't work.  You should be able to get a response:

```
AT
.OK
AT+RENEW
.OK+RENEW
AT+RESET
.OK+RESET
```

Use a BLE serial console on your Android or iOs phone to see whether a device called 'HMSoft' is there waiting for you to play with.
