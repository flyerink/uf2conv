# uf2conf

uf2conf, a tool for convert binary to uf2 format. If already in BOOT mode, audo copy new .uf2 to BOOT disk to upgrade.

Version 0.2.1

usage:
    uf2conv.exe [-s 2000|4000] -i flash.bin [-o flash.uf2]

Options:
-    -b --base 2000|4000     Set base address in hex for binary file (default: 2000)
-    -f --family SAMD21/SAML21/SAMD51/NRF52/STM32F1/STM32F4/ATMEGA32/MIMXRT10XX          <bl>Set the device family name, (default: 0)
-    -i --input flash.bin    Set the input file name to be convert, mandory
-    -o --output flash.uf2   Set the output file name, default: flash.uf2
-    -h --help               Display help information
-    -v --version            Show the program version
