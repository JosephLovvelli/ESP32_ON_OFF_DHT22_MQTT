#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Configurações da rede WiFi
const char* ssid = "NOME_DA_REDE_WIFI";
const char* password = "SENHA_DA_REDE_WIFI";

// Configurações do broker MQTT
const char* mqttServer = "IP_DO_BROKER_MQTT";
const int mqttPort = 8083;
const char* mqttUser = "USUARIO_MQTT";
const char* mqttPassword = "SENHA_MQTT";
const char* mqttTopic = "topic_on_off_dht22";

// Configurações do sensor DHT22
#define DHT_PIN 2
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// Função callback para os comandos recebidas
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Comando recebida: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Verifica o comando recebido
  if ((char)payload[0] == '0') {
    dht.begin();  // Desliga o sensor DHT22
  } else if ((char)payload[0] == '1') {
    dht.end();  // Liga o sensor DHT22
  }
}

WiFiClient wifiClient;
PubSubClient client(mqttServer, mqttPort, callback, wifiClient);

void setup() {
  Serial.begin(115200);
  
  // Conecta-se à rede WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando na rede...");
  }
  Serial.println("Conectado na rede!");

  // Conecta-se ao broker MQTT
  client.connect("ESP32Client", mqttUser, mqttPassword);
  while (!client.connected()) {
    delay(1000);
    Serial.println("Conectando ao broker MQTT...");
  }
  Serial.println("Conectado ao broker MQTT!");

  // Inscreve-se no tópico MQTT
  client.subscribe(mqttTopic);
  Serial.println("Inscrito no tópico MQTT!");
}

void loop() {
  // Verifica a conexão com o broker MQTT
  if (!client.connected()) {
    reconnect();
  }

  // Realiza a leitura do sensor DHT22
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Publica os valores lidos no tópico MQTT
  char payload[50];
  snprintf(payload, sizeof(payload), "%.2f,%.2f", temperature, humidity);
  client.publish("topic_temperatura_umidade", payload);

  delay(5000);  // Aguarda 5 segundos antes de fazer a próxima leitura
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Conexão perdida. Tentando reconectar...");
    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("Reconectado ao broker MQTT!");
      client.subscribe(mqttTopic);
    } else {
      Serial.print("Falha na conexão. Erro: ");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}