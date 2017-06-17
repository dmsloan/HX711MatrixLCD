#include <Arduino.h>
/*This example code uses bogde's excellent library: https://github.com/bogde/HX711

 bogde's library is released under a GNU GENERAL PUBLIC LICENSE

 Arduino pin
 3 -> HX711 CLK
 12 -> DOUT
 5V -> VCC
 GND -> GND

 Most any pin on the Arduino Uno will be compatible with DOUT/CLK for the HX711.

 The HX711 board can be powered from 2.7V to 5V so the Arduino 5V power should be fine.
*/

// include the graphics library code:
#include <Adafruit_GFX.h> //-----( Import library that processes graphics )-----

// include the library code for the shift register in the matrix display:
#include <Max72xxPanel.h> //-----( Import library for the Max72xx that controls the matrix)----- DEPENDS on the Adafruit_GFX.h library

// include the library code for the LCD display:
#include <LiquidCrystal.h>
// include the library code for the load cell amplifier:
#include <HX711.h>

// include the library code for the Serial Peripheral Interface (SPI) that is used by the Max72xx Arduino pin 11 for MOSI(DIN on the matrix)
#include <SPI.h>

// initialize the LCD library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// initialize the HX711 library with the numbers of the interface pins
#define DOUT 12
#define CLK 3

HX711 scale(DOUT, CLK);

//float calibration_factor = 886600; //for lbs on the 1kg scale
float calibration_factor = -1955; //1955 for g for my 1kg max scale setup

// Set up the matrix
//  DIN to MOSI(pin11 on the Uno) and CLK to SCK(pin13 on the Uno) (cf http://arduino.cc/en/Reference/SPI )
int pinCS = 10; // Attach CS to this pin
int numberOfHorizontalDisplays = 1;
int numberOfVerticalDisplays = 1;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

void setup()
{
  matrix.setIntensity(0);
  matrix.setRotation(3);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("LCD Key Shield");
  lcd.setCursor(0, 1);
  lcd.print("Press Key:");

  Serial.begin(9600);
  Serial.println("HX711 calibration sketch modified by Derek.");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");

  scale.set_scale();
  scale.tare(); //Reset the scale to 0

  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: ");           //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);

  lcd.clear();
} /*--(end setup )---*/

/*
 Adjust to your own needs
  matrix.setPosition(0, 0, 0); // The first display is at <0, 0>
  matrix.setPosition(1, 1, 0); // The second display is at <1, 0>
  matrix.setPosition(2, 2, 0); // The third display is at <2, 0>
  matrix.setPosition(3, 3, 0); // And the last display is at <3, 0>
  ...
  matrix.setRotation(0, 2);    // The first display is position upside down
  matrix.setRotation(3, 2);    // The same hold for the last display
*/
int wait = 50;
int inc = -2;

void loop() /*----( LOOP: RUNS CONSTANTLY )----*/
{
  static uint8_t PROGMEM const ibe[] =  //create an IBE logo to display on an 8X8 matrix
      {B11111111,
       B10000001,
       B10100001,
       B11110001,
       B10101111,
       B10000001,
       B11111111};

  static uint8_t PROGMEM const crosshatch[] =  //create a heart to display on an 8X8 matrix
      {B00000000,
       B01100110,
       B11111111,
       B11111111,
       B11111111,
       B01111110,
       B00111100,
       B00011000};

  matrix.fillScreen(LOW);                          //clear the display
  matrix.drawBitmap(0,0,ibe,8,8,HIGH);             //setup the image to display
  matrix.write();                                  //display the image
  delay(1000);
  matrix.fillScreen(LOW);                          //clear the display

  matrix.drawBitmap(0,0,crosshatch,8,8,HIGH);      //setup the image to display
  matrix.write();                                  //display the image
  delay(1000);
  matrix.fillScreen(LOW);                          //clear the display

  scale.set_scale(calibration_factor);             //Adjust to this calibration factor

  float reading = 0;
  reading = scale.get_units(5), 1; //Read scaled units, average 5 readings, display 1 decimal places

  if (reading < .5){
    scale.tare();                    //Reset the scale to 0
    reading = scale.get_units(5), 1; //Read scaled units, average 5 readings, display 1 decimal places
  }

  Serial.print("Reading: "); // Serial.print(scale.get_units(15), 1); //Read scaled units, average 15 readings, display 1 decimal places
  Serial.print(reading);
  Serial.print(" g"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
  Serial.print(" cal_fctr: ");
  Serial.print(calibration_factor);
  Serial.print(" RawData: ");
  Serial.println(scale.read());
  Serial.println();

  lcd.setCursor(0, 0);
  lcd.print(reading);
  lcd.setCursor(8, 0);
  lcd.print(" Grams");

  //Read inputs for Left, Right, Up, Down and Select buttons that are connected to A0

  int x;
  x = analogRead(0);
  lcd.setCursor(10, 1);
  if (x < 60){
    lcd.print("Right ");
  }
  else if (x < 200){
    lcd.print("Up    ");
  }
  else if (x < 400){
    lcd.print("Down  ");
  }
  else if (x < 600){
    lcd.print("Left  ");
  }
  else if (x < 800){
    lcd.print("Zero");
    scale.tare(); //Reset the scale to 0
  }

  if (Serial.available()){
    char temp = Serial.read();
    if (temp == '+' || temp == 'a')
      calibration_factor += 1;
    else if (temp == '-' || temp == 'z')
      calibration_factor -= 1;
  }

  matrix.setTextWrap(true);
  matrix.print("Weight=");
  matrix.print(reading);
  matrix.write();
  delay(wait * 1.0);
  matrix.setTextWrap(true);
  matrix.fillScreen(LOW);

  for (int x = 15; x > -1; x--)
  {
    matrix.drawChar(2, 0, x + 48, HIGH, LOW, 1);
    matrix.setIntensity(x);
    matrix.write();
    delay(wait * 8);
  }

  for (int x = 0; x < 16; x++)
  {
    matrix.fillScreen(HIGH);
    matrix.setIntensity(x);
    matrix.write();
    delay(wait * 4);
  }
  /*
  matrix.shutdown(true); // Shut-down the display
  matrix.setIntensity(0);
  matrix.fillScreen (HIGH);
  matrix.drawChar(2, 0, 'R', LOW, HIGH, 1);
  matrix.write();
  delay (3000);
  matrix.shutdown (false); // Turn on the display
  matrix.fillScreen (LOW);
*/
  matrix.setIntensity(0);
  matrix.setTextColor(HIGH, LOW);
  //  matrix.setTextColor(uint16_t color, uint16_t backgroundcolor);
  //  matrix.setTextSize(uint8_t size);
  matrix.setTextWrap(false);

  for (int8_t x = 8; x >= -90; x--) // 96 is number of characters to display x 8
  {
    matrix.fillScreen(LOW);
    matrix.setCursor(x, 0);
    matrix.write();
    delay(wait * 1.5);
  }
  matrix.fillScreen(LOW);

  for (int y = 0; y < matrix.height(); y++)
  { //fill the matrix from top to bottom, left to right one dot at a time
    for (int x = 0; x < matrix.width(); x++)
    {
      matrix.drawPixel(x, y, HIGH);
      matrix.write();
      delay(wait);
    }
  }

  for (int x = 0; x < matrix.width() - 1; x++)
  {
    matrix.fillScreen(LOW);
    matrix.drawLine(x, 0, matrix.width() - 1 - x, matrix.height() - 1, HIGH);
    matrix.write(); // Send bitmap to display
    delay(wait);
  }

  for (int y = 0; y < matrix.height() - 1; y++)
  {
    matrix.fillScreen(LOW);
    matrix.drawLine(matrix.width() - 1, y, 0, matrix.height() - 1 - y, HIGH);
    matrix.write(); // Send bitmap to display
    delay(wait);
  }

  for (int x = 0; x < matrix.width() / 2; x++)
  {
    matrix.fillScreen(LOW);
    matrix.drawRect(x, x, matrix.width() - x - x, matrix.height() - x - x, HIGH);
    matrix.write(); // Send bitmap to display
    delay(wait * 8);
    matrix.fillScreen(LOW);
  }

  for (int x = 0; x < matrix.width() / 2; x = x + 2)
  {
    matrix.drawRect(x, x, matrix.width() - x - x, matrix.height() - x - x, HIGH);
    matrix.write(); // Send bitmap to display
    delay(wait * 8);
  }

  for (int x = 0; x < matrix.width() / 2; x++)
  {
    matrix.fillScreen(LOW);
    matrix.drawCircle(matrix.width() / 2, matrix.width() / 2, x, HIGH);
    matrix.write(); // Send bitmap to display
    delay(wait * 8);
  }

  for (int x = 0; x < matrix.width(); x++)
  {
    matrix.fillScreen(LOW);
    matrix.drawRoundRect(0, 0, 8, 8, x, HIGH);
    matrix.write(); // Send bitmap to display
    delay(wait * 8);
  }

  for (int x = 0; x < matrix.width(); x++)
  {
    matrix.fillScreen(LOW);
    matrix.drawFastHLine(0, x, matrix.width(), HIGH);
    matrix.write();
    delay(wait);
  }

  for (int x = 0; x < matrix.width(); x++)
  {
    matrix.fillScreen(LOW);
    matrix.drawFastVLine(x, 0, matrix.width(), HIGH);
    matrix.write();
    delay(wait);
  }

  wait = wait + inc;
  if (wait == 0)
    inc = 2;
  if (wait == 50)
    inc = -2;

} /* --(end main loop )-- */