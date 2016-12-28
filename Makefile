#!/bin/make

all:
	gcc auto-rotate.c `pkg-config --cflags --libs glib-2.0 gio-2.0 xrandr x11` -o auto-rotate

install:
	cp auto-rotate /usr/local/bin
