// Blynk
#define BLYNK_TEMPLATE_ID "TMPL2vDZ45Qwz"
#define BLYNK_TEMPLATE_NAME "vitor"
#define BLYNK_AUTH_TOKEN "asrOcmhW7k3g1I3gUITYsMwKOmrcg6sa" 

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <MAX30100_PulseOximeter.h>

// Wi-Fi
const char* wifi_ssid = "Viviane";
const char* wifi_password = "92938727";

// LED
const int led_pin = 2;

// Sensores
#define SDA_PIN 21
#define SCL_PIN 22
Adafruit_MPU6050 mpu;
PulseOximeter pox;

// Tempo de leitura e alerta (2 segundos)
const unsigned long intervalo_leitura = 2000;
unsigned long ultima_leitura = 0;

// Tempo de envio para o Blynk (10 segundos)
const unsigned long intervalo_envio = 10000;
unsigned long ultimo_envio = 0;

// Tempo mínimo entre alertas (para evitar spam)
const unsigned long intervalo_alerta = 60000;
unsigned long ultimo_alerta = 0;

// Controle de envio de dados
bool enviar_dados = true;

BLYNK_WRITE(V0) {
  int estado = param.asInt();
  enviar_dados = (estado == 1);
  Serial.println(enviar_dados ? "Envio de dados ATIVADO" : "Envio de dados DESATIVADO");
}

void setup() {
  Serial.begin(115200);
  pinMode(led_pin, OUTPUT);

  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(led_pin, LOW);
  }
  Serial.println("\nWi-Fi conectado!");
  digitalWrite(led_pin, HIGH);

  Blynk.begin(BLYNK_AUTH_TOKEN, wifi_ssid, wifi_password);
  Blynk.syncVirtual(V0);  // Sincroniza estado do botão

  Wire.begin(SDA_PIN, SCL_PIN);

  ArduinoOTA.begin();
  Serial.println("OTA pronto");

  Serial.println("Inicializando MPU6050...");
  if (!mpu.begin()) {
    Serial.println("Erro: MPU6050 não encontrado.");
    while (true);
  }

  Serial.println("Inicializando MAX30100...");
  if (!pox.begin()) {
    Serial.println("Erro: MAX30100 não encontrado.");
    while (true);
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
}

void loop() {
  Blynk.run();
  pox.update();
  ArduinoOTA.handle();

  if (!enviar_dados) return;

  unsigned long agora = millis();

  // Leitura dos sensores e alertas a cada 2s
  if (agora - ultima_leitura > intervalo_leitura) {
    ultima_leitura = agora;

    float batimentos = pox.getHeartRate();
    float oximetria = pox.getSpO2();

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float aceleracao_total = sqrt(pow(a.acceleration.x, 2) +
                                  pow(a.acceleration.y, 2) +
                                  pow(a.acceleration.z, 2));

    float rotacao_total = sqrt(pow(g.gyro.x, 2) +
                               pow(g.gyro.y, 2) +
                               pow(g.gyro.z, 2));

    // Exibe no Serial Monitor a cada 2 segundos
    Serial.print("Batimentos: ");
    Serial.print(batimentos);
    Serial.print(" bpm | Oximetria: ");
    Serial.print(oximetria);
    Serial.print("% | Aceleração: ");
    Serial.print(aceleracao_total);
    Serial.print(" m/s² | Rotação: ");
    Serial.println(rotacao_total);

    // Verifica alertas
    if (agora - ultimo_alerta > intervalo_alerta) {
      if (batimentos < 50) {
        Blynk.logEvent("batimento_baixo", "Batimentos abaixo do normal (< 50 bpm)");
        ultimo_alerta = agora;
      } else if (batimentos > 150) {
        Blynk.logEvent("batimento_alto", "Batimentos acima do normal (> 150 bpm)");
        ultimo_alerta = agora;
      }

      if (oximetria < 90) {
        Blynk.logEvent("oximetria_baixa", "Oximetria baixa detectada (< 90%)");
        ultimo_alerta = agora;
      }

      if (aceleracao_total < 0.3) {
        Blynk.logEvent("queda_detectada", "Possível queda detectada");
        ultimo_alerta = agora;
      }
    }

    // Envia os dados apenas a cada 10 segundos
    if (agora - ultimo_envio > intervalo_envio) {
      ultimo_envio = agora;
      Serial.println("Enviando dados ao Blynk...");

      Blynk.virtualWrite(V1, batimentos);
      Blynk.virtualWrite(V2, oximetria);
      Blynk.virtualWrite(V3, aceleracao_total);
      Blynk.virtualWrite(V4, rotacao_total);
    }
  }
}