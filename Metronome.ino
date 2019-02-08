// Tested on Schizzo Mega v1.2
// Should work on Seeeduino Lotus or Arduino UNO with Grove shield or similar.
// - Richard Bekking

#include "MillisTimer.h"  // Brett Hagman's MillisTimer Library
#include "TM1637.h"       // Grove 4-digit Display Library

const int PIN_7SEG_CLK = 2; // Grove 4-digit Display v1.0
const int PIN_7SEG_DIO = 3; // Grove 4-digit Display v1.0
const int PIN_BEATPOT = A2; // Grove Rotary Angle Sensor (p) v1.2
const int PIN_BPMPOT = A0;  // Grove Rotary Angle Sensor (p) v1.2
const int PIN_BUZZER = 11;  // Grove Buzzer v1.2

// Prefix "gl" means Global
TM1637 glDisplay(PIN_7SEG_CLK, PIN_7SEG_DIO);

uint8_t glBPM = 0;    // Holds Beats Per Minuite value (40 .. 200)
uint8_t glBeat = 0;   // Holds rythm index (0 .. 3)
uint8_t glPitches[6] = {255, 15, 15, 63, 15, 15}; // 255 = strong beat, 63 = secondary strong beat, 15 = weak beat
uint8_t glNumBeat[4] = {4, 2, 6, 3};              // very-common-time, marching-time, 6/8, waltz-time
uint8_t glCurrentPitchIndex = 0; // Holds the index to the glPitches array
MillisTimer glPlayClick = MillisTimer();
MillisTimer glStopClick = MillisTimer();
MillisTimer glBPMCheck  = MillisTimer();
MillisTimer glBeatCheck = MillisTimer();
MillisTimer glClearDisplay = MillisTimer();

void setup() {
  Serial.begin(9600);

  glBPMCheck.setInterval(100); // Check BPM input every 100 ms
  glBPMCheck.expiredHandler(BPMCheck);
  glBPMCheck.setRepeats(0); // 0 means go on forever
  glBPMCheck.start();

  glBeatCheck.setInterval(100); // Check time-signature input every 100 ms
  glBeatCheck.expiredHandler(beatCheck);
  glBeatCheck.setRepeats(0);
  glBeatCheck.start();

  glClearDisplay.setInterval(1000); // Clear display after 1 second of inactivity
  glClearDisplay.expiredHandler(clearDisp);
  glClearDisplay.setRepeats(1);
  glClearDisplay.stop();

  glPlayClick.setInterval(1000); // We change this interval through BPMCheck
  glPlayClick.expiredHandler(playClick); // Play's a click sound
  glPlayClick.setRepeats(0);
  glPlayClick.start();

  glStopClick.setInterval(10); // If we don't call "stopClick" it will not
                               // produce a click but a BEEEEEEEEEEEEEEEEEEEEEP
  glStopClick.expiredHandler(stopClick);
  glStopClick.setRepeats(1);
  glStopClick.stop();
}

void loop() {
  glPlayClick.run();
  glStopClick.run(); // Inactive, will be activated within playClick()
  glBPMCheck.run();
  glBeatCheck.run();
  glClearDisplay.run(); // Inactive, will be activated within ...Check()
}

void playClick(MillisTimer &t)
{
  if (glNumBeat[glBeat] == 4 && glCurrentPitchIndex == 3)
    analogWrite(PIN_BUZZER, glPitches[(glCurrentPitchIndex++) + 1]); // Skip secondary strong beat in common-time
  else
    analogWrite(PIN_BUZZER, glPitches[glCurrentPitchIndex++]);  

  if (glCurrentPitchIndex >= glNumBeat[glBeat])
    glCurrentPitchIndex = 0;

  glStopClick.start(); // Stop the sound 10 ms later
}

void stopClick(MillisTimer &t)
{
  analogWrite(PIN_BUZZER, 0);
}

void BPMCheck(MillisTimer &t)
{
  static uint8_t adcval = 0;
  static uint8_t prev_adcval = 0;

  int adc = 0;

  analogRead(PIN_BPMPOT); // Read the same ADC port twice to reduce cross-talk between ADC channels
  adc = analogRead(PIN_BPMPOT);
  adcval = map(adc >> 1, 0, 511, 40, 200); // >> 1 to remove noisy least-significant bit

  if (adcval != prev_adcval) {
    prev_adcval = adcval;
    glDisplay.displayNum(adcval); // Show BPM on the right of the display
    glClearDisplay.start(); // Clear display after 1 second of inactivity
    glPlayClick.setInterval(round(60000 / adcval) - 10); // -10 ms to compensate for on-time
    glBPM = adcval;
  }
}

void beatCheck(MillisTimer &t)
{
  static uint8_t adcval = 0;
  static uint8_t prev_adcval = 0;

  int adc = 0;

  analogRead(PIN_BEATPOT); // Read the same ADC port twice to reduce cross-talk between ADC channels
  adc = analogRead(PIN_BEATPOT);
  adcval = map(adc >> 1, 0, 511, 0, 3); // >> 1 to remove noisy least-significant bit

  if (adcval != prev_adcval) {
    prev_adcval = adcval;
    glDisplay.display(0, glNumBeat[adcval]); // Show rythm on the left of the display
    glClearDisplay.start(); // Clear display after 1 second of inactivity
    glBeat = adcval;
  }
}

void clearDisp(MillisTimer &t)
{
  glDisplay.clearDisplay();
}
