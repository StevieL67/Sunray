#include "ioboard.h"
#include <Wire.h>
#include "config.h"


// choose I2C slave via I2C multiplexer (TCA9548A)
// slave: slave number (0-7)
// enable: true or false
void ioI2cMux(uint8_t addr, uint8_t slave, bool enable){
  byte mask = (1 << slave);
  Wire.requestFrom(addr, 1);
  uint8_t state = Wire.read();   // get current control register state
  //CONSOLE.print("I2cMux control=");
  //CONSOLE.println(state, BIN);
  Wire.beginTransmission(addr); // TCA9548A address  
  if (enable)
    Wire.write(state | mask);  // enable I2C device 
  else 
    Wire.write(state & (~mask) );  // disable I2C device   
  Wire.endTransmission();
}

// set I/O port expander (PCA9555) output
// port: 0-7
// pin: 0-7
// level: true or false
void ioExpanderOut(uint8_t addr, uint8_t port, uint8_t pin, bool level){
  byte mask = (1 << pin);
  Wire.beginTransmission(addr); // PCA9555 address 
  Wire.write(6+port);    // configuration port    
  Wire.endTransmission();
  Wire.requestFrom(addr, 1);  
  uint8_t state = Wire.read();   // get current configuration port

  Wire.beginTransmission(addr); // PCA9555 address 
  Wire.write(6+port); // configuration port     
  Wire.write( state & (~mask) ); // enable pin as output 
  Wire.endTransmission();

  Wire.beginTransmission(addr); // PCA9555 address 
  Wire.write(2+port);    // output port    
  Wire.endTransmission();
  Wire.requestFrom(addr, 1);  
  state = Wire.read();   // get current output port

  Wire.beginTransmission(addr); // PCA9555 address 
  Wire.write(2+port); // output port     
  // set additional pins to desired level
  if (level)
    Wire.write( state | (mask) );    
  else 
    Wire.write( state & (~mask) );  
  Wire.endTransmission();
}

// read I/O port expander (PCA9555) input
// port: 0-7
// pin: 0-7
bool ioExpanderIn(uint8_t addr, uint8_t port, uint8_t pin){
  byte mask = (1 << pin);
  Wire.beginTransmission(addr); // PCA9555 address 
  Wire.write(6+port);    // configuration port    
  Wire.endTransmission();
  Wire.requestFrom(addr, 1);  
  uint8_t state = Wire.read();   // get current configuration port

  Wire.beginTransmission(addr); // PCA9555 address 
  Wire.write(6+port); // configuration port     
  Wire.write( state | (mask) ); // enable pin as input 
  Wire.endTransmission();

  Wire.beginTransmission(addr); // PCA9555 address 
  Wire.write(0+port);    // input port    
  Wire.endTransmission();
  Wire.requestFrom(addr, 1);  
  state = Wire.read();   // get current output port
  return ((state & mask) != 0); 
}


// choose ADC multiplexer (DG408) channel  
// adc: 1-8
void ioAdcMux(uint8_t adc){
  int idx = adc - 1;
  //  ioExpanderOut(EX1_I2C_ADDR, EX1_ADC_MUX_EN_PORT, EX1_ADC_MUX_EN_PIN, false);
  ioExpanderOut(EX1_I2C_ADDR, EX1_ADC_MUX_A0_PORT, EX1_ADC_MUX_A0_PIN, (idx & 1) != 0);
  ioExpanderOut(EX1_I2C_ADDR, EX1_ADC_MUX_A1_PORT, EX1_ADC_MUX_A1_PIN, (idx & 2) != 0);
  ioExpanderOut(EX1_I2C_ADDR, EX1_ADC_MUX_A2_PORT, EX1_ADC_MUX_A2_PIN, (idx & 4) != 0);
  ioExpanderOut(EX1_I2C_ADDR, EX1_ADC_MUX_EN_PORT, EX1_ADC_MUX_EN_PIN, true);  
}


// configure ADC MCP3421
float ioAdcStart(uint8_t addr){ 
  // send config  
  Config cfg;
  cfg.reg      = 0x00;
  cfg.bit.GAIN = eGain_x1;
  cfg.bit.SR   = eSR_18Bit;
  cfg.bit.OC   = 0; // 1=repeat, 0=single shot
  Wire.beginTransmission(addr); // MCP3421 address   
  Wire.write(cfg.reg);   // config register 
  Wire.endTransmission();
}

// trigger an ADC conversion (MCP3421)
void ioAdcTrigger(uint8_t addr){
  Config cfg;
  cfg.reg      = 0x00;
  cfg.bit.GAIN = eGain_x1;
  cfg.bit.SR   = eSR_18Bit;
  cfg.bit.OC   = 0; // 1=repeat, 0=single shot  
  cfg.bit.RDY = 1;  // trigger conversion
  Wire.beginTransmission(addr); // MCP3421 address   
  Wire.write(cfg.reg);   // config register 
  Wire.endTransmission();
}


// read ADC conversion (MCP3421)
float ioAdc(uint8_t addr){

  uint8_t u8Data;
  uint8_t u8Len = 4;

  //unsigned long startTime = millis();                
  if ((u8Len != Wire.requestFrom(addr, u8Len)) ||
      (u8Len < 3)){
    CONSOLE.println("ioAdc no data");
    return -1;
  }

  u8Data     = (uint8_t)Wire.read();
  int32_t s32Value = ((u8Data & 0x80) != 0) ? -1 : 0;
  s32Value = (s32Value & 0xFFFFFF00) | u8Data;

  for (u8Len--; u8Len > 1; u8Len--)
  {
    s32Value <<= 8;
    s32Value  |= (uint8_t)Wire.read();
  }

  Config cfg;
  cfg.reg = Wire.read();
  
  //unsigned long duration = millis() - startTime;
  //CONSOLE.print("duration ");
  //CONSOLE.println(duration);  

  if (cfg.bit.RDY == 1) {    
    CONSOLE.print("ioAdc not ready - config=");
    CONSOLE.println(cfg.reg, BIN);  
    return -1;
  }    
  return ((float)s32Value) / 262143.0 * 2.048;
}
  