# What does this do?

This code implements auto-rotate functionality on the Yoga 900 by coupling iio-sensor-proxy to xrandr.

## Configuration

Install dependencies, for example:

	sudo apt-get install iio-sensor-proxy libxrandr2 
	
It might be that you need some more (not a virgin system and too lazy to check):

	sudo apt-get install libglib2.0-dev

Just compile by running:

	make

## Usage

To run as a daemon:

	./auto-rotate

To see the output (and not run it as a daemon either):

	./auto-rotate --verbose

To use with systemV:

	sudo cp auto-rotate.sh /etc/init.d/
	# and adjust run-levels

To use with systemd (RECOMMENDED):

	mkdir -p ~/.config/systemd/user
	cp auto-rotate.service ~/.config/systemd/user/

To test systemd:

	systemctl --user start auto-rotate

To enable it by start-up:

	systemctl --user enable auto-rotate

## Bugs

There might be mismatches between iio-sensor-proxy and the kernel. I have installed `iio-sensor-proxy` from source:

	git clone https://github.com/hadess/iio-sensor-proxy

	sudo apt-get install gtk-doc gtk-doc-tools libgudev-1.0-dev libgtk-3-dev

	./autogen.sh 
	./configure --prefix=/usr --sysconfdir=/etc
	make "CFLAGS=-Wno-unused-result"
	sudo make install

And run the following as root:

	G_MESSAGES_DEBUG=all /usr/sbin/iio-sensor-proxy

## Copyrights

This is created after `monitor-sensor.c` as example at [https://github.com/hadess/iio-sensor-proxy](iio-sensor-proxy). 

Copyrights of the **changes** w.r.t. the original:

* Author: Anne van Rossum
* Company: none
* Date: 28 Dec, 2016
* License (free to pick): LGPLv3, MIT, Apache 2.0

For the copyrights on the original, see code. Summarized: 2015 Bastien Nocera, LGPLv3.

Hence, complete code falls under LGPLv3.
