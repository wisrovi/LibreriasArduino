///https://esp8266hints.wordpress.com/2019/04/05/esp32-w5500-simple-working-example/
//https://github.com/PuceBaboon/ESP32_W5500_NTP_CLIENT
//https://github.com/csquared/arduino-restclient

/*
   W5500 "hardware" MAC address.
*/


bool modoFuncionamientoEth = true;
#include <Ticker.h>
Ticker TimeOutDetectionEth;
void timeOutEthernet() {
  Serial.println("[ETHERNET]: No se recibio respuesta del envio GET");
}

#include <SPI.h>
#include <Ethernet.h>
#include <RestClient.h>
EthernetClient client;
byte error_Ethernet = 0;
String datoRTAEth = "";
int statusCode;
bool esperarDatoRecibidoEth = false;


#define RESET_P  26        // Tie the Wiz820io/W5500 reset pin to ESP32 GPIO26 pin.
void WizReset() {
  /*
      Wiz W5500 reset function.  Change this for the specific reset
      sequence required for your particular board or module.
  */
  Serial.print("[ETHERNET]: Resetting Wiz W5500 Ethernet Board...  ");
  pinMode(RESET_P, OUTPUT);
  digitalWrite(RESET_P, HIGH);
  delay(250);
  digitalWrite(RESET_P, LOW);
  delay(50);
  digitalWrite(RESET_P, HIGH);
  delay(350);
  Serial.println("Done.");
}

bool configEthernetESP32(uint8_t* eth_MAC) {
  Ethernet.init(5);           // GPIO5 on the ESP32.   // Use Ethernet.init(pin) to configure the CS pin.
  WizReset();

  Serial.println("[ETHERNET]: Starting ETHERNET connection...");
  if (Ethernet.begin(eth_MAC) == 0) {
    Serial.println("[ETHERNET]: Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      error_Ethernet = 1;
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      error_Ethernet = 2;
    }
    modoFuncionamientoEth = false;
    return false;
  } else {
    /*Serial.print("DHCP assigned IP ");
      Serial.println(Ethernet.localIP());*/
    return true;
  }
}

String getIP_eth() {
  return Ethernet.localIP().toString();
}

String getErrorEthernet() {
  if (!modoFuncionamientoEth) {
    if (error_Ethernet == 1) {
      return "Ethernet shield was not found.  Sorry, can't run without hardware. :(";
    } else {
      if (error_Ethernet == 2) {
        return "Ethernet cable is not connected.";
      } else {
        return "NULL";
      }
    }
  }
}

void loopEthernet() {
  if (modoFuncionamientoEth) {
    // You only need to call maintain if you're using DHCP.
    switch (Ethernet.maintain()) {
      case 1:
        //renewed fail
        Serial.println("[ETHERNET]: Error: renewed fail");
        break;

      case 2:
        //renewed success
        Serial.println("[ETHERNET]: Renewed success");
        //print your local IP address:
        /*Serial.print("My IP address: ");
          Serial.println(Ethernet.localIP());*/
        break;

      case 3:
        //rebind fail
        Serial.println("[ETHERNET]: Error: rebind fail");
        break;

      case 4:
        //rebind success
        Serial.println("[ETHERNET]: Rebind success");
        //print your local IP address:
        /*Serial.print("My IP address: ");
          Serial.println(Ethernet.localIP());*/
        break;

      default:
        //nothing happened
        break;
    }
  }
}

bool usarOpcion1_enviarGET = false;
bool sendGetEth(String url, int puerto, String solicitudGet) {
  if (modoFuncionamientoEth) {
    esperarDatoRecibidoEth = true;
    if (usarOpcion1_enviarGET) {
      if (!client.connected()) {
        client.stop();
      }
      char server[url.length() + 1];
      url.toCharArray(server, url.length() + 1);

      if (client.connect(server, 1986)) {
        String g = "GET ";
        g.concat(solicitudGet);
        g.concat(" HTTP/1.1");
        //Serial.println(g);
        client.println(g);
        client.println("Host: www.fcv.org");
        client.println("Connection: close");
        client.println();
        TimeOutDetectionEth.attach(10, timeOutEthernet);
        return true;
      }
    } else {
      datoRTAEth = "";
      char urlGet[url.length() + 1];
      url.toCharArray(urlGet, url.length() + 1);
      RestClient client = RestClient(urlGet, puerto);
      client.dhcp();
      client.setHeader("Host: www.fcv.org");
      client.setHeader("Connection: close");

      char peticionesGet[solicitudGet.length() + 1];
      solicitudGet.toCharArray(peticionesGet, solicitudGet.length() + 1);
      statusCode = client.get(peticionesGet, &datoRTAEth);
      TimeOutDetectionEth.attach(10, timeOutEthernet);

      if (statusCode == 200) {
        return true;
      }
    }
  }
  return false;
}


bool RecibirRespuestaEthernet() {
  bool rta = false;
  if (modoFuncionamientoEth) {
    if (usarOpcion1_enviarGET) {
      int len = client.available();
      if (len > 0) {
        byte buffer[120];
        if (len > 120) len = 120;
        client.read(buffer, len);

        String linea = "";
        for (byte i; i <= len; i++) {
          linea.concat(char(buffer[i]));
        }
        byte index = linea.indexOf("GMT");
        if (index >= 0) {
          datoRTAEth = linea.substring(index + 3);
          if (datoRTAEth.length() > 0) {
            rta = true;
            TimeOutDetectionEth.detach();
          }
        }
      }
    } else {
      if (esperarDatoRecibidoEth) {
        if (statusCode == 200) {
          rta = true;
          statusCode = 0;
          TimeOutDetectionEth.detach();
        }
      }
    }
  }

  return rta;
}

String getDatosEth() {
  if (modoFuncionamientoEth) {
    datoRTAEth.replace("\n", "");
    datoRTAEth.replace("\r", "");
    datoRTAEth.replace("\t", "");
    esperarDatoRecibidoEth = false;
    return datoRTAEth;
  } else {
    return "";
  }
}
