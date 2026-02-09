/*
 * Snake game for Arduino Uno R4 WiFi
 *
 * - Game runs on the built-in 12x8 LED matrix.
 * - Control: HW-504 joystick (X=A0, Y=A1, button=D2).
 * - Score: 3461BS-1 4-digit 7-segment display on A2-A5 (digit commons)
 *   and D5-D12 (segments a..g, dp), with 220 ohm resistors on segment lines.
 */

#include "Arduino_LED_Matrix.h"

/* ----- Joystick (HW-504) pins ----- */
#define JOY_X   A0   /* X axis analog */
#define JOY_Y   A1   /* Y axis analog */
#define JOY_SW  2    /* Push button (use INPUT_PULLUP) */

ArduinoLEDMatrix matrix;

/* ----- Grid and game timing ----- */
const int ROWS = 8;
const int COLS = 12;
const int MAX_SNAKE_LEN = 96;
const int INITIAL_DELAY = 220;  /* ms per move at start */
const int MIN_DELAY = 60;
const int DELAY_STEP = 15;      /* speed increase per food */

int gameDelay = INITIAL_DELAY;

/* ----- Joystick ADC thresholds (center ~512) ----- */
const int JOY_LEFT  = 400;
const int JOY_RIGHT = 600;
const int JOY_UP    = 600;
const int JOY_DOWN  = 400;

/* ----- Direction: 0=up, 1=right, 2=down, 3=left (row/col deltas) ----- */
const int DR[] = {-1, 0, 1, 0};
const int DC[] = {0, 1, 0, -1};

/* ----- Snake state ----- */
int snakeRow[MAX_SNAKE_LEN];
int snakeCol[MAX_SNAKE_LEN];
int snakeLen = 3;
int direction = 1;   /* start moving right */
bool paused = false;
bool gameOver = false;
int score = 0;

int foodRow, foodCol;
int frameCount = 0;

/* ----- 3461BS-1: use A2-A5 for digit commons (A0,A1 are joystick) ----- */
int anodPins[] = {A2, A3, A4, A5};
int segmentsPins[] = {5, 6, 7, 8, 9, 10, 11, 12};  /* a,b,c,d,e,f,g,dp */

/* 7-segment patterns for digits 0-9 (1 = segment on). Order: a,b,c,d,e,f,g,dp */
int seg[10][8] = {
  {1, 1, 1, 1, 1, 1, 0, 0}, /* 0 */
  {0, 1, 1, 0, 0, 0, 0, 0}, /* 1 */
  {1, 1, 0, 1, 1, 0, 1, 0}, /* 2 */
  {1, 1, 1, 0, 1, 0, 1, 0}, /* 3 */
  {0, 1, 1, 0, 0, 1, 1, 0}, /* 4 */
  {1, 0, 1, 0, 1, 1, 1, 0}, /* 5 */
  {1, 0, 1, 1, 1, 1, 1, 0}, /* 6 */
  {1, 1, 1, 0, 0, 0, 0, 0}, /* 7 */
  {1, 1, 1, 1, 1, 1, 1, 0}, /* 8 */
  {1, 1, 1, 0, 1, 1, 1, 0}  /* 9 */
};

/*
 * Update the 4-digit score display by multiplexing.
 * Common-anode: digit common HIGH to select digit, segment LOW to light.
 * We cycle through digits quickly so all four appear on at once.
 */
void updateScoreDisplay(int value) {
  value = constrain(value, 0, 9999);
  int d0 = value / 1000;
  int d1 = (value / 100) % 10;
  int d2 = (value / 10) % 10;
  int d3 = value % 10;
  int digits[] = {d0, d1, d2, d3};
  for (int i = 0; i < 4; i++) {
    for (int a = 0; a < 4; a++) digitalWrite(anodPins[a], LOW);
    int d = digits[i];
    for (int k = 0; k < 8; k++) {
      digitalWrite(segmentsPins[k], (seg[d][k] == 1) ? LOW : HIGH);
    }
    digitalWrite(anodPins[i], HIGH);
    delayMicroseconds(600);
  }
}

/* Return true if cell (r,c) is part of the snake body. */
bool onSnake(int r, int c) {
  for (int i = 0; i < snakeLen; i++)
    if (snakeRow[i] == r && snakeCol[i] == c) return true;
  return false;
}

/* Place food on a random cell that is not on the snake. */
void placeFood() {
  do {
    foodRow = random(ROWS);
    foodCol = random(COLS);
  } while (onSnake(foodRow, foodCol));
}

/* Reset snake to initial length and position, clear score, restore speed. */
void resetSnake() {
  snakeLen = 3;
  direction = 1;
  score = 0;
  snakeRow[0] = ROWS / 2;
  snakeCol[0] = 2;
  snakeRow[1] = ROWS / 2;
  snakeCol[1] = 1;
  snakeRow[2] = ROWS / 2;
  snakeCol[2] = 0;
  placeFood();
  gameDelay = INITIAL_DELAY;
}

void setup() {
  matrix.begin();
  pinMode(JOY_SW, INPUT_PULLUP);
  randomSeed(analogRead(2));

  for (int i = 0; i < 4; i++) pinMode(anodPins[i], OUTPUT);
  for (int i = 0; i < 8; i++) pinMode(segmentsPins[i], OUTPUT);

  snakeRow[0] = ROWS / 2;
  snakeCol[0] = 2;
  snakeRow[1] = ROWS / 2;
  snakeCol[1] = 1;
  snakeRow[2] = ROWS / 2;
  snakeCol[2] = 0;
  placeFood();
}

void loop() {
  /* ----- Game over: show last frame, keep score visible until button press ----- */
  if (gameOver) {
    uint8_t bitmap[8][12] = {0};
    for (int i = 0; i < snakeLen; i++)
      bitmap[snakeRow[i]][snakeCol[i]] = 1;
    bitmap[foodRow][foodCol] = 1;
    matrix.renderBitmap(bitmap, 8, 12);
    while (digitalRead(JOY_SW) != LOW) {
      updateScoreDisplay(score);
      delay(2);
    }
    delay(200);
    gameOver = false;
    return;
  }

  /* ----- Joystick button toggles pause ----- */
  if (digitalRead(JOY_SW) == LOW) {
    delay(200);
    paused = !paused;
    while (digitalRead(JOY_SW) == LOW) { delay(10); }
  }

  if (paused) {
    uint8_t bitmap[8][12] = {0};
    for (int i = 0; i < snakeLen; i++)
      bitmap[snakeRow[i]][snakeCol[i]] = 1;
    frameCount++;
    if ((frameCount / 2) % 2 == 0)
      bitmap[foodRow][foodCol] = 1;
    matrix.renderBitmap(bitmap, 8, 12);
    for (int n = 0; n < 25; n++) updateScoreDisplay(score);
    delay(50);
    return;
  }

  /* ----- Read joystick and update direction (no reverse) ----- */
  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);
  int newDir = direction;
  if (x < JOY_LEFT)  newDir = 3;
  if (x > JOY_RIGHT) newDir = 1;
  if (y < JOY_DOWN)  newDir = 0;
  if (y > JOY_UP)    newDir = 2;
  if ((newDir + 2) % 4 != direction)
    direction = newDir;

  /* ----- New head position (wraparound) ----- */
  int headR = (snakeRow[0] + DR[direction] + ROWS) % ROWS;
  int headC = (snakeCol[0] + DC[direction] + COLS) % COLS;

  /* ----- Collision with body? ----- */
  bool collision = false;
  for (int i = 1; i < snakeLen; i++) {
    if (snakeRow[i] == headR && snakeCol[i] == headC) {
      collision = true;
      break;
    }
  }

  if (collision) {
    resetSnake();
    gameOver = true;
  } else {
    bool ate = (headR == foodRow && headC == foodCol);

    if (ate) {
      score++;
      /* Shift body and add new head; snake grows. */
      for (int i = snakeLen; i > 0; i--) {
        snakeRow[i] = snakeRow[i - 1];
        snakeCol[i] = snakeCol[i - 1];
      }
      snakeRow[0] = headR;
      snakeCol[0] = headC;
      snakeLen++;
      placeFood();
      gameDelay = max(MIN_DELAY, gameDelay - DELAY_STEP);
    } else {
      /* Move: shift body, new head. */
      for (int i = snakeLen - 1; i > 0; i--) {
        snakeRow[i] = snakeRow[i - 1];
        snakeCol[i] = snakeCol[i - 1];
      }
      snakeRow[0] = headR;
      snakeCol[0] = headC;
    }

    /* ----- Draw snake and blinking food on matrix ----- */
    frameCount++;
    uint8_t bitmap[8][12] = {0};
    for (int i = 0; i < snakeLen; i++)
      bitmap[snakeRow[i]][snakeCol[i]] = 1;
    if ((frameCount / 2) % 2 == 0)
      bitmap[foodRow][foodCol] = 1;
    matrix.renderBitmap(bitmap, 8, 12);
  }

  /* ----- Wait for next move; keep score display updated ----- */
  for (unsigned long t = millis(); millis() - t < (unsigned long)gameDelay; ) {
    updateScoreDisplay(score);
    delay(3);
  }
}
