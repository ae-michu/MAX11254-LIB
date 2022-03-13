#include "MAX11254.h"

MAX11254 :: MAX11254(int8_t p_sck, int8_t p_miso, int8_t p_mosi, int8_t p_cs, int8_t p_rst, int8_t p_rdy, uint32_t p_spiClock, float referenceV)
{
    sck = p_sck;
    miso = p_miso;
    mosi = p_mosi;
    cs = p_cs;
    rst = p_rst;
    rdy = p_rdy;
    spiClock = p_spiClock;
    referenceVoltage = referenceV;
}

// Reset chip to its default settings by power cycling reset pin (necessary at start)
void MAX11254 :: reset() {
    digitalWrite(rst, LOW);
    delay(100);
    digitalWrite(rst, HIGH);
}

// Initiate MAX11254 with settings (pins) provided in class declaration.
// - sets pins as inputs / outputs
// - resets adc (by power cycling and resetting its spi controller)
// - starts SPI with provided settings
void MAX11254 :: begin() {
    pinMode(rst, OUTPUT);
    pinMode(rdy, INPUT);
    pinMode(cs, OUTPUT);

    reset();

    digitalWrite(cs, LOW);                  // select and deselect chip to reset spi controller
    delay(100);
    digitalWrite(cs, HIGH);

    SPI.begin(sck, miso, mosi, cs);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV16);
    SPI.setDataMode(SPI_MODE0);
}

/** Write data to specified register
* @param reg registers address
* @param data data to be written
* @param length registers length (eg. 8bit register - length = 8)
*/
void MAX11254 :: write(uint8_t reg, uint32_t data, int length) {
    uint8_t address = MSBS | (reg << 1) | WBIT;
    
    digitalWrite(cs, LOW);
    SPI.transfer(address);

    for (int i = (length - 8); i >= 0; i -= 8) {
        uint8_t data_to_send = ((data >> i) & 0xFF);
        SPI.transfer(data_to_send);
    }

    digitalWrite(cs, HIGH);
}

/** Read data from specified register. Returns 32bit integer.
* @param reg registers address
* @param length registers length / length of data to be read in bits
*/
uint32_t MAX11254 :: read(uint8_t reg, int length) {
    uint32_t data = 0;
    uint8_t address =  MSBS | (reg << 1) | RBIT;

    digitalWrite(cs, LOW);
    SPI.transfer(address);

    for (int i = 0; i < length; i += 8) {
        data = (data << 8) | SPI.transfer(0);
    }
    
    digitalWrite(cs, HIGH);
    return data;
}

// Blocks thread until data ready pin signals ( 1 -> 0) that data is ready.
// It's required to run this command before reading new values from registers
// (once each 5 registers (or less as specified in configuration))
bool MAX11254 :: dataReady() {
    while (digitalRead(rdy));
    return true;
}

bool MAX11254 :: isPositive(uint32_t data, int length)
{
    if(bitRead(data,length-1) == 1)
    return 1;
    else
    return 0; 
}

/** Sends specified command to MAX11254
* @param cmd command to be sent
*/
void MAX11254 :: command(uint8_t cmd) {
    digitalWrite(cs, LOW);
    SPI.transfer(cmd);
    digitalWrite(cs, HIGH);
}

/** Logs provided data to serial (whole values including cases where msb starts with 0)
* @param text label of logged data (printed before value)
* @param data data to be logged
* @param length length of data in bits (eg. 8bit value - length = 8)
*/
void MAX11254 :: log(String text, uint32_t data, int length) {
  Serial.print(text);

  for (int i = (length - 1); i >= 0; i--) {
    Serial.print(bitRead(data, i));
  }

  Serial.println("");
}

/** Performs adc calibration
 * @param type provide number corresponding to the type of calibration that should be performed
 * 0 - no calibration (all calibration is disabled),
 * 1 - self-calibration,
 * 2 - full system calibration (self-calibration, offset calibration, gain calibration)
*/
void MAX11254 :: callibration(int type) {
    switch (type)
    {
    case 0:
        write(CTRL3, 0x0F, 8);
        break;
    case 1:
        write(CTRL3, 0x0C, 8);
        write(CTRL1, 0x02, 8);
        command(0xA0);
        delay(200);
        break;
    case 2:
        write(CTRL3, 0x00, 8);
        read(SOC, 24);
        read(SCGC, 24);

        write(CTRL1, 0x42, 8);
        command(0xA0);
        delay(200);

        delay(6);
        write(CTRL1, 0x82, 8);
        command(0xA0);
        delay(200);

        read(SOC, 24);
        read(SCGC, 24);
        break;
    default:
        break;
    }
}

/** Enables specified operation mode
 * @param type provide number corresponding to the type of operation that sould be used
 * 0 - single cycle,
 * 1 - single cycle continuous,
 * 2 - continuous
*/
void MAX11254 :: mode(int type) {
    uint8_t regVal = read(CTRL1, 8);
    regVal = ((regVal >> 2) << 2);

    switch (type)
    {
    case 0:
        regVal = regVal | 0x02;
        break;
    case 1:
        regVal = regVal | 0x03;
        break;
    case 2:
        regVal = regVal | 0x00;
        break;
    default:
        break;
    }
    
    write(CTRL1, regVal, 8);
}

/** Starts conversion with specified rate
 * @param rate 4 bit conversion rate value (based on table.1. in max11254 documentation) 
*/
void MAX11254 :: conversion(uint8_t rate) {
    uint8_t comm = 0xB0 | rate;
    command(comm);
}

/** Reads all available data channels.
 * CHMAP0s and CHMAP1s order bits must be set to read channels in order from 0 to 5.
 * ex. CHMAP0 = 0xE0A06; CHMAP1 = 0x1A1612 
 * @param data pointer to data array of size 6, data will be read to this array,
 * positions in array correspond to data channels number
*/
void MAX11254 :: readAllChannels(uint32_t *data) {
    data[0] = read(DATA0, 24);
    data[1] = read(DATA1, 24);
    data[2] = read(DATA2, 24);
    data[3] = read(DATA3, 24);
    data[4] = read(DATA4, 24);
    data[5] = read(DATA5, 24);
}

double MAX11254 :: getVoltage(uint32_t data) {
    if(bitRead(read(CTRL1, 8), 2) == 1) {
        double valueRead = data;
        double maxADCValue = 16777215;

        return (valueRead / (maxADCValue / referenceVoltage));
    } else {
        if(isPositive(data, 24)) {
            bitClear(data, 23);
            double valueRead = data;
            double maxADCValue = 8388607;
            return (valueRead / (maxADCValue / referenceVoltage));
        } else {
            //bitClear(data,23);
            for(int i = 0; i < 23; i++) {
                if(bitRead(data, i) == 0) {
                    bitSet(data, i);
                } else {
                    bitClear(data, i);
                }
            }
            double valueRead = data;
            double maxADCValue = 8388607;
            return (-valueRead / (maxADCValue / referenceVoltage));
        }
    }
}