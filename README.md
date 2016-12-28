# What does this do?

This code implements auto-rotate functionality on the Yoga 900 by coupling iio-sensor-proxy to xrandr.

## Configuration

Install dependencies, for example:

	sudo apt-get install iio-sensor-proxy libxrandr2 libglib2.0-dev
	
Just compile and install by running:

	make
	sudo make install

## Usage

To use with systemd (RECOMMENDED):

	make config-systemd

To just give it a test run:

	make run

To use with systemV (not tested and fully implemented):

	make config-systemv

## Bugs

### Upstream

There might be mismatches between iio-sensor-proxy and the kernel. I have also experimented with running `iio-sensor-proxy` from source:

	git clone https://github.com/hadess/iio-sensor-proxy

	sudo apt-get install gtk-doc gtk-doc-tools libgudev-1.0-dev libgtk-3-dev

	./autogen.sh 
	./configure --prefix=/usr --sysconfdir=/etc
	make "CFLAGS=-Wno-unused-result"
	sudo make install # sudo checkinstall (my preference)

And run the following as root:

	G_MESSAGES_DEBUG=all /usr/sbin/iio-sensor-proxy

The system package was actually sufficient in my case (1.3-1ubuntu1).

### Initiation

Somehow at my system it only starts working after closing and opening the lid once.

## Copyrights

This is created after `monitor-sensor.c` as example at <https://github.com/hadess/iio-sensor-proxy>. 

Copyrights of the **changes** w.r.t. the original:

* Author: Anne van Rossum
* Company: none
* Date: 28 Dec, 2016
* License (free to pick): LGPLv3, MIT, Apache 2.0

For the copyrights on the original, see code. Summarized: 2015 Bastien Nocera, LGPLv3.

Hence, complete code falls under LGPLv3.
