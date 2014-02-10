/*
 * Copied from : system/core/adb/usb_vendors.c
 */

#ifndef __UsbVendorIds_h__
#define __UsbVendorIds_h__

// Google's USB Vendor ID
#define VENDOR_ID_GOOGLE        0x18d1
// Intel's USB Vendor ID
#define VENDOR_ID_INTEL         0x8087
// HTC's USB Vendor ID
#define VENDOR_ID_HTC           0x0bb4
// Samsung's USB Vendor ID
#define VENDOR_ID_SAMSUNG       0x04e8
// Motorola's USB Vendor ID
#define VENDOR_ID_MOTOROLA      0x22b8
// LG's USB Vendor ID
#define VENDOR_ID_LGE           0x1004
// Huawei's USB Vendor ID
#define VENDOR_ID_HUAWEI        0x12D1
// Acer's USB Vendor ID
#define VENDOR_ID_ACER          0x0502
// Sony Ericsson's USB Vendor ID
#define VENDOR_ID_SONY_ERICSSON 0x0FCE
// Foxconn's USB Vendor ID
#define VENDOR_ID_FOXCONN       0x0489
// Dell's USB Vendor ID
#define VENDOR_ID_DELL          0x413c
// Nvidia's USB Vendor ID
#define VENDOR_ID_NVIDIA        0x0955
// Garmin-Asus's USB Vendor ID
#define VENDOR_ID_GARMIN_ASUS   0x091E
// Sharp's USB Vendor ID
#define VENDOR_ID_SHARP         0x04dd
// ZTE's USB Vendor ID
#define VENDOR_ID_ZTE           0x19D2
// Kyocera's USB Vendor ID
#define VENDOR_ID_KYOCERA       0x0482
// Pantech's USB Vendor ID
#define VENDOR_ID_PANTECH       0x10A9
// Qualcomm's USB Vendor ID
#define VENDOR_ID_QUALCOMM      0x05c6
// On-The-Go-Video's USB Vendor ID
#define VENDOR_ID_OTGV          0x2257
// NEC's USB Vendor ID
#define VENDOR_ID_NEC           0x0409
// Panasonic Mobile Communication's USB Vendor ID
#define VENDOR_ID_PMC           0x04DA
// Toshiba's USB Vendor ID
#define VENDOR_ID_TOSHIBA       0x0930
// SK Telesys's USB Vendor ID
#define VENDOR_ID_SK_TELESYS    0x1F53
// KT Tech's USB Vendor ID
#define VENDOR_ID_KT_TECH       0x2116
// Asus's USB Vendor ID
#define VENDOR_ID_ASUS          0x0b05
// Philips's USB Vendor ID
#define VENDOR_ID_PHILIPS       0x0471
// Texas Instruments's USB Vendor ID
#define VENDOR_ID_TI            0x0451
// Funai's USB Vendor ID
#define VENDOR_ID_FUNAI         0x0F1C
// Gigabyte's USB Vendor ID
#define VENDOR_ID_GIGABYTE      0x0414
// IRiver's USB Vendor ID
#define VENDOR_ID_IRIVER        0x2420
// Compal's USB Vendor ID
#define VENDOR_ID_COMPAL        0x1219
// T & A Mobile Phones' USB Vendor ID
#define VENDOR_ID_T_AND_A       0x1BBB
// LenovoMobile's USB Vendor ID
#define VENDOR_ID_LENOVOMOBILE  0x2006
// Lenovo's USB Vendor ID
#define VENDOR_ID_LENOVO        0x17EF
// Vizio's USB Vendor ID
#define VENDOR_ID_VIZIO         0xE040
// K-Touch's USB Vendor ID
#define VENDOR_ID_K_TOUCH       0x24E3
// Pegatron's USB Vendor ID
#define VENDOR_ID_PEGATRON      0x1D4D
// Archos's USB Vendor ID
#define VENDOR_ID_ARCHOS        0x0E79
// Positivo's USB Vendor ID
#define VENDOR_ID_POSITIVO      0x1662
// Fujitsu's USB Vendor ID
#define VENDOR_ID_FUJITSU       0x04C5
// Lumigon's USB Vendor ID
#define VENDOR_ID_LUMIGON       0x25E3
// Quanta's USB Vendor ID
#define VENDOR_ID_QUANTA        0x0408
// INQ Mobile's USB Vendor ID
#define VENDOR_ID_INQ_MOBILE    0x2314
// Sony's USB Vendor ID
#define VENDOR_ID_SONY          0x054C
// Lab126's USB Vendor ID
#define VENDOR_ID_LAB126        0x1949
// Yulong Coolpad's USB Vendor ID
#define VENDOR_ID_YULONG_COOLPAD 0x1EBF
// Kobo's USB Vendor ID
#define VENDOR_ID_KOBO          0x2237
// Teleepoch's USB Vendor ID
#define VENDOR_ID_TELEEPOCH     0x2340
// AnyDATA's USB Vendor ID
#define VENDOR_ID_ANYDATA       0x16D5
// Harris's USB Vendor ID
#define VENDOR_ID_HARRIS        0x19A5
// OPPO's USB Vendor ID
#define VENDOR_ID_OPPO          0x22D9
// Xiaomi's USB Vendor ID
#define VENDOR_ID_XIAOMI        0x2717
// BYD's USB Vendor ID
#define VENDOR_ID_BYD           0x19D1


/** built-in vendor list */
int builtInVendorIds[] = {
    VENDOR_ID_GOOGLE,
    VENDOR_ID_INTEL,
    VENDOR_ID_HTC,
    VENDOR_ID_SAMSUNG,
    VENDOR_ID_MOTOROLA,
    VENDOR_ID_LGE,
    VENDOR_ID_HUAWEI,
    VENDOR_ID_ACER,
    VENDOR_ID_SONY_ERICSSON,
    VENDOR_ID_FOXCONN,
    VENDOR_ID_DELL,
    VENDOR_ID_NVIDIA,
    VENDOR_ID_GARMIN_ASUS,
    VENDOR_ID_SHARP,
    VENDOR_ID_ZTE,
    VENDOR_ID_KYOCERA,
    VENDOR_ID_PANTECH,
    VENDOR_ID_QUALCOMM,
    VENDOR_ID_OTGV,
    VENDOR_ID_NEC,
    VENDOR_ID_PMC,
    VENDOR_ID_TOSHIBA,
    VENDOR_ID_SK_TELESYS,
    VENDOR_ID_KT_TECH,
    VENDOR_ID_ASUS,
    VENDOR_ID_PHILIPS,
    VENDOR_ID_TI,
    VENDOR_ID_FUNAI,
    VENDOR_ID_GIGABYTE,
    VENDOR_ID_IRIVER,
    VENDOR_ID_COMPAL,
    VENDOR_ID_T_AND_A,
    VENDOR_ID_LENOVOMOBILE,
    VENDOR_ID_LENOVO,
    VENDOR_ID_VIZIO,
    VENDOR_ID_K_TOUCH,
    VENDOR_ID_PEGATRON,
    VENDOR_ID_ARCHOS,
    VENDOR_ID_POSITIVO,
    VENDOR_ID_FUJITSU,
    VENDOR_ID_LUMIGON,
    VENDOR_ID_QUANTA,
    VENDOR_ID_INQ_MOBILE,
    VENDOR_ID_SONY,
    VENDOR_ID_LAB126,
    VENDOR_ID_YULONG_COOLPAD,
    VENDOR_ID_KOBO,
    VENDOR_ID_TELEEPOCH,
    VENDOR_ID_ANYDATA,
    VENDOR_ID_HARRIS,
    VENDOR_ID_OPPO,
    VENDOR_ID_XIAOMI,
    VENDOR_ID_BYD,
};

#define BUILT_IN_VENDOR_COUNT    (sizeof(builtInVendorIds)/sizeof(builtInVendorIds[0]))

#endif /* __UsbVendorIds_h__ */