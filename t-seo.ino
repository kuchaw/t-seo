

#include <NewPing.h>
#include "DRV8833.h"               // Librería para controlar el driver de motores


/*** CONFIGURACIÓN ***/

// constantes de calibración:                 //             FUNCIONA:
const int SPEED = 135;                         // 0..255      SPEED = 85
const float KP = 11;                          // Máximo: ?   KP = 11
const float KD = 0.9;                           // Máximo: ?   KD = 1

const int MAX_CORRECTION = SPEED * 1.8;       // Limita la corrección máxima, debe ser como mucho SPEED * 2
const int CENTRO_DE_CELDA = (25 / 2) - 7.5;   // -5,5 original centro = 25cm/2 - distancia del sensor lateral desde el centro del robot

const int DISTANCIA_FRONTAL_MIN = 9;          // 7cm Distancia a la pared frontal, para evitar chocarla (en cm)
const int DELAY_GIRO_FRONTAL = 190;           // Tiempo que gira cuando detecta pared al frente (en ms). La idea es que gire 90º
const int DISTANCIA_IZQUIERDA_MIN = 7.4;
const int DISTANCIA_DERECHA_MIN = 7.4;

// Pines del Arduino utilizados
//const int PULSADOR1  =  0;
//const int PULSADOR2  =  1;

const int LED1 = 8;
const int LED2 = 12;
const int LED3 = 13;
const int BUZZER = 4;

// Configración sensores de distancia:
const int ULTRA1_TRIGGER = 7;
const int ULTRA1_ECHO = 6;
const int ULTRA2_TRIGGER = A1;
const int ULTRA2_ECHO = A0;
const int ULTRA3_TRIGGER = 3;
const int ULTRA3_ECHO = 2;

#define MAX_DISTANCE 27 // 50cm Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
NewPing sonar_derecho(ULTRA1_TRIGGER, ULTRA1_ECHO, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
NewPing sonar_frontal(ULTRA2_TRIGGER, ULTRA2_ECHO, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
NewPing sonar_izquierdo(ULTRA3_TRIGGER, ULTRA3_ECHO, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

// Configuración de motores:
DRV8833 motorIzq(5, 9);         // pines del Arduino que utiliza el motor izquierdo
DRV8833 motorDer(10, 11);        // pines del Arduino que utiliza el motor derecho



/*** GLOBAL VARIABLES ***/
bool robot_activado = false;
int ultima_posicion_lateral;

/**** SETUP ***/
void setup() {
  //pinMode(PULSADOR1, INPUT_PULLUP);
  //pinMode(PULSADOR2, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  motorIzq.begin();
  motorDer.begin();

  robot_activado = true;          // Forzar activación a falta de pulsador
 indicar_estado(robot_activado);

  Serial.begin(9600);
  //test();   // probar lectura de sensores con Monitor Serial. Bloquea el resto del programa!
}


/**** LOOP ***/
void loop() {

   
  /*
    if (presiono(PULSADOR1)) {
      robot_activado = !robot_activado;
      indicar_estado(robot_activado);
    }
  */
  if (robot_activado) {
    recorrer();
  }
  else {
    motorIzq.stop();
    motorDer.stop();
  }
}


/*** CORRECCION LATERAL ***/
// lee la posición sobre la regleta, calcula la corrección y la aplica a los motores
int correccion_lateral(int distancia_lateral) {

  // calculo la posición:
  int posicion_lateral = CENTRO_DE_CELDA - distancia_lateral;

  // calculo la corrección:
  int correccion = (KP * posicion_lateral) + (KD * (posicion_lateral - ultima_posicion_lateral));

  ultima_posicion_lateral = posicion_lateral;


  // Aplico la corrección:
  if (correccion < 0) {     // giro hacia la derecha
    digitalWrite(LED1, HIGH);
    correccion = max(correccion, -MAX_CORRECTION);
    motorIzq.speed(SPEED);
    motorDer.speed(SPEED + correccion);
    
  }
  else {                            // giro hacia la izquierda
    digitalWrite(LED3, HIGH);
    correccion = min(correccion, MAX_CORRECTION);
    motorIzq.speed(SPEED - correccion);
    motorDer.speed(SPEED);
    
  }

/*
Serial.print("correccion: ");
Serial.print(correccion); // Send ping, get distance in cm and print result (0 = outside set distance range)
Serial.print(" - posicion_lateral: ");
Serial.print(posicion_lateral); // Send ping, get distance in cm and print result (0 = outside set distance range)
Serial.println("cm");
*/

  return correccion_lateral;
}

/**** CORRER ***/
// lee la posición sobre la regleta, calcula la corrección y la aplica a los motores
void recorrer(void) {

  delay(10);                     // Wait between pings. 29ms should be the shortest delay between pings.
  int distancia_frontal = sonar_frontal.ping_cm();
  if (distancia_frontal == 0) distancia_frontal = MAX_DISTANCE;
  
  delay(10);                     // Wait between pings. 29ms should be the shortest delay between pings.
  int distancia_derecha = sonar_derecho.ping_cm();
  if (distancia_derecha == 0) distancia_derecha = MAX_DISTANCE;
  
  delay(10);                     // Wait between pings. 29ms should be the shortest delay between pings.
  int distancia_izquierda = sonar_izquierdo.ping_cm();
  if (distancia_izquierda == 0) distancia_izquierda = MAX_DISTANCE;


  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  

  if (distancia_frontal > DISTANCIA_FRONTAL_MIN) {       // SI TIENE LIBRE ADELANTE APLICA LA CORRECCION LATERAL 

 correccion_lateral(distancia_derecha); 
  digitalWrite(LED2, HIGH);
  }
 /*if (distancia_frontal < DISTANCIA_FRONTAL_MIN && distancia_izquierda > DISTANCIA_IZQUIERDA_MIN && distancia_derecha > DISTANCIA_DERECHA_MIN) // SI TIENE PARED EN FRENTE Y LIBRE A LOS COSTADOS, GIRA A LA DERECHA
  
    {
    motorIzq.speed(SPEED);                      //giro hacia la derecha
    motorDer.speed(-SPEED);

    digitalWrite(LED1, HIGH);
    beep(10, 25, 10);
    delay(DELAY_GIRO_FRONTAL);
     correccion_lateral(distancia_derecha);

 }
 */else if (distancia_frontal < DISTANCIA_FRONTAL_MIN && distancia_izquierda > DISTANCIA_IZQUIERDA_MIN ) // SI TIENE PARED EN FRENTE Y A LA DERECHA, GIRA A LA IZQUIERDA
  
   { 
    motorIzq.speed(-SPEED);                      //giro hacia la izquierda
    motorDer.speed(SPEED);

    digitalWrite(LED3, HIGH);
    beep(10, 25, 10);
    delay(DELAY_GIRO_FRONTAL);
  }
  else if (distancia_frontal < DISTANCIA_FRONTAL_MIN && distancia_derecha > DISTANCIA_DERECHA_MIN ) // SI TIENE PARED EN FRENTE Y A LA IZQUIERDA, GIRA A LA DERECHA
  
   { 
    motorIzq.speed(SPEED);                      //giro hacia la derecha
    motorDer.speed(-SPEED);

    digitalWrite(LED1, HIGH);
    beep(10, 25, 10);
    delay(DELAY_GIRO_FRONTAL);
  }
  //else if (distancia_)
  
  
  
  











}

 



/**** INDICAR ESTADO ***/
void indicar_estado(bool robot_activado) {

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);

  if (robot_activado == true) {
    digitalWrite(LED1, HIGH);
    beep(1, 100, 10);
    delay(100);         // Wait for that lazy hand to get out of the way
  }
  else {
    beep(3, 100, 10);
    delay(500);
    digitalWrite(LED2, HIGH);
  }
}
