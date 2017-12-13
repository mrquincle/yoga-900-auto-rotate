# What does this do?

This code implements auto-rotate functionality on the Yoga 900 by coupling iio-sensor-proxy to xrandr.

## State

Still works perfectly. Last compiled and tested: Dec. 2017.

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

## Tests

Regarding battery use, the auto-rotate daemon is compared to a script (see `/scripts`) that use dbus-monitor in a (blocking) read loop.

Results:


| description         | auto-rotate   | dbus-monitor  |
| ------------------- | ------------- | ------------- |
| system calls        | 199           | 39284         | 
| system calls (read) | 6             | 27276    | 
| cpu (sys)           | 0.01%         | 0.20%         | 
| cpu (user)          | 0.00%         | 0.04%         | 
| context switches    | 3.52/sec      | 666.13/sec    | 


## Copyrights

This is created after `monitor-sensor.c` as example at <https://github.com/hadess/iio-sensor-proxy>.

Copyrights of the **changes** w.r.t. the original:

* Author: Anne van Rossum
* Company: none
* Date: 28 Dec, 2016
* License (free to pick): LGPLv3, MIT, Apache 2.0

For the copyrights on the original, see code. Summarized: 2015 Bastien Nocera, LGPLv3.

Hence, complete code falls under LGPLv3.

## Acceptance mainstream

It is reported as tip at https://github.com/hadess/iio-sensor-proxy/issues/129. The author of iio-sensor-proxy assumes a desktop developer should be the person to integrate it. The code in this repository directly uses X through xrandx, so can be used by many desktops as long as they use X. However, I have no clue who I should contact to get a working auto-rotate function the default experience for everyone using Linux on a Yoga 900. Help to get it in into any OS appreciated!

