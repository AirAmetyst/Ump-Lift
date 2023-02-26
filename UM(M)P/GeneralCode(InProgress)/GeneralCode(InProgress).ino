#include "Arduino.h"
#if !defined(SERIAL_PORT_MONITOR)
  #error "Arduino version not supported. Please update your IDE to the latest version."
#endif

#if defined(__SAMD21G18A__)
  // Shield Jumper on HW (for Zero, use Programming Port)
  #define port SERIAL_PORT_HARDWARE
  #define pcSerial SERIAL_PORT_MONITOR
#elif defined(SERIAL_PORT_USBVIRTUAL)
  // Shield Jumper on HW (for Leonardo and Due, use Native Port)
  #define port SERIAL_PORT_HARDWARE
  #define pcSerial SERIAL_PORT_USBVIRTUAL
#else
  // Shield Jumper on SW (using pins 12/13 or 8/9 as RX/TX)
  #include "SoftwareSerial.h"
  SoftwareSerial port(12, 13);
  #define pcSerial SERIAL_PORT_MONITOR
#endif

#include "EasyVR.h"

EasyVR easyvr(port);

int8_t bits = 4;
int8_t set = 0;
int8_t group = 0;
uint32_t mask = 0;
uint8_t train = 0;
int8_t grammars = 0;
int8_t lang = 0;
char name[33];
bool useCommands = true;
bool useGrammars = false;
bool useTokens = false;
bool isSleeping = false;
bool isBusy = false;

void setup()
{
  // setup PC serial port
  pcSerial.begin(9600);
bridge:
  // bridge mode?
  int mode = easyvr.bridgeRequested(pcSerial);
  switch (mode)
  {
  case EasyVR::BRIDGE_NONE:
    // setup EasyVR serial port
    port.begin(9600);
    // run normally
    pcSerial.println(F("Bridge not requested, run normally"));
    pcSerial.println(F("---"));
    break;
    
  case EasyVR::BRIDGE_NORMAL:
    // setup EasyVR serial port (low speed)
    port.begin(9600);
    // soft-connect the two serial ports (PC and EasyVR)
    easyvr.bridgeLoop(pcSerial);
    // resume normally if aborted
    pcSerial.println(F("Bridge connection aborted"));
    pcSerial.println(F("---"));
    break;
    
  case EasyVR::BRIDGE_BOOT:
    // setup EasyVR serial port (high speed)
    port.begin(115200);
    pcSerial.end();
    pcSerial.begin(115200);
    // soft-connect the two serial ports (PC and EasyVR)
    easyvr.bridgeLoop(pcSerial);
    // resume normally if aborted
    pcSerial.println(F("Bridge connection aborted"));
    pcSerial.println(F("---"));
    break;
  }

  // initialize EasyVR  
  while (!easyvr.detect())
  {
    pcSerial.println(F("EasyVR not detected!"));
    for (int i = 0; i < 10; ++i)
    {
      if (pcSerial.read() == EasyVR::BRIDGE_ESCAPE_CHAR)
        goto bridge;
      delay(100);
    }
  }

  pcSerial.print(F("EasyVR detected, version "));
  pcSerial.print(easyvr.getID());

  if (easyvr.getID() < EasyVR::EASYVR3)
    easyvr.setPinOutput(EasyVR::IO1, LOW); // Shield 2.0 LED off

  if (easyvr.getID() < EasyVR::EASYVR)
    pcSerial.print(F(" = VRbot module"));
  else if (easyvr.getID() < EasyVR::EASYVR2)
    pcSerial.print(F(" = EasyVR module"));
  else if (easyvr.getID() < EasyVR::EASYVR3)
    pcSerial.print(F(" = EasyVR 2 module"));
  else
    pcSerial.print(F(" = EasyVR 3 module"));
  pcSerial.print(F(", FW Rev."));
  pcSerial.println(easyvr.getID() & 7);

  easyvr.setDelay(0); // speed-up replies

  if (easyvr.getID() >= EasyVR::EASYVR3_1)
  {
    if (!easyvr.checkMessages() && easyvr.getError() == EasyVR::ERR_CUSTOM_INVALID)
    {
      pcSerial.print(F("Message recovery needed, please wait..."));
      if (easyvr.fixMessages())
        pcSerial.println(F(" done!"));
      else
        pcSerial.println(F(" failed!"));
    }
  }

  pcSerial.print(F("Recorded messages:"));
  if (easyvr.getID() >= EasyVR::EASYVR3_1)
  {
    pcSerial.println();
    for (int8_t idx = 0; idx < 32; ++idx)
    {
      int8_t bits = -1; int32_t len = -1;
	  easyvr.dumpMessage(idx, bits, len);
      if ((bits == 0) && (len == 0))
        continue; // skip empty
      pcSerial.print(idx);
      pcSerial.print(F(" = "));
      if (bits < 0 || len < 0)
        pcSerial.println(F(" has errors"));
      else
      {
        pcSerial.print(bits);
        pcSerial.print(F(" bits, size "));
        pcSerial.println(len);
      }
    }
  }
  else
    pcSerial.println(F("n/a"));

  easyvr.setTimeout(5);
  lang = EasyVR::ENGLISH;
  easyvr.setLanguage(lang);
  // use fast recognition
  easyvr.setTrailingSilence(EasyVR::TRAILING_MIN);
  easyvr.setCommandLatency(EasyVR::MODE_FAST);

  int16_t count = 0;

  pcSerial.print(F("Sound table: "));
  if (easyvr.dumpSoundTable(name, count))
  {
    pcSerial.println(name);
    pcSerial.print(F("Sound entries: "));
    pcSerial.println(count);
  }
  else
    pcSerial.println(F("n/a"));

  pcSerial.print(F("Custom Grammars: "));
  grammars = easyvr.getGrammarsCount();
  if (grammars > 4)
  {
    pcSerial.println(grammars - 4);
    for (set = 4; set < grammars; ++set)
    {
      pcSerial.print(F("Grammar "));
      pcSerial.print(set);

      uint8_t flags, num;
      if (easyvr.dumpGrammar(set, flags, num))
      {
        pcSerial.print(F(" has "));
        pcSerial.print(num);
        if (flags & EasyVR::GF_TRIGGER)
          pcSerial.println(F(" trigger"));
        else
          pcSerial.println(F(" command(s)"));
      }
      else
      {
        pcSerial.println(F(" error"));
        continue;
      }

      for (int8_t idx = 0; idx < num; ++idx)
      {
        pcSerial.print(idx);
        pcSerial.print(F(" = "));
        if (!easyvr.getNextWordLabel(name))
          break;
        pcSerial.println(name);
      }
    }
  }
  else
    pcSerial.println(F("n/a"));

  if (easyvr.getGroupMask(mask))
  {
    uint32_t msk = mask;
    for (group = 0; group <= EasyVR::PASSWORD; ++group, msk >>= 1)
    {
      if (!(msk & 1)) continue;
      if (group == EasyVR::TRIGGER)
        pcSerial.print(F("Trigger: "));
      else if (group == EasyVR::PASSWORD)
        pcSerial.print(F("Password: "));
      else
      {
        pcSerial.print(F("Group "));
        pcSerial.print(group);
        pcSerial.print(F(" has "));
      }
      count = easyvr.getCommandCount(group);
      pcSerial.print(count);
      if (group == 0)
        pcSerial.println(F(" trigger(s)"));
      else
        pcSerial.println(F(" command(s)"));
      for (int8_t idx = 0; idx < count; ++idx)
      {
        if (easyvr.dumpCommand(group, idx, name, train))
        {
          pcSerial.print(idx);
          pcSerial.print(F(" = "));
          pcSerial.print(name);
          pcSerial.print(F(", Trained "));
          pcSerial.print(train, DEC);
          if (!easyvr.isConflict())
            pcSerial.println(F(" times, OK"));
          else
          {
            int8_t confl = easyvr.getWord();
            if (confl >= 0)
              pcSerial.print(F(" times, Similar to Word "));
            else
            {
              confl = easyvr.getCommand();
              pcSerial.print(F(" times, Similar to Command "));
            }
            pcSerial.println(confl);
          }
        }
      }
    }
  }
  group = 16;
  
  mask |= 1; // force to use trigger (mixed SI/SD)
  useCommands = true;
  isSleeping = false;
  pcSerial.println(F("---"));
}
int readNum()
{
  int rx;
  int num = 0;
  delay(5);
  while ((rx = pcSerial.read()) >= 0)
  {
    delay(5);
    if (isdigit(rx))
      num = num * 10 + (rx - '0');
    else
      break;
  }
  return num;
}




void loop() {
  

  if (!isSleeping && !isBusy)
  {
    if (easyvr.getID() < EasyVR::EASYVR3)
      easyvr.setPinOutput(EasyVR::IO1, HIGH); // Shield 2.0 LED on (listening)

    
    if (useCommands)
    {
      pcSerial.println(("Waiting for a command "));
      
      easyvr.recognizeCommand(group);
    }
    
  }
  
  while (!easyvr.hasFinished());
  isSleeping = false;
  isBusy = false;

  if (easyvr.getID() < EasyVR::EASYVR3)
    easyvr.setPinOutput(EasyVR::IO1, LOW); // Shield 2.0 LED off

  
  int16_t idx;
  
  // handle voice recognition
  
  if(false){

  }
  else
  {
    idx = easyvr.getCommand();
    if (idx >= 0)
    {
      pcSerial.print(F("Command: "));
      pcSerial.print(easyvr.getCommand());
      if (easyvr.dumpCommand(group, idx, name, train))
      {
        pcSerial.print(F(" = "));
        pcSerial.println(name);
      if(idx == 0){
        pcSerial.println("UMP");
      }
      if(idx == 1){
        pcSerial.println("UMP IS MOVING");
      }
      
      
      
    }
    else // errors or timeout
    {
      int16_t err = easyvr.getError();
      if (err >= 0)
      {
        pcSerial.print(F("Error 0x"));
        pcSerial.println(err, HEX);
      }
      else if (easyvr.isTimeout())
        pcSerial.println(F("Timed out."));
      else
        pcSerial.println(F("Done."));
    } 
  }
  }

}

