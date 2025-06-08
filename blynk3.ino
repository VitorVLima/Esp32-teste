// Definições obrigatórias para o Blynk 2.0
#define BLYNK_TEMPLATE_ID "TMPL2vDZ45Qwz"
#define BLYNK_TEMPLATE_NAME "vitor"
#define BLYNK_AUTH_TOKEN "asrOcmhW7k3g1I3gUITYsMwKOmrcg6sa" 

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ArduinoOTA.h>

// Wi-Fi
const char* wifi_ssid = "";
const char* wifi_password = "";

// LED de status
const int led_pin = 2;

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

void setup() {
  Serial.begin(115200);
  pinMode(led_pin, OUTPUT);

  conectar_wifi();
  
  // Inicializa Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, wifi_ssid, wifi_password);

  // Inicializa OTA
  ArduinoOTA.begin();
  Serial.println("OTA pronto. Aguardando upload...");
}

void loop() {
  Blynk.run();
  ArduinoOTA.handle();

  static unsigned long ultima_mensagem = 0;
  if (millis() - ultima_mensagem > 2000) { // A cada 2 segundos
    ultima_mensagem = millis();

    // Simulação de dados de sensores
    int batimentos = random(60, 100);   // Frequência cardíaca
    int oximetria = random(90, 100);    // Saturação de oxigênio
    int aceleracao = random(1, 10);     // Aceleração
    int rotacao = random(0, 10);        // Rotação

    Serial.print("Enviando dados para Blynk: ");
    Serial.print(batimentos); Serial.print(", ");
    Serial.print(oximetria); Serial.print(", ");
    Serial.print(aceleracao); Serial.print(", ");
    Serial.println(rotacao);

    // Envia os dados para os widgets no app Blynk
    Blynk.virtualWrite(V1, batimentos);
    Blynk.virtualWrite(V2, oximetria);
    Blynk.virtualWrite(V3, aceleracao);
    Blynk.virtualWrite(V4, rotacao);

    // Envia alerta se condição crítica for detectada
    if ( aceleracao <= 1) {
      Blynk.logEvent("alerta", "⚠️ Atenção: Possivel queda detectada!");
    }
    if (batimentos < 5) {
      Blynk.logEvent("alerta", "⚠️ Atenção: Batimentos abaixo do normal detectado!");
    }
    if (batimentos > 95) {
      Blynk.logEvent("alerta", "⚠️ Atenção: Batimentos acima do normal detectado!");
    }
  }
}