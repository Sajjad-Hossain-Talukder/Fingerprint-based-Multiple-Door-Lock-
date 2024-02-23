#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <Servo.h>


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
bool cellState[] = {false,false,false,false,false};
int servoMode[] = {0,0,0,0,0};
int servoPin[] = {53,51,49,47,45};
int stateManageToLock ;


void setup()
{
  Serial.begin(9600);
  for(int i = 0 ; i < 5 ; i+=1 ){
    cellServo[i].attach(servoPin[i]);
    cellServo[i].write(servoMode[i]);
    servoMode[i] = 90 ;
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
/*
  Serial.println(F("Reading sensor parameters"));
  finger1.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger1.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger1.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger1.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger1.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger1.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger1.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger1.baud_rate);
*/
}






int readCellNumber(void) {


  int cellNumber = -1 ;
  while ( cellNumber == -1 ) {
    char key = keypad.getKey();
    if (key >= '1' && key <= '5')
      cellNumber = key - '1';
  }
  return cellNumber;
}






void loop(){
  Serial.println("Enter Cell Number : ");
  int cellNumber = readCellNumber();
  if (cellNumber < 0 && cellNumber > 4 ) {
    return;
  }


  Adafruit_Fingerprint finger = finger0 ;


    switch (cellNumber ){
      case 1 :
        finger = finger1 ;
        break ;
      case 2 :
        finger = finger2 ;
        break ;
      case 3 :
        finger = finger3 ;
        break ;
      default :
        finger = finger4 ;
    }




  if(cellState[cellNumber] == false ){
   


    Serial.print("Cell is Open. Enroll cell # ");
    Serial.println(cellNumber);


    while ( !getFingerprintEnroll(finger, cellNumber) );


    cellServo[cellNumber].write(servoMode[cellNumber]);
    cellState[cellNumber] = true ;
    servoMode[cellNumber] = 0 ;


    Serial.println("Cell is Locked");


  }
  else {


    Serial.println("Sell is Close. ");
    Serial.print("Waiting for valid finger to Verify Cell  #");


    if (getFingerprintID(finger) ){


      cellServo[cellNumber].write(servoMode[cellNumber]);
      cellState[cellNumber] = false ;
      servoMode[cellNumber] = 90 ;


    } else {
        Serial.println("Not matched. Start Again ");
    }




  }




}


bool getFingerprintEnroll( Adafruit_Fingerprint finger, int id) {


  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if ( p == FINGERPRINT_OK ){
      //Serial.println("Image taken");
      break;
    } else {
      Serial.print(".");
    }
  }


  // OK success!


  p = finger.image2Tz(1);
  if( p == FINGERPRINT_OK ){
    //Serial.println("Image converted");
  } else {
    //Serial.println("Failed Image converted");
    return false ;
  }




  Serial.println("");
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }






  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if ( p == FINGERPRINT_OK ){
      //Serial.println("Image taken");
      break;
    }  else {
      Serial.print(".");
    }
  }


  // OK success!


  p = finger.image2Tz(2);
  if( p == FINGERPRINT_OK ){
   // Serial.println("Image 2 converted");
  } else {
   // Serial.println("Failed 2 Image converted");
    return false ;
  }  


  // OK converted!
  Serial.println("");
  Serial.print("Creating model for #");  Serial.println(id);


  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Prints matched!");
  } else {
    //Serial.println("Unknown error");
    return false;
  }  


  p = finger.storeModel(id);


  if (p == FINGERPRINT_OK) {
    //Serial.println("Stored!");
  } else {
   // Serial.println("Unknown error");
    return false;
  }


  //Serial.print("fingerID : ");
  //Serial.println(finger.fingerID);


  return true;


}




bool getFingerprintID(Adafruit_Fingerprint finger) {


  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if(p==FINGERPRINT_OK){
      Serial.println("Image taken");
      break;
    } else if ( p == FINGERPRINT_NOFINGER ) {
        Serial.print(".");
    } else {
        Serial.println("Error");
    }
  }


  // OK success!


  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    default:
      Serial.println("Error");
      return false;
  }


  Serial.println("Remove finger");
  delay(2000);




  // OK converted!
  p = finger.fingerSearch();


  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  }  else {
    Serial.println("Did not find a match");
    return false;
  }


  // found a match!


  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);


  uint16_t fingerID = finger.fingerID;
  p = finger.deleteModel(fingerID);
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint deleted!");
  } else {
    Serial.println("Failed to delete fingerprint");
  }


  return true ;
}

