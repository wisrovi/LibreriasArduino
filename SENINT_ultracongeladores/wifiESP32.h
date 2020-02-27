//https://www.instructables.com/id/ESP8266-and-ESP32-With-WiFiManager/

bool esperarDatoRecibidoWifi = false;

#include <Ticker.h>
Ticker TimeOutDetectionWifi;
void timeOutWifi() {
  esperarDatoRecibidoWifi = false;
  Serial.println("[WIFI]: No se recibio respuesta del envio GET");
}


#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager


#include <HTTPClient.h>
HTTPClient http;
bool modoFuncionamientoWifi = true;


#include <Hash.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
AsyncWebServer server(80);
bool useOTA = false;



bool configWifiESP32(String ssid, bool withOTA) {
  char ssid_AP[ssid.length() + 1];
  ssid.toCharArray(ssid_AP, ssid.length() + 1);

  WiFiManager wifiManager;
  wifiManager.setTimeout(60);//in seconds
  //wifiManager.resetSettings();Serial.println("Borrando credenciales...");wifiManager.setBreakAfterConfig(true);delay(2000);
  bool getIP_DHCP = true;
  if (!wifiManager.autoConnect(ssid_AP)) {
    if (!wifiManager.startConfigPortal(ssid_AP)) {
      /*delay(3000);
        ESP.restart();
        delay(5000);*/
      modoFuncionamientoWifi = false;
      getIP_DHCP = false;
    }
  }

  /*if (getIP_DHCP) {
    Serial.print("DHCP assigned IP ");
    Serial.println(WiFi.localIP());
  }*/

  if(withOTA){
    AsyncElegantOTA.begin(&server);    // Start ElegantOTA
    server.begin();
    useOTA = true;
  }

  return getIP_DHCP;
}

void loopWifiESP(){
  if(useOTA){
    AsyncElegantOTA.loop();
  }
}

String getIP_wifi(){
  return WiFi.localIP().toString();
}

bool sendGetWifi(String urlSend) {
  if (modoFuncionamientoWifi) {
    http.end();
    if (http.begin(urlSend)) { //HTTP
      TimeOutDetectionWifi.attach(10, timeOutWifi);
      esperarDatoRecibidoWifi = true;
      return true;
    } else {
      return false;
    }
  }
}

String datoRTAWifi = "";
bool RecibirRespuestaWifi() {
  bool rta = false;
  if (modoFuncionamientoWifi) {
    if (esperarDatoRecibidoWifi) {      
      int httpCode = http.GET();
      if (httpCode > 0  ) {
        esperarDatoRecibidoWifi = false;
        if (httpCode == HTTP_CODE_OK) {
          datoRTAWifi = http.getString();
          rta = true;
          TimeOutDetectionWifi.detach();
        }
      }
    }
  }
  return rta;
}

String getDatosWifi() {
  if (modoFuncionamientoWifi) {
    return datoRTAWifi;
  } else {
    return "";
  }
}
