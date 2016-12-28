/*
 * Copyright (c) 2015 Bastien Nocera <hadess@hadess.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published by
 * the Free Software Foundation.
 *
 * Adjusted for auto-rotate functionality on Yoga 900 by Anne van Rossum.
 */

#include <gio/gio.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

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

			dpy = XOpenDisplay (NULL); //open current display
			if (!dpy) { 
				g_print("Error with XOpenDisplay\n");
			}
			if (dpy)
			{
				screen = DefaultScreen(dpy);
				root = DefaultRootWindow(dpy); 
				screen_config = XRRGetScreenInfo(dpy, root);
				if ( screen_config == NULL )
				{
					g_print ( "Cannot get screen info\n" ) ;
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
