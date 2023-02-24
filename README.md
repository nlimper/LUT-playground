# LUT playground
 
E-paper waveforms are electronic signals used to control the display of an e-paper screen. These waveforms drive the display's pixels to show the desired content.

It's important to note that creating and modifying e-paper waveforms requires technical knowledge and expertise. Mishandling of the waveforms may result in damage to the display and other components of the e-paper device.

That being said, experimenting with e-paper waveforms can be a fascinating and rewarding experience, as it allows for the creation of unique and customized e-paper displays.

![screenshot](/extra/screenshot.jpg?raw=true "Screenshot")

## Hardware

This project uses a 2.9" black/white/red e-paper display, an ESP32 development board, and a 6-digit LED display (TM1637) to display the timings. The e-paper display should be compatible with an SSD1619A controller such as the 2.9" HINK-E029A14 (for example, 7 LUT groups, no state repeat. The waveform is 76 bytes).

Connect the display as follows:
| epaper | ESP32 |
|--------|----|
| MOSI   | 23 |
| CLK    | 18 |
| CS     | 5  |
| DC     | 17 |
| RST    | 16 |
| BUSY   | 4  |

| TM1637 | ESP32 |
|--------|----|
| CLK    | 13 |
| DIO    | 15 |

click the image below for a video

[![hardware](/extra/hardware.jpg?raw=true "Hardware")](https://youtu.be/tAYy6mK-uXM)

## Software

Create "pass.h" file with your Wi-Fi credentials, or fill the appropriate variables in "main.cpp".
On startup, the ip-address appears on the epaper display. Point your browser there to start.

Tip: If you want to see in detail what's happening, record a video of the e-paper updating in slow motion. The LED display is synchronized with the groups and phases and counts the repeats.

## Useful background info

Zhang, Hu, Zichuan Yi, Liming Liu, Feng Chi, Yunfeng Hu, Sida Huang, Yu Miao, and Li Wang. 2022. "A Fast-Response Driving Waveform Design Based on High-Frequency Voltage for Three-Color Electrophoretic Displays" Micromachines 13, no. 1: 59. https://doi.org/10.3390/mi13010059

## Credits

LUT playground is created by Nic Limper. If you like it and find it useful, please consider [buying my a coffee on https://ko-fi.com/nicli](https://ko-fi.com/nicli).
