#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

const char* SSID     = "SUA_REDE_WIFI";
const char* PASSWORD = "SUA_SENHA_WIFI";
const char* SERVIDOR = "http://SEU_IP_LOCAL:5000/dados";

const int LIMITE_TEMPERATURA = 60;
const int LIMITE_FUMACA = 400;

#define PINO_DHT 15
#define PINO_FUMACA 34
#define PINO_LED_VM 2
#define PINO_LED_VD 4
#define PINO_BUZZER 5

DHT dht(PINO_DHT, DHT22);

void setup() {
  Serial.begin(115200);

  pinMode(PINO_LED_VM, OUTPUT);
  pinMode(PINO_LED_VD, OUTPUT);
  pinMode(PINO_BUZZER, OUTPUT);

  dht.begin();

  Serial.println("OrbitMax Sentinel — ESP32");

  // Conecta ao WiFi
  Serial.print("Conectando ao WiFi");
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi conectado!");
  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.localIP());

  digitalWrite(PINO_LED_VD, HIGH);
}

void acionarAlerta() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(PINO_LED_VD, LOW);
    digitalWrite(PINO_LED_VM, HIGH);
    digitalWrite(PINO_BUZZER, HIGH);
    delay(300);
    digitalWrite(PINO_LED_VM, LOW);
    digitalWrite(PINO_BUZZER, LOW);
    delay(200);
  }
  digitalWrite(PINO_LED_VM, HIGH);
}

void estadoNormal() {
  digitalWrite(PINO_LED_VM, LOW);
  digitalWrite(PINO_LED_VD, HIGH);
  digitalWrite(PINO_BUZZER, LOW);
}

void enviarDados(float temperatura, float umidade, int fumaca, String status) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERRO] Sem WiFi, pulando envio.");
    return;
  }

  HTTPClient http;
  http.begin(SERVIDOR);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> doc;
  doc["temperatura"] = temperatura;
  doc["umidade"] = umidade;
  doc["fumaca"] = fumaca;
  doc["status"] = status;

  String payload;
  serializeJson(doc, payload);

  Serial.println("Enviando dados para o servidor Python...");
  int codigo = http.POST(payload);

  if (codigo == 200) {
    Serial.println("Dados enviados com sucesso!");
  } else {
    Serial.print("Erro no envio. Código HTTP: ");
    Serial.println(codigo);
  }

  http.end();
}

void loop() {
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
  int   fumaca = analogRead(PINO_FUMACA);

  // Verifica leitura válida do DHT22
  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("[ERRO] Falha na leitura do DHT22!");
    delay(2000);
    return;
  }

  Serial.print("Temperatura: "); Serial.print(temperatura); Serial.println(" °C");
  Serial.print("Umidade: "); Serial.print(umidade); Serial.println(" %");
  Serial.print("Fumaça: "); Serial.println(fumaca);

  bool alerta = (temperatura > LIMITE_TEMPERATURA || fumaca > LIMITE_FUMACA);
  String status = alerta ? "ALERTA" : "NORMAL";

  if (alerta) {
    Serial.println("ALERTA DETECTADO! Acionando alarme local...");
    acionarAlerta();
  } else {
    Serial.println("Situação normal.");
    estadoNormal();
  }

  enviarDados(temperatura, umidade, fumaca, status);

  delay(5000);
}