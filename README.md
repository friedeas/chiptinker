# chiptinker
ChipTinker is an open-source IC tester, forked and modernized from an existing project, targeting modern low-cost hardware, built for learning, on hands-on tinkering.

## Project Background / Origin

While searching for a logic IC tester, I came across this overview of [IC tester](https://www.ombertech.com/cnk/ictesters/index.htm) referencing a tester published in Everyday Practical Electronics Magazine (2002), designed by Joe Farr.

Based on this hardware, MCbx later implemented a Qt-based application for Windows and Linux, released under the GPL-2.0 license and available at:
https://oldcomputer.info/software/ictester/index.html

That application and protocol form the foundation of this project, which builds upon the original work and modernizes it for current microcontroller platforms.

## Current Status
At the moment, this initial version is still very limited in functionality. The tester firmware currently returns hard-coded 000 values for all test pins and does not yet perform real IC measurements.

However, the serial communication protocol has already been slightly improved to provide more stable and reliable communication with the test microcontroller, laying the groundwork for further development.
![ChipTinker serial communication during test](/images/ChipTinker%20Initial%20Version.png)