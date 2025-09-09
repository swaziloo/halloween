// test-audio.ino
// blink on led pin
// initialize/play serial audio DFPlayer Mini
#include <DFRobotDFPlayerMini.h>
#include <Adafruit_NeoPixel.h>

const int ledPin =2;
const int txPin = 0;
const int rxPin = 1;

#define FPSerial Serial1
DFRobotDFPlayerMini myDFPlayer;
unsigned int volume = 30; // volume range from 0 to 30

#define PIN 16 // Define the pin connected to the WS2812 LED
#define NUMPIXELS 1 // Number of LEDs in the strip
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void printDetail(uint8_t type, int value);

void setup() {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);

    pixels.begin(); // Initialize the NeoPixel library
    // Indicate start of setup with green color
    pixels.setPixelColor(0, pixels.Color(0, 64, 0)); // green
    pixels.show(); 
    if (Serial) Serial.begin(115200); // Initialize Serial0 (Serial Monitor)
    initializeCaw();
    Serial.println(F("Initialization complete."));
    pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Off
    pixels.show();
}

void loop() {
    delay(4000);
    myDFPlayer.next();
    digitalWrite(ledPin, LOW);
    delay(500);
    digitalWrite(ledPin, HIGH);
}

void initializeCaw() {
    pixels.setPixelColor(0, pixels.Color(64, 42, 0)); // orange
    pixels.show();  
    FPSerial.begin(9600); // Initialize Serial1 (DFPlayer Mini)
    delay(3000);
    if (!myDFPlayer.begin(FPSerial, true, true)) { // isAck true, doReset true
        Serial.println(F("Unable to bring DFPlayer Mini online."));
        pixels.setPixelColor(0, pixels.Color(64, 0, 0)); // red
        pixels.show();
        delay(3000);
    } else {
        Serial.println(F("DFPlayer Mini online."));
        pixels.setPixelColor(0, pixels.Color(64, 0, 64)); // purple
        pixels.show();
        delay(3000);
    }
    myDFPlayer.volume(10);
    myDFPlayer.play(1);  //Play the first mp3
    delay(3000);
    myDFPlayer.volume(volume);
}

void printDetail(uint8_t type, int value) {
    switch (type) {
    case TimeOut:
        Serial.println(F("Time Out!"));
        break;
    case WrongStack:
        Serial.println(F("Stack Wrong!"));
        break;
    case DFPlayerCardInserted:
        Serial.println(F("Card Inserted!"));
        break;
    case DFPlayerCardRemoved:
        Serial.println(F("Card Removed!"));
        break;
    case DFPlayerCardOnline:
        Serial.println(F("Card Online!"));
        break;
    case DFPlayerUSBInserted:
        Serial.println("USB Inserted!");
        break;
    case DFPlayerUSBRemoved:
        Serial.println("USB Removed!");
        break;
    case DFPlayerPlayFinished:
        Serial.print(F("Number:"));
        Serial.print(value);
        Serial.println(F(" Play Finished!"));
        break;
    case DFPlayerError:
        Serial.print(F("DFPlayerError:"));
        switch (value) {
        case Busy:
            Serial.println(F("Card not found"));
            break;
        case Sleeping:
            Serial.println(F("Sleeping"));
            break;
        case SerialWrongStack:
            Serial.println(F("Get Wrong Stack"));
            break;
        case CheckSumNotMatch:
            Serial.println(F("Check Sum Not Match"));
            break;
        case FileIndexOut:
            Serial.println(F("File Index Out of Bound"));
            break;
        case FileMismatch:
            Serial.println(F("Cannot Find File"));
            break;
        case Advertise:
            Serial.println(F("In Advertise"));
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}