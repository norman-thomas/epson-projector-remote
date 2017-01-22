# epson-projector-remote

A simple EPSON projector remote via RS232 running on a Particle Photon.

This is not intended to be as exhaustive as openHAB's addon. The plan is to have a small feature set of just powering on and off, as (sadly) more than that is not yet supported by Apple's HomeKit. This small project is just a relais to use with my [homebridge particle.io plugin](https://github.com/norman-thomas/homebridge-particle-io) so I can turn on my projector via Siri.

This was tested using an EPSON EH-TW4400 projector.


# Setup

![What the setup looks like](setup.jpg)

## Equipment

* Particle Photon
* EPSON projector (EH-TW 4400, in my case)
* RS232 interface (MAX3232 chip)
* 4 jumper wires

## Wiring

The Wiring is very straight-forward. Simply connect the following pins from your Particle device to the MAX3232 (RS232) board:

* `Vin`—`Vcc`
* `GND`—`GND`
* `RX`—`TXD`
* `TX`—`RXD`

Just pay attention to really connect the RX(D) with TX(D) and vice versa.

# Commands

The functions listed below are exposed via the Particle Cloud REST interface.

Due to the limitations of the Particle Cloud, functions are required to return integers. That means that where booleans or Strings would technically have made more sense, I somehow mapped them to integer values. Functions usually return `-1` when an error occured.

* `power` groups the functionality of the next three functions (`power_on`, `power_off` and `power_status`) together. It accepts the paramters `?` to request the current power status, `ON` to turn the projector on and `OFF` to turn it off. Returns `0` if the operation was executed without errors, `-1` if an error occured.
* `power_on`, as the name suggests, powers the projector on. If it was already powered on or the projector was turned on successfully, it will just return `0`. `-1` otherwise.
* `power_off` similarly to the function above, this function powers the projector off and returns `0` upon success or if the projector was already powered off and returns `-1` otherwise.
* `power_status` returns `0` if the projector is powered off, `1` if it is powered on or still warming up and `-1` if an error occurs.
* `error` checks for an error and currently only returns the EPSON error code as integer. `0` means no error.
* `ok` is basically the same as `error` above, but maps all errors to `-1`. If no errors are reported by the projector the return value is `0`.

I disabled the default behavior of the RGB LED on the Particle Photon. It is constantly turned off. While sending a command it turns red, while reading a response from the projector it turns green.

# Resources

[EPSON Business Projector Documentation](https://files.support.epson.com/Epson_Handbook/html/p85_rs232.html)

[EPSON Home Projector Documentation](https://epson.com/Support/wa00572)

[openHAB Addon](https://github.com/openhab/openhab1-addons/tree/master/bundles/binding/org.openhab.binding.epsonprojector)

