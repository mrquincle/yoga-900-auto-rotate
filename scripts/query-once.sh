#!/bin/bash

interface=net.hadess.SensorProxy
member=AccelerometerOrientation

path="${interface//./\/}"

# Single read
qdbus --system $interface "/$path" $interface.$member

