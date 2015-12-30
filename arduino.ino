/* 
 *  ECE 445 Group 26 - Bike Rack Availability Tracking System
 *  Samuel Luo, Jason Pao, Jason Wang
 */

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <Servo.h> 

// The follow four pins are for SPI communication with the PN532 shield.
#define PN532_SCK  (13)
#define PN532_MOSI (11)
#define PN532_SS   (8)
#define PN532_MISO (12)

#define contact_switch (2)
#define base (4)

// Initializes NFC
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

// Creates a servo object to control a servo motor and initialize the starting position for the servo.
Servo myservo;
int pos = 150;

int verify_NFC = 0;
int completed = 0;
int index = 0; // for sending data to the pi in request event

// Buffer to store the UID of the phone - 4 bytes
uint8_t uid[4];

void setup()
{
  // Initialize the UID to 0.
  uid[0] = 0;
  uid[1] = 0;
  uid[2] = 0;
  uid[3] = 0;
  pinMode(contact_switch, INPUT_PULLUP);
  pinMode(base, OUTPUT);
  digitalWrite(base, HIGH); // base always set to high to constantly provide proper voltage to servo
  myservo.attach(3);  // Attach the servo on pin 3 to the servo object 
  Wire.begin(4);                // 
  Wire.onRequest(requestEvent); // Event that runs when Raspberry Pi requests a phone ID
  Wire.onReceive(receiveEvent); // Event that runs when Raspberry Pi is done querying and sends back an acknowledgement
  Serial.begin(9600);
  Serial.println("Hello!");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  Serial.println("Waiting for an ISO14443A Card ...");
  
}

void loop()
{
  boolean success;
  uint8_t uidLength;  // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  
  if (success)
  {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");

    delay(3000);
  }
  else
  {
      Serial.println("No Card Detected");
  }

  // this is where the arduino output is being set
  // lock is currently LOCKED, contact switch is PRESSED, phone is VERIFIED => unlock
  if(verify_NFC == 1 && pos <= 90)
  {
    Serial.println("A");
    Serial.println(digitalRead(contact_switch));
    for (pos = 90; pos <= 150; pos += 1) // goes from 0 degrees to 180 degrees 
    {                                    // in steps of 1 degree 
      myservo.write(pos);                // tell servo to go to position in variable 'pos' 
      delay(15);                         // waits 15ms for the servo to reach the position 
    } 
    uid[0] = 0;
    uid[1] = 0;
    uid[2] = 0;
    uid[3] = 0;
    verify_NFC = 0;
  }
  // lock is currently UNLOCKED, contact switch is PRESSED, phone is VERIFIED => lock
  else if(digitalRead(contact_switch) == 1 && verify_NFC == 1 && pos >= 150)
  {
    Serial.println("B");
    Serial.println(digitalRead(contact_switch));
    for(pos = 150; pos >= 90; pos -= 1) // goes from 0 degrees to 180 degrees 
    {                                   // in steps of 1 degree 
      myservo.write(pos);               // tell servo to go to position in variable 'pos' 
      delay(15);                        // waits 15ms for the servo to reach the position 
    } 
    verify_NFC = 0; 
  }
  // lock is currently LOCKED, contact switch is PRESSED, phone is NOT VERIFIED => remain locked
  else if(digitalRead(contact_switch) == 1 && verify_NFC == 0 && pos <= 90)
  {
    Serial.println("C");
    Serial.println("Incorrect phone");
  }
  
  // lock is currently UNLOCKED, contact switch is NOT PRESSED, phone is VERIFIED => remain unlocked
  else if(digitalRead(contact_switch) == 0 && verify_NFC == 1 && pos >= 150)
  {
    Serial.println("D");
    Serial.println("U-lock not properly inserted");
  }
  // other conditions???
  else
  {
    Serial.println("E");
    Serial.println("no unlock for you");
  }
}

void requestEvent()
{
  Wire.write(uid[index]);
  ++index;
  if (index >= 4)
  {
    index = 0;
    int i = 0;
    for (i = 0; i < 4; i++)
    {
      uid[i] = 0;
    }
  }
}

void receiveEvent(int howMany)
{
  verify_NFC = Wire.read(); // receive byte as an integer
}

