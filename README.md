# DESCRIPTION
The goal of this project is to construct lightweight blinker. It offers frequency adjustment, low-power modes and much more.
# FEATURES
The device is fulfills the following parameters:
- High refresh rate:
	- The device lets users choose their own frequency. Users have to manually calculate the period and the prescaller. The equation is pretty easy:  $$f_o = f_i / (prescaller + 1) / (period + 1)$$
	- where f_o stands for output frequency, f_i stands for CPU clock frequency (72 MHz on STM32F303RE).
- Push button reactions:
	- The device is equipped with push button. When user press the button, the LED refresh rate changes. The device has three modes in which frequency may differ.
- UART2 commands:
	- The device handles UART2 commands. All the command manuals and help is embedded in the device.
> **Note:** Type 'help' to receive quick help
# EXECUTING
This application is based on STM32F3 microcontroller family. When developing, you should not forget to:
- Correctly wire hardware LED, push button etc.
> **Note:** Recommended hardware setup is in hardware/v0/schematics.pdf. Needed parts can be found in **DODELEJ**
- Install STM32 Cube IDE
	- see download page https://www.st.com/en/development-tools/stm32cubeide.html and installation guide https://www.st.com/resource/en/user_manual/um2563-stm32cubeide-installation-guide-stmicroelectronics.pdf
- Install STM32 Cube IDE toolchain
	- You can follow this guide: https://wiki.st.com/stm32mcu/wiki/STM32StepByStep:Step1_Tools_installation, but PLEASE DO NOT FORGET TO DOWNLOAD STM32CUBEF3 package
- Build, flash and debug the binary
> **Note:** Version 1 has different MCU (Version 0 uses STM32F303RE, however version 1 uses STM32F042)
# PROJECT STRUCTURE
## HARDWARE
In this folder is stored hardware reference of the device. You can find here schematics and much more.
## SOFTWARE
In this folder is stored STM32 Cube IDE project with firmware for the device. Feel free to inspect.
## DOCS
In the docs folder are stored measurements of device properties as well as alternative solutions
# PROJECT VERSIONS

| Version number | Version description          |
| -------------- | ---------------------------- |
| 0              | Created on prototyping board |
| 1              | PCB available                |
> **Note:** Version 0 is the hihest version available.
# CONTACTS
- Jakub Kráľ
	- email: jakub6kral@centrum.cz
	- feel free to contact if any issues/misunderstandings appeared
- Ing. Libor Wagner
	- email: Libor.Wagner@cvut.cz
# TO-DO
