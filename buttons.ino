#include <Adafruit_NeoPixel.h>
#include <avr/eeprom.h>
#include <EEPROM.h>

// #include <SPI.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// #define SCREEN_WIDTH 128  // OLED display width, in pixels
// #define SCREEN_HEIGHT 32  // OLED display height, in pixels
// #define OLED_RESET -1
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);




int ledPin = 8;
int numPixels = 239;
const int arenaX = 10;
const int arenaY = 24;
int arena[arenaY][arenaX] = { 0 };
int downTime = 500;
int rightButton = 9;
int leftButton = 11;
int resetButton = 10;
int moveTime = 200;
int score = 0;
int animationTime = 100;
int piece = 0;
int color = 1;
int level = 1;
int highscore = 0;
Adafruit_NeoPixel pixels(numPixels, ledPin, NEO_GRB + NEO_KHZ800);

//////////////////////////////////// random shit



/*==============================================================================
  Call reseedRandom once in setup to start random on a new sequence.  Uses
  four bytes of EEPROM.
==============================================================================*/

void reseedRandom(uint32_t *address) {
  static const uint32_t HappyPrime = 127807;
  uint32_t raw;
  unsigned long seed;

  // Read the previous raw value from EEPROM
  raw = eeprom_read_dword(address);

  // Loop until a seed within the valid range is found
  do {
    // Incrementing by a prime (except 2) every possible raw value is visited
    raw += HappyPrime;

    // Park-Miller is only 31 bits so ignore the most significant bit
    seed = raw & 0x7FFFFFFF;
  } while ((seed < 1) || (seed > 2147483646));

  // Seed the random number generator with the next value in the sequence
  srandom(seed);

  // Save the new raw value for next time
  eeprom_write_dword(address, raw);
}

inline void reseedRandom(unsigned short address) {
  reseedRandom((uint32_t *)(address));
}

/*==============================================================================
  So the reseedRandom raw value can be initialized allowing different
  applications or instances to have different random sequences.

  Generate initial raw values...

  https://www.random.org/cgi-bin/randbyte?nbytes=4&format=h
  https://www.fourmilab.ch/cgi-bin/Hotbits?nbytes=4&fmt=c&npass=1&lpass=8&pwtype=3

==============================================================================*/

void reseedRandomInit(uint32_t *address, uint32_t value) {
  eeprom_write_dword(address, value);
}

inline void reseedRandomInit(unsigned short address, uint32_t value) {
  reseedRandomInit((uint32_t *)(address), value);
}

uint32_t reseedRandomSeed EEMEM = 0xFFFFFFFF;

//////////////////////////////////

void colorSetter(int pixel, int colorCode) {
  switch (colorCode) {
    case 0:
      pixels.setPixelColor(pixel, pixels.Color(0, 0, 0));
      break;
    case 1:
      pixels.setPixelColor(pixel, pixels.Color(0, 255, 159));
      break;
    case 2:
      pixels.setPixelColor(pixel, pixels.Color(0, 184, 255));
      break;
    case 3:
      pixels.setPixelColor(pixel, pixels.Color(0, 30, 255));
      break;
    case 4:
      pixels.setPixelColor(pixel, pixels.Color(189, 0, 255));
      break;
    case 5:
      pixels.setPixelColor(pixel, pixels.Color(214, 0, 255));
      break;
    case 6:
      pixels.setPixelColor(pixel, pixels.Color(214, 0, 255));
      break;
  }
}

void printScreen() {

  for (int i = 4; i < arenaY; ++i) {
    int offset = i - 4;
    for (int j = 0; j < arenaX; ++j) {
      int pixel;
      if (i % 2 == 0)
        pixel = offset * arenaX + j + offset;
      else
        pixel = offset * arenaX + (arenaX - j - 1) + offset;

      if (arena[i][j] == 0)
        colorSetter(pixel, 0);
      else
        colorSetter(pixel, arena[i][j]);
    }
  }
  pixels.show();
}

///////////////////// Point

class Point {
public:
  int x = 0;
  int y = 0;
  Point addPoint(int x_add, int y_add);
  void setPoint(int x_new, int y_new);
  bool checkPoint();
};

Point Point::addPoint(int x_add, int y_add) {
  Point temp;
  temp.x += x + x_add;
  temp.y += y + y_add;
  return temp;
}
void Point::setPoint(int x_new, int y_new) {
  x = x_new;
  y = y_new;
}

bool Point::checkPoint() {
  if (x >= 0 && x < arenaX && y >= 0 && y < arenaY && arena[y][x] == 0)
    return true;
  return false;
}
/////////////////////////////// square

class Player {
private:
  Point blocks[6];
  int color = 1;
  void setSquare();

public:
  Point pivot;
  int health;
  void spawn();
  void remove();
  void goLeft();
  void goRight();
  // 01
  // 23
};

void Player::setSquare() {
  blocks[0] = pivot;
  blocks[1] = pivot.addPoint(1, 1);
  blocks[2] = pivot.addPoint(0, 1);
  blocks[3] = pivot.addPoint(-1, 1);
  blocks[4] = pivot.addPoint(1, 2);
  blocks[5] = pivot.addPoint(-1, 2);
}

void Player::spawn() {
  setSquare();
  for (int i = 0; i < 6; ++i)
    arena[blocks[i].y][blocks[i].x] = 5;
}
void Player::remove() {
  setSquare();
  for (int i = 0; i < 6; ++i)
    arena[blocks[i].y][blocks[i].x] = 0;
}


void Player::goLeft() {

  if (pivot.x - 2 >= 0) {
    remove();
    pivot = pivot.addPoint(-1, 0);
    spawn();
  }
}
void Player::goRight() {

  if (pivot.x + 2 < 10) {
    remove();
    pivot = pivot.addPoint(1, 0);
    spawn();
  }
}


class enemy {
private:
  Point blocks[6];
  int color = 2;
  void setSquare();

public:
  Point pivot;
  int health;
  void spawn();
  void remove();

  void goDown();

  // 01
  // 23
};


void enemy::setSquare() {
  blocks[0] = pivot;
  blocks[1] = pivot.addPoint(1, -1);
  blocks[2] = pivot.addPoint(0, -1);
  blocks[3] = pivot.addPoint(-1, -1);
  blocks[4] = pivot.addPoint(1, -2);
  blocks[5] = pivot.addPoint(-1, -2);
}

void enemy::spawn() {
  setSquare();
  for (int i = 0; i < 6; ++i)
    arena[blocks[i].y][blocks[i].x] = 1;
}

void enemy::remove() {
  setSquare();
  for (int i = 0; i < 6; ++i)
    arena[blocks[i].y][blocks[i].x] = 0;
}


void enemy::goDown() {
  remove();
  pivot = pivot.addPoint(0, 1);
  spawn();
}

int checkButtonPress() {
  int buttonLeft = digitalRead(leftButton);
  int buttonRight = digitalRead(rightButton);

  if (buttonLeft == LOW) {
    // Serial.println("left Button Pressed");
    return 1;
  }

  else if (buttonRight == LOW)
    return 2;
  else if (resetButton == LOW)
    return 3;
  else
    return 0;
}


class Square {
private:
  Point blocks[4];
  int color = 1;
  void setSquare();

public:
  Point pivot;
  int state;
  void spawn();
  void remove();
  bool canGoDown();
  void goDown();
  void goLeft();
  void goRight();
  // 01
  // 23
};

void Square::setSquare() {
  blocks[0] = pivot;
  blocks[1] = pivot.addPoint(1, 0);
  blocks[2] = pivot.addPoint(0, 1);
  blocks[3] = pivot.addPoint(1, 1);
}

void Square::spawn() {
  setSquare();
  for (int i = 0; i < 4; ++i)
    arena[blocks[i].y][blocks[i].x] = 6;
}
void Square::remove() {
  setSquare();
  for (int i = 0; i < 4; ++i)
    arena[blocks[i].y][blocks[i].x] = 0;
}

void Square::goDown() {
  remove();
  pivot = pivot.addPoint(0, 1);
  spawn();
}




class bullet {
private:
  Point blocks[1];
  int color = 4;
  void setSquare();

public:
  Point pivot;
  int state;
  void spawn();
  void remove();
  bool canGoUp();
  void goUp();
  bool canGoDown();
  void goDown();

  // 01
  // 23
};


void bullet::setSquare() {
  blocks[0] = pivot;
}

void bullet::spawn() {
  setSquare();

  arena[blocks[0].y][blocks[0].x] = 3;
}
void bullet::remove() {
  arena[blocks[0].y][blocks[0].x] = 0;
}



bool bullet::canGoDown() {

  Point temp = blocks[0].addPoint(0, 1);
  if (!temp.checkPoint())
    return false;


  return true;
}

bool bullet::canGoUp() {

  Point temp = blocks[0].addPoint(0, -1);
  if (!temp.checkPoint())
    return false;


  return true;
}

void bullet::goUp() {
  remove();
  pivot = pivot.addPoint(0, -1);
  spawn();
}
void bullet::goDown() {
  remove();
  pivot = pivot.addPoint(0, 2);
  spawn();
}


int bulletx[20] = { -1 };
int bullety[20] = { -1 };


unsigned long timeIntial;
unsigned long timeFinal;
Player player;
enemy enemy1;
Square sqr;
// bullet playerbullet[20];
bullet enemybullet;


void updateHighScore(int score) {
  highscore = EEPROM.get(50, highscore);
  if (score > highscore) {
    highscore = score;
    EEPROM.put(50, score);
  }
}

void printScore(int score) {
  updateHighScore(score);
  Serial.print(F("S: "));
  Serial.print(score);
  Serial.print(F(" | H:"));
  Serial.println(highscore);
}

void printhealth(int score) {
  updateHighScore(player.health);
  Serial.print(F("S: "));
  Serial.print(player.health);
  Serial.print(F(" | H:"));
  // Serial.println(highscore);
}



void setup() {
  // Serial.begin(9600);
  // if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3D for 128x64
  //   Serial.println(F("SSD1306 allocation failed"));
  //   for (;;)
  //     ;
  // }
  // delay(2000);
  // display.clearDisplay();
  // display.setTextSize(1);
  // display.setTextColor(WHITE);
  // display.setCursor(0, 10);
  // display.display();
  // display.clearDisplay();
  // Serial.begin(9600);



  reseedRandom(&reseedRandomSeed);
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);
  pinMode(resetButton, INPUT_PULLUP);

  pixels.begin();
  pixels.clear();


  timeIntial = millis();
  timeFinal = millis();
  player.pivot.setPoint(5, 21);
  enemy1.pivot.setPoint(4, 2);
  sqr.pivot.setPoint(8, 7);
  enemybullet.pivot.setPoint(enemy1.pivot.x, enemy1.pivot.y);
  player.health = 10;
  enemy1.health = 3;
}
int count = 0;
bool inPlay = true;


void loop() {
  // put your main code here, to run repeatedly:
  if (inPlay) {
    count++;
    if (count == 100) {
      count == 0;
      level++;
      downTime = downTime - 50;
    }
    player.spawn();
    enemy1.spawn();
    sqr.spawn();
    // playerbullet.spawn();
    // enemybullet.pivot.setPoint(enemy1.pivot.x, enemy1.pivot.y);
    enemybullet.spawn();
    timeFinal = millis();
    if ((timeFinal - timeIntial) > downTime) {
      timeFinal = millis();
      timeIntial = timeFinal;
      if (enemy1.pivot.y < 20) {
        enemy1.goDown();
        // Serial.println("i");
      } else {
        enemy1.remove();
        // sqr.remove();
        enemybullet.remove();
        enemy1.pivot.y = 2;
        // sqr.pivot.y = 0;

        enemy1.pivot.x = random(7) + 1;
        // sqr.pivot.x = random(2) + 1;
        if (player.pivot.x == enemy1.pivot.x) {
          player.health = player.health - 2;
          printhealth(player.health);
        } else {
          player.health--;
          printhealth(player.health);
        }
        // Serial.println(player.health);
        if (player.health <= 0) {
          inPlay = false;
        }
      }

      if (sqr.pivot.y < 20) {
        sqr.goDown();
        // Serial.println("i");
      } else {
        sqr.remove();
        // sqr.remove();

        sqr.pivot.y = 2;
        // sqr.pivot.y = 0;

        sqr.pivot.x = random(2) + 1;
        // sqr.pivot.x = random(2) + 1;
        if (player.pivot.x == sqr.pivot.x) {
          player.health = player.health - 2;
          printhealth(player.health);
        }
        // Serial.println(player.health);
        if (player.health <= 0) {
          inPlay = false;
        }
      }

      for (int i = player.pivot.y - 2; i >= 0; i--) {
        if (arena[i][player.pivot.x] == 0)
          arena[i][player.pivot.x] = 2;
      }

      delay(moveTime);
      printScreen();
      if (enemybullet.pivot.y < 20) {
        enemybullet.goDown();
      } else {
        if (enemybullet.pivot.x == player.pivot.x) {
          player.health = player.health - 2;
          player.remove();
          printScreen();
          delay(moveTime);
          player.spawn();
          printScreen();
          delay(100);
          player.remove();
          printScreen();
          delay(100);
          player.spawn();
          if (player.health <= 0) {
            inPlay = false;
          }
        }
        enemybullet.remove();
        enemybullet.pivot.y = enemy1.pivot.y + 1;
        enemybullet.pivot.x = enemy1.pivot.x;
        enemybullet.spawn();
      }

      for (int i = player.pivot.y - 2; i >= 0; i--) {
        arena[i][player.pivot.x] = 0;
      }
      printScreen();
      if (player.pivot.x == enemy1.pivot.x) {
        enemy1.health--;
        if (enemy1.health <= 0) {
          enemy1.remove();
          printScore(score);
          enemybullet.remove();
          enemy1.pivot.y = 2;

          enemy1.pivot.x = random(7) + 1;
        }
      }
    }


    int input = checkButtonPress();
    if (input != 0) {
      if (input == 1) {
        player.goLeft();
        delay(moveTime);
      } else if (input == 2) {
        player.goRight();
        delay(moveTime);
      // } else if (input == 2) {
      //   player.goRight();
      //   delay(moveTime);
      }
    }
    printScreen();


    // if (Serial.available()) {
    //   // char c = Serial.read();
    //   // if(c == '\n'){

    //   display.clearDisplay();
    //   display.setCursor(0, 10);
    //   display.println(player.health);
    //   display.display();
    //   display.println(level);
    //   display.display();

    //   //   temp = "";
    //   // }else{
    //   //   temp+=c;
    //   // }
    // }
  }

  else {
    for (int i = arenaY - 1; i >= 0; --i) {
      if (i % 2 == 0) {
        for (int j = 0; j < arenaX; ++j)
          arena[i][j] = 1;
        printScreen();
        delay(50);
      } else {
        for (int j = arenaX - 1; j >= 0; --j)
          arena[i][j] = 1;
        printScreen();
        delay(50);
      }
    }
    for (int i = 0; i < arenaY; ++i) {
      for (int j = 0; j < arenaX; ++j)
        arena[i][j] == 0;
    }
    printScreen();
    printScore(score);
    player.health = 0;
  }
}
