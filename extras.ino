
/* presiono(): lee el estado de un pulsador, evitando rebotes.
 *  
 * Devuelve true si se presionó el pulsador.
 * 
 * pin: Entrada del Arduino conectada al pulsador.
 */
bool presiono(uint8_t pin) {
  
  const uint8_t PRESIONADO = 0;    // Valor que devuelve al presionar el pulsador
  
  if (digitalRead(pin) == PRESIONADO) {
    
    // Evito rebotes del pulsador:
    delay(10);
    while (digitalRead(pin) == PRESIONADO) {
      delay(10);
    }
    return true;
  } 
  else {
    return false;
  }
}


/** beep(): produce beeps de la duración indicada
 * 
 * cantidad: numero de beeps (1..255) 
 *
 * tiempo: duración de cada beep, en ms (0..65000) 
 *
 * volumen: puede usarse para reducir el volumen o desactivalo. (0..10) 
 */
void beep(uint8_t cantidad, int tiempo, uint8_t volumen) {
  
  if (volumen > 0) {

    long t_on = tiempo / (11 - volumen) / 2;
	  long t_off = tiempo - t_on;
	  
	  for (byte beeps = 0; beeps < cantidad; beeps++) {
  		digitalWrite(BUZZER, HIGH);
  		delay(t_on);
  		digitalWrite(BUZZER, LOW);
  		delay(t_off);
	  }
	}
}


/* battery_level(): devuelve el nivel de tensión de la batería en milivolts.
 * 
 * battery_pin: entrada análógica del Arduino conectada al positivo de la batería.
 * Se debe conectar mediante un divisor de tensión de la siguiente forma:
 * 
 *    ┌─────[ R1 ]────┬───────┬──────◊ battery_pin
 *    │               │       │
 *  ─────             │       │ 
 *   ───             |R|     ─┴─ 
 *  ─────            |2|     ─┬─ C1
 *   ───              │       │
 *    │               │       │
 *    └───────────────┴───────┘
 *    
 *  R1 = 6.8K   R2 = 3.3K
 *  Vmax de la batería = 15V.
 *  C1 = 100nF (Opcional, utilizar si hay picos de consumo al utilizar la función)
 *
*/
unsigned int battery_level(uint8_t battery_pin) {

                     // R2    R1     R2
  const int MAX_mV = ((3.3 + 6.8) / 3.3 * 5000);     // Tensión máxima que puede medir
  
  unsigned int vbat = map (analogRead(battery_pin), 0, 1023, 0, MAX_mV);

  return vbat;
  
}


/* battery_check(): indicación visual y sonora del nivel de de la batería. 
 *
 * Utiliza la función battery_level() para medir la tensión (ver conexión). 
 * Sirve para medir el estado de baterías de litio (tipo 18650 o lipo) de 1 a 
 * 3 celdas (3.7-11.1V)
 * 
 * celdas: número de celdas de la batería lipo (1, 2 o 3).
 * 
 * battery_pin: entrada análógica del Arduino conectada al positivo de la 
 * batería. Ver conexión en la función battery_level().
 * 
 * led1: LED indicador de batería baja (V < 3.6V)
 * led2: LED indicador de batería media (3.6V < V < 4V)
 * led3: LED indicador de batería cargada (V > 4V)
 * buzzer: buzzer para emitir aviso sonoro. Se puede reemplazar por un LED
 * 
 * Si la batería se encuentra por debajo de 3.3V se bloquea el programa y marca
 * continuamente batería baja + pitidos.
 *
*/
void battery_check(uint8_t celdas, uint8_t battery_pin, uint8_t led1, 
                   uint8_t led2, uint8_t led3, uint8_t buzzer) {

  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);  
  delay(100);

  uint16_t vbat = battery_level(battery_pin) / celdas;

  if (vbat <  3.30) {        // muy bajo!!!
    while(1) {
      digitalWrite(led1, HIGH);
      digitalWrite(BUZZER, HIGH);
      delay(500);
      digitalWrite(led1, LOW);
      digitalWrite(BUZZER, LOW);
      delay(1000);
    }
  } 
  else if (vbat < 3.60) {     // nivel bajo
      digitalWrite(led1, HIGH);
      digitalWrite(BUZZER, HIGH);
      delay(1000);
      digitalWrite(BUZZER, LOW);
      delay(1000);
  } 
  else if (vbat < 4.00) {     // nivel medio
      digitalWrite(led2, HIGH);
  } 
  else {                      // nivel alto
    digitalWrite(led3, HIGH);
  }
    
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
  delay(900);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
}

/**** TEST ***/
// Prueba LEDs y buzzer (2 destellos). Lee la distancia de los sensores y los muestra en el monitor serial.
void test(void) {

  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);


  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  delay(1000);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  delay(1000);

  Serial.begin(9600);
  int distancia_frontal = sonar_frontal.ping_cm();
  int distancia_derecha = sonar_derecho.ping_cm();

  while(1) {
    delay(500);                     // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
    distancia_frontal = sonar_frontal.ping_cm();
    distancia_derecha = sonar_derecho.ping_cm();
    Serial.print("Frontal: ");
    Serial.print(distancia_frontal); // Send ping, get distance in cm and print result (0 = outside set distance range)
    Serial.print("cm   Derecha: ");
    Serial.print(distancia_derecha); // Send ping, get distance in cm and print result (0 = outside set distance range)
    Serial.println("cm");
  }
}
