/*
 * Copyright (c) 2017 Anne van Rossum <https://annevanrossum.com>
 * Copyright (C) 2013 Shih-Yuan Lee (FourDollars) <sylee@canonical.com>
 * Copyright (c) 2015 Bastien Nocera <hadess@hadess.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published by
 * the Free Software Foundation.
 *
 * Original code is by Bastien Nocera from the monitor-sensor.c example at 
 * <https://github.com/hadess/iio-sensor-proxy>. 
 * It is adjusted for auto-rotate functionality on Yoga 900 by Anne van Rossum.
 * It is subsequently adjusted on request to rotate any touch device by Anne van Rossum
 * using code from Shih-Yuan Lee at <https://github.com/fourdollars/x11-touchscreen-calibrator>
 */

#include <gio/gio.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include <X11/extensions/XInput2.h>

static GMainLoop *loop;
static guint watch_id;
static GDBusProxy *iio_proxy, *iio_proxy_compass;

static int verbose = 0;

static void
properties_changed (GDBusProxy *proxy,
		    GVariant   *changed_properties,
		    GStrv       invalidated_properties,
		    gpointer    user_data)
{
	GVariant *v;
	GVariantDict dict;

	g_variant_dict_init (&dict, changed_properties);

	if (g_variant_dict_contains (&dict, "AccelerometerOrientation")) {
		v = g_dbus_proxy_get_cached_property (iio_proxy, "AccelerometerOrientation");

		Rotation new_rotation;

		char recognized = 1;

		if (g_strcmp0 ( g_variant_get_string (v, NULL), "normal") == 0) {
			new_rotation = RR_Rotate_0;			
		} else if (g_strcmp0 ( g_variant_get_string (v, NULL), "left-up") == 0) {
			new_rotation = RR_Rotate_90;			
		} else if (g_strcmp0 ( g_variant_get_string (v, NULL), "bottom-up") == 0) {
			new_rotation = RR_Rotate_180;			
		} else if (g_strcmp0 ( g_variant_get_string (v, NULL), "right-up") == 0) {
			new_rotation = RR_Rotate_270;			
		} else {
			recognized = 0;
		}
			
		if (recognized) {
			XRROutputChangeNotifyEvent xrrchange;
			XGravityEvent xgrav;

			Display *dpy;
			Window root;
			int screen;
			XRRScreenConfiguration * screen_config;
			Rotation rotation, current_rotation;
			SizeID current_size;

			dpy = XOpenDisplay (NULL); 
			if (!dpy) { 
				g_print("Error with XOpenDisplay\n");
				return;
			}
			if (dpy)
			{
				screen = DefaultScreen(dpy);
				root = DefaultRootWindow(dpy); 
				screen_config = XRRGetScreenInfo(dpy, root);
				if ( screen_config == NULL ) {
					g_print ( "Cannot get screen info\n" ) ;
					return;
				}
			}

			rotation = XRRRotations (dpy, screen, &current_rotation);
			current_size = XRRConfigCurrentConfiguration (screen_config, &current_rotation);

			if (verbose > 0) {
				g_print(" %d	.. XGravityEvent X Origin\n",xgrav.x);
				g_print(" %d	.. XGravityEvent Y Origin\n",xgrav.y);
				g_print(" %s	.. Display Name\n",XDisplayName((char*)dpy));
				g_print(" %d	.. Screen Number\n",screen);
				g_print(" %d	.. XRROutputChangeNotifyEvent Rotation\n",xrrchange.rotation);
				g_print(" %x	.. XRRRotations Bitmask\n", current_rotation);
				g_print(" %d	.. XRRRotations Integer\n", current_rotation);
			}

			switch (current_rotation)
			{
				case RR_Rotate_0:
					g_print("Current X Rotation is normal\n");
					break;
				case RR_Rotate_90:
					g_print("Current X Rotation is left\n");
					break;
				case RR_Rotate_180:
					g_print("Current X Rotation is upside down\n");
					break;
				case RR_Rotate_270:
					g_print("Current X Rotation is right\n");
					break;
				default:
					g_print("Error with value of current_rotation\n");
			}

			switch (new_rotation)
			{
				case RR_Rotate_0:
					g_print("New X Rotation is normal\n");
					break;
				case RR_Rotate_90:
					g_print("New X Rotation is left\n");
					break;
				case RR_Rotate_180:
					g_print("New X Rotation is upside down\n");
					break;
				case RR_Rotate_270:
					g_print("New X Rotation is right\n");
					break;
				default:
					g_print("Error with value of new_rotation\n");
			}
					
			XRRSetScreenConfig(dpy, screen_config, root, current_size, new_rotation, CurrentTime);

			// rotate also input devices with XITouchClass and XIDirectTouch mode
			float rotate0[9] = {
				1.0f, 0   , 0,
				0   , 1.0f, 0,
				0   , 0   , 1.0f
			};
			float rotate90[] = {
				0   , -1.0f, 1.0f,
				1.0f, 0    , 0,
				0   , 0    , 1.0f
			};
			float rotate180[] = {
				-1.0f, 0    , 1.0f,
				0    , -1.0f, 1.0f,
				0    , 0    , 1.0f
			};
			float rotate270[] = {
				0    , 1.0f, 0,
				-1.0f, 0   , 1.0f,
				0    , 0   , 1.0f
			};
			float reflectX[] = {
				-1.0f, 0   , 1.0f,
				0    , 1.0f, 0,
				0    , 0   , 1.0f
			};
			float reflectY[] = {
				1.0f, 0    , 0,
				0   , -1.0f, 1.0f,
				0   , 0    , 1.0f
			};

			union {
				unsigned char* _char;
				float *_float;
			} coordinates;

			Atom prop_float;
			Atom prop_matrix;
			Atom type_return;
	
			XIDeviceInfo *info = NULL;
			XIDeviceInfo *device = NULL;
			XITouchClassInfo *touch = NULL;
			int num_devices, i, j, device_id, ret, format_return;
			unsigned long nitems, bytes_after;
			if (dpy) {
				prop_float = XInternAtom(dpy, "FLOAT", False);
				if (prop_float == None) {
					g_print("ERROR : Float atom not found..\n");
					return;
				}
				prop_matrix = XInternAtom(dpy, "Coordinate Transformation Matrix", False);
				if (prop_matrix == None) {
					g_print("ERROR : Coordinate Transformation Matrix property not found.\n");
					return;
				}
				info = XIQueryDevice(dpy, XIAllDevices, &num_devices);
				for (i = 0; i < num_devices; ++i) {
					device = info + i;
					for (j = 0; j < device->num_classes; ++j) {
						touch = (XITouchClassInfo*) device->classes[j];
						if (touch->type == XITouchClass || touch->type == XIValuatorClass || 
								touch->type == XIScrollClass) {
							device_id = device->deviceid;

							ret = XIGetProperty(dpy, device_id, prop_matrix, 0, 9, False, prop_float,
									&type_return, &format_return, &nitems, &bytes_after, &coordinates._char);
							if (ret != Success || prop_float != type_return || format_return != 32 ||
									nitems != 9 || bytes_after != 0)
							{
								g_print("ERROR : Failed to return property values.\n");
								return;
							}

							switch (new_rotation)
							{
								case RR_Rotate_0:
									coordinates._float = rotate0;
									break;
								case RR_Rotate_90:
									coordinates._float = rotate90;
									break;
								case RR_Rotate_180:
									coordinates._float = rotate180;
//									coordinates._float = reflectX;
									break;
								case RR_Rotate_270:
									coordinates._float = rotate270;
									break;
								default:
									g_print("Error with value of new_rotation\n");
							}
							
							XIChangeProperty(dpy, device_id, prop_matrix, prop_float, format_return, 
									PropModeReplace, coordinates._char, nitems);
						}
					}
				}
				XIFreeDeviceInfo(info);
			}
		}
						
		g_print ("    Accelerometer orientation changed: %s\n", g_variant_get_string (v, NULL));
		g_variant_unref (v);
	}
}

static void
appeared_cb (GDBusConnection *connection,
	     const gchar     *name,
	     const gchar     *name_owner,
	     gpointer         user_data)
{
	GError *error = NULL;

	g_print ("+++ iio-sensor-proxy appeared\n");

	iio_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
						   G_DBUS_PROXY_FLAGS_NONE,
						   NULL,
						   "net.hadess.SensorProxy",
						   "/net/hadess/SensorProxy",
						   "net.hadess.SensorProxy",
						   NULL, NULL);

	g_signal_connect (G_OBJECT (iio_proxy), "g-properties-changed",
			  G_CALLBACK (properties_changed), NULL);

	/* Accelerometer */
	g_dbus_proxy_call_sync (iio_proxy,
				"ClaimAccelerometer",
				NULL,
				G_DBUS_CALL_FLAGS_NONE,
				-1,
				NULL, &error);
	g_assert_no_error (error);
}

static void
vanished_cb (GDBusConnection *connection,
	     const gchar *name,
	     gpointer user_data)
{
	if (iio_proxy || iio_proxy_compass) {
		g_clear_object (&iio_proxy);
		g_clear_object (&iio_proxy_compass);
		g_print ("--- iio-sensor-proxy vanished, waiting for it to appear\n");
	}
}

int main (int argc, char **argv)
{
	if (argc > 1) {
		if (g_strcmp0(argv[1], "--verbose") == 0) {
			verbose = 1;
		}
	}

	if (!verbose) {
		daemon(0, 0);
	}

	watch_id = g_bus_watch_name (G_BUS_TYPE_SYSTEM,
				     "net.hadess.SensorProxy",
				     G_BUS_NAME_WATCHER_FLAGS_NONE,
				     appeared_cb,
				     vanished_cb,
				     NULL, NULL);

	g_print ("    Waiting for iio-sensor-proxy to appear\n");
	loop = g_main_loop_new (NULL, TRUE);
	g_main_loop_run (loop);

	return 0;
}
