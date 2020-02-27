#include "Internet.h"

void setup() {
  Serial.begin(115200);
  delay(1000);

  delay(1000);
  if (ConfigInternetConection()) {
    Serial.println("Conexion exitosa");
  } else {
    Serial.println("Sin IP, activando modo local");
  }
}





void loop() {
  static unsigned long tiempo = 0;
  if (  (  millis() - tiempo  ) > 11000 ) {
    tiempo = millis();
    EnviarGet("172.16.66.84", 1986, "/esp32?Datos=pinVida");
  }

  if (thereResponse()) {
    Serial.println(getResponseInternet());
  }
}
