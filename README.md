# SPI-Tick_Example
Spi Tick Example

Must download bcm2835 from http://www.airspayce.com/mikem/bcm2835/
Install instructions on that website

Compile with gcc -o spitest spitest.c -l bcm2835

Sends SPI out on GPIO pins at the delay set, holds pin GPIO 17 high during transmission, CS hold low during each data bit send (16 bits). Counts from 0 to 4095.

Tick is made with code from http://yosh.ke.mu/. (quicker then getsystemtick())

Must run as root
