# uf2conv

uf2conv, a tool for convert binary to uf2 format. If already in BOOT mode, will copy new .uf2 to BOOT disk to upgrade.

Version 0.3.0

usage:
    uf2conv.exe [-b 2000|4000] [-f FAMILY][-o flash.uf2] flash.bin

Options:
-    -b --base 2000|4000     Set base address in hex for binary file (default: 2000)
-    -f --family SAMD21/SAML21/SAMD51/NRF52/STM32F1/STM32F4/ATMEGA32/MIMXRT10XX <br>Set the device family name, (default: 0)</br>
-    -o --output flash.uf2   Set the output file name, default: flash.uf2
-    -h --help               Display help information
-    -v --version            Show the program version
