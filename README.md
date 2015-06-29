#Digital Clock
The "Hello World" project of any electrical engineer, a digital clock. This is a basic ATMegax8 design with the addition of linear constant current shift register sinks to switch low-power 7-segment LED displays that may need a voltage higher than the microcontroller supply voltage to work (I'm using 12V here).

You can read more about the project on my blog: http://htonoyan.blogspot.com. For any questions feel free to contact me by email or posting on the blog.

* Hardware/ Contains all the circuit board files (Altium Designer format, but a PDF version and gerbers are provided).
* Software/ Contains the code running on the AVR ATMega48. I used Atmel Studio with gcc to compile the code.