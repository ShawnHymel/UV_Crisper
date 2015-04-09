/**
 * UV_Crisper
 *
 * Author: Shawn Hymel
 * Date: April 8, 2015
 *
 * Set potentiometer to desired "crsipness." Turn on switch and UV sensor
 * will measure once per minute. Counter will count down to zero, and a
 * sound file will play once the UV crisping is done.
 *
 * Beerware license
 */
 
#include <SPI.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "firebat_burning_pcm.h"
 
// Pins
#define UV_PIN            A1
#define KNOB_PIN          A2
#define SPEAKER_PIN       3
#define SS_PIN            10

// Crispy levels
#define JUST_VIT_D        50
#define TOASTY            500
#define FEEL_THE_BURN     1000
#define EXTRA_CRISPY      2000

// Constants
#define SAMPLE_DELAY_MS   1000
#define SAMPLE_RATE       8000
                          
// Global variables
char temp_string[10];
int uv_approx;
int uv_read_raw;
float uv_intensity;
volatile uint16_t sample;
byte last_sample;

void setup()
{
  float uv_energy;
  int knob_level;
  
  // Start serial comms for debugging
  Serial.begin(9600);
  Serial.println("Welcome to your new UV crisper!");
  
  // Initialize SPI
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV64);
  
  // Initialize 7 segment display (counter)
  clearDisplaySPI();
  setBrightnessSPI(255);
  
  // Measure knob potentiometer and set uv energy threshold
  knob_level = analogRead(KNOB_PIN);
  if ( (0 >= knob_level) && (knob_level < 256) ) {
    uv_energy = JUST_VIT_D;
  } else if ( (256 >= knob_level) && (knob_level < 512) ) {
    uv_energy = TOASTY;
  } else if ( (512 >= knob_level) && (knob_level < 768) ) {
    uv_energy = FEEL_THE_BURN;
  } else {
    uv_energy = EXTRA_CRISPY;
  }
  Serial.print("UV threshdold set at: ");
  Serial.println(uv_energy);
 
  // Magical sprintf creates a string for us to send to the s7s
  uv_approx = roundFloat(uv_energy);
  sprintf(temp_string, "%4d", uv_approx);
  s7sSendStringSPI(temp_string);
  Serial.print("Approx: ");
  Serial.print(temp_string);
  Serial.print(" : ");
  Serial.println(uv_approx);
  
  // Periodically take a UV sample and decrement UV energy level
  while ( uv_energy > 0 ) {
    
    // Delay specified amount
    delay(SAMPLE_DELAY_MS);
    
    // Take several UV readings, average them, and
    // convert to intensity (mw/cm^2)
    uv_read_raw = averageAnalogRead(UV_PIN);
    uv_intensity = (3.3 / 1023) * uv_read_raw;
    uv_intensity = mapFloat(uv_intensity, 0.99, 2.8, 0.0, 15.0);
    Serial.print("UV read raw: ");
    Serial.print(uv_read_raw);
    Serial.print("\t");
    Serial.print("Intensity: ");
    Serial.print(uv_intensity);
    Serial.print("\t");
    
    // Decrement energy level and display it on 7-seg counter
    uv_energy -= uv_intensity;
    if ( uv_energy < 0 ) {
      uv_energy = 0;
    }
    uv_approx = roundFloat(uv_energy);
    sprintf(temp_string, "%4d", uv_approx);
    clearDisplaySPI();
    s7sSendStringSPI(temp_string);
    Serial.print("Energy remaining: ");
    Serial.println(uv_energy);
  }
  
  // Play our sound clip to let us know we're done!
  startPlayback();
}

void loop()
{
  // Do nothing
  delay(1000);
}

/************************************************************************
 * 7 Segment Display SPI Functions
 ***********************************************************************/

// Send the clear display command (0x76)
//  This will clear the display and reset the cursor
void clearDisplaySPI()
{
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(0x76);  // Clear display command
  digitalWrite(SS_PIN, HIGH);
}

// Set the displays brightness. Should receive byte with the value
//  to set the brightness to
//  dimmest------------->brightest
//     0--------127--------255
void setBrightnessSPI(byte value)
{
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(0x7A);  // Set brightness command byte
  SPI.transfer(value);  // brightness data byte
  digitalWrite(SS_PIN, HIGH);
}

// This custom function works somewhat like a serial.print.
//  You can send it an array of chars (string) and it'll print
//  the first 4 characters in the array.
void s7sSendStringSPI(String toSend)
{
  digitalWrite(SS_PIN, LOW);
  for (int i=0; i<4; i++)
  {
    SPI.transfer(toSend[i]);
  }
  digitalWrite(SS_PIN, HIGH);
}

/************************************************************************
 * UV Sensor Functions
 ***********************************************************************/

//Takes an average of readings on a given pin
//Returns the average
int averageAnalogRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 

  for(int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;

  return(runningValue);  
}

//The Arduino Map function but for floats
//From: http://forum.arduino.cc/index.php?topic=3922.0
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/************************************************************************
 * PCM Playback Functions
 * From: http://playground.arduino.cc/Code/PCMAudio
 ***********************************************************************/

void stopPlayback()
{
    // Disable playback per-sample interrupt.
    TIMSK1 &= ~_BV(OCIE1A);

    // Disable the per-sample timer completely.
    TCCR1B &= ~_BV(CS10);

    // Disable the PWM timer.
    TCCR2B &= ~_BV(CS10);

    digitalWrite(SPEAKER_PIN, LOW);
}

// This is called at 8000 Hz to load the next sample.
ISR(TIMER1_COMPA_vect) {
    if (sample >= sounddata_length) {
        if (sample == sounddata_length + last_sample) {
            stopPlayback();
        }
        else {
            if(SPEAKER_PIN == 11){
                // Ramp down to zero to reduce the click at the end of playback.
                OCR2A = sounddata_length + last_sample - sample;
            } else {
                OCR2B = sounddata_length + last_sample - sample;                
            }
        }
    }
    else {
        if(SPEAKER_PIN == 11){
            OCR2A = pgm_read_byte(&sounddata_data[sample]);
        } else {
            OCR2B = pgm_read_byte(&sounddata_data[sample]);            
        }
    }

    ++sample;
}

void startPlayback()
{
    pinMode(SPEAKER_PIN, OUTPUT);

    // Set up Timer 2 to do pulse width modulation on the speaker
    // pin.

    // Use internal clock (datasheet p.160)
    ASSR &= ~(_BV(EXCLK) | _BV(AS2));

    // Set fast PWM mode  (p.157)
    TCCR2A |= _BV(WGM21) | _BV(WGM20);
    TCCR2B &= ~_BV(WGM22);

    if(SPEAKER_PIN == 11){
        // Do non-inverting PWM on pin OC2A (p.155)
        // On the Arduino this is pin 11.
        TCCR2A = (TCCR2A | _BV(COM2A1)) & ~_BV(COM2A0);
        TCCR2A &= ~(_BV(COM2B1) | _BV(COM2B0));
        // No prescaler (p.158)
        TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

        // Set initial pulse width to the first sample.
        OCR2A = pgm_read_byte(&sounddata_data[0]);
    } else {
        // Do non-inverting PWM on pin OC2B (p.155)
        // On the Arduino this is pin 3.
        TCCR2A = (TCCR2A | _BV(COM2B1)) & ~_BV(COM2B0);
        TCCR2A &= ~(_BV(COM2A1) | _BV(COM2A0));
        // No prescaler (p.158)
        TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

        // Set initial pulse width to the first sample.
        OCR2B = pgm_read_byte(&sounddata_data[0]);
    }

    // Set up Timer 1 to send a sample every interrupt.

    cli();

    // Set CTC mode (Clear Timer on Compare Match) (p.133)
    // Have to set OCR1A *after*, otherwise it gets reset to 0!
    TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);
    TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));

    // No prescaler (p.134)
    TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

    // Set the compare register (OCR1A).
    // OCR1A is a 16-bit register, so we have to do this with
    // interrupts disabled to be safe.
    OCR1A = F_CPU / SAMPLE_RATE;    // 16e6 / 8000 = 2000

    // Enable interrupt when TCNT1 == OCR1A (p.136)
    TIMSK1 |= _BV(OCIE1A);

    last_sample = pgm_read_byte(&sounddata_data[sounddata_length-1]);
    sample = 0;
    sei();
}

/************************************************************************
 * Misc Functions
 ***********************************************************************/

// Rounds a floating value to an integer
int roundFloat(float x) {
  if ( x >= 0 ) {
    return (int) (x + 0.5);
  }
  return (int) (x - 0.5);
}
