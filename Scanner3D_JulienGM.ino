//--------------------------------------------------------------------
//  Programme : Scanner_3D_TFE.ino
//  Auteur : Julien GOMEZ MELERO 6éme Electronique INRACI
//  µP : Arduino UNO
//  Date : 05/04/2023
//  Modues : ECRAN LCD I2C,CAPTEUR IR,LECTEUR CARTE µSD,MOTEURS PAS A PAS
//-----------------------DEFINITION DES MODULES------------------------
#include "Mes_fonctions.h"
//--------------------------ECRAN LCD I2C-----------------------------
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
bool lcdMessageDisplayed = false;
//----------------------------CAPTEUR IR------------------------------
#define SHARP_IR A0
const float distance_to_center = 11.55;
float distance = 0;
//----------------------------CCAPTEUR IR-------------------------------
#include <SPI.h>
#include <SD.h>
#define SD_CS 53
File MonFichier;
//-------------------------MOTTEUR PAS A PAS--------------------------
#include <Arduino.h>
#include "DRV8825.h"

#define MOTOR_STEPS 800
#define RPM_Z 10
#define RPM_PLT 10
#define STEPS 4

#define enPin_Z 6
#define DIR_PIN_Z 3
#define STEP_PIN_Z 5

#define MS1_PIN_Z 17
#define MS2_PIN_Z 18
#define MS3_PIN_Z 19

#define enPin_PLT 9
#define DIR_PIN_PLT 7
#define STEP_PIN_PLT 8

#define MS1_PIN_PLT 17
#define MS2_PIN_PLT 18
#define MS3_PIN_PLT 19

DRV8825 motteur_Z(MOTOR_STEPS, DIR_PIN_Z, STEP_PIN_Z, MS1_PIN_Z, MS2_PIN_Z, MS3_PIN_Z);
DRV8825 motteur_PLT(MOTOR_STEPS, DIR_PIN_PLT, STEP_PIN_PLT, MS1_PIN_PLT, MS2_PIN_PLT, MS3_PIN_PLT);

float angle = (1.8 * 1 / 4);
// 1    --> 1,8°
// 1/2  --> 1,8°*1/2
// 1/4  --> 1,8°*1/4
// 1/8  --> 1,8°*1/8
// 1/16 --> 1,8°*1/16

#define LED1 2
#define BP_1 0   // UP
#define BP_2 22  // OK
#define BP_3 23  // RETURN
#define BP_4 10  // DOWN
#define FIN_DE_COURSE 4
//bool ref_BP_1 = 0, ref_BP_2 = 0, ref_BP_3 = 0, ref_BP_4 = 0;
int menu = 0;
#define Nmbr_mesureX 800
#define Nmbr_mesureY 800
#define Nmbr_mesureZ 65
float MesureX[Nmbr_mesureX];
float MesureY[Nmbr_mesureY];
float MesureZ[Nmbr_mesureZ];
float x = 0;
float y = 0;
float z = 0;
void setup() {
  Serial.begin(9600);
  pinMode(LED1, OUTPUT);
  pinMode(SD_CS, OUTPUT);
  pinMode(FIN_DE_COURSE, INPUT_PULLUP);
  pinMode(BP_1, INPUT_PULLUP);
  pinMode(BP_2, INPUT_PULLUP);
  pinMode(BP_3, INPUT_PULLUP);
  pinMode(BP_4, INPUT_PULLUP);
  //------------------INITIALISATION DE L'ECRAN-----------------------
  lcd.begin();
  lcd.backlight();  // Allume le rétroéclairage
  //------------------INITIALISATION DU MOTEUR AXE Z------------------
  motteur_Z.begin(RPM_Z, STEPS);
  motteur_Z.enable();
  //-----------------INITIALISATION DU MOTEUR PLATEAU-----------------
  motteur_PLT.begin(RPM_PLT, STEPS);
  motteur_PLT.enable();
  //------------------INITIALISATION DU LECTEUR µSD-------------------
  lcd.clear();
  lcd.setCursor(0, 0);  // Définit le curseur à la position (0, 0)
  lcd.print("Initialising SD card...");
  delay(3000);
  while (!SD.begin(SD_CS)) {  // Véification de l'initialisation de la carte µSD
    lcd.setCursor(0, 2);      // Définit le curseur à la position (0, 2)
    lcd.print("initialisation fail!");
  }
  lcd.clear();
  MonFichier = SD.open("test.txt", FILE_WRITE);  // Ouverture et ecriture du fichier
  lcd.print("initialisation done.");
  delay(500);
}

//-------------------------------------------------------------------
// LOOP    LOOP    LOOP    LOOP    LOOP    LOOP    LOOP    LOOP
//-------------------------------------------------------------------
void loop() {

  if (digitalRead(BP_1) == LOW && (menu >= 0) && (menu <= 2)) {
    incrementVariable();
    delay(500);  // Délai pour éviter les rebonds
  }

  if (digitalRead(BP_4) == LOW && (menu >= 1) && (menu <= 3)) {
    decrementVariable();
    delay(500);  // Délai pour éviter les rebonds
  }


  switch (menu) {
    case 0:
      if (!lcdMessageDisplayed) {  // verifie si le texte a été afficher
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("0-menu-0");
        lcd.setCursor(0, 1);
        lcd.print("SCAN3D");
        lcd.setCursor(12, 3);  // Définit le curseur à la position (12, 3)
        lcd.print("Start?");
        lcdMessageDisplayed = true;  //message afficher
      }

      if (!digitalRead(BP_3)) {
        lcd.clear();
        lcd.setCursor(0, 0);  // Définit le curseur à la position (0, 0)
        lcd.print("Descente en position zero ...");

        //while (digitalRead(BP_1) == HIGH) {
        //  motteur_Z.rotate(-360);
        //  Serial.println("étape 3");}
        
        motteur_Z.stop();
        motteur_Z.disable();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("moteur stop");  // Affiche le message "moteur stop"
        digitalWrite(LED1, HIGH);
        lcd.clear();

        for (int niveau = 0; niveau < Nmbr_mesureZ; niveau++) {
          for (int i = 0; i < Nmbr_mesureX; i = i + 1) {
            mesure_distance();
            delay(100);

            MesureX[i] = x;
            MesureY[i] = y;
            motteur_PLT.move(1);  //move --> commande en pas
            lcd.setCursor(4, 3);  // Définit le curseur à la position (5, 3)
            lcd.print("Distance :");
            lcd.print(distance);
            delay(1000);

            lcd.clear();
            lcd.setCursor(0, 0);  // Définit le curseur à la position (0, 0)
            lcd.print("Niveau : ");
            lcd.print(niveau + 1);
            lcd.setCursor(0, 2);  // Définit le curseur à la position (0, 2)
            lcd.print("Mesure : ");
            lcd.print(i + 1);
          }
          //motteur_Z.enable();
          motteur_Z.rotate(45);  //rotate --> commande en angle
          motteur_Z.stop();
          //motteur_Z.disable();
          z = z + 0.8;  // un tour complet équivaut a 8mm pour la tige sans fin
          MesureZ[niveau] = z;
        }
        digitalWrite(LED1, LOW);

        for (int niveau = 0; niveau < Nmbr_mesureZ; niveau++) {
          for (int i = 0; i < Nmbr_mesureX; i = i + 1) {
            MonFichier.print(MesureX[i]);
            MonFichier.print(",");
            MonFichier.print(MesureY[i]);
            MonFichier.print(",");
            MonFichier.println(MesureZ[niveau]);
          }
        }
        MonFichier.close();
      }
      break;
    case 1:
      if (!lcdMessageDisplayed) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("1-menu-1");
        lcdMessageDisplayed = true;
        //Fonctionnalité a prévoir
      }
      break;

    case 2:
      if (!lcdMessageDisplayed) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("2-menu-2");
        lcdMessageDisplayed = true;
        //Fonctionnalité a prévoir
      }
      break;

    case 3:
      if (!lcdMessageDisplayed) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("3-menu-3");
        lcdMessageDisplayed = true;
        //Fonctionnalité a prévoir
      }
      break;
  }
}