#include <arduino.h>
extern const float distance_to_center = 11.55;
extern float x;
extern float y;
extern float distance;
extern float angle;
extern int menu;
extern bool lcdMessageDisplayed;
#define SHARP_IR A0

int mesure_distance(void) {
  int sensorValue = analogRead(SHARP_IR);
  float voltage = sensorValue * (5.0 / 1024);      // Convertion en voltage
  distance = 27.108 * pow(voltage, -1.173);  // Calcule de distance en cm

  distance = distance_to_center - distance;        //la distance d = distance du centre depuis le capteur - la distance mesuré
  y = (cos(angle) * distance);
  x = (sin(angle) * distance);

  return distance, y, x;
}

// Fonction d'incrémentation de la variable
void incrementVariable(void) {
  menu++;
  lcdMessageDisplayed = false; 
}

// Fonction de décrémentation de la variable
void decrementVariable(void) {
  menu--;
  lcdMessageDisplayed = false;
}