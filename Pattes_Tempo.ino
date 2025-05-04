#include "ArduinoTapTempo.h"

// DIGITAL PINS

#define LED 0
#define TIMES_1 1
#define DIV_2 2
#define DIV_3 3
#define DIV_4 4
#define DIV_8 5
#define DIV_16 6
#define LED_OFF 7
#define TIMES_1_OFF 8
#define DIV_2_OFF 9
#define DIV_4_OFF 10
#define DIV_8_OFF 11
#define TIMES_2 12
#define TIMES_4 13
#define RESET 18
#define CLOCK 19

// ANALOG PINS

#define GATE A6
#define OFFSET A7

// OTHER CONSTANTS

#define DEBOUNCE 30
#define LONG_PRESS 2000

// VARIABLES

int resetState = LOW;
int resetLastRawState = LOW;
unsigned long resetLastDebounceTime = 0;
bool resetChanged = false;
int clockState = LOW;
int clockLastRawState = LOW;
unsigned long clockLastDebounceTime = 0;
unsigned long clockLastStateTime = 0;
bool clockChanged = false;
float gate = 0.0;
float offset = 0.0;
ArduinoTapTempo tapTempo;
float beat = 0.0;
float beat_t2 = 0.0;
float beat_t4 = 0.0;
float beat_d2 = 0.0;
float beat_d3 = 0.0;
float beat_d4 = 0.0;
float beat_d8 = 0.0;
float beat_d16 = 0.0;
bool paused = true;
bool ledState = false;
bool ledOffsetState = false;
bool times4State = false;
bool times2State = false;
bool times1State = false;
bool times1OffsetState = false;
bool div2State = false;
bool div2OffsetState = false;
bool div3State = false;
bool div4State = false;
bool div4OffsetState = false;
bool div8State = false;
bool div8OffsetState = false;
bool div16State = false;

// FUNCTIONS

bool debounceRead(int pin, int *state, int *lastRawState, unsigned long *lastDebounceTime)
{
  int raw = digitalRead(pin);
  bool changed = false;

  if (raw != *lastRawState)
  {
    *lastDebounceTime = millis();
  }

  if ((millis() - *lastDebounceTime) > DEBOUNCE)
  {
    changed = raw != *state;

    *state = raw;
  }

  *lastRawState = raw;

  return changed;
}

void writeOnChange(int pin, bool *state, bool value)
{
  if (*state != value)
  {
    digitalWrite(pin, *state ? HIGH : LOW);
  }

  *state = value;
}

// SCRIPT START

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(LED_OFF, OUTPUT);
  pinMode(TIMES_4, OUTPUT);
  pinMode(TIMES_2, OUTPUT);
  pinMode(TIMES_1, OUTPUT);
  pinMode(TIMES_1_OFF, OUTPUT);
  pinMode(DIV_2, OUTPUT);
  pinMode(DIV_2_OFF, OUTPUT);
  pinMode(DIV_3, OUTPUT);
  pinMode(DIV_4, OUTPUT);
  pinMode(DIV_4_OFF, OUTPUT);
  pinMode(DIV_8, OUTPUT);
  pinMode(DIV_8_OFF, OUTPUT);
  pinMode(DIV_16, OUTPUT);

  pinMode(RESET, INPUT);
  pinMode(CLOCK, INPUT);

  tapTempo.resetTapChain();
}

void loop()
{
  // INPUTS

  resetChanged = debounceRead(RESET, &resetState, &resetLastRawState, &resetLastDebounceTime);
  clockChanged = debounceRead(CLOCK, &clockState, &clockLastRawState, &clockLastDebounceTime);

  if (clockChanged)
  {
    clockLastStateTime = millis();
  }

  gate = (float)analogRead(GATE) / 1024.0;
  offset = (float)analogRead(OFFSET) / 1024.0;

  // LOGIC

  if (resetChanged && resetState == HIGH || clockState == HIGH && (millis() - clockLastStateTime) > LONG_PRESS)
  {
    if (!paused)
    {
      tapTempo.resetTapChain();
      paused = true;
    }
  }
  else
  {
    if (clockState == HIGH)
    {
      paused = false;
    }

    tapTempo.update(clockState == HIGH);
  }

  beat_t4 = tapTempo.beatProgress(0.25) * 4.0;

  writeOnChange(TIMES_4, &times4State, !paused && beat_t4 < gate);

  beat_t2 = tapTempo.beatProgress(0.5) * 2.0;

  writeOnChange(TIMES_2, &times2State, !paused && beat_t2 < gate);

  beat = tapTempo.beatProgress();

  writeOnChange(TIMES_1, &times1State, !paused && beat < gate);
  writeOnChange(TIMES_1_OFF, &times1OffsetState, !paused && fmod(beat + offset, 1.0) < gate);

  beat_d2 = tapTempo.beatProgress(2.0) * 0.5;

  writeOnChange(DIV_2, &div2State, !paused && beat_d2 < gate);
  writeOnChange(DIV_2_OFF, &div2OffsetState, !paused && fmod(beat_d2 + offset, 1.0) < gate);

  beat_d3 = tapTempo.beatProgress(3.0) * 0.333333;

  writeOnChange(DIV_3, &div3State, !paused && beat_d3 < gate);

  beat_d4 = tapTempo.beatProgress(4.0) * 0.25;

  writeOnChange(DIV_4, &div4State, !paused && beat_d4 < gate);
  writeOnChange(DIV_4_OFF, &div4OffsetState, !paused && fmod(beat_d4 + offset, 1.0) < gate);

  beat_d8 = tapTempo.beatProgress(8.0)* 0.125;

  writeOnChange(DIV_8, &div8State, !paused && beat_d8 < gate);
  writeOnChange(DIV_8_OFF, &div8OffsetState, !paused && fmod(beat_d8 + offset, 1.0) < gate);

  beat_d16 = tapTempo.beatProgress(16.0) * 0.0625;

  writeOnChange(DIV_16, &div16State, !paused && beat_d16 < gate);

  // LEDS

  writeOnChange(LED, &ledState, clockState == HIGH || !paused && beat_t4 < gate);
  writeOnChange(LED_OFF, &ledOffsetState, !paused && fmod(beat_t4 + offset, 1.0) < gate);
}
