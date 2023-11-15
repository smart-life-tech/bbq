#include "MAX31865.h"

MAX318650::MAX318650(int8_t spi_cs, int8_t spi_mosi, int8_t spi_miso,
                    int8_t spi_clk){
  
  SPI.begin(spi_clk,spi_miso,spi_mosi,spi_cs);
  chipSelectPin = spi_cs;
  pinMode(chipSelectPin, OUTPUT); //because CS is manually switched  
}

void MAX318650::readRegister()
{
  SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE1));
  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(0x80); //80h = 128 - config register
  SPI.transfer(0xB0); //B0h = 176 - 10110000: bias ON, 1-shot, start 1-shot, 3-wire, rest are 0
  digitalWrite(chipSelectPin, HIGH);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer(1);
  reg1 = SPI.transfer(0xFF);
  reg2 = SPI.transfer(0xFF);
  digitalWrite(chipSelectPin, HIGH);

  fullreg = reg1; //read MSB
  fullreg <<= 8;  //Shift to the MSB part
  fullreg |= reg2; //read LSB and combine it with MSB
  fullreg >>= 1; //Shift D0 out.
  resistance = fullreg; //pass the value to the resistance variable
  //note: this is not yet the resistance of the RTD!

  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(0x80); //80h = 128
  SPI.transfer(144); //144 = 10010000
  SPI.endTransaction();
  digitalWrite(chipSelectPin, HIGH);

}

void MAX318650::convertToTemperature(){
  Rt = resistance;
  Rt /= 32768;
  Rt *= 430; //This is now the real resistance in Ohms

  Z1 = -RTDa;
  Z2 = RTDa * RTDa - (4 * RTDb);
  Z3 = (4 * RTDb) / 100;
  Z4 = 2 * RTDb;

  temperature = Z2 + (Z3 * Rt);
  temperature = (sqrt(temperature) + Z1) / Z4;

  if (temperature >= 0)
  {
    Serial.print("Temperature: ");
    Serial.println(temperature); //Temperature in Celsius degrees
    return; //exit
  }
  else
  {
    Rt /= 100;
    Rt *= 100; // normalize to 100 ohm

    rpoly = Rt;

    temperature = -242.02;
    temperature += 2.2228 * rpoly;
    rpoly *= Rt; // square
    temperature += 2.5859e-3 * rpoly;
    rpoly *= Rt; // ^3
    temperature -= 4.8260e-6 * rpoly;
    rpoly *= Rt; // ^4
    temperature -= 2.8183e-8 * rpoly;
    rpoly *= Rt; // ^5
    temperature += 1.5243e-10 * rpoly;

    Serial.print("Temperature: ");
    Serial.println(temperature); //Temperature in Celsius degrees
  }
   
}
double MAX318650::getTemperature(){
  Serial.println(temperatureRead());
  Serial.println("Geting temp:");
  readRegister();
  convertToTemperature();
  // test = !test;
  // digitalWrite(chipSelectPin,test);
  return temperature;

}