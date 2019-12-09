#include <Arduino.h>
#include "ESP8266WiFi.h"

#define LED 2

const char *password = "12345678";

bool reconnect = false;
const String ssid = "Tenda_Pruebas";

IPAddress local_IP(192, 168, 3, 6);
IPAddress gateway(192, 168, 0, 254);
IPAddress subnet(255, 255, 0, 0);

WiFiServer server(80);

bool isSsidAvailable();
void connectToWiFi();
void handleLed();
void handleWiFiConnection();
void handleServer();

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  //Definir led como salida
  pinMode(LED, OUTPUT);

  //Iniciar conexión
  connectToWiFi();
}

void loop()
{
  handleLed();
  handleWiFiConnection();
  handleServer();
}

void connectToWiFi()
{
  Serial.println("Iniciando conexion WiFi");

  // No conectarse a la ssid almacenada en flash
  WiFi.disconnect();

  // Definir modo Access Point + Station
  WiFi.mode(WIFI_AP_STA);

  // Crear Access point
  WiFi.softAP("mESP8266", "12345678");

  // Configurar ip estática, Gw y máscara
  WiFi.config(local_IP, gateway, subnet);
  delay(10);

  // Conectarse a la ssid definida en programa
  WiFi.begin(ssid.c_str(), password);

  // Iniciar servidor
  server.begin();
}

void handleLed()
{
  static unsigned long timeRef;

  if (millis() - timeRef > 1000)
  {
    digitalWrite(LED, !digitalRead(LED));
    timeRef = millis();
  }
}

void handleWiFiConnection()
{
  static wl_status_t mStatus = WL_IDLE_STATUS;

  // Evaluar cambio de estado
  if (WiFi.status() != mStatus)
  {
    mStatus = WiFi.status();
    Serial.printf("Nuevo estado -> %d\n", mStatus);

    if (mStatus == WL_CONNECTED)
    {
      Serial.println("Conectado a la red WiFi");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.print(WiFi.RSSI());
      Serial.println(" dBm");
    }
    else if (mStatus == WL_NO_SSID_AVAIL)
    {
      Serial.println("La ssid no esta disponible");
      WiFi.setAutoReconnect(false);

      // Activar bandera para reconectar manualmente
      reconnect = true;
    }
    else
    {
      Serial.println("No conexion a internet :(");
    }
  }

  // Evaluar reconexión
  if (reconnect)
  {
    if (isSsidAvailable())
    {
      WiFi.setAutoReconnect(true);
      WiFi.reconnect();
      Serial.println("Reconectando!");
      reconnect = false;
    }
  }
}

void handleServer()
{
  static WiFiClient client;

  // Evaluar si ya había un cliente conectado
  if (client.connected())
  {
    // Esperar nuevo dato
    if (client.available())
    {
      client.print("Que ondas.. bye");
      client.stop();
      Serial.println("Mensaje recibido, socket cerrado");
    }
  }
  else
  {
    // Esperar cliente nuevo
    client = server.available();
    if (client)
    {
      Serial.println("Nuevo cliente...");
    }
  }
}

bool isSsidAvailable()
{
  static unsigned long timeRef;
  static int step = 0;
  static int networks;

  switch (step)
  {
  case 0:
    if (millis() - timeRef > 8000)
    {
      // Hacer escaneo de redes
      networks = WiFi.scanNetworks();
      step++;

      // Tomar referencia de tiempo
      timeRef = millis();
    }

    break;

  case 1:
    for (int i = 0; i < networks; i++)
    {
      String foundSsid = WiFi.SSID(i);
      Serial.println("Red encontrada -> " + foundSsid);
      if (foundSsid == ssid)
      {
        Serial.println("SSID disponible!");
        step = 0;
        return true;
      }
    }

    Serial.println("No se encontro la SSID\n\n");
    step = 0;
    break;
  }

  return false;
}