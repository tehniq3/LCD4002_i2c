
//************************************************************
//********Program name;- _4002_LCD_clocks_006*****************
//*********ALL code written by Erney Stephens G8XEU***********
//***Program written in c++ for micro ************************
//***controller type ATMEGA328P, which in turn controls*******
//************40 X 02 LCD via PCF8574 I2C chip****************
//***Where possible I try not to use 3rd Party libraries******
//***all set and controlled manually so I can see how they**** 
//*********work and for that reason only. I thank you.********
//************************************************************


#include <Wire.h>
uint8_t RTC[20], temperature, initdata = 2, bkl, I2C_address, screensize = 4;
int16_t timeinfo = 1e3; //1 second between displays
String dow1[] =
{
  "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY", "SUNDAY"
};
String months[] =
{
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November", "December"
};
void setup()
{
  //Serial.begin(9600); Serial.println("=");
  Wire.begin();
  //I2Cscanner(); //prints out I2C connected devices

  Write_Clock_Date();//used to set DS3231 or DS1307 clock. remark this line out with // once clock has been set

 // next four line determin address of LCD normally 0x27 or 0x3f, if not you'll have to run I2C scanner 
  Wire.beginTransmission(0x27);
  if (Wire.endTransmission() == 0) I2C_address = 0x27;
  Wire.beginTransmission(0x3f);
  if (Wire.endTransmission() == 0) I2C_address = 0x3f;

  LCD_setup();
  LCD_set_custom_characters();
  LCD_cls();

  for (int8_t g = 0; g < 3; g++)
  {
    for (int8_t f = 0; f < 2; f++)
    {
      LCD_backlight(f); delay(250);
    }
  }
  c_big(); delay(5000);//This line just displays custom characters. Remove if not needed.
}

void loop()
{
  getDS3231();//Gets clock day month year
  getTemperature();//Gets temperature

  //TIME
  LCD_cls();
  String tictoc = "";
  for (byte f = 2; f < 6; f++)
  {
    tictoc += (String (RTC[f]));
    if (f == 3) tictoc += (":");
  }
  Mr_Big("TIME", 1, 0, 4);
  Mr_Big(tictoc, 20, 0, 4);
  delay(timeinfo);

  // DATE
  LCD_cls();
  tictoc = "";
  for (byte f = 7; f < 11; f++)
  {
    tictoc += (String (RTC[f]));
    if (f == 8) tictoc += ("[");//puts custom dots into date
  }
  Mr_Big("DATE", 1, 0, 4);
  Mr_Big(tictoc, 20, 0, 4);
  delay(timeinfo);

  // Day Of Week dow
  LCD_cls();
  Mr_Big(dow1[RTC[6]], 2, 0, 4);
  delay(timeinfo);

  // YEAR
  LCD_cls();
  tictoc = "";
  for (byte f = 11; f < 15; f++) tictoc += (String (RTC[f]));
  Mr_Big("YEAR", 3, 0, 4);
  Mr_Big(tictoc, 21, 0, 4);
  delay(timeinfo);

  // TEMPERATURE IN C
  if (1 == 1)
  {
    LCD_cls();
    Mr_Big("TEMP", 3, 0, 4);
    Mr_Big(String (temperature), 24, 0, 4);
    LCD_cursor(31, 0); I2C(0xdf, 1);
    Mr_Big("C", 32, 0, 4);
    delay(timeinfo);
  }
  //LCD_cls(); c_big(); delay(5000);//This line just displays custom characters. Remove if not needed.
}

void LCD_setup()
{
  screensize = 4; // screensize = 0;

  // Initializing by instruction. Page 46 Figure 24  4-Bit Interface
  // wait 40ms after power incase voltage is below 2.7V

  uint8_t Initializing [9] = {0x33, 0x32, 0x28, 0x0c, 0x01, 0x02};
  // Yes I know this looks strange, but the data sheet says send three
  // 8bit mode commands and one 4bit command ie
  // 0011 0011 0011 0010 which is what hex 0x33 and 0x32 do.
  // (Remember each byte is shifted by << 4 bits)
  // Nevertheless this unusual Initializing procedure correctly sets the LCD.

  delay(50);
  for (uint8_t f = 0; f < 6; f++)
  {
    I2C(Initializing [f], 0);
  }
  initdata = 0;
}

void LCD_print_at (uint8_t a, uint8_t b, String as)
{
  LCD_cursor(a, b);
  LCD_print (as);
}

void LCD_print (String massage)
{
  for (uint8_t f = 0; f < massage.length(); f++) I2C(massage[f], 1);
}

void LCD_cursor(uint8_t row, uint8_t col)
{
  uint8_t from[] {128, 192, 144 + screensize, 208 + screensize};
  I2C (from[col] + row, 0);
}

void LCD_backlight(uint8_t onoff)
{
  bkl = onoff;//backlight
  I2C (0x02, 0);//return home
  delay(2);
}

void  LCD_cls()
{
  I2C (0x01, 0);//Clear screen
  delay(2);
  I2C (0x02, 0); //Cursor 0,0
  delay(2);
}

void I2C (uint8_t cmd, uint8_t rs)
{
  for (uint8_t shift_bit = 0; shift_bit < 5; shift_bit += 4)
  { //when shift_bit=0 (cmd bits 4567 into cmd_bits bits 4567)
    //then when shift_bit=4 (cmd bits 0123 shifted into cmd_bits bits 4567)
    uint8_t cmd_bits = cmd << shift_bit;
    bitWrite(cmd_bits, 0, rs);// RegisterSelect  0=command  1=char.
    bitWrite(cmd_bits, 1, 0);// R/W 1=read 0=write
    bitWrite(cmd_bits, 2, 1); //Enable bit set HIGH.
    bitWrite(cmd_bits, 3, bkl); //led lcd 1=on, 0=off.
    Write_I2C (cmd_bits); //Send enable set High, with data.
    delay(initdata);//wait while LCD captures data
    bitWrite(cmd_bits, 2, 0); //Enable bit set LOW.
    Write_I2C (cmd_bits); //Send enable bit set low.
    delay(initdata);//wait before next action
  }
}

void Write_I2C(uint8_t txData)
{
  Wire.beginTransmission(I2C_address);
  Wire.write(txData);
  Wire.endTransmission();
}
void LCD_set_custom_characters()
{
  uint8_t bigchars [64] =
  {
    7, 15, 31, 31, 31, 31, 31, 31, 31, 31, 31, 0, 0, 0, 0, 0, 28, 30, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
    31, 31, 15, 7, 0, 0, 0, 0, 0, 31, 31, 31, 31, 31, 31, 31, 31, 31, 30, 28, 31, 31, 31, 0, 0, 0, 31, 31,
    14, 14, 14, 14, 31, 27, 27, 27
  };
  I2C(0x40, 0); //these two lines set custom charactors
  for (uint8_t f = 0; f < 64; f++) I2C(bigchars [f], 1);
}

void c_big()
{
  LCD_print_at (0, 0, "These are the custom characters");
  LCD_cursor(0, 1);
  uint8_t reso [] {254, 255, 0, 1, 2, 3, 4, 5, 6, 7};
  char ares[] = "P012345678";
  for (int8_t f = 0; f < 10; f++)
  {
    I2C (ares[f], 1); I2C (61, 1); I2C (reso[f], 1); I2C (32, 1);
  }
}

void Mr_Big (String My_Mes, uint8_t xcord, uint8_t ycord, uint8_t space)
{
  char big_Font[] =
    "12345651PP0P273155273556450PP0077556177456223P1P173456173556"
    //0     1     2     3     4     5     6     7     8     9
    "P5PP5PPPPP5P12P45P55P55PP23P56PPP22PPG8XEU"
    //:=:   .=;   (=<   ===   )=>   '=?   +=@
    "1230200730561224550230560720550720PP1224560500P0202505PP04560520250PP0553510P0"
    //A     B     C     D     E     F     G     H     I     J     K     L     M
    "0230P01234560730PP123457023023172556202P0P0P04563P14560P04864561P33P1P0P276175"
    // N     O     P     Q     R     S     T     U     V     W     X     Y     Z
    "P5PP2PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP";
  // [     \     ]     ^     _     '     a     b     c     d     e     f
  for (uint8_t f = 0; f < My_Mes.length(); f++) {
    uint8_t poc = My_Mes.charAt(f) - 48; //poc=posisiot of character
    for (uint8_t g = 0; g < 4; g += 3) {
      for (uint8_t i = 0; i < 3; i++) {
        LCD_cursor( xcord + (f * space) + i, ycord + (g / 3));
        I2C((big_Font[poc * 6 + i + g] - 49), 1);
      }
    }
  }
}
void I2Cscanner()
{
  uint8_t counter1;
  for (int f = 0; f < 127; f++)
  {
    Wire.beginTransmission(f);
    if (Wire.endTransmission() == 0)
    {
      counter1++;
      Serial.print ("I2C Device found at Hex = 0x");
      Serial.println (f, HEX);
    }
  }
  Serial.print ("Number of I2C Devices found = ");
  Serial.println (counter1);
}

void getTemperature()
{
  Wire.beginTransmission(0x68);
  Wire.write(0x11);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 2);
  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read() >> 6;
  temperature = 0.25 * lsb + msb;
}

void getDS3231()// new routine from Dec 2107 to get time date
// year and put that data straight into RTC[]
{
  uint8_t a;
  Wire.beginTransmission(0x68);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 7);
  a = b2d(Wire.read() & 0x7f); //sec
  RTC[0] = a / 10; RTC[1] = a % 10;
  a = b2d(Wire.read());//mins
  RTC[4] = a / 10; RTC[5] = a % 10;
  a = b2d(Wire.read() & 0x3f); //hours
  RTC[2] = a / 10; RTC[3] = a % 10;
  a = b2d(Wire.read());//dow
  RTC[6] = a;
  a = b2d(Wire.read());//day
  RTC[7] = a / 10; RTC[8] = a % 10;
  a = b2d(Wire.read());//date
  RTC[9] = a / 10; RTC[10] = a % 10;
  RTC[11] = 2; RTC[12] = 0;
  a = b2d(Wire.read());//year
  RTC[13] = a / 10; RTC[14] = a % 10;
}

byte b2d(byte val) {
  return ( (val / 16 * 10) + (val % 16) );
}
byte d2b(byte val) {
  return ( (val / 10 * 16) + (val % 10) );
}

void Write_Clock_Date()//Data to set clock and date
{
  Wire.beginTransmission(0x68);
  Wire.write(0);
  Wire.write(d2b(30)); // seconds
  Wire.write(d2b(44)); // minutes
  Wire.write(d2b(22)); // hours
  Wire.write(d2b(3)); // dow
  Wire.write(d2b(07)); // date
  Wire.write(d2b(5)); // month
  Wire.write(d2b(18)); // year
  Wire.endTransmission();
  delay(10);
}
