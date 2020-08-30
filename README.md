# DAB-OWRT-A64 industrial router

## Linux build instructions (from QorIQ SDK 2.0)

Default configuration: ls1012a_a64_defconfig

Standalone Linux build:
```
make mrproper
ARCH=arm64 make ls1012a_a64_defconfig
CROSS_COMPILE=aarch64-linux-gnu- ARCH=arm64 make -j 8
```
