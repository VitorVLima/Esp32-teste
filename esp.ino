#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

// Wi-Fi
const char* wifi_ssid = "";
const char* wifi_password = "";


const int mqtt_port = 1883;
const char* mqtt_broker = "broker.hivemq.com";
const char* mqtt_topic = "sensor/leitura/app1234";

// LED
const int led_pin = 2;

WiFiClient client;
PubSubClient mqtt_client(client);

unsigned long ultima_tentativa_reconexao = 0;
unsigned long ultimo_alerta = 0;
const unsigned long intervalo_alerta = 60 * 1000;

void conectar_wifi() {
  Serial.println("Conectando à rede Wi-Fi...");
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(led_pin, LOW);
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.print("IP local: ");
  Serial.println(WiFi.localIP());
  digitalWrite(led_pin, HIGH);
}

void reconectar_mqtt() {
  Serial.println("Conectando ao broker MQTT...");
  
  if (mqtt_client.connect("esp32_80989")) {
    Serial.println("Conectado ao MQTT!");
  } else {
    Serial.print("Falha na conexão MQTT, rc=");
    Serial.println(mqtt_client.state());
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(led_pin, OUTPUT);

  conectar_wifi();
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  ArduinoOTA.begin();
  Serial.println("OTA pronto");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectar_wifi();
  }

  if (!mqtt_client.connected()) {
    unsigned long agora = millis();
    if (agora - ultima_tentativa_reconexao > 5000) {
      ultima_tentativa_reconexao = agora;
      reconectar_mqtt();
    }
  } else {
    mqtt_client.loop();

    static unsigned long ultima_mensagem = 0;
    if (millis() - ultima_mensagem > 2000) {
      ultima_mensagem = millis();

      int batimentos = random(0, 100);
      int oximetria = random(0, 100);
      int acelerecao = random(0, 10);
      int rotacao = random(0, 10);

      String mensagem_json = "{";
      mensagem_json += "\"dispositivo\":\"ESP32\",";
      mensagem_json += "\"batimentos\":" + String(batimentos) + ",";
      mensagem_json += "\"oximetria\":" + String(oximetria) + ",";
      mensagem_json += "\"aceleracao\":" + String(acelerecao) + ",";
      mensagem_json += "\"rotacao\":" + String(rotacao);
      mensagem_json += "}";

      Serial.print("Enviando JSON MQTT: ");
      Serial.println(mensagem_json);

      mqtt_client.publish(mqtt_topic, mensagem_json.c_str());

      if ((batimentos > 95 || batimentos < 5 || acelerecao == 0) && (millis() - ultimo_alerta > intervalo_alerta)) {
        mqtt_client.publish("sensor/alerta/app1234",mensagem_json.c_str());
        ultimo_alerta = millis();
      }
    }
  }

  ArduinoOTA.handle();
}