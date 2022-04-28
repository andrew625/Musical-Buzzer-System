#include <SPI.h>
#include <SD.h>

// https://learn.adafruit.com/adafruit-neopixel-uberguide/the-magic-of-neopixels
#include <Adafruit_NeoPixel.h>

// https://github.com/connornishijima/arduino-polytone
#include <digitalWriteFast.h>
#include <Polytone.h>

// Pins
#define BUZZER_1_PIN 2
#define BUZZER_2_PIN 3
#define BUZZER_3_PIN 4
#define BUZZER_4_PIN 5
#define BUTTON_PIN 6
#define LED_PIN 7
Polytone poly;
Adafruit_NeoPixel pixel(4, LED_PIN, NEO_RGB + NEO_KHZ800); // 4 LEDs

// Settings
bool freqBasedLightEnabled = true;
double tempoMultiplier = 1; // Lower = Faster & Higher = Slower

// Lights
void rgbLight(int pixelNum, int r, int g, int b) {
  pixel.setPixelColor(pixelNum, r, g, b);
  pixel.show();
}
void randomLight(int pixelNum) {
  rgbLight(pixelNum, random(0, 255), random(0, 255), random(0, 255));
}
void freqBasedLight(int pixelNum, long frequency) {
  double r = 0, g = 0, b = 0;
  double thresholds[] = {10, 150, 300, 600, 1200, 2400};
  if (frequency >= thresholds[0] && frequency < thresholds[1]) {
    r = (thresholds[1] - frequency) / (thresholds[1] - thresholds[0]);
    b = 1;
  } else if (frequency >= thresholds[1] && frequency < thresholds[2]) {
    g = (frequency - thresholds[1]) / (thresholds[2] - thresholds[1]);
    b = 1;
  } else if (frequency >= thresholds[2] && frequency < thresholds[3]) {
    b = (thresholds[3] - frequency) / (thresholds[3] - thresholds[2]);
    g = 1;
  } else if (frequency >= thresholds[3] && frequency < thresholds[4]) {
    r = (frequency - thresholds[3]) / (thresholds[4] - thresholds[3]);
    g = 1;
  } else if (frequency >= thresholds[4] && frequency < thresholds[5]) {
    g = (thresholds[5] - frequency) / (thresholds[5] - thresholds[4]);
    r = 1;
  } else if (frequency >= thresholds[5]) {
    r = 1;
  }
  rgbLight(pixelNum, r * 255, g * 255, b * 255);
}

// MIDI Buzzing
double MIDItoFreq(char keynum) {
  return 440.0 * pow(2.0, ((double)keynum - 69.0) / 12.0);
}
void buzz(int melodyNum, int noteNum, double noteLength) {
  if (noteNum > 0) {
    long frequency = MIDItoFreq(noteNum);
    if (freqBasedLightEnabled == true) {
      freqBasedLight(melodyNum, frequency);
    } else {
      randomLight(melodyNum);
    }
    poly.tone(frequency, noteLength, melodyNum + 1);
  } else {
    rgbLight(melodyNum, 0, 0, 0);
  }
}
void playFile(File melodyFile) {
  int recordNum = 0;
  while (melodyFile.available()) {
    // Set tempo
    if (recordNum == 0) {
      tempoMultiplier = melodyFile.readStringUntil('\n').toDouble();
      if (tempoMultiplier <= 0) {
        tempoMultiplier = 1;
      }
    }
    recordNum++;
    
    // Read note numbers
    String notes[4] = {
      melodyFile.readStringUntil('\t'), melodyFile.readStringUntil('\t'),
      melodyFile.readStringUntil('\t'), melodyFile.readStringUntil('\t')
    };

    // Read note duration
    double noteDuration = melodyFile.readStringUntil('\n').toDouble() * tempoMultiplier;
    if (noteDuration == 0) {
      noteDuration = 100;
    }

    // Stop song when button is pressed
    if (digitalRead(BUTTON_PIN) == HIGH) {
      break;
    }

    // Buzz the notes
    for (int i = 0; i < 4; i++) {
      int noteNum = notes[i].toInt();
      buzz(i, noteNum, noteDuration);
    }

    // Delay slightly longer than actual note duration
    delay(noteDuration * 1.25);
  }

  // Turn off all LEDs when song ends
  for (int i = 0; i < 4; i++) {
    rgbLight(i, 0, 0, 0);
  }
  
  melodyFile.close();
}

// Initailization
void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));

  // SD Card
  Serial.print("Initializing SD card...");
  if (!SD.begin(10)) {
    Serial.println("Initialization failed");
    while (1);
  }
  Serial.println("Initialization done");

  // Setup Pins
  pixel.begin();
  poly.begin();
  poly.setPins(BUZZER_1_PIN, BUZZER_2_PIN, BUZZER_3_PIN, BUZZER_4_PIN);
  pinMode(BUTTON_PIN, INPUT);
}

// Main Loop
void loop() {
  File dir = SD.open("/");
  while (true) {
    File entry =  dir.openNextFile();
    if (!entry) {
      break;
    }
    playFile(entry);
    delay(1000);
  }
}
