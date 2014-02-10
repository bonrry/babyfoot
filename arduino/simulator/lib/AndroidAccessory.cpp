/*
 * Copyright (C) 2014 Thierry Dassonville
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
#include <iostream>
using namespace std;

#include "AndroidAccessory.h"
#include "UsbVendorIds.h"

/* Android Open Accessory 1.0: http://source.android.com/tech/accessories/aoap/aoa.html */
#define PID_AOA     	  0x2d00 /* Supported - AOA only, one interface */
#define PID_AOA_ADB       0x2d01 /* Supported - AOA on first interface, second is ADB */
/* AOA 2.0 additions: http://source.android.com/tech/accessories/aoap/aoa2.html */
#define PID_AUDIO      	  0x2d02 /* Audio mode, not supported by us */
#define PID_AUDIO_ADB     0x2d03 /* Audio mode, not supported by us */
#define PID_AOA_AUDIO     0x2d04 /* Supported */
#define PID_AOA_AUDIO_ADB 0x2d05 /* Supported */

#define ACCESSORY_STRING_MANUFACTURER   0
#define ACCESSORY_STRING_MODEL          1
#define ACCESSORY_STRING_DESCRIPTION    2
#define ACCESSORY_STRING_VERSION        3
#define ACCESSORY_STRING_URI            4
#define ACCESSORY_STRING_SERIAL         5

#define ACCESSORY_GET_PROTOCOL          51
#define ACCESSORY_SEND_STRING           52
#define ACCESSORY_START                 53

#define AOA_REENUMERATION_DELAY         1  // in seconds
#define USB_WRITE_TIMEOUT             342 // in milliseconds

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
                                                         usb_handle(NULL),
                                                         connected(false),
                                                         error(0)
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
	fprintf(stderr, "Found device with AOA version %d\n", le32toh(version));
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
	libusb_claim_interface(handle, 0);
    int protocol = getProtocol(handle);

    if (protocol < 1) {
        fprintf(stderr, "Could not read device protocol version\n");
        libusb_release_interface(handle, 0);
        return false;
    }

	// Send strings to put device in AOA mode
    sendString(handle, ACCESSORY_STRING_MANUFACTURER, manufacturer);
    sendString(handle, ACCESSORY_STRING_MODEL, model);
    sendString(handle, ACCESSORY_STRING_DESCRIPTION, description);
    sendString(handle, ACCESSORY_STRING_VERSION, version);
    sendString(handle, ACCESSORY_STRING_URI, uri);
    sendString(handle, ACCESSORY_STRING_SERIAL, serial);

	fprintf(stderr, "Starting accessory mode\n");
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
	fprintf(stderr, "Started, now re-opening the device\n");
	sleep(AOA_REENUMERATION_DELAY);
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
		fprintf(stderr, "AOA configured\n");
        fprintf(stderr, "\n\n******  Press b<ENTER> or r<ENTER> to simulate a ball falling in the blue or red goal  ******\n\n");

	}
    return in && out;
}

/*
 * Is this a pid identified as an AOA pid ?
 */
bool AndroidAccessory::isAccessoryDevice(int pid)
{
	if ((pid == PID_AOA) || (pid == PID_AOA_ADB) || (pid == PID_AOA_AUDIO) || (pid == PID_AOA_AUDIO_ADB)) {
		return true;
	}
	if ((pid == PID_AUDIO) || (pid == PID_AUDIO_ADB)) {
		fprintf(stderr, "Unsupported mode: PID_AUDIO or PID_AUDIO_ADB\n");
	}
	return false;
}

void printdev(libusb_device *dev) {
	libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
		cout<<"failed to get device descriptor"<<endl;
		return;
	}
	cout<<"VendorID: "<<desc.idVendor<<"  ";
	cout<<"ProductID: "<<desc.idProduct<<endl;
    cout<<"Number of possible configurations: "<<(int)desc.bNumConfigurations<<"  ";
	cout<<"Device Class: "<<(int)desc.bDeviceClass<<endl;
	libusb_config_descriptor *config;
	libusb_get_config_descriptor(dev, 0, &config);
	cout<<"Interfaces: "<<(int)config->bNumInterfaces<<" ||| ";
	const libusb_interface *inter;
	const libusb_interface_descriptor *interdesc;
	const libusb_endpoint_descriptor *epdesc;
	for(int i=0; i<(int)config->bNumInterfaces; i++) {
		inter = &config->interface[i];
		cout<<"Number of alternate settings: "<<inter->num_altsetting<<" | ";
		for(int j=0; j<inter->num_altsetting; j++) {
			interdesc = &inter->altsetting[j];
			cout<<"Interface Number: "<<(int)interdesc->bInterfaceNumber<<" | ";
			cout<<"Number of endpoints: "<<(int)interdesc->bNumEndpoints<<" | ";
			for(int k=0; k<(int)interdesc->bNumEndpoints; k++) {
				epdesc = &interdesc->endpoint[k];
				cout<<"Descriptor Type: "<<(int)epdesc->bDescriptorType<<" | ";
				cout<<"EP Address: "<<(int)epdesc->bEndpointAddress<<" | ";
			}
		}
	}
	cout<<endl<<endl<<endl;
	libusb_free_config_descriptor(config);
}

int AndroidAccessory::detectKnownDevice(libusb_device_handle **o_handle) {
	int res = -1; // result
	libusb_device_descriptor desc;

	libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
	int r; //for return values
	ssize_t cnt; //holding number of devices in list

	//libusb_set_debug(NULL, 3); //set verbosity level to 3, as suggested in the documentation
	cnt = libusb_get_device_list(NULL, &devs); //get the list of devices
	if(cnt < 0) {
		cerr<<"Get Device Error"<<endl; //there was an error
	}
	//cerr<<cnt<<" Devices in list."<<endl; //print total number of usb devices
	unsigned long nb_devices, nb_vendors;
    // Iterate through all the devices, starting from the end as the devices is probably the last on plugged
	for(nb_devices = (unsigned long) cnt - 1; nb_devices > 0; nb_devices--) {
		libusb_device *dev = devs[nb_devices];
		r = libusb_get_device_descriptor(dev, &desc); // Get config descr
		if (r < 0) {
			cerr<<"failed to get device descriptor"<<endl;
			continue;
		}
        // Check if this device is from a known Android device manufacturer (ie appears in the list grabbed from adb source code)
        // Caution: we will match here some not-android-hardware made by samsung or asus...
        // That's why we iterate starting from the tail of the usb devices list. Still, it's dirty. Must find a better way to 
        // recognize android devices...
		for(nb_vendors = 0; nb_vendors < BUILT_IN_VENDOR_COUNT; nb_vendors++) {
			if (builtInVendorIds[nb_vendors] == desc.idVendor) {
				if (0 == libusb_open(dev, o_handle)) {
					res = desc.idProduct;
				} else {
					cerr<<"failed to open this device: ";
				}
				printdev(dev); //print specs of this device
				break;
			}
		}
		if (res != -1)
			break;
	}
	libusb_free_device_list(devs, 1); //free the list, unref the devices in it
	return res;
}

bool AndroidAccessory::configureAndroid(void)
{
	int accessory_mode = 0;
	libusb_device_handle *handle = NULL;
	
	int pid = detectKnownDevice(&handle);
	if (handle == NULL) {
		fprintf(stderr, "No device detected. Please plug your phone in...\n");
		return false;
	}
	if (isAccessoryDevice(pid)) {
		// Device was detected with an AOA registered PID
		accessory_mode = 1;
	} else {
		fprintf(stderr, "Phone detected with pid=%X\n", pid);
	}
	
	// We have a device...
	if (accessory_mode == 0) {
		// It's not in AOA mode, try to switch it
		if (!switchDevice(handle)) {
			fprintf(stderr, "failed to switch device to AOA mode\n");
			return false;
		}
		// Switched, reopen it...
	    pid = detectKnownDevice(&handle);
	    if (handle == NULL || !isAccessoryDevice(pid)) {
		    fprintf(stderr, "Device lost after switch to AOA mode, pid=%d\n", pid);
		    return false;
	    }
	}
	
	fprintf(stderr, "Found a device in accessory mode\n");
	// configure accessory
	if (findEndpoints(handle) == 0) {
		fprintf(stderr, "failed to get endpoints!\n");
	}
	int cfg = 0;
	libusb_get_configuration(handle, &cfg);
	if (cfg != 1) {
		fprintf(stderr, "Switch device to configuration 1...");
		if (libusb_set_configuration(handle, 1) != 0) {
			fprintf(stderr, "failed !!!!!\n");		
		} else {
			fprintf(stderr, " done!\n");	
		}
	}
	libusb_claim_interface(handle, 0);
	usb_handle = handle;
    return true;
}

void AndroidAccessory::disconnect(int) {
	fprintf(stderr, "Disconnect\n");
	if (usb_handle != NULL) {
		libusb_release_interface(usb_handle, 0);
		libusb_close(usb_handle);
	}
	usb_handle = NULL;
	error = 0;
	connected = false;
}

bool AndroidAccessory::isConnected(void)
{
    if (!connected && configureAndroid()) {
        connected = true;
        usleep(1000000);
    } else {
    	// Still connected ?
    	if (error != 0 && connected) {
    		fprintf(stderr, "error = %s\n", libusb_error_name(error));
	    	disconnect(LIBUSB_ERROR_NO_DEVICE);
    	}
    }
    return connected;
}

int AndroidAccessory::read(void *buff, int len, unsigned int timeout)
{
    int res = 0;
	error = libusb_bulk_transfer(usb_handle, in, (unsigned char*) buff, len, &res, timeout);
	if (error == LIBUSB_ERROR_TIMEOUT) {
		error = 0;
        return 0;
    }
	if (error == LIBUSB_ERROR_NO_DEVICE)
		disconnect(LIBUSB_ERROR_NO_DEVICE);
	return res;
}

int AndroidAccessory::write(void *buff, int len)
{
	int res = 0;
    error = libusb_bulk_transfer(usb_handle, out, (unsigned char*) buff, len, &res, USB_WRITE_TIMEOUT);
	if (error == LIBUSB_ERROR_NO_DEVICE)
		disconnect(LIBUSB_ERROR_NO_DEVICE);
    return res;
}

void AndroidAccessory::close()
{
	if (connected) {
		disconnect(error);
	}
	libusb_exit(NULL);
}
