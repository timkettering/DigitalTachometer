#include <SmoothAnalogInput.h>
#include <Adafruit_NeoPixel.h>
#include <SwitecX25.h>
#include <Wire.h>

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
#define LED_RING_PIN 8
#define LED_RING_BRIGHTNESS 80
Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, LED_RING_PIN, NEO_GRB + NEO_KHZ800);

// Switec setup
// standard X25.168 range 315 degrees at 1/3 degree steps
#define GAUGESTEPRANGE 750
#define GAUGERPMLIMIT 12000
#define WARNRPMLIMIT 9000
#define NORMALRPMLIMIT 7000
#define LASTMOVEINTERVAL 500

unsigned int rpmPerStep = 0;
unsigned int normalStepLimit = 0;
unsigned int warnStepLimit = 0;
// the gaugesteplimit is the GAUGESTEPRANGE 
unsigned long lastNeedleMoveMillis = 0;

// Switech X27 stepper motor (tach motor)
SwitecX25 tachMotor(GAUGESTEPRANGE,4,5,6,7);
int needlePos = 0;

// tach input
#define TACH_INPUT_PIN 2
unsigned int engineRpm = 0;
unsigned long previousChange = 0;
unsigned long currentChange = 0;
unsigned long lastPulseDuration = 0;
unsigned long lastPulseDurationRAW = 0;

// data smoothing
#define FILTER_SAMPLES 30
unsigned long sensSmoothArray1 [FILTER_SAMPLES];

void setup() {

  // calculate steps we are using for 
  // the program run based on REVLIMIT params
  rpmPerStep = GAUGERPMLIMIT / GAUGESTEPRANGE;
  int normalStepLimit = rpmPerStep * NORMALRPMLIMIT;
  int warnStepLimit = rpmPerStep * WARNRPMLIMIT;
  
  // setup the input pin and attach interrupt
  pinMode(TACH_INPUT_PIN, INPUT);
  attachInterrupt(0, tachInputChanged, CHANGE);

  // init the LED RING
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(LED_RING_BRIGHTNESS);
  ringColor(strip.Color(25, 25, 25));

  // zero the tach motor
  tachMotor.zero();

  // do a full sweep of the tach
  tachMotor.setPosition(GAUGESTEPRANGE);
  tachMotor.updateBlocking();
  tachMotor.setPosition(0);
  tachMotor.updateBlocking();

  // set timing state
  lastNeedleMoveMillis = millis();
  previousChange = micros();
}

void loop() {

  getEngineSpeed();
  doesNeedleNeedToMove();
  updateBacklightIfNecessary();

  // continuously run update
  tachMotor.update();
}

void tachInputChanged() {
  
  currentChange = micros();
  lastPulseDurationRAW = roundDown(currentChange - previousChange, 100);
  previousChange = currentChange;
}

void getEngineSpeed() {

  lastPulseDuration = digitalSmooth(lastPulseDurationRAW, sensSmoothArray1);
//  lastPulseDuration = lastPulseDurationRAW;

  if(lastPulseDuration > 0) {
    unsigned long engineFreq = (1000000 / (lastPulseDuration * 2));
    engineRpm = engineFreq * 60;
  }
  else {
    engineRpm = 0;
  }
}

void doesNeedleNeedToMove() {

  unsigned long currentMillis = millis();
  if(lastNeedleMoveMillis + LASTMOVEINTERVAL < currentMillis) { 

    lastNeedleMoveMillis = currentMillis;
    needlePos = engineRpm / rpmPerStep;
    tachMotor.setPosition(needlePos);
  }
}

void updateBacklightIfNecessary() {

  if(tachMotor.currentStep > normalStepLimit && tachMotor.currentStep <= warnStepLimit) {
    ringColor(strip.Color(255,255,51));
  }
  else if(tachMotor.currentStep > warnStepLimit) {
    ringColor(strip.Color(255,0,0));
  }
  else {
    ringColor(strip.Color(0,255,255));
  }
}

void ringColor(uint32_t c) {
  for(uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

unsigned long digitalSmooth(unsigned long rawIn, unsigned long *sensSmoothArray) {    

  unsigned long j, k, temp, top, bottom;
  long total;
  static unsigned long i;
  static unsigned long sorted[FILTER_SAMPLES];
  boolean done;

  i = (i + 1) % FILTER_SAMPLES;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[i] = rawIn;                 // input new data into the oldest slot

  // Serial.print("raw = ");

  for (j=0; j<FILTER_SAMPLES; j++){     // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[j];
  }

  done = 0;                // flag to know when we're done sorting              
  while(done != 1){        // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (FILTER_SAMPLES - 1); j++){
      if (sorted[j] > sorted[j + 1]){     // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted [j+1] =  sorted[j] ;
        sorted [j] = temp;
        done = 0;
      }
    }
  }

  // throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
  bottom = max(((FILTER_SAMPLES * 15)  / 100), 1); 
  top = min((((FILTER_SAMPLES * 85) / 100) + 1  ), (FILTER_SAMPLES - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for ( j = bottom; j< top; j++){
    total += sorted[j];  // total remaining indices
    k++; 
  }
  return total / k;    // divide by number of samples  
}

unsigned long roundDown(unsigned long numToRound, unsigned long multiple) 
{ 
 if(multiple == 0) 
 { 
  return numToRound; 
 } 

 int remainder = numToRound % multiple;
 if (remainder == 0)
  return numToRound;
 return numToRound - remainder;
} 






