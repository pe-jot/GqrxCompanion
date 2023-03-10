## GqrxCompanion

A quick &amp; simple [Qt](https://www.qt.io/) GUI based program to assist [Gqrx](https://gqrx.dk/) SDR software via its TCP remote interface.

The main purpose is to read the current signal level received from Gqrx and after a certain threshold is reached, a screenshot is captured.
If the level falls below a certain threshold, a screenshot is captured again. It's intended use is to automatically monitor a frequency for interferences and log them.
The software is intended to run on a [Raspberry Pi 4](https://www.raspberrypi.com/products/raspberry-pi-4-model-b/).

**NOTE** for Linux users: The function used for taking the screenshot `QScreen::grabWindow()` doesn't work on Linux systems running Wayland window manager - a black image would be the result on those platforms.
Luckily, [Raspberry Pi OS](https://www.raspberrypi.com/software/operating-systems/#raspberry-pi-os-64-bit) is using LXDE desktop based on X11.

### Imperfections
* Absolutely minimalistic
* The program does not save any settings
* GUI design is not cross-platform capable (development was done on a Windows 10 machine)
* Data is polled from Gqrx in a fixed interval and synchrously (synchronous TCP write + subsequent read)
* One could also add those features to Gqrx directly, however setting up a build environment looks time consuming
