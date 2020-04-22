//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                          Plugin-41 LaCrosse                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding LaCrosse  weatherstation outdoor sensors
 * It also works for all non LaCrosse sensors that follow this protocol.
 * Lacrosse WS2355, WS3600 and compatibles
 *
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:
 * Partially based on http://makin-things.com/articles/decoding-lacrosse-weather-sensor-rf-transmissions/
 *
 * WS2355  
 * Each packet is 52 bits long. 4 different packet formats are transmitted. They are composed of: 
 * 
 * data  0    1     2    3   4    5   6    7   8    9   10  11   12
 * 1) 0000 1001 01 00 00100010 01111000 01010011 0011 10101100 0001   0+9+4+2+2+7+8+5+3+3+A+C=41 = 1
 * 2) 0000 1001 00 01 00100010 01111000 01010000 1101 10101111 1000   0+9+1+2+2+7+8+5+0+D+A+F=48 = 8
 * 3) 0000 1001 00 10 00100010 01111000 00001000 1100 11110111 1000
 * 4) 0000 1001 01 11 00100010 01111000 00000000 1100 11111111 1101
 *    SSSS PPPP QR AA BBBBBBBB CCCCCCCC DDDDDDDD dddd EEEEEEEE FFFF    
 *    0000 1001 01 01 01100110 01011110 10001000 1001 01110111 0100
 *    0000 1001 00 11 01100110 01011110 00000000 1101 11111111 0110 
 *    0000 1001 01 00 01100110 01011110 01000110 1000 10111001 0010
 *    0000 1001 00 01 01100110 01011110 01100111 1001 10011000 000.   9+1+6+6+5+E+6+7+9+9+8=50  missing bit is 0
 *    0000 1001 01 11 01100110 01011110 00000000 1001 11111111 011.   9+7+6+6+5+E+0+0+9+F+F=56  missing bit is 0
 * S = Sync 
 * P = Preamble (1001 for WS2300, 0110 for WS3600?)
 * A = packet type  00=TEMP, 01=HUM, 10=RAIN, 11=WIND
 * B = Rolling Code
 * C = Flags
 * D = 12 bit value depending on the device/packet type
 * E = Inverted data 
 * F = Checksum
 * Q = 0/1 1=windpacket reports windgust 0=windpacket reports windspeed
 * R = Error checking bit 
 *
 * TEMP Dd = temperature - 30 degrees offset, 12 bits, 0x533 - 0x300 = 0x233 = 23.3 degrees
 * HUM  D  = humidity value, 8 bits, 0x50 = RH of 50 
 * RAIN D = number of tips * 0.508mm (range=0-4095, Once the count reaches 4095 it wraps back to 0. Powerloss results in a reset of the count)  
 * WIND d = Wind direction (0-15 in 22.5 degrees steps) D= wind speed
 *
 * Sample:
 * 20;D3;DEBUG;Pulses=104;Pulses(uSec)=1400,1300,1325,1300,1325,1275,1350,1150,225,1300,1325,1275,1325,1275,225,1300,1325,1275,225,1275,1350,1275,225,1300,1325,1275,225,1300,225,1275,1350,1275,1350,1275,250,1275,225,1275,1350,1275,1350,1300,225,1300,1350,1275,225,1275,225,1275,225,1275,225,1275,1325,1275,225,1300,1325,1275,1325,1275,1325,1275,250,1275,1350,1275,1325,1300,1325,1275,250,1275,1350,1275,1325,1275,250,1275,1325,1275,250,1275,225,1275,225,1275,1350,1275,225,1275,250,1275,225,1275,1325,1275,250,1275,1350,1300,1325;
 * 20;D4;DEBUG;Pulses=104;Pulses(uSec)=1400,1275,1350,1275,1350,1275,1325,1150,250,1275,1350,1275,1325,1275,250,1275,1325,1275,1350,1275,225,1275,225,1275,1350,1300,225,1275,225,1275,1350,1275,1325,1275,225,1275,225,1275,1325,1275,1325,1275,250,1275,1350,1300,225,1275,225,1275,225,1275,225,1275,1350,1275,1325,1275,1350,1275,1325,1275,1350,1275,1325,1275,1350,1275,1325,1300,1325,1275,225,1275,225,1275,1350,1275,225,1275,225,1300,225,1275,250,1275,225,1275,225,1275,250,1275,225,1275,225,1275,1350,1275,250,1275,225,1275,1325;
 * 20;D5;DEBUG;Pulses=104;Pulses(uSec)=1400,1275,1350,1275,1350,1275,1325,1150,225,1275,1350,1275,1325,1275,225,1300,1325,1275,225,1300,1325,1275,1325,1275,1350,1275,225,1300,225,1275,1350,1275,1350,1300,225,1300,225,1275,1350,1275,1325,1275,250,1275,1350,1275,250,1275,225,1275,225,1275,225,1275,1325,1275,1350,1275,250,1275,1325,1275,1350,1275,1350,1275,225,1275,225,1275,1350,1275,225,1300,1325,1275,1325,1275,1350,1275,250,1275,1325,1275,250,1275,250,1275,225,1275,1350,1275,1350,1275,225,1275,1350,1275,1350,1275,225,1275,1325;
 * 20;C3;DEBUG;Pulses=102;Pulses(uSec)=1400,1275,1325,1275,1325,1275,1325,1175,225,1300,1350,1275,1350,1275,225,1300,1325,1300,1325,1275,1325,1300,225,1300,1325,1275,225,1275,225,1300,1325,1275,1325,1275,250,1275,225,1275,1325,1275,1350,1275,225,1275,1325,1275,225,1225,300,1275,250,1275,225,1275,1325,1275,1325,1300,225,1275,225,1275,1325,1300,1325,1275,225,1275,225,1275,225,1275,225,1275,1325,1275,1325,1275,250,1275,250,1275,1325,1275,1350,1275,225,1275,225,1300,1325,1275,1350,1275,1325,1300,1325,1275,1350,1275,1325;
 * 20;E1;DEBUG;Pulses=102;Pulses(uSec)=1425,1275,1325,1275,1325,1275,1350,1150,225,1275,1350,1275,1350,1275,250,1275,1350,1275,225,1275,225,1275,250,1275,1350,1275,225,1300,225,1275,1325,1275,1350,1300,225,1275,225,1275,1350,1275,1325,1300,225,1275,1350,1275,250,1275,225,1275,225,1275,250,1275,1325,1275,1350,1275,1325,1275,1325,1275,1325,1275,1350,1275,1350,1275,1325,1300,1325,1275,250,1275,1325,1275,1325,1275,225,1275,250,1275,225,1275,250,1275,225,1300,225,1275,225,1275,225,1300,225,1275,1350,1275,250,1275,225;
  \*********************************************************************************************/


#define LACROSSE_PLUGIN_ID 002
#define LACROSSE_PULSECOUNT 104 // also handles 102 pulses!

#define LACROSSE_MIDLO 1100 / RAWSIGNAL_SAMPLE_RATE
#define LACROSSE_MIDHI 1400 / RAWSIGNAL_SAMPLE_RATE

#define LACROSSE_PULSEMID 1000 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_002
#include "../4_Display.h"



// static const char PLUGIN_DESC_002[] PROGMEM  = {"LaCrosseV2"};   // tentative d'optmisation mémoire...
String PLUGIN_DESC_002 = "LaCrosseV2";


boolean Plugin_002(byte function, char *string)
{
   if ((RawSignal.Number != LACROSSE_PULSECOUNT) && (RawSignal.Number != (LACROSSE_PULSECOUNT - 2)))
      return false;

   unsigned long bitstream1 = 0L; // holds first 5x4=20 bits
   unsigned long bitstream2 = 0L; // holds last  8x4=32 bits
   byte bitcounter = 0;           // counts number of received bits (converted from pulses)
   byte data[13];
   byte checksum = 0;
   byte sensortype = 0;
   int temperature = 0;
   byte humidity = 0;
   unsigned int rain = 0;
   unsigned int windspeed = 0;
   unsigned int windgust = 0;
   unsigned int winddirection = 0;
   //==================================================================================
   // Get all 52 bits
   //==================================================================================
   for (byte x = 1; x < RawSignal.Number + 1; x += 2)
   {
      if ((RawSignal.Pulses[x + 1] < LACROSSE_MIDLO) || (RawSignal.Pulses[x + 1] > LACROSSE_MIDHI))
      {
         if ((x + 1) < RawSignal.Number) // in between pulse check
            return false;
      }
      if (RawSignal.Pulses[x] > LACROSSE_PULSEMID)
      {
         if (bitcounter < 20)
         {
            bitstream1 <<= 1;
            bitcounter++; // only need to count the first 20 bits
         }
         else
            bitstream2 <<= 1;
      }
      else
      {
         if (bitcounter < 20)
         {
            bitstream1 <<= 1;
            bitstream1 |= 0x1;
            bitcounter++; // only need to count the first 20 bits
         }
         else
         {
            bitstream2 <<= 1;
            bitstream2 |= 0x1;
         }
      }
   }
   if (RawSignal.Number == (LACROSSE_PULSECOUNT - 2))
      bitstream2 <<= 1; // add missing zero bit
   //==================================================================================
   // all bytes received, sort data, do sanity checks and make sure checksum is okay
   //==================================================================================
   if (bitstream1 == 0) // && (bitstream2 == 0)
      return false;

   // prepare nibbles from bit stream
   data[0] = (bitstream1 >> 16) & 0xF;
   if (data[0] != 0x0)
      return false;

   data[1] = (bitstream1 >> 12) & 0xF;       // type verification
   if ((data[1] != 0x9) && (data[1] != 0x6)) // 1001 for WS2300, 0110 for WS3600
      return false;

   data[2] = (bitstream1 >> 8) & 0xF; // Various other checks are possible
   data[3] = (bitstream1 >> 4) & 0xF; // Like parity checks and bit tests
   data[4] = (bitstream1 >> 0) & 0xF; // but false positives do not seem to be a problem
   data[5] = (bitstream2 >> 28) & 0xF;
   data[6] = (bitstream2 >> 24) & 0xF;
   data[7] = (bitstream2 >> 20) & 0xF;
   data[8] = (bitstream2 >> 16) & 0xF;
   data[9] = (bitstream2 >> 12) & 0xF;
   data[10] = (bitstream2 >> 8) & 0xF;
   data[11] = (bitstream2 >> 4) & 0xF;
   data[12] = (bitstream2 >> 0) & 0xF; // CRC
   //==================================================================================
   // first perform a checksum check to make sure the packet is a valid LaCrosse packet
   for (byte i = 0; i < 12; i++)     // max. value = 12*0xF 0xB4
      checksum = checksum + data[i]; // less with real values

   checksum = checksum & 0xF;
   if (checksum != data[12])
      return false;
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   unsigned long tmpval = (bitstream1 << 4) | (data[12]); // sensor type + ID + checksum

   if ((SignalHash != SignalHashPrevious) || (RepeatingTimer < millis()) || (SignalCRC != tmpval))
      SignalCRC = tmpval; // not seen this RF packet recently, save value for later
   else
      return true; // already seen the RF packet recently
   //==================================================================================
   // now process the various sensor types
   //==================================================================================
   // Output
   // ----------------------------------
   data[2] = (data[2] & B1011); // get sensor type from bitstream
   char c_ID[4];
   sprintf(c_ID, "%02X%02X", data[3], data[4]);

   if (data[2] == B0000) // Temperature
   {
      temperature = (data[7] * 100);
      temperature += (data[8] * 10);
      temperature += (data[9]);

      if ((data[1] == 0x9)) // WS2300
         temperature -= 300;
      else // WS3600
         temperature -= 400;
      // ----------------------------------
      display_Header();
      display_Name(PSTR("LaCrosseV2"));
      display_IDc(c_ID);
      display_TEMP(temperature);
      display_Footer();
   }
   else if (data[2] == B0001) // Humidity
   {
      humidity = ((data[7]) << 4) | data[8];
      if (humidity == 0) // Humidity should not be 0
         return false;
      // ----------------------------------
      display_Header();
      display_Name(PSTR("LaCrosseV2"));
      display_IDc(c_ID);
      display_HUM(humidity, HUM_HEX);
      display_Footer();
   }
   else if (data[2] == B0010)
   { // Rain
      rain = ((data[7] << 8) | (data[8] << 4) | (data[9]));
      if ((data[0]) == 0x9)
      {               // WS2300
         rain *= 508; // 0-4095 * 0.508mm
         rain *= 350; // divide by 3.5
      }
      else
      {               // WS3600
         rain *= 518; // 0-4095 * 0.518mm
         rain /= 100;
      }
      // ----------------------------------
      display_Header();
      display_Name(PSTR("LaCrosseV2"));
      display_IDc(c_ID);
      display_RAIN(rain);
      display_Footer();
   }
   else if (data[2] == B0011) // Wind Speed
   {
      winddirection = data[9];                // wind direction in 22.5 degree steps
      windspeed = ((data[7]) << 4) + data[8]; // possibly 9 bits?
      windspeed *= 36;                        // go from m/s to tenth of kph
      windspeed /= 10;                        // go from m/s to kph

      if ((data[0]) == 0x9) // WS2300
         windspeed /= 10;   // divide by 10
      // ----------------------------------
      display_Header();
      display_Name(PSTR("LaCrosseV2"));
      display_IDc(c_ID);
      display_WINDIR(winddirection);
      display_WINSP(windspeed);
      display_Footer();
   }
   else if (sensortype == B1011) // Wind Gust
   {
      winddirection = data[9] & 0xF;         // wind direction in 22.5 degree steps
      windgust = ((data[7]) << 4) + data[8]; // possibly 9 bits?
      windgust *= 36;                        // go from m/s to tenth of kph

      if ((data[0]) == 0x9) // WS2300
         windgust /= 10;    // divide by 10
      // ----------------------------------
      display_Header();
      display_Name(PSTR("LaCrosseV2"));
      display_IDc(c_ID);
      display_WINDIR(winddirection);
      display_WINGS(windgust);
      display_Footer();
   }
   else
   {
      display_Header();
      display_Name(PSTR("LaCrosseV2"));
      display_IDc(c_ID);
      display_Name(PSTR(";DEBUG"));
      for (byte i = 0; i < 12; i++)
         Serial.printf("%02x", data[i]);
      display_Footer();
      //return false;
   }
   //==================================================================================
   RawSignal.Repeats = false;
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_002
