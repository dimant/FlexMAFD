# FlexMAFD
This is a custom firmware for the [Sonoclast MAFD](https://sonoclast.com/products/mafd/), a device adding MIDI to the [Moog DFAM](https://www.moogmusic.com/products/dfam-drummer-another-mother).
The original firmware maps MIDI notes to specific DFAM steps and gives CV output for MIDI velocity and modwheel inputs.

With FlexMAFD it is possible to map any trigger or MIDI input to CV.

## Technical Details
The MAFD is controlled by an [Arduino Teensy LC](https://www.pjrc.com/store/teensylc.html) device. It controls:
- an LED
- 1 trigger output
- 2 analog outputs

The LED is connected to pin 10. The trigger output, jack labelled "to ADV/Clock", is routed to GPIO pin 17. The 2 analog outputs aree controlled by  [MPC4902](http://ww1.microchip.com/downloads/en/devicedoc/22250a.pdf) over SPI. The default pins are used for SPI and SPI chip select is on pin 14.

**Note** that CV is 0 - 5V, this is a hardware limitation. One example for using FlexMAFD is so you can sync your DFAM to a MIDI clock.

## Installation
The following steps need to be completed for installation:
1. Install [Arduino IDE](https://www.arduino.cc/en/Main/Software)
2. Install [Teensyduino extension](https://www.pjrc.com/teensy/td_download.html)
3. Install [FortySevenEffects MIDI library](https://github.com/FortySevenEffects/arduino_midi_library)

If you are unfamiliar with uploading software to Arduino devices, it may be helpful to follow the [Teensy tutorial](https://www.pjrc.com/teensy/tutorial.html) first. Note that the LED on the MAFD is on pin 10.

## Uninstallation
Sonoclast provides the default firmware for download along with instructions for installing it [here](https://sonoclast.com/products/mafd/). If you find yourself in a corner, you can get yourself back to normal by uploading the default firmware.

## Configuration
The FlexMAFD code provides a few values for easy configuration:
- **int cc** changes the control change # sent to an analog jack. By default this is **1** which maps to the mod wheel but you can change it to any other number.
- **ccDACChannel** chooses to which jack to send the CC value. **0** routes to the jack labelled "pressure/mw" and **1** routes to the jack labelled "velocity"
- **vlDACChannel** chooses to which jack to send velocity information. **0** routes to the jack labelled "pressure/mw" and **1** routes to the jack labelled "velocity"
- **int clockDiv** typically the MIDI clock is way faster than is useful for clocking the DFAM. This value allows changing the trigger so it is sent only every **clockDiv** clock cycles. By default the value is 6 but your MIDI device might work better with another value.
- **int pulseLen** allows changing the pulse length of the trigger sent to the DFAM. Longer pulses will be detected more reliably by the DFAM but shorter pulses allow clocking the device faster. This value is in milliseconds and I found that on my DFAM 30ms works fine.