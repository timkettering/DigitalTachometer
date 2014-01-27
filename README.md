DigitalTachometer
=================

Arduino code for a conversion of a 1971 Honda CB tachometer to digital input

In the process of my 1971 Honda CB350 build, I retrofitted the mechanical style ignition with a more
modern computer controlled ignition module.  This computer controlled ignition module came with a 
digital tach output. 

I saw this as an opportunity to get rid of the mechanical tachometer cable linkage on my bike and replace
it with a much simpler wire.  But I did not like the state of the aftermarket tach offerings.  They were
all unecessarily modern, complex or expensive.  I really liked the simplicty of the Honda CB gauges.  

So I removed the mechanical workings of the CB350 tachometer and have replaced it with a Switec X27-168 
digital stepper motor and am using an Arduino microcontroller to read in the tach input and drive 
the stepper motor.

This repo contains the code I've written for the Arduino and I will also be uploading some schematics
and 3D print files for the rest of the assembly housing to fit it in the tach housing.
