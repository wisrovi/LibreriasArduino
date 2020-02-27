bool modoEthernet = true;
bool modoWifi = true;
bool usarOTA = true;

#include "wifiESP32.h"
#include "ethernetESP32.h"

bool ConfigInternetConection() {
  if (modoEthernet) {
    uint8_t eth_MAC[] = { 0x02, 0xF0, 0x0D, 0xBE, 0xEF, 0x01 };
    if (!configEthernetESP32(eth_MAC)) {
      Serial.println(getErrorEthernet());
      modoEthernet = false;
    } else {
      Serial.print("DHCP assigned IP BY ETHERNET: ");
      Serial.println(getIP_eth());
    }
  }
  if (!modoEthernet) {
    Serial.println("No se inicio el eternet, intentar con el wifi");
    if (!configWifiESP32("SENINT_ultracongeladores", usarOTA)) {
      modoWifi = false;
    } else {
      Serial.print("DHCP assigned IP BY WIFI: ");
      Serial.println(getIP_wifi());
    }
  }

  return modoEthernet || modoWifi;
}



void EnviarGet(String url, int puerto, String solicitudGet) {
  //Serial.println("Enviando dato...");
  if (modoEthernet) {
    bool statusSend = sendGetEth(url, puerto, solicitudGet);
    if (statusSend) {
      Serial.println("Paquete enviado");
    } else {
      Serial.println("Fallo enviar paquete");
    }
  } else {
    if (modoWifi) {
      String urlSend = "http://";
      urlSend.concat(url);
      urlSend.concat(":");
      urlSend.concat(String(puerto));
      urlSend.concat(solicitudGet);
      bool statusSend = sendGetWifi(urlSend);
      if (statusSend) {
        Serial.println("Paquete enviado");
      } else {
        Serial.println("Fallo enviar paquete");
      }
    } else {
      Serial.println("hola mundo");
    }
  }
}

String responseInternet = "";
bool thereResponse() {
  if (modoEthernet) {
    loopEthernet();
    if (RecibirRespuestaEthernet()) {
      responseInternet = getDatosEth();
      return true;
    }
  } else {
    if (modoWifi) {
      loopWifiESP();
      if (RecibirRespuestaWifi()) {
        responseInternet = getDatosWifi();
        return true;
      }
    }
  }
  return false;
}


String getResponseInternet() {
  return responseInternet;
}
