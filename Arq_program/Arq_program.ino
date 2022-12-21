#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
//----------------------------------------
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

const int DHTPin = 14;
  String t;
#define ON_Board_LED 2  //--> LED On Board, usado para indicadores durante o processo de conexão a um roteador wi-fi

//----------------------------------------SSID dan Password wifi mu gan.
const char* ssid = "brisa-2456070"; //--> NOME Wifi / SSID.
const char* password = "bqsygrmv"; //-->  Password wifi .
//----------------------------------------

//Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------
// Inicializar DHT sensor.
DHT dht(DHTPin, DHTTYPE);

WiFiClientSecure client; //--> WiFiClientSecure object.

// Variáveis auxiliares dos temporizadores
long now = millis();
long lastMeasure = 0;

String GAS_ID = "AKfycbwARXlfEcXeTDOY241R9KttuIC62FekA4SdyaK_JMZmLj0doe0p0cHo9blKRL-KUm6A"; //--> spreadsheet script ID

//============================================ 
void setup() {
  // código de configuração aqui, para executar uma vez:
  Serial.begin(115200);
  delay(500);
  dht.begin();

  WiFi.begin(ssid, password); //--> Conecte-se ao seu roteador Wi-Fi
  Serial.println("");
    
  pinMode(ON_Board_LED,OUTPUT); //--> Saída de direção da porta LED integrada
  digitalWrite(ON_Board_LED, HIGH); 

  //Aguarde a conexão
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    //LED On Board Flashing no processo de conexão com o roteador wi-fi.
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
    //----------------------------------------
  }
  //----------------------------------------
  digitalWrite(ON_Board_LED, HIGH); //--> Desliga o On Board LED quando estiver conectado ao roteador wi-fi.
  // Conectado com sucesso ao roteador wi-fi, o endereço IP que será visitado é exibido no monitor serial
  Serial.println("");
  Serial.print("Conectado com sucesso a : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------

  client.setInsecure();
}

void loop() {

  now = millis();
  // Publica nova temperatura e umidade a cada 8 segundos
  if (now - lastMeasure > 8000) {
    lastMeasure = now;
    
    float h = dht.readHumidity();
    // Leitura da temperatura como Celsius (o padrão)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Verifica se alguma leitura falhou e saia antes (para tentar novamente).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Falha ao ler do sensor DHT!");
      return;
    }

    // Calcula valores de temperatura em Celsius
    float hic = dht.computeHeatIndex(t, h, false);
    static char temperatureTemp[7];
    dtostrf(hic, 6, 2, temperatureTemp);
    
    static char humidityTemp[7];
    dtostrf(h, 6, 2, humidityTemp);
     
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t Heat index: ");
    Serial.print(hic);
    Serial.println(" *C ");
    // Serial.print(hif);
    // Serial.println(" *F");
     sendData(t,h);
    
  }

  
}
//==============================================================================

void sendData(float value,float value2) {
  Serial.println("==========");
  Serial.print("conectando à ");
  Serial.println(host);
  
  //Conecta ao host do Google
  if (!client.connect(host, httpsPort)) {
    Serial.println("falha na conexão");
    return;
  }
  
  //Processar e enviar dados 

  float string_temp = value; 
  float string_humi = value2;
  String url = "/macros/s/" + GAS_ID + "/exec?temp=" + string_temp + "&humi="+string_humi; //  2 variables 
  Serial.print("solicitando URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("pedido enviado");

  //---------------------------------------
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  //----------------------------------------
} 
//===============================================
