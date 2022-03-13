#ifndef MAX11254_h
#define MAX11254_h

#include <Arduino.h>
#include <SPI.h>

// read / write bits
#define MSBS        0xC0
#define RBIT        1
#define WBIT        0

// status and settings registers
#define STAT        0x00            // R    24 bit
#define CTRL1       0x01            // R/W  8bit
#define CTRL2       0x02            // R/W  8bit
#define CTRL3       0x03            // R/W  8bit
#define GPIO_CTRL   0x04            // R/W  8bit
#define DELAY       0x05            // R/W  16bit
#define CHMAP1      0x06            // R/W  24bit
#define CHMAP0      0x07            // R/W  24bit
#define SEQ         0x08            // R/W  8bit
#define GPO_DIR     0x09            // R/W  8bit
#define SOC         0xA             // R/W  24bit
#define SCGC        0xD             // R/W  24bit

// data registers
#define DATA0       0xE             // R    24bit
#define DATA1       0xF             // R    24bit
#define DATA2       0x10            // R    24bit
#define DATA3       0x11            // R    24bit
#define DATA4       0x12            // R    24bit
#define DATA5       0x13            // R    24bit

class MAX11254
{
private:
    int8_t sck;
    int8_t miso;
    int8_t mosi;
    int8_t cs;
    int8_t rst;
    int8_t rdy;
    uint32_t spiClock;
    float referenceVoltage;
    void reset();
public:
    MAX11254(int8_t p_sck, int8_t p_miso, int8_t p_mosi, int8_t p_cs, int8_t p_rst, int8_t p_rdy, uint32_t p_spiClock, float referenceV);

    void begin();
    uint32_t read(uint8_t reg, int length);
    void write(uint8_t reg, uint32_t data, int length);
    void command(uint8_t cmd);
    bool dataReady();
    void log(String text, uint32_t data, int length);
    void callibration(int type);
    void mode(int type);
    void conversion(uint8_t rate);
    void readAllChannels(uint32_t *data);
    double getVoltage(uint32_t data);
    bool isPositive(uint32_t data, int length);
};

#endif