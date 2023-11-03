# Password Typer

This device emulates a USB keyboard to type a username and password automatically when a CAPS LOCK sequence is triggered.

> [!WARNING]
> It is not definitively not recommended to use this device.
> The username and password can be recovered with a mere text editor by anyone who knows the sequence...
> Pretty weak...

## Usage

1. Plug in a USB port
2. Typing starts when the CAPS LOCK key is pressed Enabled/Disabled 2 times within 1 second (by default)

> [!NOTE]
> The sequence for CAPS LOCK is: `ON` -> `OFF` -> `ON` -> `OFF`

> [!NOTE]
> The onboard LED will light-up when the first ON is detected, until the timeout expires or the sequence is completed

### Hardware

Runs on a Digispark USB (from Digistump, using an Attiny85).

No other hardware is required

## Installation/Programming

### Prerequisites
- [Digistump USB drivers](https://github.com/digistump/DigistumpArduino/releases/download/1.6.7/Digistump.Drivers.zip)
- [Visual Studio Code](https://code.visualstudio.com/)
- [PlatformIO](https://platformio.org/)

### Procedure

1. Download and open the project in VSCode/PlatformIO

1. Initialize the project (terminal within VSCode)
   ```shell
   pio project init --ide=vscode
   ```

1. Update the bootloader for a better one (faster and leaner)
   ```shell
   [PATH_TO]\.platformio\packages\tool-micronucleus\micronucleus --run --timeout 60 .\bootloader\upgrade-t85_entry_on_powerOn_activePullup_fastExit.hex
   ```

1. Build and upload the project

## Configuration

See [main.cpp](./src/main.cpp) file...

## Shout-outs
 - [ArminJo](https://github.com/ArminJo/micronucleus-firmware) - Micronucleus
 - [Danjovic](https://github.com/Danjovic/DigistumpArduino/tree/master/digistump-avr/libraries/DigisparkKeyboard) - USB Keyboard with LED
 - [Axlan](https://github.com/axlan/haunted_doll/tree/main) - USB Keyboard with LED #2
