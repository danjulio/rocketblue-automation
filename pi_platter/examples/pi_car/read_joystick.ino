// Joystick code with serial output
//   - x, y pot (on A1, A0)
//   - button (on D9)
//
// Output format S<B>,<X>,<Y>E when something changed
//   <B> - 0/1 button
//   <X>,<Y> - 0-1023 pot position (center @ 512)
//
#define NUM_SAMPLES     5

#define HYSTERESIS      3
#define CENTER_DELTA    48

#define ST_IDLE         0
#define ST_ACTIVE       1
#define ST_CAL          2



uint8_t myState;

uint16_t xArray[NUM_SAMPLES];
uint16_t yArray[NUM_SAMPLES];
int pushI;
uint8_t tickCount;
uint16_t stateCount;

uint16_t xCenter;
uint16_t yCenter;

uint16_t xPrev;
uint16_t yPrev;
int bPrev;

int debug = 0;



void setup() {
  pinMode(9, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  Serial.begin(115200);

  // Look for special entry into debug mode
  if (digitalRead(9) == 0) {
    debug = 1;
    digitalWrite(13, HIGH);
    while (digitalRead(9) == 0) {
      delay(10);
    }
    digitalWrite(13, LOW);
    delay(1000);
  }
  
  for (int i=0; i<NUM_SAMPLES; i++) {
    xArray[i] = GetXAdc();
    yArray[i] = GetYAdc();
    delay(10);
  }
  xCenter = ComputeAvg(xArray);
  yCenter = ComputeAvg(yArray);
  if (debug) {
    Serial.print(F("Center = "));
    Serial.print(xCenter);
    Serial.print(F(" "));
    Serial.println(yCenter);
  }

  xPrev = 512;
  yPrev = 512;
  bPrev = 0;

  myState = ST_IDLE;
  pushI = 0;
  tickCount = 0;
  stateCount = 0;

  // Send an initial value
  SendData(0, 512, 512);
}


void loop() {
  int bCur;
  uint16_t xRaw;
  uint16_t yRaw;
  uint16_t xCur;
  uint16_t yCur;

  // Update values
  xArray[pushI] = GetXAdc();
  yArray[pushI] = GetYAdc();
  if (++pushI == NUM_SAMPLES) pushI=0;

  // Evaluate periodically (50 mSec - 20/sec)
  if (++tickCount == 5) {
    tickCount = 0;
    bCur = (digitalRead(9) == 0);

    xRaw = ComputeAvg(xArray);
    yRaw = ComputeAvg(yArray);
    xCur = CalibrateValue(xRaw, xCenter);
    yCur = CalibrateValue(yRaw, yCenter);

    switch (myState) {
      case ST_IDLE:
        // Look for button or change greater than hysteresis limits to move to active
        if ((bCur != bPrev) || ValBeyondHysteresis(xCur, yCur)) {
          stateCount = 0;
          myState = ST_ACTIVE;
          SendData(bCur, xCur, yCur);
          bPrev = bCur;
          xPrev = xCur;
          yPrev = yCur;
          if (debug) {
            Serial.println(F("IDLE->ACTIVE"));
          }
        } else {
          // Self-cal every once-in-a-while while we're idle
          if (++stateCount == 40) {
            digitalWrite(13, HIGH);
            stateCount = 0;
            myState = ST_CAL;
            xCenter = xRaw;
            yCenter = yRaw;
            if (debug) {
              Serial.print(F("IDLE->CAL : Center = "));
              Serial.print(xCenter);
              Serial.print(F(" "));
              Serial.println(yCenter);
            }
          }
        }
        break;
        
      case ST_ACTIVE:
        // Sensitive to any change while active
        if ((bCur != bPrev) || (xCur != xPrev) || (yCur != yPrev)) {
          SendData(bCur, xCur, yCur);
          bPrev = bCur;
          xPrev = xCur;
          yPrev = yCur;
        }
        
        // Look for remaining near center for a second to go back to idle
        if (ValNearCenter(xCur, yCur)) {
          if (++stateCount == 20) {
            stateCount = 0;
            myState = ST_IDLE;
            if (debug) {
              Serial.println(F("ACTIVE->IDLE"));
            }
          }
        } else {
          stateCount = 0;
          if (debug) {
            Serial.print(F("Val not near center : val, center = "));
            Serial.print(xCur);
            Serial.print(F(" "));
            Serial.print(yCur);
            Serial.print(F(", "));
            Serial.print(xCenter);
            Serial.print(F(" "));
            Serial.println(yCenter);
          }
        }
        break;
        
      case ST_CAL:
          digitalWrite(13, LOW);
          myState = ST_IDLE;
          if (debug) {
              Serial.println(F("CAL->IDLE"));
            }
        break;
        
      default:
        myState = ST_IDLE;
    }

    
  }
  delay(10);
}


uint16_t GetYAdc() {
  return (uint16_t) analogRead(A0);
}


uint16_t GetXAdc() {
  return (uint16_t) (1023 - analogRead(A1));
}


uint16_t ComputeAvg(uint16_t* sP) {
  uint16_t sum = 0;
  uint16_t avg;

  for (uint8_t i=0; i<NUM_SAMPLES; i++) {
    sum += *(sP + i);
  }

  avg = sum / NUM_SAMPLES;
  if ((sum % NUM_SAMPLES) >= (NUM_SAMPLES/2)) {
    avg++;
  }

  if (avg > 1023) avg = 1023;

  return avg;
}


uint16_t CalibrateValue(uint16_t v, uint16_t c) {
  if (v < c) {
    // map v in (0, c) to (0, 511) : v/c * 511
    return (uint16_t) map(v, 0, c, 0, 511);
  } else {
    // map v in (c, 1023) to (512, 1023)
    return (uint16_t) map(v, c, 1023, 512, 1023);
  }
}


int ValBeyondHysteresis(uint16_t x, uint16_t y) {
  return ((x < (xPrev - HYSTERESIS)) || (x > (xPrev + HYSTERESIS)) ||
          (y < (yPrev - HYSTERESIS)) || (y > (yPrev + HYSTERESIS)));
}


int ValNearCenter(uint16_t x, uint16_t y) {
  return ((((x >= (xCenter - CENTER_DELTA)) && (x <= (xCenter + CENTER_DELTA))) ||
           ((x >= (512 - CENTER_DELTA)) && (x <= (512 + CENTER_DELTA)))) &&
          (((y >= (yCenter - CENTER_DELTA)) && (y <= (yCenter + CENTER_DELTA))) ||
           ((y >= (512 - CENTER_DELTA)) && (y <= (512 + CENTER_DELTA)))));
}


void SendData(int b, uint16_t x, uint16_t y) {
  if (b) {
    Serial.print(F("S1,"));
  } else {
    Serial.print(F("S0,"));
  }
  Serial.print(x);
  Serial.print(F(","));
  Serial.print(y);
  Serial.println(F("E"));
}

