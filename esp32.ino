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
const char* wifi_ssid = "";
const char* wifi_password = "";

// LED
const int led_pin = 2;

// Sensores
#define SDA_MPU 21
#define SCL_MPU 22

#define SDA_MAX 18   // Novo barramento I2C para o MAX30100
#define SCL_MAX 19

Adafruit_MPU6050 mpu;
PulseOximeter pox;

// Novo barramento I2C para MAX30100
TwoWire I2C_2 = TwoWire(1);

// Variáveis dos sensores
float batimentos = 0;
float oximetria = 0;
float aceleracao_total = 0;
float rotacao_total = 0;

// Controle
bool enviar_dados = true;
unsigned long ultimo_alerta = 0;
const unsigned long intervalo_alerta = 60000;

// Timer
BlynkTimer timer;

BLYNK_WRITE(V0) {
  int estado = param.asInt();
  enviar_dados = (estado == 1);
  Serial.println(enviar_dados ? "Envio de dados ATIVADO" : "Envio de dados DESATIVADO");
}

void lerSensores() {
  pox.update();

  batimentos = pox.getHeartRate();
  oximetria = pox.getSpO2();

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  aceleracao_total = sqrt(pow(a.acceleration.x, 2) +
                          pow(a.acceleration.y, 2) +
                          pow(a.acceleration.z, 2));

  rotacao_total = sqrt(pow(g.gyro.x, 2) +
                       pow(g.gyro.y, 2) +
                       pow(g.gyro.z, 2));

  Serial.print("Batimentos: ");
  Serial.print(batimentos);
  Serial.print(" bpm | Oximetria: ");
  Serial.print(oximetria);
  Serial.print("% | Aceleração: ");
  Serial.print(aceleracao_total);
  Serial.print(" m/s² | Rotação: ");
  Serial.println(rotacao_total);

  verificarAlertas();
}

void verificarAlertas() {
  unsigned long agora = millis();

  if (agora - ultimo_alerta < intervalo_alerta) return;

  if (batimentos < 50) {
    Blynk.logEvent("alerta", "Batimentos abaixo do normal (< 50 bpm)");
    ultimo_alerta = agora;
  } else if (batimentos > 150) {
    Blynk.logEvent("alerta", "Batimentos acima do normal (> 150 bpm)");
    ultimo_alerta = agora;
  }

  if (oximetria < 90) {
    Blynk.logEvent("alerta", "Oximetria baixa detectada (< 90%)");
    ultimo_alerta = agora;
  }

  if (aceleracao_total < 0.3) {
    Blynk.logEvent("alerta", "Possível queda detectada");
    ultimo_alerta = agora;
  }
}

void enviarAoBlynk() {
  if (!enviar_dados) return;

  Serial.println("Enviando dados ao Blynk...");

  Blynk.virtualWrite(V1, batimentos);
  Blynk.virtualWrite(V2, oximetria);
  Blynk.virtualWrite(V3, aceleracao_total);
  Blynk.virtualWrite(V4, rotacao_total);
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
  Blynk.syncVirtual(V0);

  // Inicializa I2C para MPU6050
  Wire.begin(SDA_MPU, SCL_MPU);
  ArduinoOTA.begin();
  Serial.println("OTA pronto");

  Serial.println("Inicializando MPU6050...");
  if (!mpu.begin()) {
    Serial.println("Erro: MPU6050 não encontrado.");
    while (true);
  }

  // Inicializa I2C2 para MAX30100
  I2C_2.begin(SDA_MAX, SCL_MAX);
  Serial.println("Inicializando MAX30100...");
  if (!pox.begin(&I2C_2)) {  // Usa o segundo barramento I2C
    Serial.println("Erro: MAX30100 não encontrado.");
    while (true);
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);

  // Agenda tarefas
  timer.setInterval(2000L, lerSensores);      // Leitura e alerta a cada 2 segundos
  timer.setInterval(10000L, enviarAoBlynk);   // Envio ao Blynk a cada 10 segundos
}

void loop() {
  Blynk.run();
  ArduinoOTA.handle();
  timer.run();
}
