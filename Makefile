#!/bin/make

all: build

build: auto-rotate

src = auto-rotate.c

auto-rotate: $(src)
	gcc $(src) `pkg-config --cflags --libs glib-2.0 gio-2.0 xrandr x11` -o $@

install: build
	cp auto-rotate /usr/local/bin

config-systemd:
	mkdir -p ~/.config/systemd/user
	cp auto-rotate.service ~/.config/systemd/user/
	systemctl --user enable auto-rotate
	systemctl --user start auto-rotate
	systemctl --user status auto-rotate
	loginctl enable-linger $(USER)

config-systemv:
	echo "Use systemd instead"
	sudo cp auto-rotate.sh /etc/init.d/
	echo "You still have to adjust the run-levels"

run: build
	echo "Make sure you have closed the lid or suspended first: systemctl suspend"
	./auto-rotate --verbose

.PHONY: all install config-systemd config-systemv 
