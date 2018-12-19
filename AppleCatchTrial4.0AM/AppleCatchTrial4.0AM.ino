#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"

#define JOYSTICK_Y A0
#define JOYSTICK_BUTTON 13
#define THRESHOLD_JOYSTICK_MINIM 340
#define THRESHOLD_JOYSTICK_MAXIM 680

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

int playerPosition = 4; // pozitia din centru a cosului
int score = 0;
int highScore;
int joyStickButtonState;
int intervalUpdateScreen = 230;
int timeBetweenApplesRegeneration = 3000; // timp intre regenerarea merelor / timpul intre care incep sa cada merele
bool gameSetup = 1; // variabila pentru meniul jocului
unsigned long lastUpdateScreen = 0;
unsigned long lastApplesGeneration = 0; //ultima updatare a generarii merelor
int increaseSpeed = 0; // varibila de verifica pentru marirea vitezei de generare a merelor, altfel ar cadea un mar foarte repede la schimbare
//int configuration = 0;

int applesMatrix[][8] = // matrice goala pe care retinem merele
{
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0}
};

int sadFace[][8] = // matricea celei mai urate fete triste
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
  matrixDisplay.setIntensity(0, 15); // intensitatea matricii
  matrixDisplay.clearDisplay(0);

  pinMode(JOYSTICK_Y, INPUT);
  EEPROM_readAnything(0, highScore);

  lcd.begin (16, 2);
  lcd.clear();
  lcd.setCursor (1, 0);


  pinMode(JOYSTICK_BUTTON, INPUT_PULLUP); // setat buttonul ca input default pe 1

  pinMode(LCD_ARDUINO_PIN, OUTPUT);
  analogWrite(LCD_ARDUINO_PIN, 90);

  randomSeed(analogRead(A5)); // seed-ul pentru functia random care v-a fi dat de zgomotul de pe pinul analog A5
  //EEPROM_readAnything(0, configuration);
  Serial.begin(9600);

  startGame();

}

void loop()
{
  joyStickButtonState = digitalRead(JOYSTICK_BUTTON); // prin apasarea butonului se trece de la meniul de intrare la joc
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
    if ((millis() - lastUpdateScreen) >= intervalUpdateScreen) //functia de delay cu millis
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

void drawPlayer(int middlePoint)  // functia de desenare a jucatorului
{
  //x e punctul din mijloc al jucatorului

  matrixDisplay.setLed(0, middlePoint, 7, 1);
  matrixDisplay.setLed(0, middlePoint - 1, 7, 1);
  matrixDisplay.setLed(0, middlePoint + 1, 7, 1);
  matrixDisplay.setLed(0, middlePoint - 1, 6, 1);
  matrixDisplay.setLed(0, middlePoint + 1, 6, 1);
}

void updatePlayerPosition() //verifica daca joystick-ul trece de prag si ajunge in margine si muta jucatorul
{
  int xJoyStick = analogRead(JOYSTICK_Y);

  if (xJoyStick > THRESHOLD_JOYSTICK_MAXIM)
  {
    if (playerPosition > 1)
    {
      playerPosition--;
    }
  }
  else if (xJoyStick < THRESHOLD_JOYSTICK_MINIM)
  {
    if (playerPosition < 6)
    {
      playerPosition++;
    }
  }
}

void drawApples()  // functia de desenare a matricei
{
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
    {
      if (applesMatrix[i][j] == 1)
      {
        matrixDisplay.setLed (0, j, i, 1);

      }
    }
}

void generateApplesFall (int level) //functia de generare a merelor pe matrice
{
  if (millis() - lastApplesGeneration >= timeBetweenApplesRegeneration)
  {
    lastApplesGeneration = millis();
    long randomPosition = random(0, 8); // generam numar random

    applesMatrix[0][randomPosition] = 1; // punem pe prima linie un mar la pozitia generata random
  }

}

void updateApplesFall ()
{
  for (int i = 7; i > 0; i--) // mergem pe linii de jos in sus si copiem linia precedenta
    for (int j = 0; j <= 7; j++)
    {
      applesMatrix[i][j] = applesMatrix[i - 1][j];
      int x = playerPosition; // initializez pozitia player-ului intr-o variabila
      if (applesMatrix[6][x - 1] == 1) // verific coliziunea cu player-ul, cresc scorul in caz afirmativ si sterg punctul care a atins player-ul pentru a nu ajunge pe ultima linie
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
      if (score == 5 && increaseSpeed == 0)   // cresc nivelul dificultatii in functie de numarul de puncte
      { // variabila increaseSpeed opreste primul mar dupa schimbarea vitezei sa cada foarte repede
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
        timeBetweenApplesRegeneration /= 2;
      }
      for (int rowX = 0; rowX <= 7; rowX++) //Verific daca marul ajunge pe ultima linie fara sa aiba contact cu punctul din mijloc al player-ului, iar in caz afirmativ se incheie jocul
      {
        if (applesMatrix[7][rowX] == 1)
        {
          finishGame();
          displayScore();
          applesMatrix[7][rowX] = 0; //ar ramane un mar generat random la fiecare reset de joc
          for (int i = 0; i < 8; i++)
            for (int j = 0; j < 8; j++)
            {
              applesMatrix[i][j] = 0;
            }
        }
      }

      /* else if (applesMatrix[7][0] == 1)  // versiunea cruda, fara for
         {
           score = 0;
         }
         else if (applesMatrix[7][1] == 1)
         {
           score = 0;
         }
         else if (applesMatrix[7][2] == 1)
         {
           score = 0;
         }
         else if (applesMatrix[7][2] == 1)
         {
           score = 0;
         }
         else if (applesMatrix[7][3] == 1)
         {
           score = 0;
         }
         else if (applesMatrix[7][4] == 1)
         {
           score = 0;
         }
         else if (applesMatrix[7][5] == 1)
         {
           score = 0;
         }
         else if (applesMatrix[7][6] == 1)
         {
           score = 0;
         }
         else if (applesMatrix[7][7] == 1)
         {
           score = 0;
         }*/
    }
  for (int i = 0; i <= 7; i++)  // stergem merele de pe linia 0;
  {
    applesMatrix[0][i] = 0;
  }

}

void finishGame()  // terminam jocul si afisam fata trista
{
  matrixDisplay.clearDisplay(0);
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
    {
      if (sadFace[i][j] == 1)
      {
        matrixDisplay.setLed (0, j, i, 1);
      }
    }
  // redefinim varibilele care se modifica cu valorile initiale
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
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
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

void displayScore() //functia pentru tinerea scorului in timp real
{
  lcd.clear();
  lcd.setCursor(0, 0);
  char firstPrintline[30] = "Score: ";
  char conversionVariable[3];
  itoa(score, conversionVariable, 10);
  strcat(firstPrintline, conversionVariable);
  lcd.print(firstPrintline);
  lcd.setCursor(0, 1);
  char secondPrintline[30] = "Highscore: ";
  char secondSonversionVariable[3];
  if (score > highScore)
  {
    highScore = score;
  }
  itoa(highScore, secondSonversionVariable, 10);
  strcat(secondPrintline, secondSonversionVariable);
  lcd.print(secondPrintline);
}

void displayHighScore()
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
