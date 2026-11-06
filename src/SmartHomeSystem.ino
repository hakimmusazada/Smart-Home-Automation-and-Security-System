/*
  ============================================================================
  EMBEDDED IoT & SECURITY SYSTEM - ARDUINO UNO
  ============================================================================
  MODES:
    - AUTONOMOUS MODE
    - MANUAL MODE

  MANUAL IR BUTTONS:
    1 -> Temperature
    2 -> Humidity
    3 -> Light Level
    4 -> Room Light ON
    5 -> Room Light OFF
    6 -> Door LOCK (Servo)
    7 -> Door UNLOCK (Servo)
    8 -> Enter / Exit DINO GAME

  DINO GAME:
    0x15 -> JUMP
    0x43 -> RESTART
    0x52 -> EXIT GAME

  ============================================================================
*/

#include <DHT.h>
#include <Servo.h>

#define DECODE_NEC
#include <IRremote.hpp>

#include <MD_MAX72xx.h>


// ============================================================================
// PIN MAP
// ============================================================================

#define PIN_IR_RECV      2
#define PIN_DHT          3
#define PIN_TRIG         4
#define PIN_ECHO         5
#define PIN_SERVO        6
#define PIN_BUZZER       7
#define PIN_BUTTON       8
#define PIN_MATRIX_DIN   9
#define PIN_MATRIX_CLK   10
#define PIN_MATRIX_CS    11
#define PIN_ROOM_LED     12
#define PIN_ALARM_LED    13
#define PIN_LDR          A5


// ============================================================================
// DEBUG
// ============================================================================

#define DEBUG_IR         false
#define DEBUG_BUTTON     true


// ============================================================================
// IR REMOTE COMMANDS
// ============================================================================

#define IR_BTN_1         0x0C
#define IR_BTN_2         0x18
#define IR_BTN_3         0x5E
#define IR_BTN_4         0x08
#define IR_BTN_5         0x1C
#define IR_BTN_6         0x5A
#define IR_BTN_7         0x42
#define IR_BTN_8         0x52


// ============================================================================
// DINO GAME IR COMMANDS
// ============================================================================

#define JUMP_BUTTON      0x15
#define RESTART_BUTTON   0x43


// ============================================================================
// TIMING / THRESHOLDS
// ============================================================================

const unsigned long DEBOUNCE_MS            = 50;
const unsigned long SENSOR_READ_INTERVAL   = 2000;
const unsigned long DISPLAY_SCROLL_SPEED   = 60;
const unsigned long VALUE_SHOW_DURATION    = 4000;
const unsigned long SECURITY_SCAN_INTERVAL = 300;
const unsigned long SECURITY_TIMEOUT_MS    = 10000;
const unsigned long UNLOCK_HOLD_MS         = 3000;
const unsigned long ALARM_DURATION_MS      = 5000;

const float DISTANCE_THRESHOLD_CM = 30.0;

const int LDR_DARK_THRESHOLD = 60;

const uint8_t CHAR_SPACING = 1;


// ============================================================================
// HARDWARE OBJECTS
// ============================================================================

DHT dht(PIN_DHT, DHT11);

Servo doorServo;


#define MATRIX_HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MATRIX_DEVICES       4

MD_MAX72XX mx = MD_MAX72XX(
  MATRIX_HARDWARE_TYPE,
  PIN_MATRIX_DIN,
  PIN_MATRIX_CLK,
  PIN_MATRIX_CS,
  MATRIX_DEVICES
);


// ============================================================================
// SYSTEM MODE
// ============================================================================

enum SystemMode {
  MODE_AUTONOMOUS,
  MODE_MANUAL
};

SystemMode systemMode = MODE_AUTONOMOUS;


// ============================================================================
// SECURITY STATE
// ============================================================================

enum SecurityState {
  SEC_IDLE,
  SEC_WAITING_FOR_VERDICT,
  SEC_UNLOCKED,
  SEC_ALARM
};

SecurityState securityState = SEC_IDLE;

unsigned long securityStateChangedAt = 0;


// ============================================================================
// DISPLAY STATE
// ============================================================================

enum DisplayState {
  DISP_SCROLL,
  DISP_FORCED_VALUE,
  DISP_GAME
};

DisplayState displayState = DISP_SCROLL;

unsigned long forcedValueShownAt = 0;


// ============================================================================
// SYSTEM VARIABLES
// ============================================================================

bool doorUnlocked = false;

bool roomLightOn = false;

bool manualLightOverride = false;

bool statusDirty = true;


float lastTemp = NAN;

float lastHum = NAN;

int lastLight = 0;


unsigned long lastSensorRead = 0;

unsigned long lastSecurityScan = 0;


// ============================================================================
// BUTTON STATE
// ============================================================================

bool buttonPressed = false;

bool lastRawButtonState = HIGH;

unsigned long lastDebounceTime = 0;

unsigned long lastButtonDebugPrint = 0;


// ============================================================================
// SCROLLING TEXT
// ============================================================================

enum ScrollState {
  S_IDLE,
  S_NEXT_CHAR,
  S_SHOW_CHAR,
  S_SHOW_SPACE
};

ScrollState scrollState = S_IDLE;

char displayMessage[48] = "SYSTEM READY";

uint8_t scrollMsgIndex = 0;

uint8_t scrollCharLen = 0;

uint8_t scrollColIndex = 0;

uint8_t scrollCharBuf[8];

unsigned long lastScrollTime = 0;


// ============================================================================
// DINO GAME
// ============================================================================

#define GAME_WIDTH 32
#define GAME_HEIGHT 8


// ---------------------------------------------------------------------------
// DINO SPRITE
// ---------------------------------------------------------------------------

const byte dinoSprite[5] = {

  B0110,
  B1111,
  B0111,
  B1110,
  B1010

};


const int DINO_X = 3;


// ---------------------------------------------------------------------------
// GAME DISPLAY BUFFER
// ---------------------------------------------------------------------------

byte gameDisplayBuffer[GAME_HEIGHT][4];


// ---------------------------------------------------------------------------
// DINO PHYSICS
// ---------------------------------------------------------------------------

float dinoY = 2.0;

float velocityY = 0;


const float GRAVITY = 0.35;

const float JUMP_FORCE = -2.8;


bool jumping = false;


// ---------------------------------------------------------------------------
// GAME STATE
// ---------------------------------------------------------------------------

bool gameOver = false;


// ---------------------------------------------------------------------------
// CACTUS
// ---------------------------------------------------------------------------

int obstacleX = GAME_WIDTH + 5;

int cactusType = 0;

int cactusWidth = 2;

int cactusHeight = 4;


// ---------------------------------------------------------------------------
// GAME SPEED
// ---------------------------------------------------------------------------

unsigned long frameInterval = 70;

unsigned long lastGameFrame = 0;


// ---------------------------------------------------------------------------
// SCORE
// ---------------------------------------------------------------------------

int score = 0;


// ============================================================================
// GAME LETTER FONT
// ============================================================================

const byte Y_letter[7] = {

  B10001,
  B01010,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100

};


const byte O_letter[7] = {

  B01110,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B01110

};


const byte U_letter[7] = {

  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B01110

};


const byte L_letter[7] = {

  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B11111

};


const byte S_letter[7] = {

  B01111,
  B10000,
  B10000,
  B01110,
  B00001,
  B00001,
  B11110

};


const byte T_letter[7] = {

  B11111,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100

};


const byte C_letter[7] = {

  B11111,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B11111

};


const byte R_letter[7] = {

  B11110,
  B10001,
  B10010,
  B11100,
  B10010,
  B10001,
  B10001

};


const byte E_letter[7] = {

  B11111,
  B10000,
  B10000,
  B11111,
  B10000,
  B10000,
  B11111

};


// ============================================================================
// DIGIT FONT
// ============================================================================

const byte digits[10][5] = {

  {
    B01110,
    B10001,
    B10011,
    B10101,
    B01110
  },

  {
    B00100,
    B01100,
    B00100,
    B00100,
    B01110
  },

  {
    B01110,
    B10001,
    B00010,
    B00100,
    B11111
  },

  {
    B11110,
    B00001,
    B00110,
    B00001,
    B11110
  },

  {
    B00010,
    B00110,
    B01010,
    B11111,
    B00010
  },

  {
    B11111,
    B10000,
    B11110,
    B00001,
    B11110
  },

  {
    B01110,
    B10000,
    B11110,
    B10001,
    B01110
  },

  {
    B11111,
    B00001,
    B00010,
    B00100,
    B00100
  },

  {
    B01110,
    B10001,
    B01110,
    B10001,
    B01110
  },

  {
    B01110,
    B10001,
    B01111,
    B00001,
    B01110
  }

};


// ============================================================================
// NORMAL DISPLAY MESSAGE
// ============================================================================

void setDisplayMessage(const char *msg) {

  strncpy(
    displayMessage,
    msg,
    sizeof(displayMessage) - 1
  );

  displayMessage[
    sizeof(displayMessage) - 1
  ] = '\0';

  scrollState = S_IDLE;

  mx.clear();
}


// ============================================================================
// NORMAL DISPLAY SCROLL
// ============================================================================

void scrollTick() {

  if (
    millis() - lastScrollTime <
    DISPLAY_SCROLL_SPEED
  ) {
    return;
  }


  lastScrollTime = millis();


  mx.transform(
    MD_MAX72XX::TSL
  );


  switch (scrollState) {

    case S_IDLE:

      scrollMsgIndex = 0;

      scrollState = S_NEXT_CHAR;

      break;


    case S_NEXT_CHAR:

      if (
        displayMessage[scrollMsgIndex]
        == '\0'
      ) {

        scrollState = S_IDLE;

      } else {

        scrollCharLen =
          mx.getChar(
            displayMessage[scrollMsgIndex++],
            sizeof(scrollCharBuf),
            scrollCharBuf
          );

        scrollColIndex = 0;

        scrollState = S_SHOW_CHAR;
      }

      break;


    case S_SHOW_CHAR:

      mx.setColumn(
        0,
        scrollCharBuf[
          scrollColIndex++
        ]
      );


      if (
        scrollColIndex >=
        scrollCharLen
      ) {

        scrollColIndex = 0;

        scrollState = S_SHOW_SPACE;
      }

      break;


    case S_SHOW_SPACE:

      mx.setColumn(
        0,
        0
      );

      scrollColIndex++;


      if (
        scrollColIndex >=
        CHAR_SPACING
      ) {

        scrollState = S_NEXT_CHAR;
      }

      break;
  }
}


// ============================================================================
// DINO GAME - CLEAR BUFFER
// ============================================================================

void clearGameBuffer() {

  for (
    int y = 0;
    y < GAME_HEIGHT;
    y++
  ) {

    for (
      int x = 0;
      x < 4;
      x++
    ) {

      gameDisplayBuffer[y][x] = 0;
    }
  }
}


// ============================================================================
// DINO GAME - SET PIXEL
// ============================================================================
// IMPORTANT:
// Horizontal direction is reversed.
// This is the corrected mapping from your working Dino game.
// ============================================================================

void gameSetPixel(
  int x,
  int y,
  bool state
) {

  if (
    x < 0 ||
    x >= GAME_WIDTH ||
    y < 0 ||
    y >= GAME_HEIGHT
  ) {

    return;
  }


  int reversedX =
    GAME_WIDTH - 1 - x;


  int matrixNumber =
    reversedX / 8;


  int column =
    reversedX % 8;


  if (state) {

    gameDisplayBuffer[y][matrixNumber] |=
      (1 << column);

  } else {

    gameDisplayBuffer[y][matrixNumber] &=
      ~(1 << column);
  }
}


// ============================================================================
// DINO GAME - DRAW DINO
// ============================================================================

void drawDino() {

  int startY =
    (int)dinoY;


  for (
    int y = 0;
    y < 5;
    y++
  ) {

    for (
      int x = 0;
      x < 4;
      x++
    ) {

      if (
        bitRead(
          dinoSprite[y],
          3 - x
        )
      ) {

        gameSetPixel(
          DINO_X + x,
          startY + y,
          true
        );
      }
    }
  }
}


// ============================================================================
// DINO GAME - GENERATE CACTUS
// ============================================================================

void generateCactus() {

  /*
     0 = single small
     1 = double small
     2 = single tall
     3 = double + tall
     4 = triple small
  */

  cactusType =
    random(0, 5);


  switch (cactusType) {

    case 0:

      cactusWidth = 2;

      cactusHeight = 3;

      break;


    case 1:

      cactusWidth = 5;

      cactusHeight = 3;

      break;


    case 2:

      cactusWidth = 2;

      cactusHeight = 4;

      break;


    case 3:

      cactusWidth = 5;

      cactusHeight = 4;

      break;


    case 4:

      cactusWidth = 8;

      cactusHeight = 3;

      break;
  }
}


// ============================================================================
// DINO GAME - DRAW SINGLE CACTUS
// ============================================================================

void drawSingleCactus(
  int x,
  int height
) {

  int bottomRow = 7;


  int topRow =
    bottomRow - height + 1;


  // Main body

  for (
    int y = topRow;
    y <= bottomRow;
    y++
  ) {

    gameSetPixel(
      x,
      y,
      true
    );
  }


  // Left arm

  if (height >= 3) {

    if (x - 1 >= 0) {

      gameSetPixel(
        x - 1,
        topRow + 2,
        true
      );
    }
  }


  // Right arm

  if (height >= 3) {

    if (x + 1 < GAME_WIDTH) {

      gameSetPixel(
        x + 1,
        topRow + 1,
        true
      );
    }
  }
}


// ============================================================================
// DINO GAME - DRAW CACTUS GROUP
// ============================================================================

void drawCactus() {

  switch (cactusType) {

    case 0:

      drawSingleCactus(
        obstacleX,
        3
      );

      break;


    case 1:

      drawSingleCactus(
        obstacleX,
        3
      );

      drawSingleCactus(
        obstacleX + 3,
        3
      );

      break;


    case 2:

      drawSingleCactus(
        obstacleX,
        4
      );

      break;


    case 3:

      drawSingleCactus(
        obstacleX,
        4
      );

      drawSingleCactus(
        obstacleX + 3,
        3
      );

      break;


    case 4:

      drawSingleCactus(
        obstacleX,
        3
      );

      drawSingleCactus(
        obstacleX + 3,
        3
      );

      drawSingleCactus(
        obstacleX + 6,
        3
      );

      break;
  }
}


// ============================================================================
// DINO GAME - GROUND
// ============================================================================

void drawGround() {

  for (
    int x = 0;
    x < GAME_WIDTH;
    x++
  ) {

    gameSetPixel(
      x,
      7,
      true
    );
  }
}


// ============================================================================
// DINO GAME - DISPLAY
// ============================================================================

void updateGameDisplay() {

  for (
    int row = 0;
    row < GAME_HEIGHT;
    row++
  ) {

    for (
      int device = 0;
      device < 4;
      device++
    ) {

      mx.setRow(
        device,
        row,
        gameDisplayBuffer[row][device]
      );
    }
  }
}


// ============================================================================
// DINO GAME - JUMP
// ============================================================================

void jump() {

  if (
    !jumping &&
    !gameOver
  ) {

    velocityY =
      JUMP_FORCE;

    jumping = true;
  }
}


// ============================================================================
// DINO GAME - COLLISION
// ============================================================================

bool checkCollision() {

  int dinoLeft =
    DINO_X;


  int dinoRight =
    DINO_X + 3;


  int dinoTop =
    (int)dinoY;


  int dinoBottom =
    (int)dinoY + 4;


  int cactusLeft =
    obstacleX;


  int cactusRight =
    obstacleX +
    cactusWidth - 1;


  int cactusTop =
    7 - cactusHeight;


  int cactusBottom =
    7;


  if (
    dinoRight >= cactusLeft &&
    dinoLeft <= cactusRight &&
    dinoBottom >= cactusTop &&
    dinoTop <= cactusBottom
  ) {

    return true;
  }


  return false;
}


// ============================================================================
// DINO GAME - DRAW LETTER
// ============================================================================

void drawGameLetter(
  const byte letter[7],
  int startX
) {

  for (
    int y = 0;
    y < 7;
    y++
  ) {

    for (
      int x = 0;
      x < 5;
      x++
    ) {

      if (
        bitRead(
          letter[y],
          4 - x
        )
      ) {

        gameSetPixel(
          startX + x,
          y,
          true
        );
      }
    }
  }
}


// ============================================================================
// DINO GAME - DRAW DIGIT
// ============================================================================

void drawGameDigit(
  int digit,
  int startX
) {

  if (
    digit < 0 ||
    digit > 9
  ) {

    return;
  }


  for (
    int y = 0;
    y < 5;
    y++
  ) {

    for (
      int x = 0;
      x < 5;
      x++
    ) {

      if (
        bitRead(
          digits[digit][y],
          4 - x
        )
      ) {

        gameSetPixel(
          startX + x,
          y + 1,
          true
        );
      }
    }
  }
}


// ============================================================================
// DINO GAME - YOU LOST
// ============================================================================

void drawYouLost(
  int offset
) {

  drawGameLetter(
    Y_letter,
    offset
  );


  drawGameLetter(
    O_letter,
    offset + 6
  );


  drawGameLetter(
    U_letter,
    offset + 12
  );


  drawGameLetter(
    L_letter,
    offset + 24
  );


  drawGameLetter(
    O_letter,
    offset + 30
  );


  drawGameLetter(
    S_letter,
    offset + 36
  );


  drawGameLetter(
    T_letter,
    offset + 42
  );
}


// ============================================================================
// DINO GAME - SCORE SCREEN
// ============================================================================

void showGameScoreScreen() {

  // --------------------------------------------------
  // SCORE
  // --------------------------------------------------

  for (
    int i = 0;
    i < 20;
    i++
  ) {

    clearGameBuffer();


    drawGameLetter(
      S_letter,
      1
    );


    drawGameLetter(
      C_letter,
      7
    );


    drawGameLetter(
      O_letter,
      13
    );


    drawGameLetter(
      R_letter,
      19
    );


    drawGameLetter(
      E_letter,
      25
    );


    updateGameDisplay();

    delay(50);
  }


  // --------------------------------------------------
  // SCORE NUMBER
  // --------------------------------------------------

  for (
    int i = 0;
    i < 30;
    i++
  ) {

    clearGameBuffer();


    if (score < 10) {

      drawGameDigit(
        score,
        13
      );

    } else {

      int tens =
        score / 10;

      int ones =
        score % 10;


      // Keep score display safe
      if (tens > 9)
        tens = 9;


      drawGameDigit(
        tens,
        10
      );


      drawGameDigit(
        ones,
        17
      );
    }


    updateGameDisplay();

    delay(50);
  }
}


// ============================================================================
// DINO GAME - GAME OVER SCREEN
// ============================================================================

void gameOverScreen() {

  // --------------------------------------------------
  // YOU LOST
  // --------------------------------------------------

  for (
    int offset = GAME_WIDTH;
    offset > -49;
    offset--
  ) {

    clearGameBuffer();


    drawYouLost(
      offset
    );


    updateGameDisplay();


    delay(70);


    // -----------------------------------------------
    // Check restart / exit
    // -----------------------------------------------

    if (IrReceiver.decode()) {

      uint8_t command =
        IrReceiver.decodedIRData.command;


      // 8 = exit game

      if (
        command == IR_BTN_8
      ) {

        IrReceiver.resume();

        return;
      }


      // 0x43 = restart

      if (
        command == RESTART_BUTTON
      ) {

        IrReceiver.resume();

        return;
      }


      IrReceiver.resume();
    }
  }


  // --------------------------------------------------
  // SCORE
  // --------------------------------------------------

  showGameScoreScreen();


  // --------------------------------------------------
  // WAIT FOR RESTART OR EXIT
  // --------------------------------------------------

  while (true) {

    clearGameBuffer();

    updateGameDisplay();


    if (IrReceiver.decode()) {

      uint8_t command =
        IrReceiver.decodedIRData.command;


      // 8 = EXIT

      if (
        command == IR_BTN_8
      ) {

        IrReceiver.resume();

        return;
      }


      // 0x43 = RESTART

      if (
        command == RESTART_BUTTON
      ) {

        IrReceiver.resume();

        return;
      }


      IrReceiver.resume();
    }
  }
}


// ============================================================================
// DINO GAME - RESET
// ============================================================================

void resetGame() {

  clearGameBuffer();

  updateGameDisplay();


  dinoY = 2.0;

  velocityY = 0;

  jumping = false;

  gameOver = false;


  obstacleX =
    GAME_WIDTH + 5;


  score = 0;


  frameInterval = 70;


  generateCactus();


  lastGameFrame =
    millis();
}


// ============================================================================
// ENTER GAME
// ============================================================================

void enterGame() {

  displayState =
    DISP_GAME;


  resetGame();


  Serial.println(
    F("STATUS:GAME:STARTED")
  );
}


// ============================================================================
// EXIT GAME
// ============================================================================

void exitGame() {

  displayState =
    DISP_SCROLL;


  statusDirty = true;

  gameOver = false;

  clearGameBuffer();

  mx.clear();


  Serial.println(
    F("STATUS:GAME:EXITED")
  );
}


// ============================================================================
// DINO GAME - MAIN UPDATE
// ============================================================================

void updateGame() {

  // --------------------------------------------------
  // GAME OVER
  // --------------------------------------------------

  if (gameOver) {

    gameOverScreen();


    /*
       IMPORTANT:

       If the user pressed 8,
       we leave the game.

       Otherwise 0x43 restarts.
    */

    if (displayState == DISP_GAME) {

      resetGame();

    } else {

      exitGame();
    }


    return;
  }


  // --------------------------------------------------
  // GAME SPEED
  // --------------------------------------------------

  if (
    millis() - lastGameFrame <
    frameInterval
  ) {

    return;
  }


  lastGameFrame =
    millis();


  // --------------------------------------------------
  // DINO PHYSICS
  // --------------------------------------------------

  if (jumping) {

    dinoY += velocityY;

    velocityY += GRAVITY;


    if (dinoY >= 2.0) {

      dinoY = 2.0;

      velocityY = 0;

      jumping = false;
    }
  }


  // --------------------------------------------------
  // MOVE CACTUS
  // --------------------------------------------------

  obstacleX--;


  // --------------------------------------------------
  // CACTUS PASSED
  // --------------------------------------------------

  if (
    obstacleX <
    -cactusWidth
  ) {

    score++;


    Serial.print(
      F("Score: ")
    );

    Serial.println(score);


    obstacleX =
      GAME_WIDTH +
      random(5, 13);


    generateCactus();


    // Increase speed

    if (
      frameInterval > 35
    ) {

      frameInterval -= 2;
    }
  }


  // --------------------------------------------------
  // COLLISION
  // --------------------------------------------------

  if (
    checkCollision()
  ) {

    gameOver = true;


    Serial.println(
      F("GAME OVER!")
    );


    return;
  }


  // --------------------------------------------------
  // BUILD GAME FRAME
  // --------------------------------------------------

  clearGameBuffer();


  drawGround();

  drawDino();

  drawCactus();


  updateGameDisplay();
}


// ============================================================================
// BUTTON - MODE SWITCH
// ============================================================================

void handleButton() {

  bool raw =
    digitalRead(PIN_BUTTON);


  if (
    DEBUG_BUTTON &&
    millis() - lastButtonDebugPrint > 1000
  ) {

    lastButtonDebugPrint =
      millis();


    Serial.print(
      F("DEBUG:BUTTON_RAW:")
    );

    Serial.println(raw);
  }


  if (
    raw != lastRawButtonState
  ) {

    lastDebounceTime =
      millis();
  }


  if (
    millis() - lastDebounceTime >
    DEBOUNCE_MS
  ) {

    if (
      raw == LOW &&
      !buttonPressed
    ) {

      buttonPressed = true;

      toggleMode();

    } else if (
      raw == HIGH &&
      buttonPressed
    ) {

      buttonPressed = false;
    }
  }


  lastRawButtonState =
    raw;
}


// ============================================================================
// TOGGLE MODE
// ============================================================================

void toggleMode() {

  if (
    systemMode ==
    MODE_AUTONOMOUS
  ) {

    switchToManualMode();

  } else {

    switchToAutonomousMode();
  }
}


// ============================================================================
// SWITCH TO AUTONOMOUS
// ============================================================================

void switchToAutonomousMode() {

  if (
    systemMode ==
    MODE_AUTONOMOUS
  ) {

    return;
  }


  // If currently in game, exit it

  if (
    displayState ==
    DISP_GAME
  ) {

    exitGame();
  }


  systemMode =
    MODE_AUTONOMOUS;


  manualLightOverride =
    false;


  displayState =
    DISP_SCROLL;


  statusDirty =
    true;


  Serial.println(
    F("MODE:AUTONOMOUS")
  );
}


// ============================================================================
// SWITCH TO MANUAL
// ============================================================================

void switchToManualMode() {

  if (
    systemMode ==
    MODE_MANUAL
  ) {

    return;
  }


  systemMode =
    MODE_MANUAL;


  displayState =
    DISP_SCROLL;


  statusDirty =
    true;


  Serial.println(
    F("MODE:MANUAL")
  );
}


// ============================================================================
// SENSOR READING
// ============================================================================

void readSensorsPeriodically() {

  if (
    millis() - lastSensorRead <
    SENSOR_READ_INTERVAL
  ) {

    return;
  }


  lastSensorRead =
    millis();


  float h =
    dht.readHumidity();


  float t =
    dht.readTemperature();


  int light =
    analogRead(PIN_LDR);


  if (!isnan(h))
    lastHum = h;


  if (!isnan(t))
    lastTemp = t;


  lastLight =
    light;


  statusDirty =
    true;


  Serial.print(
    F("DATA:TEMP:")
  );

  Serial.println(
    lastTemp,
    1
  );


  Serial.print(
    F("DATA:HUM:")
  );

  Serial.println(
    lastHum,
    1
  );


  Serial.print(
    F("DATA:LIGHT:")
  );

  Serial.println(
    lastLight
  );
}


// ============================================================================
// ULTRASONIC
// ============================================================================

float readDistanceCM() {

  digitalWrite(
    PIN_TRIG,
    LOW
  );


  delayMicroseconds(2);


  digitalWrite(
    PIN_TRIG,
    HIGH
  );


  delayMicroseconds(10);


  digitalWrite(
    PIN_TRIG,
    LOW
  );


  long duration =
    pulseIn(
      PIN_ECHO,
      HIGH,
      25000
    );


  if (
    duration == 0
  ) {

    return -1;
  }


  return duration *
         0.0343 /
         2.0;
}


// ============================================================================
// AUTONOMOUS MODE
// ============================================================================

void runAutonomousLogic() {

  bool shouldBeOn =
    (
      lastLight <
      LDR_DARK_THRESHOLD
    );


  if (
    shouldBeOn !=
    roomLightOn
  ) {

    roomLightOn =
      shouldBeOn;


    digitalWrite(
      PIN_ROOM_LED,
      roomLightOn
      ? HIGH
      : LOW
    );


    Serial.print(
      F("STATUS:ROOM_LIGHT:")
    );


    Serial.println(
      roomLightOn
      ? F("ON")
      : F("OFF")
    );
  }


  if (
    securityState == SEC_IDLE &&
    millis() - lastSecurityScan >=
    SECURITY_SCAN_INTERVAL
  ) {

    lastSecurityScan =
      millis();


    float dist =
      readDistanceCM();


    if (
      dist > 0 &&
      dist < DISTANCE_THRESHOLD_CM
    ) {

      triggerSecuritySequence();
    }
  }
}


// ============================================================================
// SECURITY TRIGGER
// ============================================================================

void triggerSecuritySequence() {

  securityState =
    SEC_WAITING_FOR_VERDICT;


  securityStateChangedAt =
    millis();


  displayState =
    DISP_FORCED_VALUE;


  forcedValueShownAt =
    millis();


  Serial.println(
    F("EVENT:PERSON_DETECTED")
  );


  setDisplayMessage(
    "SCANNING"
  );
}


// ============================================================================
// SECURITY STATE MACHINE
// ============================================================================

void updateSecurityStateMachine() {

  switch (securityState) {

    case SEC_WAITING_FOR_VERDICT:

      if (
        millis() -
        securityStateChangedAt >
        SECURITY_TIMEOUT_MS
      ) {

        Serial.println(
          F("EVENT:SECURITY_TIMEOUT")
        );


        securityState =
          SEC_IDLE;


        displayState =
          DISP_SCROLL;


        statusDirty =
          true;
      }

      break;


    case SEC_UNLOCKED:

      if (
        millis() -
        securityStateChangedAt >
        UNLOCK_HOLD_MS
      ) {

        doorServo.write(0);

        doorUnlocked =
          false;


        Serial.println(
          F("STATUS:DOOR:LOCKED")
        );


        securityState =
          SEC_IDLE;


        displayState =
          DISP_SCROLL;


        statusDirty =
          true;
      }

      break;


    case SEC_ALARM:

      if (
        millis() -
        securityStateChangedAt >
        ALARM_DURATION_MS
      ) {

        digitalWrite(
          PIN_ALARM_LED,
          LOW
        );


        noTone(
          PIN_BUZZER
        );


        securityState =
          SEC_IDLE;


        displayState =
          DISP_SCROLL;


        statusDirty =
          true;

      } else {

        digitalWrite(
          PIN_ALARM_LED,
          (millis() / 200) % 2
        );
      }

      break;


    case SEC_IDLE:

    default:

      break;
  }
}


// ============================================================================
// FACE KNOWN
// ============================================================================

void handleFaceKnown() {

  securityState =
    SEC_UNLOCKED;


  securityStateChangedAt =
    millis();


  doorServo.write(90);

  doorUnlocked =
    true;


  Serial.println(
    F("STATUS:DOOR:UNLOCKED")
  );


  successBeep();


  setDisplayMessage(
    "WELCOME"
  );
}


// ============================================================================
// FACE UNKNOWN
// ============================================================================

void handleFaceUnknown() {

  securityState =
    SEC_ALARM;


  securityStateChangedAt =
    millis();


  alarmTone();


  Serial.println(
    F("STATUS:ALARM:TRIGGERED")
  );


  setDisplayMessage(
    "ALERT"
  );
}


// ============================================================================
// SUCCESS BEEP
// ============================================================================

void successBeep() {

  tone(
    PIN_BUZZER,
    1500,
    150
  );


  delay(180);


  tone(
    PIN_BUZZER,
    2000,
    200
  );
}


// ============================================================================
// ALARM
// ============================================================================

void alarmTone() {

  tone(
    PIN_BUZZER,
    2800
  );
}


// ============================================================================
// SERIAL INPUT
// ============================================================================

void handleSerialIncoming() {

  if (
    !Serial.available()
  ) {

    return;
  }


  String line =
    Serial.readStringUntil('\n');


  line.trim();


  if (
    line.length() == 0
  ) {

    return;
  }


  if (
    line.equalsIgnoreCase(
      "CMD:FACE_KNOWN"
    )
  ) {

    handleFaceKnown();

  } else if (
    line.equalsIgnoreCase(
      "CMD:FACE_UNKNOWN"
    )
  ) {

    handleFaceUnknown();

  } else if (
    line.equalsIgnoreCase(
      "CMD:PING"
    )
  ) {

    Serial.println(
      F("STATUS:PONG")
    );

  } else {

    Serial.print(
      F("STATUS:UNKNOWN_CMD:")
    );


    Serial.println(line);
  }
}


// ============================================================================
// IR REMOTE - MANUAL MODE
// ============================================================================

void handleIRRemote() {

  if (
    !IrReceiver.decode()
  ) {

    return;
  }


  uint8_t cmd =
    IrReceiver.decodedIRData.command;


  if (DEBUG_IR) {

    Serial.print(
      F("DEBUG:IR_RAW:0x")
    );


    Serial.println(
      cmd,
      HEX
    );
  }


  // ========================================================================
  // GAME MODE
  // ========================================================================

  if (
    displayState ==
    DISP_GAME
  ) {

    // ----------------------------------------------------
    // 8 -> EXIT GAME
    // ----------------------------------------------------

    if (
      cmd == IR_BTN_8
    ) {

      IrReceiver.resume();

      exitGame();

      return;
    }


    // ----------------------------------------------------
    // JUMP
    // ----------------------------------------------------

    if (
      cmd == JUMP_BUTTON
    ) {

      jump();
    }


    // ----------------------------------------------------
    // RESTART
    // ----------------------------------------------------

    if (
      cmd == RESTART_BUTTON &&
      gameOver
    ) {

      resetGame();
    }


    IrReceiver.resume();

    return;
  }


  // ========================================================================
  // NORMAL MANUAL MODE BUTTONS 1-7
  // ========================================================================

  char buf[24];


  switch (cmd) {

    // ----------------------------------------------------------------------
    // BUTTON 1 - TEMPERATURE
    // ----------------------------------------------------------------------

    case IR_BTN_1:

      dtostrf(
        lastTemp,
        4,
        1,
        buf + 5
      );


      memcpy(
        buf,
        "TEMP ",
        5
      );


      showForcedValue(
        buf
      );

      break;


    // ----------------------------------------------------------------------
    // BUTTON 2 - HUMIDITY
    // ----------------------------------------------------------------------

    case IR_BTN_2:

      dtostrf(
        lastHum,
        4,
        1,
        buf + 4
      );


      memcpy(
        buf,
        "HUM ",
        4
      );


      showForcedValue(
        buf
      );

      break;


    // ----------------------------------------------------------------------
    // BUTTON 3 - LIGHT
    // ----------------------------------------------------------------------

    case IR_BTN_3:

      snprintf(
        buf,
        sizeof(buf),
        "LIGHT %d",
        lastLight
      );


      showForcedValue(
        buf
      );

      break;


    // ----------------------------------------------------------------------
    // BUTTON 4 - LIGHT ON
    // ----------------------------------------------------------------------

    case IR_BTN_4:

      manualLightOverride =
        true;


      roomLightOn =
        true;


      digitalWrite(
        PIN_ROOM_LED,
        HIGH
      );


      Serial.println(
        F("STATUS:ROOM_LIGHT:ON")
      );

      break;


    // ----------------------------------------------------------------------
    // BUTTON 5 - LIGHT OFF
    // ----------------------------------------------------------------------

    case IR_BTN_5:

      manualLightOverride =
        true;


      roomLightOn =
        false;


      digitalWrite(
        PIN_ROOM_LED,
        LOW
      );


      Serial.println(
        F("STATUS:ROOM_LIGHT:OFF")
      );

      break;


    // ----------------------------------------------------------------------
    // BUTTON 6 - LOCK DOOR
    // ----------------------------------------------------------------------

    case IR_BTN_6:

      doorServo.write(0);


      doorUnlocked =
        false;


      Serial.println(
        F("STATUS:DOOR:LOCKED")
      );

      break;


    // ----------------------------------------------------------------------
    // BUTTON 7 - UNLOCK DOOR
    // ----------------------------------------------------------------------

    case IR_BTN_7:

      doorServo.write(90);


      doorUnlocked =
        true;


      Serial.println(
        F("STATUS:DOOR:UNLOCKED")
      );

      break;


    // ----------------------------------------------------------------------
    // BUTTON 8 - ENTER GAME
    // ----------------------------------------------------------------------

    case IR_BTN_8:

      enterGame();

      break;


    default:

      break;
  }


  IrReceiver.resume();
}


// ============================================================================
// SHOW FORCED VALUE
// ============================================================================

void showForcedValue(
  const char *text
) {

  displayState =
    DISP_FORCED_VALUE;


  forcedValueShownAt =
    millis();


  setDisplayMessage(
    text
  );


  Serial.print(
    F("STATUS:DISPLAY:")
  );


  Serial.println(text);
}


// ============================================================================
// UPDATE DISPLAY
// ============================================================================

void updateDisplay() {

  // --------------------------------------------------------------------------
  // GAME
  // --------------------------------------------------------------------------

  if (
    displayState ==
    DISP_GAME
  ) {

    updateGame();

    return;
  }


  // --------------------------------------------------------------------------
  // FORCED VALUE
  // --------------------------------------------------------------------------

  if (
    displayState ==
    DISP_FORCED_VALUE
  ) {

    if (
      millis() -
      forcedValueShownAt >
      VALUE_SHOW_DURATION
    ) {

      displayState =
        DISP_SCROLL;


      statusDirty =
        true;
    }
  }


  // --------------------------------------------------------------------------
  // NORMAL SCROLL
  // --------------------------------------------------------------------------

  if (
    displayState ==
    DISP_SCROLL &&
    statusDirty &&
    scrollState == S_IDLE
  ) {

    refreshStatusMessage();

    statusDirty =
      false;
  }


  scrollTick();
}


// ============================================================================
// REFRESH STATUS MESSAGE
// ============================================================================

void refreshStatusMessage() {

  char buf[48];


  const char *modeStr =
    (
      systemMode ==
      MODE_AUTONOMOUS
    )
    ? "AUTO"
    : "MANUAL";


  char tBuf[8];

  char hBuf[8];


  dtostrf(
    lastTemp,
    4,
    1,
    tBuf
  );


  dtostrf(
    lastHum,
    4,
    1,
    hBuf
  );


  snprintf(
    buf,
    sizeof(buf),
    "%s T:%sC H:%s%% L:%d",
    modeStr,
    tBuf,
    hBuf,
    lastLight
  );


  setDisplayMessage(
    buf
  );
}


// ============================================================================
// SETUP
// ============================================================================

void setup() {

  Serial.begin(115200);


  Serial.println(
    F("BOOT:ARDUINO_READY")
  );


  // --------------------------------------------------------------------------
  // DHT
  // --------------------------------------------------------------------------

  dht.begin();


  // --------------------------------------------------------------------------
  // PIN MODES
  // --------------------------------------------------------------------------

  pinMode(
    PIN_TRIG,
    OUTPUT
  );


  pinMode(
    PIN_ECHO,
    INPUT
  );


  pinMode(
    PIN_BUZZER,
    OUTPUT
  );


  pinMode(
    PIN_BUTTON,
    INPUT_PULLUP
  );


  pinMode(
    PIN_ROOM_LED,
    OUTPUT
  );


  pinMode(
    PIN_ALARM_LED,
    OUTPUT
  );


  // --------------------------------------------------------------------------
  // INITIAL OUTPUT STATES
  // --------------------------------------------------------------------------

  digitalWrite(
    PIN_ROOM_LED,
    LOW
  );


  digitalWrite(
    PIN_ALARM_LED,
    LOW
  );


  noTone(
    PIN_BUZZER
  );


  // --------------------------------------------------------------------------
  // SERVO
  // --------------------------------------------------------------------------

  doorServo.attach(
    PIN_SERVO
  );


  doorServo.write(
    0
  );


  // --------------------------------------------------------------------------
  // IR
  // --------------------------------------------------------------------------

  IrReceiver.begin(
    PIN_IR_RECV,
    DISABLE_LED_FEEDBACK
  );


  // --------------------------------------------------------------------------
  // MATRIX
  // --------------------------------------------------------------------------

  mx.begin();


  mx.control(
    MD_MAX72XX::INTENSITY,
    4
  );


  mx.clear();


  setDisplayMessage(
    "SYSTEM READY"
  );


  Serial.println(
    F("MODE:AUTONOMOUS")
  );


  Serial.println(
    F("GAME:8=ENTER/EXIT")
  );


  Serial.println(
    F("GAME:0x15=JUMP")
  );


  Serial.println(
    F("GAME:0x43=RESTART")
  );
}


// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {

  // --------------------------------------------------------------------------
  // Physical mode button
  // --------------------------------------------------------------------------

  handleButton();


  // --------------------------------------------------------------------------
  // Raspberry Pi serial commands
  // --------------------------------------------------------------------------

  handleSerialIncoming();


  // --------------------------------------------------------------------------
  // Sensors
  // --------------------------------------------------------------------------

  readSensorsPeriodically();


  // --------------------------------------------------------------------------
  // AUTO / MANUAL
  // --------------------------------------------------------------------------

  if (
    systemMode ==
    MODE_AUTONOMOUS
  ) {

    runAutonomousLogic();

  } else {

    handleIRRemote();
  }


  // --------------------------------------------------------------------------
  // Security
  // --------------------------------------------------------------------------

  updateSecurityStateMachine();


  // --------------------------------------------------------------------------
  // Display
  // --------------------------------------------------------------------------

  updateDisplay();
}
