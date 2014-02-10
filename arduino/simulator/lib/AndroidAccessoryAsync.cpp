/*
 * Copyright (C) 2013 Jerome et moi
 * Based on the work by The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "AndroidAccessory.h"

#define VID_SAMSUNG 0x04E8 /* Samsung */
#define VID_GOOGLE  0x18D1 /* Google */
#define NB_VID      2

#define PID_SAMSUNG_S3 	0x6860 /* Galaxy S3 - MTP mode */
#define PID_ADK     	0x2d00 /* ADK */
#define PID_ADK_2      	0x2D01 /* Other ADK */
#define PID_ADB     	0x2d04 /* ADB */
#define PID_ADB_2      	0x2d05 /* Other ADB */
#define NB_PID          5

#define ACCESSORY_STRING_MANUFACTURER   0
#define ACCESSORY_STRING_MODEL          1
#define ACCESSORY_STRING_DESCRIPTION    2
#define ACCESSORY_STRING_VERSION        3
#define ACCESSORY_STRING_URI            4
#define ACCESSORY_STRING_SERIAL         5

#define ACCESSORY_GET_PROTOCOL          51
#define ACCESSORY_SEND_STRING           52
#define ACCESSORY_START                 53

#define ADK_REENUMERATION_DELAY         2
#define TIMEOUT                         1

int vid[NB_VID] = { VID_SAMSUNG, VID_GOOGLE };

int pid[NB_PID] = { PID_SAMSUNG_S3, PID_ADK, PID_ADK_2, PID_ADB, PID_ADB_2 };

AndroidAccessory::AndroidAccessory(const char *manufacturer,
                                   const char *model,
                                   const char *description,
                                   const char *version,
                                   const char *uri,
                                   const char *serial) : manufacturer(manufacturer),
                                                         model(model),
                                                         description(description),
                                                         version(version),
                                                         uri(uri),
                                                         serial(serial),
                                                         connected(false),
                                                         error(0),
                                                         usb_handle(NULL)
{

}

void AndroidAccessory::powerOn(void)
{
	libusb_init(NULL);
}

int AndroidAccessory::getProtocol(libusb_device_handle *handle)
{
    uint16_t version = -1;
	int r = libusb_control_transfer(handle,
									LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR |
									LIBUSB_RECIPIENT_DEVICE, ACCESSORY_GET_PROTOCOL, 0, 0,
									(unsigned char *) &version, sizeof(version), 0);

	if (r != sizeof(version)) {
		if (r < 0)
			error = r;
		fprintf(stderr, "GetProtocol command is not supported : %d\n", version);
		return -ENOTSUP;
	}
	fprintf(stderr, "found device, ADK version is %d\n", le32toh(version));
    return version;
}

int AndroidAccessory::sendString(libusb_device_handle *handle, int index, const char *str)
{
	int len = strlen(str);
	int r = libusb_control_transfer(handle,
							LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR |
							LIBUSB_RECIPIENT_DEVICE, ACCESSORY_SEND_STRING, 0, index,
							(unsigned char *) str, len, 0);
	if (r != len) {
		if (r < 0)
			error = r;
		fprintf(stderr, "failed to send string: %s\n", str);
		return -EINVAL;
	}
	return 0;
}


bool AndroidAccessory::switchDevice(libusb_device_handle *handle)
{
	int r;
    int protocol = getProtocol(handle);

    if (protocol < 1) {
        fprintf(stderr, "could not read device protocol version\n");
        return false;
    }

	// Send strings to put device in ADK mode
    sendString(handle, ACCESSORY_STRING_MANUFACTURER, manufacturer);
    sendString(handle, ACCESSORY_STRING_MODEL, model);
    sendString(handle, ACCESSORY_STRING_DESCRIPTION, description);
    sendString(handle, ACCESSORY_STRING_VERSION, version);
    sendString(handle, ACCESSORY_STRING_URI, uri);
    sendString(handle, ACCESSORY_STRING_SERIAL, serial);

	fprintf(stderr, "starting accessory mode\n");
	r = libusb_control_transfer(handle,
									LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR |
									LIBUSB_RECIPIENT_DEVICE, ACCESSORY_START, 0, 0, NULL, 0, 0);
	if (r != 0) {
		if (r < 0)
			error = r;
		return false;
	}

	libusb_release_interface(handle, 0);
	libusb_close(handle);

	// re-open the device after enumeration
	fprintf(stderr, "started. now re-opening the device\n");
	sleep(ADK_REENUMERATION_DELAY);
	return true;
}

/*
 * Finds the first bulk IN and bulk OUT endpoints
 */
bool AndroidAccessory::findEndpoints(libusb_device_handle *handle)
{
	int i;
	struct libusb_config_descriptor 	     *cfg = NULL;
	const struct libusb_interface_descriptor *ifdsc = NULL;
	
	out = 0;
	in  = 0;
	if (0 == libusb_get_active_config_descriptor(libusb_get_device(handle), &cfg)) {
		ifdsc = &cfg->interface[0].altsetting[0];
		for (i = 0; i < ifdsc->bNumEndpoints; i++) {
			uint8_t ep = ifdsc->endpoint[i].bEndpointAddress;
			if ((ep & 0x80) && (in == 0)) { // The one with its bit 0x80 set is IN
				in = ep;
			}
			if (((ep & 0x80) == 0) && (out == 0)) { // bit 0x80 not set, must be OUT
				out = ep;
			}
		}
		libusb_free_config_descriptor(cfg);
		fprintf(stderr, "adk configured\n");
	}
    return in && out;
}

/*
 * Is this a pid identified as an ADK pid ?
 */
bool AndroidAccessory::isAccessoryDevice(int pid)
{
	
	if ((pid == PID_ADK) || (pid == PID_ADK_2)) {
		return true;
	}
	return false;
}

int AndroidAccessory::detectKnownDevice(libusb_device_handle **o_handle) {
	int i, j;
	libusb_device_handle *handle;

	// TODO: instead of this dirty stuff, do an universal detection
	// using libusb_get_device_list(libusb_context *ctx, libusb_device ***list)
	for (i = 0; i < NB_VID; ++i) {
		for (j = 0; j < NB_PID; ++j) {
			handle = libusb_open_device_with_vid_pid(NULL, vid[i], pid[j]);
			if (handle != NULL) {
				*o_handle = handle;
				return pid[j];
			}
		}
	}
	return -1;
}

bool AndroidAccessory::configureAndroid(void)
{
	int r;
	int accessory_mode = 0;
	libusb_device_handle *handle = NULL;
	
	int pid = detectKnownDevice(&handle);
	if (handle == NULL) {
		fprintf(stderr, "no known device detected\n");
		return false;
	}
	if (isAccessoryDevice(pid)) {
		// Device was detected with an ADK registered PID
		accessory_mode = 1;
	}
	
	// We have a device...
	libusb_claim_interface(handle, 0);
	if (accessory_mode == 0) {
		// It's not in ADK mode, try to switch it
		if (!switchDevice(handle)) {
			fprintf(stderr, "failed to switch device to ADK mode\n");
			return false;
		}
		// Switched, reopen it...
		return configureAndroid();
	}
	
	fprintf(stderr, "found a device in accessory mode\n");
	// configure accessory
	findEndpoints(handle);
	usb_handle = handle;
    return true;
}

void AndroidAccessory::disconnect() {
	fprintf(stderr, "disconnect\n");
	if (usb_handle != NULL) {
		libusb_close(usb_handle);
	}
	usb_handle = NULL;
	error = 0;
	connected = false;
}

bool AndroidAccessory::isConnected(void) {
    if (!connected && configureAndroid()) {
        connected = true;
    } else {
    	// Still connected ?
    	if (error == LIBUSB_ERROR_NO_DEVICE) {
	        if (connected) {
	            disconnect();
	        }
    	}
    }
    return connected;
}

int AndroidAccessory::read(void *buff, int len, unsigned int timeout) {
    int res = 0;
	error = libusb_bulk_transfer(usb_handle, in, (unsigned char*) buff, len, &res, timeout);
	if (error == LIBUSB_ERROR_TIMEOUT)
		return 0;
	if (error == LIBUSB_ERROR_NO_DEVICE)
		disconnect();
	return res;
}

int AndroidAccessory::write(void *buff, int len) {
	int res = 0;
    error = libusb_bulk_transfer(usb_handle, out, (unsigned char*) buff, len, &res, TIMEOUT);
	if (error == LIBUSB_ERROR_NO_DEVICE)
		disconnect();
    return res;
}

void AndroidAccessory::close() {
	disconnect();
	libusb_exit(NULL);
}
