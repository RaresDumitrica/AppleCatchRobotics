#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"

#define JOYSTICK_Y A0
#define JOYSTICK_BUTTON 13
#define THRESHOLD_JOYSTICK_MINIM 200
#define THRESHOLD_JOYSTICK_MAXIM 800
#define MAX_LENGTH_MATRIX 7   //
#define MIN_LENGTH_MATRIX 0   //
#define CLK_PIN 11
#define MAX7219_1 12
#define LOAD_PIN 10
#define NR_DRIVER 1

#define LCD_ARDUINO_PIN 9
#define RS 2
#define E 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7

LedControl matrixDisplay = LedControl(MAX7219_1, CLK_PIN, LOAD_PIN, NR_DRIVER);
LiquidCrystal lcd (RS, E, D4, D5, D6, D7);

int playerPosition = 4; // the middle point of the basket
int score = 0;
int highScore;
int joyStickButtonState;
int intervalUpdateScreen = 200;
int timeBetweenApplesRegeneration = 3000; // time between the fall of an apple
bool gameSetup = 1; // variable for the game menu
unsigned long lastUpdateScreen = 0;
unsigned long lastApplesGeneration = 0; // the last uptate of the apple generation
int increaseSpeed = 0; // variabile that cheks the increase of the speed, without it an apple would fall too fast
// variable that verifies the speed change of the falling apples (without it after a speed change an apple would fall too fast)
int applesMatrix[][8] = // empty 8x8 matrix for the falling apples
{
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

int sadFace[][8] = // ugliest sad face ever
{
  {1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 1, 0, 0, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 1, 1, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1}
};

void setup()
{
  matrixDisplay.shutdown(0, false);
  matrixDisplay.setIntensity(0, 15); // set the matrix intensity [between 1 and 15]
  matrixDisplay.clearDisplay(0);

  pinMode(JOYSTICK_Y, INPUT);
  EEPROM_readAnything(0, highScore);

  lcd.begin (16, 2);
  lcd.clear();
  lcd.setCursor (1, 0); // set the lcd cursor's collum and row


  pinMode(JOYSTICK_BUTTON, INPUT_PULLUP); // sets button to be 1/HIGH/ON by default

  pinMode(LCD_ARDUINO_PIN, OUTPUT);
  analogWrite(LCD_ARDUINO_PIN, 90);

  randomSeed(analogRead(A5)); // seed for random function. it takes as input the noise from the analog pin A5
  startGame();

}

void loop()
{
  joyStickButtonState = digitalRead(JOYSTICK_BUTTON); // by pressing the button you switch from the start menu to the actual game
  if (gameSetup == 1)
  {
    if (joyStickButtonState == 0)
    {
      gameSetup = 0;
      displayScore();
    }
  }
  if (gameSetup == 0)
  {
    if ((millis() - lastUpdateScreen) >= intervalUpdateScreen) //delay function using millis
    {
      lastUpdateScreen = millis();
      updatePlayerPosition();
      matrixDisplay.clearDisplay(0);
      drawPlayer(playerPosition);

      updateApplesFall();
      generateApplesFall(1);
      drawApples();
      Serial.println(score);
    }
  }

}

void drawPlayer(int middlePoint)  // the function that draws the player
{
  matrixDisplay.setLed(MIN_LENGTH_MATRIX, middlePoint, MAX_LENGTH_MATRIX, 1); //AAAAAAAAAAAAAAAIIIIIIIIIIIIIIICCCCCCCCCCCCCCCCCCCIIIIIIIIIIIIIIIIIII
  matrixDisplay.setLed(MIN_LENGTH_MATRIX, middlePoint - 1, MAX_LENGTH_MATRIX, 1);
  matrixDisplay.setLed(MIN_LENGTH_MATRIX, middlePoint + 1, MAX_LENGTH_MATRIX, 1);
  matrixDisplay.setLed(MIN_LENGTH_MATRIX, middlePoint - 1, MAX_LENGTH_MATRIX, 1);
  matrixDisplay.setLed(MIN_LENGTH_MATRIX, middlePoint + 1, MAX_LENGTH_MATRIX, 1);
}

void updatePlayerPosition() // the function verifies if the joystick gets over the threshold and and moves it to the MARGINE of the matrix
{
  int xJoyStick = analogRead(JOYSTICK_Y);

  if (xJoyStick > THRESHOLD_JOYSTICK_MAXIM)
  {
    if (playerPosition > MIN_LENGTH_MATRIX + 1 )
    {
      playerPosition--;
    }
  }
  else if (xJoyStick < THRESHOLD_JOYSTICK_MINIM)
  {
    if (playerPosition < MAX_LENGTH_MATRIX - 1)
    {
      playerPosition++;
    }
  }
}

void drawApples()  // the function that draws the matrix
{
  for (int i = MIN_LENGTH_MATRIX; i <= MAX_LENGTH_MATRIX; i++)
    for (int j = MIN_LENGTH_MATRIX; j <= MAX_LENGTH_MATRIX; j++)
    {
      if (applesMatrix[i][j] == 1)
      {
        matrixDisplay.setLed (0, j, i, 1);

      }
    }
}

void generateApplesFall (int level) //function that generates the apples on the matrix
{
  if (millis() - lastApplesGeneration >= timeBetweenApplesRegeneration)
  {
    lastApplesGeneration = millis();
    long randomPosition = random(MIN_LENGTH_MATRIX , MAX_LENGTH_MATRIX + 1); // generate a random number

    applesMatrix[MIN_LENGTH_MATRIX][randomPosition] = 1; // put an apple on the first line on the random position that was previously generate
  }

}

void updateApplesFall ()
{
  for (int i = MAX_LENGTH_MATRIX; i > MIN_LENGTH_MATRIX; i--) // we go through the the collums from the bottom to the top copying the last line
    for (int j = MIN_LENGTH_MATRIX; j <= MAX_LENGTH_MATRIX; j++)
    {
      applesMatrix[i][j] = applesMatrix[i - 1][j];
      int x = playerPosition; // retain the player position into a variable
      if (applesMatrix[6][x - 1] == 1) // check the colision with the player and increase the score, also delete the apple that touched the player
      {
        score++ ;
        applesMatrix[6][x - 1] = 0;
        displayScore();

      }
      else if (applesMatrix[7][x] == 1)
      {
        score++;
        applesMatrix[7][x] = 0;
        displayScore();

      }
      else if (applesMatrix[6][x + 1] == 1)
      {
        score++;
        applesMatrix[6][x + 1] = 0;
        displayScore();

      }
      if (score == 5 && increaseSpeed == 0) // increase the diffuclty 
      {
        increaseSpeed = 1;
        timeBetweenApplesRegeneration -= 1500;
      }
      if (score == 15  && increaseSpeed == 1)
      {
        increaseSpeed = 2;
        timeBetweenApplesRegeneration /= 1.5;
      }
      if (score == 30 && increaseSpeed == 2)
      {
        increaseSpeed = 3;
        timeBetweenApplesRegeneration /= 2;
      }
      if (score == 45 && increaseSpeed == 3)
      {
        increaseSpeed = 4;
        timeBetweenApplesRegeneration /= 1.3;
      }
      for (int rowX = MIN_LENGTH_MATRIX ; rowX <= MAX_LENGTH_MATRIX ; rowX++) // verify if the apple reaches the last line without touching the middle point of the player [if that happens the game ends]
      {
        if (applesMatrix[7][rowX] == 1)
        {
          finishGame();
          displayScore();
          applesMatrix[7][rowX] = 0; //delete the apple matrix [as so the apple that made you lose the game will not remain on the matrix and make you lose the game again]
          for (int i = MIN_LENGTH_MATRIX ; i <= MAX_LENGTH_MATRIX; i++)
            for (int j = 0; j < 8; j++)
            {
              applesMatrix[i][j] = 0;
            }
        }
      }
    }
  for (int i = MIN_LENGTH_MATRIX; i <= MAX_LENGTH_MATRIX; i++)  //delete the apples from the line 0;
  {
    applesMatrix[0][i] = 0;
  }

}

void finishGame // ends the game and shows the ugly sad face
{
  matrixDisplay.clearDisplay(0);
  for (int i = MIN_LENGTH_MATRIX; i <= MAX_LENGTH_MATRIX; i++)
    for (int j = MIN_LENGTH_MATRIX; j <= MAX_LENGTH_MATRIX; j++)
    {
      if (sadFace[i][j] == 1)
      {
        matrixDisplay.setLed (0, j, i, 1);
      }
    }
  // redefine the start variables so the game wont start from the level you lost on [if i think better it could be a cool checkpoint mechanic (inner thoughts)]
  gameSetup = 1;
  playerPosition = 4;
  score = 0;
  intervalUpdateScreen = 230;
  timeBetweenApplesRegeneration = 3000;
  lastUpdateScreen = 0;
  lastApplesGeneration = 0;
  increaseSpeed = 0;

}
void startGame()
{
  for (int i = MIN_LENGTH_MATRIX; i <= MAX_LENGTH_MATRIX; i++)
    for (int j = MIN_LENGTH_MATRIX; j <= MAX_LENGTH_MATRIX; j++)
    {
      applesMatrix[i][j] = 0;
    }
  lcd.clear();
  matrixDisplay.clearDisplay(0);
  lcd.setCursor(2, 0);
  lcd.print ("Wanna start?");
  lcd.setCursor(0, 1);
  lcd.print ("Prees the button");
}

void displayScore() // displays score in real time
{
  lcd.clear();
  lcd.setCursor(0, 0);
  char firstPrintline[30] = "Score: ";
  char conversionVariable[3];
  itoa(score, conversionVariable, 10);
  strcat(firstPrintline, conversionVariable);
  lcd.print(firstPrintline);
  /*lcd.setCursor(0, 1);
    char secondPrintline[30] = "Highscore: ";
    char secondSonversionVariable[3];
    if (score > highScore)
    {
    highScore = score;
    }
    itoa(highScore, secondSonversionVariable, 10);
    strcat(secondPrintline, secondSonversionVariable);
    lcd.print(secondPrintline);*/
}

void displayHighScore() // displays highscore and stores it in the EEPROM memory
{
  lcd.setCursor(0, 1);
  lcd.print("HighScore: ");
  lcd.setCursor(12, 1);
  EEPROM_readAnything(0, highScore);
  lcd.print(highScore);
  if (score > highScore)
  {
    EEPROM_writeAnything(0, score);
  }
}
