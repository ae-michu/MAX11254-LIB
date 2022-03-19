# MAX11254 Library
Arduino / PlatformIO library for the MAX11254 ADC.

## Table of contents
- [General information](#introduction)
- [Installation instructions](#installation-instructions)
- [Get started](#get-started)
    - [Declaring main object](#declare-the-main-object)
    - [Initialization](#initialization)
    - [Writing to registers](#writing-to-registers)
    - [Reading from registers](#reading-from-registers)
        - [Single register](#reading-from-single-register)
        - [All data channels](#reading-from-all-data-channels)
        - [Data ready flag](#data-ready-flag)
    - [Starting adc conversion](#starting-the-conversion)
    - [ADC modes](#adc-modes)
    - [ADC commands](#adc-commands)
    - [ADC callibration](#adc-callibration)
    - [Logging](#logging)
- [Examples](#examples)
    - [Multi channel readings](#multi-channel-readings)
    - [Fast single channel readings](#fast-single-channel-readings)
- [Contributions](#contributions)

## General information
The library was created to simplify the interaction with MAX11254 adc. It was tested on the ESP32. Other microcontrollers were not tested but it does not exclude them from working with this library. It supports all the basic functions like writing / reading from registers, specific functions for the adc and logging.

## Installation instructions
The library isn't published in the pio library registry so the easiest way to add it to your project is by using the url of this repository.
<br>
<br>
Add the url to dependecies in your `platformio.ini` file like so:
```ini
[env]
lib_deps =
    https://github.com/ae-michu/MAX11254-LIB.git
```

then include the library in your `main.cpp`:
```cpp
#include <MAX11254.h>
```

## Get started
### Declare the main object
After including the library you have to declare a class object which you will then use to execute various operations (for this example we will name our object as `adc`). It is suggested to declare it at the top of your file (global):
```cpp
MAX11254 adc(gpio_sck, gpio_miso, gpio_mosi, gpio_cs, gpio_rst,gpio_ready_pin, spi_clock, reference_V);
```

`gpio_sck` - clock pin of the SPI interface <br>
`gpio_miso` - miso pin of the SPI interface <br>
`gpio_mosi` - mosi pin of the SPI interface <br>
`gpio_cs` - chip select pin, choose any gpio pin which will be connected to cs pin of the adc <br>
`gpio_rst` - chip reset pin, choose any gpio pin which will be connected to reset pin of the adc <br>
`gpio_ready_pin` - choose any gpio pin which will be connected to ready pin of the adc <br>
`spi_clock` - integer value respresenting clock speed of the SPI interface <br>
`reference_V` - reference voltage value (used for calculating voltage from readings), value of the reference voltage provided to the adc <br>

### Initialization
Before you begin writing any operations you have to call the `begin` function. It's purpose is to set outputs, start the SPI interface and reset the adc. It should be called in the `setup()`:
```cpp
void setup() {
    adc.begin();
}
```

### Writing to registers
To configure the adc you will have to write values to registers. The library allows you to use single command `write` to do it. This approach enables ease of use and at the same time maintains allows for full user configuration.
```cpp
adc.write(register, value, size_in_bits);
```

`register` - 8bit register adress or registers macro (list of available macros below) <br>
`value` - value which should be written to register (either in hex or binary) <br>
`size_in_bits` - registers size in bits (list below) <br><br>

|   register    |   size [bit]    |   operation   |   register    |   size [bit]    |   operation   |
|---------------|-----------------|---------------|---------------|-----------------|---------------|
|   STAT        |   24            |      R        |     GPO_DIR   |   8             |      R/W      |
|   CTRL1       |   8             |      R/W      |     SOC       |   24            |      R/W      |
|   CTRL2       |   8             |      R/W      |     SCGC      |   24            |      R/W      |
|   CTRL3       |   8             |      R/W      |     DATA0     |   24            |      R        |
|   GPIO_CTRL   |   8             |      R/W      |     DATA1     |   24            |      R        |
|   DELAY       |   16            |      R/W      |     DATA2     |   24            |      R        |
|   CHMAP1      |   24            |      R/W      |     DATA3     |   24            |      R        |
|   CHMAP0      |   24            |      R/W      |     DATA4     |   24            |      R        |
|   SEQ         |   8             |      R/W      |     DATA5     |   24            |      R        |

**For more information about registers and settings which can be set please head to MAX11254s documentation.**

### Reading from registers
The library allows for reading from each register with single function `read`.

#### Reading from single register
To read single register use said function like so:
```cpp
uint32_t data = adc.read(register, size_in_bits);
```

`data` - your variable which will store the registers value <br>
`register` - register address, please refer to registers list in *Writing to registers* section for available read registers <br>
`size_in_bits` - registers size in bits (also in *Writing to registers* section list) <br>

#### Reading from all data channels
You can also read all data registers to array which allows for cleaner code. The function below will read all data channels (**if the channel isn't set in adcs settings registers then this function will return 0 in its place in the array**):
```cpp
uint32_t data[6];
adc.readAllChannels(data);
```

#### Data ready flag
The IC comes with functionality that pulls down the `data ready pin` whenever new data is available in its data registers. You can block the threads execution (wait until new data is available) with the following function:
```cpp
adc.dataReady();
```

#### Data conversion to volts
You can also convert received binary data to volts with the following example. If you use bipolar mode the function will automaticly display the value as negative / positive.
```cpp
uint32_t data =  adc.read(register, size_in_bits);
double volts = adc.getVoltage(data);
```

`data` - binary data received from adc by reading function<br>

### Starting the conversion
Before you start receving any readings you need to initiate the adc conversion.
```cpp
adc.conversion(rate);
```

`rate` - conversion speed, all available rates are listed in MAX11254s documentation<br>

### ADC modes
The adc comes with 3 modes of data acquisition. You can enable each mode by using the following function. Please read the adcs documentation to get to know what they do.
```cpp
adc.mode(number);
```

`number` - integer of value corresponding to the modes number (look list below)<br>
<br>

| number |      mode                |
|--------|--------------------------|
|   0    |  single cycle            |
|   1    |  single cycle continuous |
|   2    |  continuous              |

### ADC commands
The library also allows you to send custom 8bit commands to the adc. This can be helpful if you'd like to pass a command different then setting a `mode` (ADC recognizes mode selection if the first two msb's are 0 so the mode function inserts those first two bits for you).
```cpp
adc.command(cmd);
```

`cmd` - 8bit value command to be sent do ADC (refer to MAX11254s documentation) <br>

### ADC callibration
The adc enables you to use 3 callibration modes: no callibration, self-callibration and full system callibration (which includes self-callibration, offset callibration and gain callibration). To enable it use the following command:
```cpp
adc.callibration(number);
```

`number` - integer value corresponding to the selected callibration (numbering displayed below) <br>
<br>

| number |      callibration type   |
|--------|--------------------------|
|   0    |  no callibration         |
|   1    |  self-calibration        |
|   2    |  full system calibration |

### Logging
To make your life easier the library also includes a logging function which prints whole values in binary (including 0 in the first positions, standard `Serial.print` function doesn't do that).
```cpp
adc.log(label, data, data_length);
```

`label` - string which will be printed in front of the logged value<br>
`data` - data to be logged (max 32 bit)<br>
`data_length` - integer value corresponding to data length (eg. 8/16/24/32)<br>

<br>

## Examples
### Multi channel readings
```cpp
#include <Arduino.h>
#include <SPI.h>
#include <MAX11254.h>

MAX11254 adc(11, 12, 13, 15, 9, 10, 1000000, 5);

uint32_t data[6];

void setup() {
    Serial.begin(115200);

    adc.begin();

    // configuring the chip
    adc.write(SEQ, 0xF2, 8);
    adc.write(CTRL3, 0x5C, 8);
    adc.write(CHMAP0, 0xE0A06, 24);
    adc.write(CHMAP1, 0x1A1612, 24);
    adc.write(CTRL2, 0x3F, 8);

    adc.mode(1);
}

void loop() {
    adc.conversion(0xF);
    adc.dataReady();

    adc.readAllChannels(data);

    for (int i = 0; i < 6; ++i) {
        adc.log("data" + String(i) + ": ", data[i], 24);
    }
}
```

### Fast single channel readings
```cpp
#include <Arduino.h>
#include <SPI.h>
#include <MAX11254.h>

MAX11254 adc(11, 12, 13, 15, 9, 10, 1000000, 5);

void setup() {
    Serial.begin(115200);

    adc.begin();

    // configuring the chip
    adc.write(SEQ, 0xA3, 8);
    adc.write(CHMAP0, 0xE0A06, 24);
    adc.write(CHMAP1, 0x1A1612, 24);
    adc.write(CTRL1, 0x5, 8);
    adc.write(CTRL2, 0x6F, 8);
    adc.write(CTRL3, 0x5C, 8);

    adc.command(0xAF);
}

void loop() {
    adc.readAllChannels(data);

    for (int i = 0; i < 6; ++i) {
        Serial.print("data" + String(i) + ": ");
        Serial.println(adc.getVoltage(data[i]), 10);
    }

    adc.dataReady();

    // we read all the registers but you will see output only on the first register
    // this is because the sequencer in the fastest setting can only read from one channel
}
```

## Contributions
All contributions are welcome :) :fire: <br>

<table>
<td align="center"><a href="https://github.com/MikePaq"><img src="https://avatars.githubusercontent.com/u/90452066?v=4" width="100px;" alt=""/><br /><sub><b>MikePaq</b></sub></a><br /><p style="font-size:10px">Contributed unipolar/bipolar mode</p></td>
</table>

<br>
<br>

### Legal note
THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.