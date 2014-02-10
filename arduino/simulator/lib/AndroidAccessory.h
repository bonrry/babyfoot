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

#ifndef __AndroidAccessory_h__
#define __AndroidAccessory_h__

#ifdef __APPLE__ // defined(HAVE_COREFOUNDATION_COREFOUNDATION_H)
	#include <CoreFoundation/CoreFoundation.h>
	#define htole32(x) CFSwapInt32HostToLittle(x)
	#define le32toh(x) CFSwapInt32LittleToHost(x)
#endif


#include <libusb.h>

class AndroidAccessory {

private:
    const char *manufacturer;
    const char *model;
    const char *description;
    const char *version;
    const char *uri;
    const char *serial;


	libusb_device_handle *usb_handle;
    bool connected;
    uint8_t in;
    uint8_t out;
    int error;

	bool isAccessoryDevice(int pid);
    int  getProtocol(libusb_device_handle *handle);
    int  sendString(libusb_device_handle *handle, int index, const char *str);
    bool switchDevice(libusb_device_handle *handle);
    bool findEndpoints(libusb_device_handle *handle);
    bool configureAndroid(void);
    int detectKnownDevice(libusb_device_handle **o_handle);
    void disconnect(int errCode);

public:
    AndroidAccessory(const char *manufacturer,
                     const char *model,
                     const char *description,
                     const char *version,
                     const char *uri,
                     const char *serial);

    void powerOn(void);

    bool isConnected(void);
    int read(void *buff, int len, unsigned int timeout);
    int write(void *buff, int len);
    void close();

};

#endif /* __AndroidAccessory_h__ */
