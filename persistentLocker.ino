#include <EEPROM.h>
#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <Servo.h>
#include <LiquidCrystal.h>




#define LCD_RS A0
#define LCD_EN A1
#define LCD_D4 A2
#define LCD_D5 A3
#define LCD_D6 A4
#define LCD_D7 A5




#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(18, 19);
#else
#define mySerial Serial1
#endif




Adafruit_Fingerprint finger0 = Adafruit_Fingerprint(&mySerial);
Adafruit_Fingerprint finger1 = Adafruit_Fingerprint(&mySerial);
Adafruit_Fingerprint finger2 = Adafruit_Fingerprint(&mySerial);
Adafruit_Fingerprint finger3 = Adafruit_Fingerprint(&mySerial);
Adafruit_Fingerprint finger4 = Adafruit_Fingerprint(&mySerial);




Servo cellServo[5];
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);




const byte ROW_NUM    = 4;
const byte COLUMN_NUM = 4;




char keys[ROW_NUM][COLUMN_NUM] = {
 {'1','2','3','A'},
 {'4','5','6','B'},
 {'7','8','9','C'},
 {'*','0','#','D'}
};




byte pin_rows[ROW_NUM] = {9, 8, 7, 6};
byte pin_column[COLUMN_NUM] = {5, 4, 3, 2};




Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);
bool cellState[] = {false, false, false, false, false};
int servoMode[] = { 0,  0,  0, 0, 0};
int servoPin[] = {53, 51, 49, 47, 45};
int ledPins[] = {31, 33, 35, 37, 39}; // LED pins corresponding to each servo




void setup() {
 Serial.begin(9600);
 lcd.begin(16, 2);
 lcd.clear();




 for (int i = 0; i < 5; i += 1) {
  cellServo[i].attach(servoPin[i]);
   pinMode(ledPins[i], OUTPUT);
  
  boolean state;
  EEPROM.get(i,state);

  if (state == 0xFF) {
    pinMode(ledPins[i], LOW); // Set LED pins as outputs
    cellServo[i].write(0);
  } else {
    if(state){
      pinMode(ledPins[i], HIGH);
      cellServo[i].write(90);
    } else {
      pinMode(ledPins[i], LOW);
      cellServo[i].write(0);
    }
  }

 }




 while (!Serial);
 delay(100);




 Serial.println("\n\nAdafruit Fingerprint sensor enrollment");
 finger0.begin(57600);
 finger1.begin(57600);
 finger2.begin(57600);
 finger3.begin(57600);
 finger4.begin(57600);




 if (finger1.verifyPassword()) {
   Serial.println("Found fingerprint sensor!");
 } else {
   Serial.println("Did not find fingerprint sensor :(");
   while (1) { delay(1); }
 }




Serial.println("Door Lock System");
 lcd.print("Door Lock System");
 delay(2000);  // Adding a delay to see the "Door Lock System" message
}




int readCellNumber(void) {
 int cellNumber = -1;
 while (cellNumber == -1) {
   char key = keypad.getKey();
   if (key >= '1' && key <= '5'){
     cellNumber = key - '1';
   }
 }
 return cellNumber;
}




bool readBackContinueState(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.println("Enter # to Back ");
  lcd.setCursor(0, 1);
  lcd.println("& * to Continue ");
 
 int state = -1;




 while (state == -1) {
   char key = keypad.getKey();
   if (key == '*'){
     state = 1;
   } else if ( key == '#'){
    state = 2;
   }
 }
 if(state==1) return true ;
 return false ;
}




void loop() {
 lcd.clear();
 lcd.print("Enter Cell : ");




 int cellNumber = readCellNumber();
 if (cellNumber < 0 && cellNumber > 4) {
   return;
 }




  lcd.print(cellNumber+1);
  delay(2000);




 if(readBackContinueState()){
   Adafruit_Fingerprint finger = finger0;




   switch (cellNumber) {
     case 1:
       finger = finger1;
       break;
     case 2:
       finger = finger2;
       break;
     case 3:
       finger = finger3;
       break;
     default:
       finger = finger4;
   }




    if (cellState[cellNumber] == false) {
     lcd.clear();
     lcd.print("Cell #");
     lcd.print(cellNumber+1);
     lcd.print(" Open. Enroll");
     delay(2000);




     while (!getFingerprintEnroll(finger, cellNumber));


    servoMode[cellNumber] = 90;
     cellServo[cellNumber].write(servoMode[cellNumber]);
     digitalWrite(ledPins[cellNumber], HIGH); // Turn on LED
     cellState[cellNumber] = true;
     EEPROM.put(cellNumber, true);






     lcd.clear();
     lcd.print("Cell #");
     lcd.print(cellNumber+1);
     lcd.print(" Locked");
     delay(2000);
   } else {
     lcd.clear();
     lcd.print("Cell #");
     lcd.print(cellNumber+1);
     lcd.print(" Closed.");
     delay(2000);
     lcd.print("Waiting for valid");
     lcd.setCursor(0, 1);
     lcd.print("finger to Verify");
     delay(2000);




     if (getFingerprintID(finger)) {
       servoMode[cellNumber] = 0;
       cellServo[cellNumber].write(servoMode[cellNumber]);
       digitalWrite(ledPins[cellNumber], LOW); // Turn off LED
       cellState[cellNumber] = false;
       EEPROM.put(cellNumber, false);

       lcd.clear();
       lcd.print("Successfully Opened");
       delay(2000);
     } else {
       lcd.clear();
       lcd.print("Not matched. Start Again");
       delay(2000);
     }
   }
 } else {
     return ;
 }
}




bool getFingerprintEnroll(Adafruit_Fingerprint finger, int id) {
 int p = -1;
  lcd.clear();
 lcd.print("Place finger");
 lcd.setCursor(0, 1);
 lcd.print("on sensor");
  while (p != FINGERPRINT_OK) {
   p = finger.getImage();
   if (p == FINGERPRINT_OK) {
     lcd.clear();
     lcd.print("Image taken");
     delay(1000);
     break;
   } else {
     lcd.print(".");
   }
 }




 p = finger.image2Tz(1);
 if (p == FINGERPRINT_OK) {
   lcd.clear();
   lcd.print("Image converted");
   delay(1000);
 } else {
   lcd.clear();
   lcd.print("Failed Image converted");
   delay(1000);
   return false;
 }




 lcd.clear();
 lcd.print("Remove finger");
 delay(2000);
 p = 0;
 while (p != FINGERPRINT_NOFINGER) {
   p = finger.getImage();
 }




 p = -1;
 lcd.clear();
 lcd.print("Place same finger");
 lcd.setCursor(0, 1);
 lcd.print("again");
 while (p != FINGERPRINT_OK) {
   p = finger.getImage();
   if (p == FINGERPRINT_OK) {
     lcd.clear();
     lcd.print("Image taken");
     delay(1000);
     break;
   } else {
     lcd.print(".");
   }
 }




 p = finger.image2Tz(2);
 if (p == FINGERPRINT_OK) {
   lcd.clear();
   lcd.print("Image 2 converted");
   delay(1000);
 } else {
   lcd.clear();
   lcd.print("Failed 2 Image converted");
   delay(1000);
   return false;
 }




 lcd.clear();
 lcd.print("Creating model");
 lcd.setCursor(0, 1);
 lcd.print("for #");
 lcd.print(id+1);
 delay(2000);




 p = finger.createModel();
 if (p == FINGERPRINT_OK) {
   lcd.clear();
   lcd.print("Prints matched!");
   delay(2000);
 } else {
   lcd.clear();
   lcd.print("Unknown error");
   delay(2000);
   return false;
 }




 p = finger.storeModel(id);
 if (p == FINGERPRINT_OK) {
   lcd.clear();
   lcd.print("Stored!");
   delay(2000);
 } else {
   lcd.clear();
   lcd.print("Unknown error");
   delay(2000);
   return false;
 }




 return true;
}




bool getFingerprintID(Adafruit_Fingerprint finger) {
 int p = -1;
 lcd.clear();
 lcd.print("Place finger");
 lcd.setCursor(0, 1);
 lcd.print("on sensor");




 while (p != FINGERPRINT_OK) {
   p = finger.getImage();
   if (p == FINGERPRINT_OK) {
     lcd.clear();
     lcd.print("Image taken");
     delay(1000);
     break;
   } else if (p == FINGERPRINT_NOFINGER) {
     lcd.print(".");
   } else {
     lcd.clear();
     lcd.print("Error");
   }
 }




 p = finger.image2Tz(1);
 switch (p) {
   case FINGERPRINT_OK:
     lcd.clear();
     lcd.print("Image converted");
     delay(1000);
     break;
   default:
     lcd.clear();
     lcd.print("Error");
     delay(1000);
     return false;
 }




 lcd.clear();
 lcd.print("Remove finger");
 delay(2000);




 p = finger.fingerSearch();




 if (p == FINGERPRINT_OK) {
   lcd.clear();
   lcd.print("Found a print match!");
   delay(1000);
 } else {
   lcd.clear();
   lcd.print("Did not find a match");
   delay(1000);
   return false;
 }




 lcd.clear();
 lcd.print("Found ID #");
 lcd.print(" with confidence of ");
 lcd.print(finger.confidence);
 delay(2000);




 uint16_t fingerID = finger.fingerID;
 p = finger.deleteModel(fingerID);
 if (p == FINGERPRINT_OK) {
   lcd.clear();
   lcd.print("Fingerprint deleted!");
   delay(2000);
 } else {
   lcd.clear();
   lcd.print("Failed to delete fingerprint");
   delay(2000);
 }




 return true;
}
