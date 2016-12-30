#!/bin/bash

interface=org.freedesktop.DBus.Properties
member=PropertiesChanged
string=AccelerometerOrientation

# Get the second line after $string
i=0
dbus-monitor --system --monitor "type='signal',interface='$interface',member='$member'" |
while read -r line; do
  if [ "$i" -eq 1 ]; then
    i=0 
    orientation=$(echo $line | cut -f3 -d' ')
    echo $orientation
  fi
  accelero=$(echo $line | grep $string)
  if [ -n "${accelero}" ]; then
    ((i++))
  fi
done

