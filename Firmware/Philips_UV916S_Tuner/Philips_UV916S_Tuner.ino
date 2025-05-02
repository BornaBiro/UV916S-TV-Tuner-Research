// Include Wire (I2C) Arduino Library.
#include <Wire.h>

// Tuner PLL Information.
#define PLL_FREQ    104.4E6
#define PLL_PRES    8.0
#define PLL_IF      38E6  // TV Tuners have weird IF frequency... 10.7MHz doesn't work!
#define PLL_REF     4E6
#define PLL_REF_DIV 512

// Set the I2C device address.
#define PLL_I2C_ADDR  0b11000000 >> 1

void setup()
{
    // Initialze I2C library in master mode.
    Wire.begin();

    // Initialize serial librarzy and send hello message (so we know that Arduino is alive).
    Serial.begin(115200);
    Serial.println("Philips UV916S Tuner Test Code Started!");

    // Set frequency!
    if (!setPllDivider(PLL_I2C_ADDR, PLL_FREQ + PLL_IF))
    {
        Serial.println("I2C Send Failed!");
    }
    else
    {
        Serial.println("I2C Send ok!");
    }
}

void loop() {
  // put your main code here, to run repeatedly:

}

bool setPllDivider(uint8_t _i2cAddr, double _freq)
{
    // Let's calculate the devider ratio!
    uint16_t _n = (uint16_t)((PLL_REF_DIV * _freq) / (PLL_REF * PLL_PRES));

    // Send what divider you got!
    Serial.print("PLL Divider: ");
    Serial.println(_n, DEC);

    // And also show frequency data.
    Serial.print("Set freq: ");
    Serial.print(_freq / 1E6, 2);
    Serial.print("MHz, IF: ");
    Serial.print((double)PLL_IF / 1E6, 2);
    Serial.print("MHz, real PLL Freq.: ");
    Serial.print(((PLL_REF * _n * PLL_PRES / PLL_REF_DIV) - PLL_IF) / 1E6, 3);
    Serial.println("MHz");

    // Split it into two bytes!
    uint8_t _pllDivH = (_n >> 8) & 0b01111111;
    uint8_t _pllDivL = _n & 0xFF;

    // Make a data byte for the PLL!
    // FM_Frequency Information [  0 N14 N13 N12 N11 N10  N9  N8]
    // FL_Frequency Information [ N7  N6  N5  N4  N3  N2  N1  N0]
    // CO_Information           [  1 T14 T13 T12 T11 T10  T9  T8]
    // BA_Band Information      [  X  B6  B5  B4   X   X   X   X]
    // T14 - Controls the Charge Pump Current of the Phase Comparator. 1 = Normal operation (125uA).
    // T13 - Switches the Internal Signals Fref and FBY2 to the Band Buffer Outputs (Test). 0 = Normal Operation.
    // T12, T9 - Control the Phase Comparator. T9 = 1; T12 = 0 -> Normal Operation.
    // T11, T10 - Control the Reference Divider. T11 = 0; T10 = 0 -> 512
    // T8 - Controls the Output of the Operational Amplifier. T8 = 0 -> Normal Operaion
    uint8_t _coInfomration = 0b11000010;
    uint8_t _baInformation = 0b01100000;

    // Send everything!
    Wire.beginTransmission(_i2cAddr);
    Wire.write(_coInfomration);
    Wire.write(_baInformation);
    Wire.write(_pllDivH);
    Wire.write(_pllDivL);
    int _res = Wire.endTransmission();

    // Check for success.
    return _res == 0?true:false;
}