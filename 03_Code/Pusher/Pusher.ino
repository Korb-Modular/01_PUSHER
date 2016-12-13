// Pusher_06092016_ver01

#include <SPI.h>
#include <math.h>
#include <EEPROM.h>
#include "TimerOne.h"

//#include "Timer.h"


// *************************************************************************************************
// **************************************Global Variables*******************************************
// *************************************************************************************************

// Variable for HW-SW Interface
const int DAC1_CS = 7;      // Chipselect DAC 1 PP1-PP2
const int DAC2_CS = 8;      // Chipselect DAC 2 PP3-PP4
const int DAC3_CS = 9;      // Chipselect DAC 3 POT1-8
const int CS_Gate = 5;      // Chipselect 74HC595
const int Gate_Out = 6;     // Pin for Global Gate Out

// Variables for DAC Pre-Settings
const int GAIN_1 = 0x1;     // Global Gain DAC
const int GAIN_2 = 0x0;     // Global Gain DAC

// Variables for 4051 Multiplexer
int MUL_s0 = 2;           //Select PIN 4051 (s0)
int MUL_s1 = 3;           //Select PIN 4051 (s1)
int MUL_s2 = 4;           //Select PIN 4051 (s2)
int count = 0;            //which y pin we are selecting
int Value[8];

int Gate[4];
int Trigger[4];

// Raw Pressure of FSR1-4
double PP1_raw = 0;
double PP2_raw = 0;
double PP3_raw = 0;
double PP4_raw = 0;

// Pressure of FSR1-4 after processing
double PP1_curve = 0;
double PP2_curve = 0;
double PP3_curve = 0;
double PP4_curve = 0;


int Trigger_Threshold = 5;
int Pressure_Threshold = 60;
int Range = 0;
int Curve = 0;
int GateMode = 0;
int RelaseTime = 0;
int TriggerTime = 0;


int Show_Settings_Page = 0;
int Counter = 0;
int UserSetting_1 = 1;
int UserSetting_2 = 1;
int UserSetting_3 = 1;
int UserSetting_4 = 1;

// Variables for Eeprom Access
int EEP_Adr_Range_Settings = 1;
int EEP_Adr_Curve_Settings = 2;
int EEP_Adr_GateMode_Settings = 3;
int EEP_Adr_Pressure_Threshold_Settings = 4;

// *************************************************************************************************
// **************************************Setup******************************************************
// *************************************************************************************************
void setup() {
  pinMode(MUL_s0, OUTPUT);
  pinMode(MUL_s1, OUTPUT);
  pinMode(MUL_s2, OUTPUT);
  pinMode(DAC1_CS, OUTPUT);
  pinMode(DAC2_CS, OUTPUT);
  pinMode(DAC3_CS, OUTPUT);
  pinMode(CS_Gate, OUTPUT);
  pinMode(Gate_Out, OUTPUT);

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);

  Serial.begin(9600);
  delay(100);

  // Load UserSetting from Eeprom
  Range = EEPROM.read(EEP_Adr_Range_Settings);
  Curve = EEPROM.read(EEP_Adr_Curve_Settings);
  GateMode = EEPROM.read(EEP_Adr_GateMode_Settings);
  Pressure_Threshold = EEPROM.read(EEP_Adr_Pressure_Threshold_Settings);

  // Initialize Timer-Interrupt
  //  Timer1.initialize(500000);              // initialize timer1, and set a 1/2 second period
  //  Timer1.attachInterrupt(TimerCallback);  // attaches TimerCallback() as a timer overflow interrupt
}

// *************************************************************************************************
// **************************************LOOP*******************************************************
// *************************************************************************************************

void loop () {

  /*
    while (Show_Settings_Page== 3) {
      if ((analogRead(A0) < Trigger_Threshold ) && (analogRead(A1) < Trigger_Threshold ) && (analogRead(A2) < Trigger_Threshold) && (analogRead(A3) < Trigger_Threshold ))  Show_Settings_Page = 0;

    }
  */
  // --------------------------------------------------Read out POT 1-8-----------------------------------------------------
  /*
    // POT 1-4 Upper Row
    for (count = 4; count <= 7; count++) {
      // select the bit
      MUL_s0 = bitRead(count, 0);
      MUL_s1 = bitRead(count, 1);
      MUL_s2 = bitRead(count, 2);

      digitalWrite(2,  MUL_s0);
      digitalWrite(3,  MUL_s1);
      digitalWrite(4,  MUL_s2);

      Value[count] = map(analogRead(A4), 0, 1023, 0, 4093);
    }
  */
  // POT 4-8 Bottom Row
  for (int count = 0; count < 8; count++) {
    // select the bit

      MUL_s0 = bitRead(count, 0);
      MUL_s1 = bitRead(count, 1);
      MUL_s2 = bitRead(count, 2);

      digitalWrite(2,  MUL_s0);
      digitalWrite(3,  MUL_s1);
      digitalWrite(4,  MUL_s2);
  

    Value[count] = map(analogRead(A4), 0, 1023, 0 , 4093);
/*    Serial.print("POT: ");
    Serial.print(count);
    Serial.print(": ");
    Serial.println(Value[count]);*/

  }
  /*
    PP1 =  analogRead(A0);
    PP2=  analogRead(A1);
    PP3e=  analogRead(A2);
    PP4_curve=  analogRead(A3);


      Serial.print("PP1: ");
      Serial.println(PP1);
      Serial.print("PP2: ");
      Serial.println(PP2);
      Serial.print("PP3: ");
      Serial.println(PP3);
        Serial.print("PP4: ");
      Serial.println(PP4);*/

  // --------------------------------------------------Read FSR 1-4---------------------------------------------------------
  // Read out the Raw-Values of FSR 1-4
  PP1_raw =  analogRead(A0);
  PP2_raw =  analogRead(A1);
  PP3_raw =  analogRead(A2);
  PP4_raw =  analogRead(A3);

  //  Serial.print("Pressure_Threshold");
  //  Serial.println(Pressure_Threshold);
  //  Serial.print("PP1: ");
  //Serial.println(PP1_raw);

  // --------------------------------------------------Set Response Curve---------------------------------------------------
  //Further processing of PP1-PP4
  //according to curve settings

  switch (Curve) {
    case 1 :
      PP1_curve = PP1_raw;
      PP2_curve = PP2_raw;
      PP3_curve = PP3_raw;
      PP4_curve = PP4_raw;
      break;
    case 2 :
      PP1_curve = (log10 (PP1_raw)) * 340;
      PP2_curve = (log10 (PP2_raw)) * 340;
      PP3_curve = (log10 (PP3_raw)) * 340;
      PP4_curve = (log10 (PP4_raw)) * 340;
      break;
    case 3 :
      PP1_curve = (exp (PP1_raw / 144));
      PP2_curve = (exp (PP2_raw / 144));
      PP3_curve = (exp (PP3_raw / 144));
      PP4_curve = (exp (PP4_raw / 144));
      break;
  }

  // --------------------------------------------------FSR 1 pressed--------------------------------------------------------
  if ((PP1_curve >= Trigger_Threshold ) && (PP2_curve < Trigger_Threshold ) && (PP3_curve < Trigger_Threshold ) && (PP4_curve < Trigger_Threshold ))  {
    setOutput(3, 0, GAIN_1, 1, Value[0]);
    setOutput(3, 1, GAIN_1, 1, Value[4]);
    //   setOutput(1, 0, GAIN_1, 1, PP1_curve* Range);
    Gate[0] = 1;
    setGates();
    //    Serial.print("PP1: ");
    //    Serial.println(PP1);
    if (PP1_curve > Pressure_Threshold ) setOutput(1, 0, GAIN_1, 1, (PP1_curve - Pressure_Threshold) * Range);
  }
  else  {
    setOutput(1, 0, GAIN_1, 1, 0);
    Gate[0] = 0;
    setGates();
  }

  // --------------------------------------------------FSR 2 pressed--------------------------------------------------------
  if ((PP2_curve >= Trigger_Threshold ) && (PP1_curve < Trigger_Threshold ) && (PP3_curve < Trigger_Threshold ) && (PP4_curve < Trigger_Threshold ))  {
    setOutput(3, 0, GAIN_1, 1, Value[1]);
    setOutput(3, 1, GAIN_1, 1, Value[5]);
    //    setOutput(1, 1, GAIN_1, 1, PP2_curve* Range);
    if (PP2_curve > Pressure_Threshold ) setOutput(1, 1, GAIN_1, 1, (PP2_curve - Pressure_Threshold) * Range);
    Gate[1] = 1;
    setGates();
    //    Serial.print("PP2: ");
    //    Serial.println(PP2);
  }
  else  {
    setOutput(1, 1, GAIN_1, 1, 0);
    Gate[1] = 0;
    setGates();
  }

  // --------------------------------------------------FSR 3 pressed--------------------------------------------------------
  if ((PP3_curve >= Trigger_Threshold ) && (PP1_curve < Trigger_Threshold ) && (PP2_curve < Trigger_Threshold ) && (PP4_curve < Trigger_Threshold ))  {
    setOutput(3, 0, GAIN_1, 1, Value[2]);
    setOutput(3, 1, GAIN_1, 1, Value[6]);
    //   setOutput(2, 0, GAIN_1, 1, PP3_curve* Range);
    if (PP3_curve > Pressure_Threshold ) setOutput(2, 0, GAIN_1, 1, (PP3_curve - Pressure_Threshold) * Range);
    Gate[2] = 1;
    setGates();
    //    Serial.print("PP3: ");
    //    Serial.println(PP3);
  }
  else  {
    setOutput(2, 0, GAIN_1, 1, 0);
    Gate[2] = 0;
    setGates();

  }

  // --------------------------------------------------FSR 4 pressed--------------------------------------------------------
  if ((PP4_curve >= Trigger_Threshold ) && (PP1_curve < Trigger_Threshold ) && (PP2_curve < Trigger_Threshold ) && (PP3_curve < Trigger_Threshold ))  {
    setOutput(3, 0, GAIN_1, 1, Value[3]);
    setOutput(3, 1, GAIN_1, 1, Value[7]);
    //    setOutput(2, 1, GAIN_1, 1, PP4_curve* Range);
    if (PP4_curve > Pressure_Threshold ) setOutput(2, 1, GAIN_1, 1, (PP4_curve - Pressure_Threshold) * Range);
    Gate[3] = 1;
    setGates();
    //    Serial.print("PP4: ");
    //    Serial.println(PP4);
  }
  else  {
    setOutput(2, 1, GAIN_1, 1, 0);
    Gate[3] = 0;
    setGates();
  }

  // --------------------------------------------------FSR 1-4--------------------------------------------------------------
  if ((PP1_curve >= Trigger_Threshold ) && (PP2_curve >= Trigger_Threshold ) && (PP3_curve >= Trigger_Threshold) && (PP3_curve >= Trigger_Threshold ))  {
    //   Serial.println("Settings");
    Counter++;
    //    Serial.println(Counter);
    if (Counter == 300) {
      Counter = 0;
      Show_Settings_Page = 2;
      UserSettings(2500);
    }
  }
}

// **********************************************************************************************************************
// **************************************************Functions***********************************************************
// **********************************************************************************************************************

// ---------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------setOutput)---------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void setOutput(int DAC, byte channel, byte gain, byte shutdown, unsigned int val)
{
  byte lowByte = val & 0xff;
  byte highByte = ((val >> 8) & 0xff) | channel << 7 | gain << 5 | shutdown << 4;

  if (DAC == 1) digitalWrite(DAC1_CS, LOW);
  if (DAC == 2) digitalWrite(DAC2_CS, LOW);
  if (DAC == 3) digitalWrite(DAC3_CS, LOW);
  SPI.transfer(highByte);
  SPI.transfer(lowByte);
  if (DAC == 1) digitalWrite(DAC1_CS, HIGH);
  if (DAC == 2) digitalWrite(DAC2_CS, HIGH);
  if (DAC == 3) digitalWrite(DAC3_CS, HIGH);
}


// ---------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------setGates()---------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void setGates() {
  int LEDs;
  int Gates;

  LEDs = Gate[0] << 4 | Gate[1] << 5 | Gate[2] << 6 | Gate[3] << 7;
  Gates = Gate[0] << 0 | Gate[1] << 1 | Gate[2] << 2 | Gate[3] << 3;

  digitalWrite(CS_Gate, LOW);
  SPI.transfer(LEDs | Gates);
  digitalWrite(CS_Gate, HIGH);

  if (GateMode == 1) {
    if (Gate[0] == 1 | Gate[1] == 1 | Gate[2] == 1 | Gate[3] == 1) {
      digitalWrite(Gate_Out, HIGH);
    }
    else {

      digitalWrite(Gate_Out, LOW);
    }
  }

  if (GateMode == 2) {
    if (Gate[0] == 1 | Gate[1] == 1 | Gate[2] == 1 | Gate[3] == 1) {
      TriggerTime++;
      if (TriggerTime <= 8) {
        digitalWrite(Gate_Out, HIGH);
      }
      else {

        digitalWrite(Gate_Out, LOW);
      }
    }
    else {
      TriggerTime = 0;
      digitalWrite(Gate_Out, LOW);
    }
  }
}

// ---------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------User Settings------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
// --------------------------
// Setting 1 :  Range Select
// Setting 2 :  Gate or Trigger for Main Gate
// Setting 3 : Lin/Log/Exp Response
// 4 => Range 0 - 10,0 V
// -------------------------

void UserSettings(int Timeout)
{ int LEDs;
  int flag[4] = {0, 0, 0, 0};

  // Wait for Release of FSR 1-4
  while (Show_Settings_Page == 2) {
    if ((analogRead(A0) < Trigger_Threshold ) && (analogRead(A1) < Trigger_Threshold ) && (analogRead(A2) < Trigger_Threshold) && (analogRead(A3) < Trigger_Threshold ))  {
      Show_Settings_Page = 1;
      SetLEDs(0xF0);
    }
  }

  while (Show_Settings_Page == 1) {
    // ---------------------------------------------------------------------------------------------------------------------
    // --------------------------------------------------// Setting for CV Range--------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    // CV - Range
    // --------------------------
    // 1 => Range 0 - 1,25 V
    // 2 => Range 0 - 2,50 V
    // 3 => Range 0 - 5,00 V
    // 4 => Range 0 - 10,0 V
    // -------------------------
    if ((analogRead(A0) > Trigger_Threshold) && (flag[0] == 0)) {
      Timeout = 5000;
      UserSetting_1++;
      if (UserSetting_1 > 4) UserSetting_1 = 1;
      flag[0] = 1;

      switch (UserSetting_1) {
        case 1 :
          SetLEDs(0x10);
          Range = 1;
          break;
        case 2 :
          SetLEDs(0x20);
          Range = 2;
          break;
        case 3 :
          SetLEDs(0x40);
          Range = 3;
          break;
        case 4 :
          SetLEDs(0x80);
          Range = 4;
          break;
      }

    }
    if ((analogRead(A0) < Trigger_Threshold) && (flag[0] == 1)) {
      flag[0] = 0;
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // --------------------------------------------------// Setting Response Curve-------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    //Response Settings
    // --------------------------
    // 1 => Linear Response
    // 2 => Log Response
    // 3 => Exp Response
    // -------------------------
    if ((analogRead(A1) > Trigger_Threshold) && (flag[1] == 0)) {
      Timeout = 2500;
      UserSetting_2++;
      if (UserSetting_2 > 3) UserSetting_2 = 1;
      flag[1] = 1;

      switch (UserSetting_2) {
        case 1 :
          SetLEDs(0x10);
          Curve = 1;
          break;
        case 2 :
          SetLEDs(0x20);
          Curve = 2;
          break;
        case 3 :
          SetLEDs(0x40);
          Curve = 3;
          break;
      }
    }
    if ((analogRead(A1) < Trigger_Threshold) && (flag[1] == 1)) {
      flag[1] = 0;
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // --------------------------------------------------// Setting Gate or Trigger-----------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    //Gate vs. Trigger
    // --------------------------
    // 1 => Gate on Main
    // 2 => Trigger on Main
    // -------------------------
    if ((analogRead(A2) > Trigger_Threshold) && (flag[2] == 0)) {
      Timeout = 2500;
      UserSetting_3++;
      if (UserSetting_3 > 2) UserSetting_3 = 1;
      flag[2] = 1;

      switch (UserSetting_3) {
        case 1 :
          GateMode = 1;
          SetLEDs(0x30);
          break;
        case 2 :
          GateMode = 2;
          SetLEDs(0xC0);
          break;
      }
    }
    if ((analogRead(A2) < Trigger_Threshold) && (flag[2] == 1)) {
      flag[2] = 0;
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // --------------------------------------------------// Settings Pressure Threshold-------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    //Gate vs. Trigger
    // --------------------------
    // 1 => No Threshold (0)
    // 2 => Low Threshold (20)
    // 3 => Middle Threshold (40)
    // 3 => High Threshold (80)
    // -------------------------
    if ((analogRead(A3) > Trigger_Threshold) && (flag[3] == 0)) {
      Timeout = 2500;
      UserSetting_4++;
      if (UserSetting_4 > 4) UserSetting_4 = 1;
      flag[3] = 1;

      switch (UserSetting_4) {
        case 1 :
          Pressure_Threshold = 0;
          SetLEDs(0x10);
          break;
        case 2 :
          Pressure_Threshold = 50;
          SetLEDs(0x20);
          break;
        case 3 :
          Pressure_Threshold = 100;
          SetLEDs(0x40);
          break;
        case 4 :
          Pressure_Threshold = 200;
          SetLEDs(0x80);
          break;
      }
    }
    if ((analogRead(A3) < Trigger_Threshold) && (flag[3] == 1)) {
      flag[3] = 0;
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // --------------------------------------------------// Close Settings Menue--------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    Timeout--;
    if (Timeout <= 0) {
      Show_Settings_Page = 0;

      // Write Settings to Eeprom
      EEPROM.update(EEP_Adr_Range_Settings, Range);
      EEPROM.update(EEP_Adr_Curve_Settings, Curve);
      EEPROM.update(EEP_Adr_GateMode_Settings, GateMode);
      EEPROM.update(EEP_Adr_Pressure_Threshold_Settings, Pressure_Threshold);
    }
  }
}


// ---------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------// Set LEDs--------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void SetLEDs(int LED) {
  // Set LEDs according User-Setting 1-4
  digitalWrite(CS_Gate, LOW);
  SPI.transfer(LED | 0);
  digitalWrite(CS_Gate, HIGH);

}

// ---------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------// Timer Callback Function-----------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void TimerCallback()
{

}

