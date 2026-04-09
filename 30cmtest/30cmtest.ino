const int motorLeftPWM  = 5;//pin motor
const int motorLeftDir  = 6;//pin motor
const int motorRightPWM = 9;//pin motor
const int motorRightDir = 10;//pin motor

const int clkLEFT  = 4;//pin encoder
const int dtLEFT   = 3;//pin encoder
const int clkRIGHT = 1;//pin encoder
const int dtRIGHT  = 20;//pin encoder

volatile int counterLEFT  = 0;
volatile int counterRIGHT = 0;

float dia = 4.3;
int   i   = 0;   // MA LEZMHECH TKOUN 0

float Kp = 0.0, Ki = 0.0, Kd = 0.0;

float filter      = 0.5;
float dt;
float lasterror   = 0;
float integral    = 0;
float Xderivative = 0, Yderivative = 0;

bool  moving         = false;
float targetDistance = 30.0;
float d              = 0;

float encoderCorrection = 0;

unsigned long currenttime, lasttime;

void isrLeft() {
  if (digitalRead(dtLEFT) != digitalRead(clkLEFT)) counterLEFT++;
  else counterLEFT--;
}

void isrRight() {
  if (digitalRead(dtRIGHT) != digitalRead(clkRIGHT)) counterRIGHT++;
  else counterRIGHT--;
}

void iniit() {
  counterLEFT       = 0;
  counterRIGHT      = 0;
  d                 = 0;
  lasterror         = 0;
  integral          = 0;
  Xderivative       = 0;
  Yderivative       = 0;
  encoderCorrection = 0;
  moving            = true;
}

void loopp() {
  if (!moving) return;

  currenttime = micros();
  dt = (currenttime - lasttime) / 1000000.0;
  if (dt <= 0 || dt > 0.1) return;
  lasttime = currenttime;

  float circum       = 2 * PI * (dia / 2.0);
  float averageTicks = (abs(counterLEFT) + abs(counterRIGHT)) / 2.0;
  d = (averageTicks / i) * circum;

  float error = targetDistance - d;

  if (error <= 0.5) {
    moving            = false;
    encoderCorrection = 0;
    return;
  }

  integral += error * dt;
  integral  = constrain(integral, -100, 100);

  Xderivative = (error - lasterror) / dt;
  Yderivative = filter * Xderivative + (1 - filter) * Yderivative;
  lasterror   = error;

  encoderCorrection = (error * Kp) + (integral * Ki) + (Yderivative * Kd);
}

void setup() {
  Serial.begin(9600);

  pinMode(motorLeftPWM,  OUTPUT);
  pinMode(motorLeftDir,  OUTPUT);
  pinMode(motorRightPWM, OUTPUT);
  pinMode(motorRightDir, OUTPUT);

  pinMode(clkLEFT,  INPUT);
  pinMode(dtLEFT,   INPUT);
  pinMode(clkRIGHT, INPUT);
  pinMode(dtRIGHT,  INPUT);
  attachInterrupt(digitalPinToInterrupt(clkLEFT),  isrLeft,  CHANGE);
  attachInterrupt(digitalPinToInterrupt(clkRIGHT), isrRight, CHANGE);

  lasttime = micros();
  iniit();
}

void loop() {
  loopp();
  if (moving) {
    int motorSpeed = constrain((int)encoderCorrection, 50, 150);
    analogWrite(motorLeftPWM,  motorSpeed);
    analogWrite(motorRightPWM, motorSpeed);
  } else {
    analogWrite(motorLeftPWM,  0);
    analogWrite(motorRightPWM, 0);
  }
}
