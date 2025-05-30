#define BLYNK_TEMPLATE_ID "TMPL2xsup6xi0"
#define BLYNK_TEMPLATE_NAME "MonitorarEsp32"
#define BLYNK_AUTH_TOKEN "QnAPRj2EPd_odGjcHKPhLkBfKEtBoEoZ"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ArduinoOTA.h>

// Wi-Fi
char ssid[] = "";
char pass[] = "";

// LED interno
const int led_pin = 2;

// Blynk Timer
BlynkTimer timer;

// Função para enviar dados periodicamente
void enviarDados() {
  int batimentos = random(0, 100);
  int oximetria = random(0, 100);
  int aceleracao = random(0, 10);
  int rotacao = random(0, 10);

  Serial.println("Enviando dados para Blynk...");

  // Envia os dados para os pinos virtuais
  Blynk.virtualWrite(V0, batimentos);
  Blynk.virtualWrite(V1, oximetria);
  Blynk.virtualWrite(V2, aceleracao);
  Blynk.virtualWrite(V3, rotacao);

  // Envia para o Terminal no App (opcional)
  Blynk.virtualWrite(V10, String("Batimentos: ") + batimentos + 
                               ", Oximetria: " + oximetria + 
                               ", Aceleração: " + aceleracao + 
                               ", Rotação: " + rotacao);

  // Alertas
  if (batimentos < 5) {
    Blynk.logEvent("batimentos_baixos", "Batimentos abaixo do normal!");
  
  }
  if (batimentos > 95) {
    Blynk.logEvent("batimentos_altos", "Batimentos acima do normal!");
  }
  if (aceleracao == 0) {
    Blynk.logEvent("queda_detectada", "Possível queda detectada!");
 
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);

  // Conectar ao Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  ArduinoOTA.begin();
  Serial.println("OTA pronto");

  // Timer a cada 2 segundos
  timer.setInterval(2000L, enviarDados);
}

void loop() {
  Blynk.run();
  timer.run();
  ArduinoOTA.handle();
}