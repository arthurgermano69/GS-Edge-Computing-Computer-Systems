#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// Credenciais da rede WiFi — substituir pelo IP e rede local antes de usar.

const char* SSID     = "SUA_REDE_WIFI";
const char* PASSWORD = "SUA_SENHA_WIFI";

// Endereço do servidor Python que receberá os dados do ESP32.

const char* SERVIDOR = "http://SEU_IP_LOCAL:5000/dados";

// Limiares de alerta — valores acima desses disparam o sistema.

const int LIMITE_TEMPERATURA = 60;  // graus Celsius
const int LIMITE_FUMACA      = 400; // valor analógico do sensor (0 a 1023)

// Definição dos pinos utilizados no circuito.

#define PINO_DHT    15  // Sensor de temperatura e umidade DHT22
#define PINO_FUMACA 34  // Potenciômetro simulando sensor de fumaça MQ-2
#define PINO_LED_VM  2  // LED vermelho — alerta de perigo
#define PINO_LED_VD  4  // LED verde — status normal
#define PINO_BUZZER  5  // Buzzer — alarme sonoro

// Inicializa o sensor DHT22 no pino definido.

DHT dht(PINO_DHT, DHT22);

void setup() {
  Serial.begin(115200);

  // Configura os pinos de saída para LEDs e buzzer.
  
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

  // Liga o LED verde ao iniciar indicando que o sistema está operacional.
  
  digitalWrite(PINO_LED_VD, HIGH);
}

// Aciona o alarme local — LED vermelho pisca 3 vezes com buzzer e permanece aceso.

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
  
  // Mantém o LED vermelho aceso após as piscadas.
  
  digitalWrite(PINO_LED_VM, HIGH);
}

// Retorna o sistema ao estado normal — LED verde aceso, LED vermelho e buzzer desligados.

void estadoNormal() {
  digitalWrite(PINO_LED_VM, LOW);
  digitalWrite(PINO_LED_VD, HIGH);
  digitalWrite(PINO_BUZZER, LOW);
}

// Monta o payload JSON com os dados dos sensores e envia ao servidor Python via HTTP POST.

void enviarDados(float temperatura, float umidade, int fumaca, String status) {

  // Verifica se o WiFi ainda está conectado antes de tentar enviar.
 
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERRO] Sem WiFi, pulando envio.");
    return;
  }

  HTTPClient http;
  http.begin(SERVIDOR);
  http.addHeader("Content-Type", "application/json");

  // Monta o JSON com os dados lidos pelos sensores.
  
  StaticJsonDocument<200> doc;
  doc["temperatura"] = temperatura;
  doc["umidade"]     = umidade;
  doc["fumaca"]      = fumaca;
  doc["status"]      = status;

  String payload;
  serializeJson(doc, payload);

  Serial.println("Enviando dados para o servidor Python...");
  int codigo = http.POST(payload);

  // Verifica se o envio foi bem sucedido pelo código HTTP retornado.
  
  if (codigo == 200) {
    Serial.println("Dados enviados com sucesso!");
  } else {
    Serial.print("Erro no envio. Código HTTP: ");
    Serial.println(codigo);
  }

  http.end();
}

void loop() {

  // Lê os valores atuais dos sensores.
  
  float temperatura = dht.readTemperature();
  float umidade     = dht.readHumidity();
  int   fumaca      = analogRead(PINO_FUMACA);

  // Verifica leitura válida do DHT22
 
  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("[ERRO] Falha na leitura do DHT22!");
    delay(2000);
    return;
  }

  // Exibe os valores lidos no monitor serial.
  
  Serial.print("Temperatura: "); Serial.print(temperatura); Serial.println(" °C");
  Serial.print("Umidade: ");     Serial.print(umidade);     Serial.println(" %");
  Serial.print("Fumaça: ");      Serial.println(fumaca);

  // Verifica se algum limiar foi ultrapassado e define o status do sistema.
  
  bool alerta = (temperatura > LIMITE_TEMPERATURA || fumaca > LIMITE_FUMACA);
  String status = alerta ? "ALERTA" : "NORMAL";

  // Aciona o alarme local ou mantém o estado normal conforme o status.
  
  if (alerta) {
    Serial.println("ALERTA DETECTADO! Acionando alarme local...");
    acionarAlerta();
  } else {
    Serial.println("Situação normal.");
    estadoNormal();
  }

  // Envia os dados ao servidor Python para cruzamento com dados da NASA.
  
  enviarDados(temperatura, umidade, fumaca, status);

  // Aguarda 5 segundos antes da próxima leitura.
  
  delay(5000);
}