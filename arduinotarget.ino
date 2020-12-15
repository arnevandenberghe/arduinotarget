/*
  Target
  
  A game of target practice.

  The goal of the game is to push the "fire" button when the middle LED (PIN 7) is lit up.
  when you hit, you go to the next level.
  when you miss, you loose and the game have to restart.

  LEDs should be connected to PINs 4 - 11
  A "fire" button should be connected to PIN 2
  A "reset" button can optionally be connected to PIN 3, functionality is not great though.
*/

// PIN numbers in use
const int DISPLAY_OFFSET = 4; // the first BIT in the target display
const int FIRE_BTN = 2;
const int RESET_BTN = 3;

// game states
const int STATE_INIT = 0;
const int STATE_PLAY = 1;
const int STATE_WIN = 2;
const int STATE_LOOSE = 3;

const int TARGET = B00001000;

// the intial update rate of the leds (in ms)
const int INTERVAL_DEFAULT = 1024;

// current game state
int state = STATE_INIT;
// current update interval
int interval = INTERVAL_DEFAULT;
// current aim position
byte aim = TARGET;
// travel direction
bool rightToLeft = true;

void setup()
{
  Serial.begin(9600);
  Serial.println("setup");

  // diplay pins
  for (int i = 0; i < 8; i++)
    pinMode(DISPLAY_OFFSET + i, OUTPUT);

  // fire and reset button
  pinMode(FIRE_BTN, INPUT_PULLUP);
  pinMode(RESET_BTN, INPUT_PULLUP);

  // interrupt program and execute fire() when button is pushed
  attachInterrupt(digitalPinToInterrupt(FIRE_BTN), fire, RISING);
  // interrupt program and execute reset() when button is pushed
  attachInterrupt(digitalPinToInterrupt(RESET_BTN), reset, RISING);
}

void loop()
{
  // depending on the state of the game, different animations happen.
  switch (state)
  {
    // loading animation
    case STATE_INIT:
      state = STATE_PLAY;
      interval = INTERVAL_DEFAULT;
      loading();
      break;

    // moving target animation
    case STATE_PLAY:
      play();
      break;

    // hit animation
    case STATE_WIN:
      state = STATE_PLAY;
      youWin();
      break;
  
    // miss animation
    case STATE_LOOSE:
      state = STATE_INIT;
      youLoose();
      break;

    // failsafe
    // this should not happen, but if it does, restart the game.
    default:
      Serial.println("ERROR - default");
      state = STATE_INIT;
      interval = INTERVAL_DEFAULT;
      break;
  }
}

// executed when the fire button is pushed
void fire()
{
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if( !debounce(200) )
    return;

  Serial.print("fired - ");

  // if you hit the target, you win
  if(aim == TARGET)
  {
    Serial.println("hit");

    state = STATE_WIN;
    interval = interval / 2; // but it gets more difficult
  }
  // if you miss, you loose
  else 
  {
    Serial.println("miss");

    state = STATE_LOOSE;
    interval = INTERVAL_DEFAULT;
  }
}

// executed when the reset button is pushed
void reset()
{
  Serial.println("reset");

  // reset all initial values
  state = STATE_INIT;
  aim = TARGET;
  interval = INTERVAL_DEFAULT;
  rightToLeft = true;
}

// a loading animation
void loading()
{
  Serial.println("loading");

  display(B00011000, 150);
  display(B00111100, 150);
  display(B01100110, 150);
  display(B11000011, 150);
  display(B10000001, 150);
  display(B11000011, 150);
  display(B01100110, 150);
  display(B00111100, 150);
  display(B00011000, 150);
  display(B00000000, 150);
}

// the moving target
void play()
{
  // move the LED left or right and show it
  aim = rightToLeft ? aim << 1 : aim >> 1; 
  display(aim, interval);
  
  // reverse the direction when you hit the boundaties.
  if(aim >= B10000000 || aim <= B00000001 )
    rightToLeft = !rightToLeft;
}

// a hit animation
void youWin()
{
  Serial.println("You win!");

  //display(B0001000, 150);
  display(B0011100, 150);
  display(B0111110, 150);
  display(B1111111, 300);
  display(B0111110, 150);
  display(B0011100, 150);
  display(B0001000, 150);
}

// a miss animation
void youLoose()
{
  Serial.println("You loose!");

  int blinkCount = 3;
  for (int i = 0; i < blinkCount; i++)
  {
    display(aim, 300);
    display(B00000, 300);
  }
}

// output a byte value to the display
// allows displaying a bit pattern
void display(byte value, unsigned long duration)
{
  for (int i = 0; i < 8; i++)
    digitalWrite(i + DISPLAY_OFFSET, bitRead(value, i));

  delay(duration);
}


static unsigned long last = 0;
static bool debounce(int treshold)
{
  unsigned long now = millis();
  if (now - last < treshold)
    return false;

  last = now;
  return true;
}