#include <Wire.h>
#include <WildFire.h>
#include <MCP342x.h>
#include <LMP91000.h>
#include <SHT25.h>

WildFire wf;
LMP91000 lmp91000;
MCP342x adc;
SHT25 sht25;

#define TEST_VARIOUS_LMP91000_SETTINGS_PER_SLOT 0

void selectSlot1(void);
void selectSlot2(void);
void selectSlot3(void);

void (*slot_select[])(void) = { selectSlot1, selectSlot2, selectSlot3 };

void setup() {
  // put your setup code here, to run once:
  wf.begin();
  Wire.begin();

  Serial.begin(115200);

  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  pinMode(10, OUTPUT);
  digitalWrite(10, LOW);

  Serial.println(F("================================================="));

  selectNoSlot();
  boolean pass_status = true;
  for (uint8_t ii = 0; ii < 3; ii++) {
    pass_status = true;

    Serial.println(F("+--------+"));
    Serial.print(F("| SLOT "));
    Serial.print(ii + 1);
    Serial.println(F(" |"));
    Serial.println(F("+--------+"));

    slot_select[ii]();

    Serial.print(F("  Info: Expect Slot "));
    Serial.print(ii + 1);
    Serial.println(F(" to pass because it is selected..."));
    if (testSlot(ii)) {
      pass_status = true;
    }
    else {
      pass_status = false;
    }

    selectNoSlot();

    if (pass_status) {
      Serial.print(F("  Info: Expect Slot "));
      Serial.print(ii + 1);
      Serial.println(F(" to fail because it is not selected..."));
      if (!testSlot(ii)) {
        pass_status = true;
      }
      else {
        pass_status = false;
      }
    }

    if (pass_status) {
      Serial.println(F("PASS"));
    }
    else {
      Serial.println(F("FAIL"));
    }
    Serial.println();
  }

  // Test the SHT25
  Serial.println(F("+--------+"));
  Serial.println(F("| SHT25  |"));
  Serial.println(F("+--------+"));
  Serial.print(F("  Info: SHT25 Initization..."));

  pass_status = true;
  if (sht25.begin()) {
    Serial.println(F("OK."));
    float raw_value = 0.0f;

    Serial.print(F("  Info: SHT25 Temperature..."));
    if (sht25.getTemperature(&raw_value)) {
      Serial.print(raw_value, 2);
      Serial.println(F(" degC"));
    }
    else {
      Serial.println(F("Failed."));
      pass_status = false;
    }

    Serial.print(F("  Info: SHT25 Humidity..."));
    if (sht25.getRelativeHumidity(&raw_value)) {
      Serial.print(raw_value, 2);
      Serial.println(F(" %"));
    }
    else {
      pass_status = false;
      Serial.println(F("Failed."));
    }

    Serial.print(F("  Info: SHT25 Serial Number..."));
    uint8_t serial_number[8] = {0};
    uint8_t tmp[32] = {0};
    sht25.getSerialNumber(serial_number);
    snprintf((char *) tmp, 31, "egg%02x%02x%02x%02x%02x%02x%02x%02x",
             serial_number[0],
             serial_number[1],
             serial_number[2],
             serial_number[3],
             serial_number[4],
             serial_number[5],
             serial_number[6],
             serial_number[7]);
    Serial.println((char *) tmp);
  }
  else {
    pass_status = false;
    Serial.println(F("Failed."));
  }

  if (pass_status) {
    Serial.println(F("PASS"));
  }
  else {
    Serial.println(F("FAIL"));
  }
  Serial.println();

}

boolean testSlot(uint8_t ii) {
  boolean ret = true;

  Serial.print(F("  Info: Slot "));
  Serial.print(ii + 1);
  Serial.print(F(" AFE Initization..."));

  if(ii = 0){
    if (lmp91000.configure(
          LMP91000_TIA_GAIN_350K | LMP91000_RLOAD_10OHM,
          LMP91000_REF_SOURCE_EXT | LMP91000_INT_Z_20PCT
          | LMP91000_BIAS_SIGN_POS | LMP91000_BIAS_1PCT,
          LMP91000_FET_SHORT_DISABLED | LMP91000_OP_MODE_AMPEROMETRIC)) {
      Serial.println(F("OK."));
      //lmp91k_read();
    }
    else {
      Serial.println(F("Failed."));
      ret = false;
    }
  }
  else{
    if (lmp91000.configure(
          LMP91000_TIA_GAIN_350K | LMP91000_RLOAD_10OHM,
          LMP91000_REF_SOURCE_EXT | LMP91000_INT_Z_67PCT
          | LMP91000_BIAS_SIGN_NEG | LMP91000_BIAS_8PCT,
          LMP91000_FET_SHORT_DISABLED | LMP91000_OP_MODE_AMPEROMETRIC)) {
      Serial.println(F("OK."));
      //lmp91k_read();
    }
    else {
      Serial.println(F("Failed."));
      ret = false;
    }    
  }

  Serial.print(F("  Info: Slot "));
  Serial.print(ii + 1);
  Serial.print(F(" ADC Initization..."));
  MCP342x::Config status;
  int32_t value = 0;
  uint8_t err = adc.convertAndRead(MCP342x::channel1, MCP342x::oneShot,
                                   MCP342x::resolution16, MCP342x::gain1, 75000, value, status);

  if (err == 0) {
    Serial.println(F("OK."));
  }
  else {
    Serial.println(F("Failed."));
    ret = false;
  }

#if TEST_VARIOUS_LMP91000_SETTINGS_PER_SLOT
  if (ret) {
    for (uint8_t ii = 0; ii < 6; ii++) {
      float target = 0.0f;
      switch (ii) {
        case 0:
          lmp91000.configure(
            LMP91000_TIA_GAIN_350K | LMP91000_RLOAD_10OHM,
            LMP91000_REF_SOURCE_EXT | LMP91000_INT_Z_20PCT
            | LMP91000_BIAS_SIGN_NEG | LMP91000_BIAS_8PCT,
            LMP91000_FET_SHORT_DISABLED | LMP91000_OP_MODE_AMPEROMETRIC);

          target = -2.01f;
          break;
        case 1:
          lmp91000.configure(
            LMP91000_TIA_GAIN_350K | LMP91000_RLOAD_10OHM,
            LMP91000_REF_SOURCE_EXT | LMP91000_INT_Z_50PCT
            | LMP91000_BIAS_SIGN_NEG | LMP91000_BIAS_8PCT,
            LMP91000_FET_SHORT_DISABLED | LMP91000_OP_MODE_AMPEROMETRIC);

          target = -1.26f;
          break;
        case 2:
          lmp91000.configure(
            LMP91000_TIA_GAIN_350K | LMP91000_RLOAD_10OHM,
            LMP91000_REF_SOURCE_EXT | LMP91000_INT_Z_67PCT
            | LMP91000_BIAS_SIGN_NEG | LMP91000_BIAS_8PCT,
            LMP91000_FET_SHORT_DISABLED | LMP91000_OP_MODE_AMPEROMETRIC);

          target = -0.83f;
          break;
        case 3:
          lmp91000.configure(
            LMP91000_TIA_GAIN_350K | LMP91000_RLOAD_10OHM,
            LMP91000_REF_SOURCE_EXT | LMP91000_INT_Z_20PCT
            | LMP91000_BIAS_SIGN_POS | LMP91000_BIAS_16PCT,
            LMP91000_FET_SHORT_DISABLED | LMP91000_OP_MODE_AMPEROMETRIC);

          target = -2.01f;
          break;
        case 4:
          lmp91000.configure(
            LMP91000_TIA_GAIN_350K | LMP91000_RLOAD_10OHM,
            LMP91000_REF_SOURCE_EXT | LMP91000_INT_Z_50PCT
            | LMP91000_BIAS_SIGN_POS | LMP91000_BIAS_24PCT,
            LMP91000_FET_SHORT_DISABLED | LMP91000_OP_MODE_AMPEROMETRIC);

          target = -1.26f;
          break;
        case 5:
          lmp91000.configure(
            LMP91000_TIA_GAIN_350K | LMP91000_RLOAD_10OHM,
            LMP91000_REF_SOURCE_EXT | LMP91000_INT_Z_67PCT
            | LMP91000_BIAS_SIGN_POS | LMP91000_BIAS_1PCT,
            LMP91000_FET_SHORT_DISABLED | LMP91000_OP_MODE_AMPEROMETRIC);

          target = -0.83f;
          break;

      }
      delay(250);

      uint8_t err = adc.convertAndRead(MCP342x::channel1, MCP342x::oneShot,
                                       MCP342x::resolution16, MCP342x::gain1, 75000, value, status);

      if (err == 0) {
        float voltage = 62.5e-6f * value;

        Serial.print(F("  "));
        Serial.print(F("ADC: "));
        Serial.print(value);
        Serial.print(F(", Voltage: "));
        Serial.print(voltage, 2);
        Serial.print(F("..."));

        float diff = voltage - target;
        if (diff < -0.01f || diff > 0.01f) {
          Serial.println(F("Failed."));
          ret = false;
        }
        else {
          Serial.println(F("OK."));
        }
      }
      else {
        Serial.println(F("Failed."));
        ret = false;
      }
    }
  }
#endif  

  return ret;
}

void loop() {
  // put your main code here, to run repeatedly:

}

/****** GAS SENSOR SUPPORT FUNCTIONS ******/

void selectNoSlot(void) {
  Serial.println(F("  Info: De-selecting All Slots"));
  digitalWrite(7, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
}

void selectSlot1(void) {
  selectNoSlot();
  Serial.println(F("  Info: Selecting Slot 1"));
  digitalWrite(10, HIGH);
}

void selectSlot2(void) {
  selectNoSlot();
  Serial.println(F("  Info: Selecting Slot 2"));
  digitalWrite(9, HIGH);
}

void selectSlot3(void) {
  selectNoSlot();
  Serial.println(F("  Info: Selecting Slot 3"));
  digitalWrite(7, HIGH);
}

void lmp91k_read(void) {
  uint8_t value = 0;

  value = lmp91000.read(LMP91000_STATUS_REG);
  Serial.print(F("STATUS: "));
  hex_println(value);
  if (value == 0) {
    Serial.println(F("  not ready"));
  }
  else if (value == 1) {
    Serial.println(F("  ready"));
  }
  else {
    Serial.println(F("  undefined"));
  }


  value = lmp91000.read(LMP91000_LOCK_REG);
  Serial.print(F("LOCK:   "));
  hex_println(value);
  if (value == 0) {
    Serial.println(F("  Registers 0x10, 0x11 in write mode"));
  }
  else if (value == 1) {
    Serial.println(F("  Registers 0x10, 0x11 in read only mode (default)"));
  }
  else {
    Serial.println(F("  undefined"));
  }

  value = lmp91000.read(LMP91000_TIACN_REG);
  Serial.print(F("TIACN:  "));
  hex_println(value);
  switch (value & B00000011) {
    case 0:
      Serial.println(F("  RLoad selection = 10 Ohm"));
      break;
    case 1:
      Serial.println(F("  RLoad selection = 33 Ohm"));
      break;
    case 2:
      Serial.println(F("  RLoad selection = 50 Ohm"));
      break;
    case 3:
      Serial.println(F("  RLoad selection = 100 Ohm (default)"));
      break;
  }
  switch ((value >> 2) & B00000111) {
    case 0:
      Serial.println(F("  TIA feedback resistance selection = External resistance (default)"));
      break;
    case 1:
      Serial.println(F("  TIA feedback resistance selection =  2.75kOhm"));
      break;
    case 2:
      Serial.println(F("  TIA feedback resistance selection =  3.5kOhm"));
      break;
    case 3:
      Serial.println(F("  TIA feedback resistance selection =  7kOhm"));
      break;
    case 4:
      Serial.println(F("  TIA feedback resistance selection =  14kOhm"));
      break;
    case 5:
      Serial.println(F("  TIA feedback resistance selection =  35kOhm"));
      break;
    case 6:
      Serial.println(F("  TIA feedback resistance selection =  120kOhm"));
      break;
    case 7:
      Serial.println(F("  TIA feedback resistance selection =  350kOhm"));
      break;
  }

  value = lmp91000.read(LMP91000_REFCN_REG);
  Serial.print(F("REFCN:  "));
  hex_println(value);
  switch (value & 0x80) {
    case 0:
      Serial.println(F("  Reference voltage source selection = Internal (default)"));
      break;
    case 0x80:
      Serial.println(F("  Reference voltage source selection = external"));
      break;
    default:
      Serial.println(F("  undefined"));
  }
  switch ((value >> 5) & 3) {
    case 0:
      Serial.println(F("  Internal zero selection = 20% of the source reference"));
      break;
    case 1:
      Serial.println(F("  Internal zero selection = 50% of the source reference (default)"));
      break;
    case 2:
      Serial.println(F("  Internal zero selection = 67% of the source reference"));
      break;
    case 3:
      Serial.println(F("  Internal zero selection = Internal zero circuitry bypassed"));
      break;
  }
  if ((value & B00010000) == 0) {
    Serial.println(F("  Selection of the Bias polarity = Negative (VWE - VRE) < 0V (default)"));
  }
  else {
    Serial.println(F("  Selection of the Bias polarity = Positive (VWE - VRE) > 0V"));
  }
  switch (value & B00001111) {
    case 0:
      Serial.println(F("  BIAS selection =  0% of the source reference (default)"));
      break;
    case 1:
      Serial.println(F("  BIAS selection =  1% of the source reference"));
      break;
    case 2:
      Serial.println(F("  BIAS selection =  2% of the source reference"));
      break;
    case 3:
      Serial.println(F("  BIAS selection =  4% of the source reference"));
      break;
    case 4:
      Serial.println(F("  BIAS selection =  6% of the source reference"));
      break;
    case 5:
      Serial.println(F("  BIAS selection =  8% of the source reference"));
      break;
    case 6:
      Serial.println(F("  BIAS selection =  10% of the source reference"));
      break;
    case 7:
      Serial.println(F("  BIAS selection =  12% of the source reference"));
      break;
    case 8:
      Serial.println(F("  BIAS selection =  14% of the source reference"));
      break;
    case 9:
      Serial.println(F("  BIAS selection =  16% of the source reference"));
      break;
    case 10:
      Serial.println(F("  BIAS selection =  18% of the source reference"));
      break;
    case 11:
      Serial.println(F("  BIAS selection =  20% of the source reference"));
      break;
    case 12:
      Serial.println(F("  BIAS selection =  22% of the source reference"));
      break;
    case 13:
      Serial.println(F("  BIAS selection =  24% of the source reference"));
      break;
    default:
      Serial.println(F("  undefined"));
      break;
  }

  value = lmp91000.read(LMP91000_MODECN_REG);
  Serial.print(F("MODECN: "));
  hex_println(value);
  if ((value & 0x80) == 0) {
    Serial.println(F("  Shorting FET feature = Disabled (default)"));
  }
  else {
    Serial.println(F("  Shorting FET feature = Enabled"));
  }
  switch (value & B00000111) {
    case 0:
      Serial.println(F("  Mode of Operation selection = Deep Sleep (default)"));
      break;
    case 1:
      Serial.println(F("  Mode of Operation selection = 2-lead ground referred galvanic cell"));
      break;
    case 2:
      Serial.println(F("  Mode of Operation selection = Standby"));
      break;
    case 3:
      Serial.println(F("  Mode of Operation selection = 3-lead amperometric cell"));
      break;
    case 6:
      Serial.println(F("  Mode of Operation selection = Temperature measurement (TIA OFF)"));
      break;
    case 7:
      Serial.println(F("  Mode of Operation selection =  Temperature measurement (TIA ON)"));
      break;
    default:
      Serial.println(F("  undefined"));
      break;
  }

}

void hex_println(uint8_t value) {
  Serial.print(F("0x"));
  if (value < 0x10) Serial.print(F("0"));
  Serial.println(value, HEX);
}
