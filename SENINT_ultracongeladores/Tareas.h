#define useWifi
#define useEth


#ifdef useWifi
bool thereConectionWifi = false;
#endif


#ifdef useEth
bool thereConectionEthernet = false;
#endif




/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *                                                                          *
                                 WIFI TASK
 *                                                                          *
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *****************************************************************************/

#ifdef useWifi
#define DEVICE_NAME "SENINT"
#define WIFI_TIMEOUT 20000 // 20 seconds



#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#define EspacioMemoriaWifi  7000

#include <Hash.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
AsyncWebServer server(80);

TaskHandle_t nucleo1WifiKeepAlive_Handle = NULL;
void keepWiFiAlive( void *pvParameters );

void ConfigWifiKeepAlive() {
  //Esta tarea se ejecuta en el nucleo 1, y se encargar de mantener la conexión wifi activa,
  //si pierde conexión, la reestablece nuevamente con las credenciales guardadas
  //siempre que haya conexion, esta libreria monta su actualización por OTA
  //cuando se pierde la conexión, la libreria reinicia los servicios asociados a la libreria para evitar que un servicio no vaya a funcionar

  xTaskCreatePinnedToCore(
    keepWiFiAlive,      // Function that should be called
    "keepWiFiAlive",    // Name of the task (for debugging)
    EspacioMemoriaWifi,         // Stack size (bytes),                             EL USADO Y FUNCIONAL: 8000
    NULL,         // Parameter to pass
    1 ,           // Task priority
    &nucleo1WifiKeepAlive_Handle,         // Task handle
    1             // Core you want to run the task on (0 or 1)
  );
}


void keepWiFiAlive(void * parameter) {
  Serial.print("[WIFI]: [Core 1]: "); Serial.println(xPortGetCoreID());

  int retardoConexionEstablecida = 0;
  bool debugWifiManager = true;
  bool haveIpOutFor = false;
  thereConectionWifi = false;

  WiFiManager wifiManager;
  wifiManager.setTimeout(300);
  wifiManager.setDebugOutput(debugWifiManager);
  wifiManager.autoConnect(DEVICE_NAME);

  if (WiFi.status() != WL_CONNECTED) {
    //Serial.println("[WIFI]: Iniciando AP para configuración...");
  }

  //Serial.println(wifiManager.getSSID());
  //Serial.println(wifiManager.getPassword());

  if (WiFi.status() == WL_CONNECTED) {
    haveIpOutFor = true;
  }
  String ssid = WiFi.SSID();
  String PSK = WiFi.psk();

  int conteoIntentosConexion = 0;
  for (;;) {
    if (WiFi.status() == WL_CONNECTED && haveIpOutFor == false) {
      bool thereConection = true;
      while (thereConection) {
        retardoConexionEstablecida++;
        if (retardoConexionEstablecida > 1000) {
          retardoConexionEstablecida = 0;
          thereConection = false;
        }
        vTaskDelay(15 / portTICK_PERIOD_MS);
        AsyncElegantOTA.loop();
      }
      Serial.print("[WIFI]: [Core 1]: "); Serial.println(xPortGetCoreID());
      conteoIntentosConexion = 0;
      thereConectionWifi = true;
      continue;
    } else {
      thereConectionWifi = false;
    }

    Serial.println("[WIFI] Connecting...");
    if (conteoIntentosConexion <= 2) {
      char ssid_char[ssid.length() + 1];
      ssid.toCharArray(ssid_char, ssid.length() + 1);

      char psk_char[PSK.length() + 1];
      PSK.toCharArray(psk_char, PSK.length() + 1);

      WiFi.mode(WIFI_STA);
      WiFi.setHostname(DEVICE_NAME);
      WiFi.begin(ssid_char, psk_char);
    } else {
      wifiManager.setTimeout(300);
      wifiManager.setDebugOutput(debugWifiManager);
      //wifiManager.autoConnect(DEVICE_NAME);
      wifiManager.startConfigPortal(DEVICE_NAME);
    }

    unsigned long startAttemptTime = millis();

    // Keep looping while we're not connected and haven't reached the timeout
    while (WiFi.status() != WL_CONNECTED &&
           millis() - startAttemptTime < WIFI_TIMEOUT) {}

    // If we couldn't connect within the timeout period, retry in 30 seconds.
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WIFI]: FAILED");
      conteoIntentosConexion++;
      vTaskDelay(30000 / portTICK_PERIOD_MS);
      continue;
    } else {
      conteoIntentosConexion = 0;
    }

    Serial.print("[WIFI]: Connected: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(200, "text/plain", "Version Libreria Internet: 1.0.");
    });

    AsyncElegantOTA.begin(&server);    // Start ElegantOTA
    server.begin();
    haveIpOutFor = false;
    Serial.println("[WIFI]: Activando OTA.");
  }
}

#endif









/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *                                                                          *
                                ETHERNET TASK
 *                                                                          *
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *****************************************************************************/

#ifdef useEth
#define ETH_TIMEOUT 20000 // 20 seconds


#include <WiFi.h>   //para obtener la MAC wifi
//#include <esp_wifi.h>    //aveces se requiere para obtener el ChipID
#include <SPI.h>
#include <Ethernet.h>
#define EspacioMemoriaEthernet  1500
#define RESET_P  26        // Tie the Wiz820io/W5500 reset pin to ESP32 GPIO26 pin.

TaskHandle_t nucleo1EthKeepAlive_Handle = NULL;
void keepEthAlive( void *pvParameters );

int ConvertChar(char c) {
  switch (c) {
    case '0':
      return 0;
    case '1':
      return 1;
    case '2':
      return 2;
    case '3':
      return 3;
    case '4':
      return 4;
    case '5':
      return 5;
    case '6':
      return 6;
    case '7':
      return 7;
    case '8':
      return 8;
    case '9':
      return 9;
    case 'A':
      return 10;
    case 'B':
      return 11;
    case 'C':
      return 12;
    case 'D':
      return 13;
    case 'E':
      return 14;
    case 'F':
      return 15;
  }
  return -1;
}

uint8_t ConvertChartoMac(char h1, char h0) {
  h1 = ConvertChar(h1);
  h0 = ConvertChar(h0);
  return int(h1) * 16 + h0;
}

void ConfigEthernetKeepAlive() {
  xTaskCreatePinnedToCore(
    keepEthAlive,      // Function that should be called
    "keepEthAlive",    // Name of the task (for debugging)
    EspacioMemoriaEthernet,         // Stack size (bytes)
    NULL,         // Parameter to pass
    1 ,           // Task priority
    &nucleo1EthKeepAlive_Handle,         // Task handle
    1             // Core you want to run the task on (0 or 1)
  );
}

void keepEthAlive(void * parameter) {
  //Serial.print("[ETHERNET]: [Core 1]: "); Serial.println(xPortGetCoreID());

  thereConectionEthernet = false;

  Ethernet.init(5);           // GPIO5 on the ESP32.   // Use Ethernet.init(pin) to configure the CS pin.

  { //WizReset
    /**
        Reset Ethernet: Wiz W5500 reset function.
    **/
    Serial.print("[ETHERNET]: Resetting Wiz W5500 Ethernet Board...  ");
    pinMode(RESET_P, OUTPUT);
    digitalWrite(RESET_P, HIGH);
    vTaskDelay(250 / portTICK_PERIOD_MS);
    digitalWrite(RESET_P, LOW);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    digitalWrite(RESET_P, HIGH);
    vTaskDelay(350 / portTICK_PERIOD_MS);
    //Serial.println("Done.");
  }
  /****************************************/

  uint8_t eth_MAC[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  { //Asignar MAC para el modulo Ethernet
    bool GenerarMacAutomatica = true;
    /*
      Para asignar la MAC que se usará en el modulo Ethernet, tomo los primeros 3 hexetos de la mac wifi,
      estos tres primeros hexetos me definen el fabricante del producto, por lo cual uso el mismo fabricante del ESP para generar la MAC Ethernet

      Los siguientes tres hexetos finales los defino usando el chipID del ESP, este ID es un codigo unico donde está el serial del dispositivo, contiene entre 7 u 8 indices
      para lo cual divido en dos caracteres iniciando por las unidades, es decir,
      decenas y unidades             corresponden a el hexeto 6
      milesimas y centenas           corresponden a el hexeto 5
      centenar_de_mil y diezmilesima corresponden a el hexeto 4

      por ejemplo:
        MAC    = AA:BB:CC:DD:EE:FF
        ChipID = 76543210

        Quedaría:
        MAC_Ethernet = AA:BB:CC:54:32:10
    */
    if (GenerarMacAutomatica) {
      String mac_string = WiFi.macAddress();

      //String chipID_string = String(ESP.getChipId());
      String chipID_string = String((uint32_t)ESP.getEfuseMac());
      byte sizeChipID = chipID_string.length();
      char chipID_char[sizeChipID + 1];
      chipID_string.toCharArray(chipID_char, sizeChipID + 1);

      char mac_char[mac_string.length() + 1];
      mac_string.toCharArray(mac_char, mac_string.length() + 1);

      eth_MAC[0] = ConvertChartoMac(mac_char[0], mac_char[1]);
      eth_MAC[1] = ConvertChartoMac(mac_char[3], mac_char[4]);
      eth_MAC[2] = ConvertChartoMac(mac_char[6], mac_char[7]);
      //eth_MAC[3] = ConvertChartoMac(mac_char[9], mac_char[10]);
      //eth_MAC[4] = ConvertChartoMac(mac_char[12], mac_char[13]);
      //eth_MAC[5] = ConvertChartoMac(mac_char[15], mac_char[16]);

      eth_MAC[3] = ConvertChartoMac(chipID_char[sizeChipID - 5], chipID_char[sizeChipID - 4]);
      eth_MAC[4] = ConvertChartoMac(chipID_char[sizeChipID - 3], chipID_char[sizeChipID - 2]);
      eth_MAC[5] = ConvertChartoMac(chipID_char[sizeChipID - 1], chipID_char[sizeChipID]);
    }
  }

  { //Imprimir MAC
    Serial.print("[ETHERNET]: MAC: ");
    for (byte i = 0; i < 6; i++) {
      Serial.print(eth_MAC[i], HEX);
      if (i < 5) {
        Serial.print(":");
      }
    }
    Serial.println();
  }

  bool thereConection = false;
  { //Buscar IP
    while (!thereConection) {
      Serial.println("[ETHERNET]: Connecting...");
      if (Ethernet.begin(eth_MAC) == 0) {
        //Serial.println("[ETHERNET]: Failed to configure Ethernet using DHCP");
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
          //Serial.println("[ETHERNET]: Ethernet shield was not found.  Sorry, can't run without hardware. :(");
        }
        if (Ethernet.linkStatus() == LinkOFF) {
          //Serial.println("[ETHERNET]: Ethernet cable is not connected.");
        }
        vTaskDelay(30000 / portTICK_PERIOD_MS);
      } else {
        thereConection = true;
        thereConectionEthernet = true;
      }
    }
  }
  Serial.print("[ETHERNET]: DHCP assigned IP: ");
  Serial.println(Ethernet.localIP());

  unsigned long startAttemptTime = millis();
  for (;;) {
    if (millis() - startAttemptTime > ETH_TIMEOUT) {
      startAttemptTime = millis();
      Serial.print("[ETHERNET]: [Core 1]: "); Serial.println(xPortGetCoreID());
    }

    { //Buscar fallos en el ethernet relacionados con el cambio de ip por dhcp para corregirlo
      switch (Ethernet.maintain()) {
        case 1:
          //renewed fail
          //Serial.println("[ETHERNET]: Error: renewed fail");
          thereConectionEthernet = false;
          break;
        case 2:
          //renewed success
          //Serial.println("[ETHERNET]: Renewed success");
          //print your local IP address:
          /*Serial.print("My IP address: ");
            Serial.println(Ethernet.localIP());*/
          break;
        case 3:
          //rebind fail
          //Serial.println("[ETHERNET]: Error: rebind fail");
          thereConectionEthernet = false;
          break;
        case 4:
          //rebind success
          //Serial.println("[ETHERNET]: Rebind success");
          //print your local IP address:
          /*Serial.print("My IP address: ");
            Serial.println(Ethernet.localIP());*/
          break;
        default:
          //nothing happened
          break;
      }
    }

    vTaskDelay(15 / portTICK_PERIOD_MS);
  }
}
#endif




/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *                                                                          *
                       SEND AND RESPONSE TASK
 *                                                                          *
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *****************************************************************************/

String _ip;
int _port;
String _urlSolicitud;
bool _enviar_get = false;

#include <RestClient.h>
RestClient client = RestClient("192.168.1.5");
String respuestaGet = "";

#include <HTTPClient.h>

TaskHandle_t nucleo1InternetResponse_Handle = NULL;
void internetResponse( void *pvParameters );
#define EspacioMemoriaInternetResponse  1800

bool ConfigInternet(bool usarWifi, bool usarEth) {
  bool existConfigRed = false;
#ifdef useWifi
  if (usarWifi) {
    thereConectionWifi = true;
    existConfigRed = true;
    ConfigWifiKeepAlive();
  }
#endif

#ifdef useEth
  if (usarEth) {
    thereConectionEthernet = true;
    existConfigRed = true;
    ConfigEthernetKeepAlive();

    client.setHeader("Host: www.wisrovi.com");
    client.setHeader("Connection: close");
  }
#endif

  if (existConfigRed) {
    xTaskCreatePinnedToCore(
      internetResponse,      // Function that should be called
      "internetResponse",    // Name of the task (for debugging)
      EspacioMemoriaInternetResponse,         // Stack size (bytes)
      NULL,         // Parameter to pass
      1 ,           // Task priority
      &nucleo1InternetResponse_Handle,         // Task handle
      1             // Core you want to run the task on (0 or 1)
    );
  }
  return existConfigRed;
}

void internetResponse(void * parameter) {
  respuestaGet = "";
  _enviar_get = false;

  for(;;){
    vTaskDelay(15 / portTICK_PERIOD_MS);
    if(_enviar_get){
      _enviar_get = false;

      bool thereInternet = false;
      respuestaGet = "";

    #ifdef useEth  
      if (thereConectionEthernet) {    
        bool checkHardwareEthernet = true;
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
          //Serial.println("[ETHERNET]: Ethernet shield was not found.  Sorry, can't run without hardware. :(");
          checkHardwareEthernet = false;
        }
        if (Ethernet.linkStatus() == LinkOFF) {
          //Serial.println("[ETHERNET]: Ethernet cable is not connected.");
          checkHardwareEthernet = false;
        }
    
        if (checkHardwareEthernet) {
          thereInternet = true;   
          char ip_char[_ip.length() + 1];
          _ip.toCharArray(ip_char, _ip.length() + 1);
    
          char urlSolicitud_char[_urlSolicitud.length() + 1];
          _urlSolicitud.toCharArray(urlSolicitud_char, _urlSolicitud.length() + 1);
    
          int statusCode = client.get(ip_char, _port, urlSolicitud_char, &respuestaGet);
          if (statusCode == 200) {
            //Serial.print("[ETHERNET]: Rta get: ");
            //Serial.println(respuestaGet);
          } else {
            Serial.print("[ETHERNET]: Status get: ");
            Serial.println(statusCode);
          }
        }
      }
    #endif

    #ifdef useWifi
      if (!thereInternet) {
        if (thereConectionWifi) {
          thereInternet = true;
    
          String urlSend = "http://";
          urlSend.concat(_ip);
          urlSend.concat(":");
          urlSend.concat(String(_port));
          urlSend.concat(_urlSolicitud);
          //Serial.println(urlSend);
    
          HTTPClient http;
          http.begin(urlSend); // configure traged server and url
          int httpCode = http.GET(); // start connection and send HTTP header
          if (httpCode > 0  ) {  // httpCode will be negative on error
            // HTTP header has been send and Server response header has been handled
            if (httpCode == HTTP_CODE_OK) {  // file found at server
              respuestaGet = http.getString();
              //Serial.print("[WIFI]: Rta get: ");
              //Serial.println(respuestaGet);
            }
          } else {
            Serial.print("[WIFI]: Error get: ");
            Serial.println(http.errorToString(httpCode).c_str());
          }
          http.end();
        }
      }
    #endif

      if (!thereInternet) {
        Serial.println("No hay internet, no se puede enviar el mensaje.");
      }
    }
  }  
  //vTaskDelete(NULL); //borrar esta tarea
}

void SendGet(String ip, int port, String urlSolicitud) {
  _ip = ip;
  _port = port;
  _urlSolicitud = urlSolicitud;
  _enviar_get = true;
}

String getResponse(){
  return respuestaGet;
}




























/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *                                                                          *
                                  EXAMPLE
 *                                                                          *
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *****************************************************************************/



/**
Configurar que deseo activar de la libreria: ethernet, wifi o ambos
esto va en el setup


bool usarWifi = true;
  bool usarEth = true;
  ConfigInternet(usarWifi, usarEth);

*/



/***


    Para enviar un mensaje se debe:
    SendGet("ip", puerto, "rutaGet");

*/



/**

    Para recibir la respuesta se debe esperar al menos 2 segundos, luego se captura la respuesta con:
    getResponse()

*/
