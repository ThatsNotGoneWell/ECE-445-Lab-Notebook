// rf95 demo tx rx.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95  if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf95_server.
// Demonstrates the use of AES encryption, setting the frequency and modem 
// configuration

#include <SPI.h>
#include <RH_RF95.h>
#include <Servo.h>

Servo myservo;  // create servo object to control a servo
Servo myservo1;

/************ Radio Setup ***************/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
  #define RFM95_CS      8
  #define RFM95_INT     7
  #define RFM95_RST     4
  #define LED           13
#endif

#if defined(ADAFRUIT_FEATHER_M0) || defined(ADAFRUIT_FEATHER_M0_EXPRESS) || defined(ARDUINO_SAMD_FEATHER_M0)
  // Feather M0 w/Radio
  #define RFM95_CS      8
  #define RFM95_INT     3
  #define RFM95_RST     4
  #define LED           13
#endif

#if defined (__AVR_ATmega328P__)  // Feather 328P w/wing
  #define RFM95_INT     3  // 
  #define RFM95_CS      4  //
  #define RFM95_RST     2  // "A"
  #define LED           13
#endif

#if defined(ESP8266)    // ESP8266 feather w/wing
  #define RFM95_CS      2    // "E"
  #define RFM95_IRQ     15   // "B"
  #define RFM95_RST     16   // "D"
  #define LED           0
#endif

#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2) || defined(ARDUINO_NRF52840_FEATHER) || defined(ARDUINO_NRF52840_FEATHER_SENSE)
  #define RFM95_INT     9  // "A"
  #define RFM95_CS      10  // "B"
  #define RFM95_RST     11  // "C"
  #define LED           13

#elif defined(ESP32)    // ESP32 feather w/wing
  #define RFM95_RST     13   // same as LED
  #define RFM95_CS      33   // "B"
  #define RFM95_INT     27   // "A"
  #define LED           13
#endif

#if defined(ARDUINO_NRF52832_FEATHER)
  /* nRF52832 feather w/wing */
  #define RFM95_RST     7   // "A"
  #define RFM95_CS      11   // "B"
  #define RFM95_INT     31   // "C"
  #define LED           17
#endif

/* Teensy 3.x w/wing
#define RFM95_RST     9   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_IRQ     4    // "C"
#define RFM95_IRQN    digitalPinToInterrupt(RFM95_IRQ )
*/
 
/* WICED Feather w/wing 
#define RFM95_RST     PA4     // "A"
#define RFM95_CS      PB4     // "B"
#define RFM95_IRQ     PA15    // "C"
#define RFM95_IRQN    RFM95_IRQ
*/

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

int16_t packetnum = 0;  // packet counter, we increment per xmission

void setup() 
{
  pinMode(LED, OUTPUT);     
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial);
  Serial.begin(115200);
  delay(100);

  Serial.println("Arduino LoRa RX Test!");
  
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  rf95.setTxPower(23, false);

  myservo.attach(8);
  myservo1.attach(7);
}


void loop() {
// if (rf95.available())
//  {
//    // Should be a message for us now   
//    uint8_t buf[51];
//    uint8_t len = sizeof(buf);
//    
//    if (rf95.recv(buf, &len))
//    {
//      digitalWrite(LED, HIGH);
//      Serial.println((char*)buf);
//      digitalWrite(LED, LOW);
//    }
//    else
//    {
//      Serial.println("Receive failed");
//    }
//  }
  if (rf95.available())
{
  uint8_t buf[51];
  uint8_t len = sizeof(buf);
    
  if (rf95.recv(buf, &len))
  {
    digitalWrite(LED, HIGH);
    Serial.println((char*)buf);
    digitalWrite(LED, LOW);

    // Split message into tokens using space as delimiter
    char *token;
    token = strtok((char*)buf, " ");

    // Extract pitch and pitch1 values
    float pitch = 0.0;
    float pitch1 = 0.0;

    for (int i = 0; token != NULL; i++)
    {
      if (i == 1)
      {
        pitch = atof(token);
      }
      else if (i == 3)
      {
        pitch1 = atof(token);
      }

      token = strtok(NULL, " ");


    }

    float pitch1_converted = 0;
    pitch1_converted = 180-abs(pitch1);
    
    pitch1_converted -= 60;

    if (pitch1 < 0) {
      pitch1_converted = 180;
    } 

    if (pitch1_converted < 30) {
      pitch1_converted = 30;
    } else if (pitch1_converted > 120) {
      pitch1_converted = 120;
    }

    myservo.write(pitch1_converted);

    delay(10); 

    float pitch_converted = 0;
    pitch_converted = 180-abs(pitch);
    
    pitch_converted -= 60;

    if (pitch < 0) {
      pitch_converted = 180;
    } 

    pitch_converted -= pitch1_converted;

    if (pitch_converted < 30) {
      pitch_converted = 30;
    } else if (pitch_converted > 170) {
      pitch_converted = 150;
    }

    myservo1.write(pitch_converted);

    delay(10); 
  }
  else
  {
    Serial.println("Receive failed");
  }
}

}
