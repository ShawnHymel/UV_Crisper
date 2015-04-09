UV Crisper
==================

Set the desired level of "crispness" with the potentiometer and hit the ON switch. An arbitrary "UV energy level" will appear on the display and count down as UV light falls on the sensor. Once the counter reaches 0, the sound clip will play.

Currently, the Arduino is programmed to sample every second, and it is calibrated to get about 1.5 minutes of UV on a cloudy day for the lowest setting (for the purposes of the video). Play around with the constants for the desired level. The amount of UV needed to actually produce sufficient vitamin D levels is highly dependent on the time of day and skin color. Read more about Vitamin D [here](http://ods.od.nih.gov/factsheets/VitaminD-HealthProfessional/).

Getting too much sun exposure can be [really bad](http://www.webmd.com/beauty/sun/sun-exposure-skin-cancer), so use the UV Crisper with extreme caution. Seriously, it's a demo project. I can't really recommend using it without some sort of sun protection.

Hardware
--------

Parts list: [https://www.sparkfun.com/wish_lists/109871](https://www.sparkfun.com/wish_lists/109871)

Change the solder jumper on the Power Cell to output 3.3V.

Connect the Power Cell's VCC to the switch, and connect the other pin of the switch to all the components' VCC. Connect the Power Cell's GND to all the other components' GND. Additionally:

 * VCC -> + of speaker
 * VCC -> One of the outside pins of the potentiometer
 * GND -> The other outside pin of the potentiometer
 * GND -> Emitter of transistor
 
Make the following connections to the Arduino:

 * A1 -> UV sensor out
 * A2 -> Middle potentiometer pin
 * 3 -> 330 Ohm Resistor -> Base of transistor
 * 10 -> SS pin on 7 segment display
 * 11 -> SDI pin on 7 segment display
 * 13 -> SCK pin on 7 segment display
 
Other connections:

 * - of speaker -> Collector of transistor
 * UV Sensor EN pin -> VCC (3.3V)
 
Software
--------

Open the .ino file in Arduino, and upload it to the Arduino Pro Mini!

More Information
----------------

The following tutorials were used as resources in this project:

 * [ML8511 UV Sensor Hookup Guide](https://learn.sparkfun.com/tutorials/ml8511-uv-sensor-hookup-guide)
 * [Using the Serial 7-Segment Display](https://learn.sparkfun.com/tutorials/using-the-serial-7-segment-display)
 * [PowerCell Quickstart Guide](https://www.sparkfun.com/tutorials/379)
 * [Arduino PCM Audio](http://playground.arduino.cc/Code/PCMAudio)

License Information
-------------------
The hardware is released under [Creative Commons ShareAlike 4.0 International](https://creativecommons.org/licenses/by-sa/4.0/).
The code is beerware; if you see me (or any other SparkFun employee) at the local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
