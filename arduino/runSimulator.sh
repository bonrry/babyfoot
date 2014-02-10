#!/bin/sh

binary="out/babyfootSimulator"
ldvarname="LD_LIBRARY_PATH"
host=`uname`
if [ $host = "Darwin" ]; then
	ldvarname="DYLD_LIBRARY_PATH"
elif [ $host = "Linux" ]; then
	ldvarname="sudo LD_LIBRARY_PATH"
fi

if [ -f $binary ]; then
	echo "====================================================="
	echo "STARTING SIMULATOR ON $host. To quit, press Ctrl+c."
	echo "====================================================="
	echo ""
	if [ $host = "Linux" ]; then
	    echo "/!\\ Sudo may be required to open usb devices in Linux."
    	echo ""
    fi
  	eval "$ldvarname=external/libusb/libusbx-1.0.15-rc3/out/lib/ $binary"
else
	echo "=====================================================" 
	echo "    $binary binary not found." 
	echo "    To build it, run './getExternal.sh' then run 'make'" 
	echo "====================================================="
fi
