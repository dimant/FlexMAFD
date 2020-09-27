#include <MIDI.h>
#include <SPI.h>

// note that this implementation assumes 7 bit MIDI data

/// Options:
const int midiChannel = 3; // midi channel on which to listen for CC messages

const int cc1 = 70; // the control change # which will be sent to the configured jack
const int cc2 = 71; // the control change # which will be sent to the configured jack

// DAC channel 0 is labelled "pressure/mw"
// DAC channel 1 is labelled velocity
const int cc1DACChannel = 0; // the DAC channel on which to output the control change message for cc1 value
const int cc2DACChannel = 1; // the DAC channel on which to output the control change message for cc2 CC value

const int clockDiv = 6; // send a trigger pulse each x clock cycles

// Create and bind the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();

const int ledPin = 10; // LED pin
const int trgPin = 17; // pin to jack labelled "to ADV/CLOCK"
const int cslPin = 14; // SPI chip select pin

bool ledStatus = false;

int clockDivCnt = 0;
bool dfamRunning = false;
int dfamStep = 0;


/// sends a pulse to the jack labelled "to ADV/CLOCK"
// pulse length is controlled by pulseLen option
void sendTrigger()
{
  digitalWrite(trgPin, HIGH);
  digitalWrite(trgPin, LOW);
}

/// writes a value to a given SPI channel
// the code is written to comply with MCP4902  protocol
void sendDAC(int dac, int value)
{
  // 7 bit midi allows for 127 values
  // change this scaling if you switch to 14bit midi
  value = value * 2;

  // send in the address and value via SPI:
  // datasheet: http://ww1.microchip.com/downloads/en/devicedoc/22250a.pdf
  // bit: 15  14  13  12  11  10  09  08  
  //      a/b buf ga  shd d7  d6  d5  d4
  // bit: 07  06  05  04  03  02  01  00
  //      d3  d2  d1  d0  x   x   x   x
  // a/b: select dac a or b
  // 1 = write to B
  // 0 = write to A
  // buf: input buffer
  // 1 = buffered
  // 0 = unbuffered
  // GA: output gain
  // 1 = 1x
  // 0 = 2x
  // shd: output shutdown
  // 1 = active
  // 0 = channel is not active
  // bits d7...d0: 8 bits of data
  // x: ignored 

  byte low = (value & 0x0f) << 4;
  byte high = (value & 0xf0) >> 4;
  dac = (dac & 1) << 7; 

  // 0x30:
  // bit 13: output gain 1x
  // bit 12: output is active
  high = dac | 0x30 | high;
  
  // take the chip select pin low to select the DAC:
  digitalWrite(cslPin, LOW);

  SPI.transfer(high); 
  SPI.transfer(low); 
  
  // take the SS pin high to de-select the chip:  
  digitalWrite(cslPin, HIGH);
}

/// called on each MIDI clock event
// each LED cycle (on/off) runs for 1 clock cycle
// by changing the clockDiv option, you change how many MIDI clock cycles
// equal to one CV trigger. For example, clockDiv 4 means that for every 4
// MIDI clock cycles, 1 CV trigger will be sent
void onClock()
{
  if(clockDivCnt == clockDiv)
  {
    clockDivCnt = 0;
  }

  if(clockDivCnt == 0 && dfamRunning)
  {
    if(false == ledStatus)
    {
      digitalWrite(ledPin, HIGH);
      ledStatus = true;
    }
    else
    {
      digitalWrite(ledPin, LOW);
      ledStatus = false;
    }

    dfamStep++;

    if(dfamStep == 8)
    {
      dfamStep = 0;
    }
    
    sendTrigger();
  }

  
  clockDivCnt += 1;
}

void onControlChange(byte inChannel, byte inNumber, byte inValue)
{
  Serial.printf("channel: %d, number: %d, value: %d\n", inChannel, inNumber, inValue);

  if(midiChannel == inChannel)
  {
    if(cc1 == inNumber)
    {
      Serial.printf("sending: channel %d, number: %d, value: %d", inChannel, inNumber, inValue);
      sendDAC(cc1DACChannel, inValue); 
    }
    else if(cc2 == inNumber)
    {
      Serial.printf("sending: channel %d, number: %d, value: %d", inChannel, inNumber, inValue);
      sendDAC(cc2DACChannel, inValue);     
    }
  }
}

void onStart()
{
  dfamRunning = true;
  clockDivCnt = 0;
  dfamStep = 0;
}

void onStop()
{
  dfamRunning = false;

  if(dfamStep == 0)
  {
    return;
  }
  
  int i;
  
  for(i = 0; i < 8 - dfamStep; i++)
  {
    sendTrigger();
  }

  clockDivCnt = 0;
  dfamStep = 0;
}

void setup() {
  Serial.begin(9600);

  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);
  pinMode(trgPin, OUTPUT);
  pinMode(cslPin, OUTPUT);

  MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages
  MIDI.setHandleClock(onClock);
  MIDI.setHandleControlChange(onControlChange);
  MIDI.setHandleStart(onStart);
  MIDI.setHandleStop(onStop);

  SPI.begin();
}

void loop() {
  MIDI.read();
}
