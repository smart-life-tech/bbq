#include <Arduino.h>
#include <SPI.h>

class MAX318650{
  public:
  //Variables for the PT100 boards
    double resistance;
    uint8_t reg1, reg2; //reg1 holds MSB, reg2 holds LSB for RTD
    uint16_t fullreg; //fullreg holds the combined reg1 and reg2
    double temperature;
    //Variables and parameters for the R - T conversion
    double Z1, Z2, Z3, Z4, Rt;
    double RTDa = 3.9083e-3;
    double RTDb = -5.775e-7;
    double rpoly = 0;

    int chipSelectPin;
    bool test = false;

    MAX318650(int8_t spi_cs, int8_t spi_mosi, int8_t spi_miso,
                    int8_t spi_clk);
    void readRegister(void);
    void convertToTemperature();
    double getTemperature();

  // private:

};

