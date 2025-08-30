#include <WiFi.h>
#include <WebServer.h>
#include <Ticker.h>

const char* ssid = "GalaxyA31";       // seu Wi-Fi
const char* password = "775775775";   // senha do Wi-Fi

// Definição dos pinos
#define ENA 26  // PWM Motor A
#define IN1 25  // Motor A frente
#define IN2 33  // Motor A ré

#define ENB 27  // PWM Motor B
#define IN3 32  // Motor B frente
#define IN4 14  // Motor B ré

const int buzzerPin = 13; // Pino do Buzzer

WebServer server(80);
Ticker buzzerTicker;

bool buzzerState = false;

// Funções de controle do buzzer
void toggleBuzzer() {
  buzzerState = !buzzerState;
  digitalWrite(buzzerPin, buzzerState);
}

// HTML + CSS + JS dentro do Arduino
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Controle do Carrinho - ESP32</title>
<style>
body { font-family: Arial, sans-serif; display: flex; justify-content: center; align-items: center; min-height: 100vh; background-color: #f0f0f0; margin:0; text-align:center;}
.container { background-color:#fff; padding:2em; border-radius:10px; box-shadow:0 4px 8px rgba(0,0,0,0.1);}
h1 { color:#333; margin-bottom:1em; }
.status { font-size:1.2em; color:#666; margin-bottom:1.5em;}
.button-group { display:flex; flex-wrap:wrap; gap:20px; justify-content:center;}
.control-button { padding:15px 30px; font-size:1em; border:none; border-radius:5px; cursor:pointer; color:white; transition:0.3s;}
#forward { background-color:#4CAF50; } #forward:hover { background-color:#45a049; }
#back { background-color:#f44336; } #back:hover { background-color:#e53935; }
#left { background-color:#ff9800; } #left:hover { background-color:#e68a00; }
#right { background-color:#2196F3; } #right:hover { background-color:#1976d2; }
#stop { background-color:#9E9E9E; } #stop:hover { background-color:#7e7e7e; }
</style>
</head>
<body>
<div class="container">
<h1>Controle do Carrinho</h1>
<p class="status" id="status-text">Status: parado</p>
<div class="button-group">
  <button id="forward" class="control-button">Frente</button>
  <button id="back" class="control-button">Ré</button>
  <button id="left" class="control-button">Esquerda</button>
  <button id="right" class="control-button">Direita</button>
  <button id="stop" class="control-button">Parar</button>
</div>
</div>
<script>
const statusText = document.getElementById('status-text');

document.getElementById('forward').addEventListener('click',()=>toggleCar('FORWARD'));
document.getElementById('back').addEventListener('click',()=>toggleCar('BACK'));
document.getElementById('left').addEventListener('click',()=>toggleCar('LEFT'));
document.getElementById('right').addEventListener('click',()=>toggleCar('RIGHT'));
document.getElementById('stop').addEventListener('click',()=>toggleCar('STOP'));

async function toggleCar(command){
    try{
        const response = await fetch('/'+command);
        await response.text();
        statusText.textContent = 'Status: ' + command.toLowerCase();
    }catch(err){
        statusText.textContent = 'Erro de conexão';
        console.error(err);
    }
}
</script>
</body>
</html>
)rawliteral";

// Funções de controle do carrinho
void moveForward() {
  buzzerTicker.detach(); // Garante que o buzzer pare de apitar
  digitalWrite(buzzerPin, LOW); // Garante que o buzzer esteja desligado
  analogWrite(ENA, 255);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENB, 255);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  Serial.println("Frente");
}

void moveBack() {
  buzzerTicker.attach_ms(500, toggleBuzzer); // Toca o buzzer a cada 500ms
  analogWrite(ENA, 255);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENB, 255);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  Serial.println("Ré");
}

void turnLeft() {
  buzzerTicker.detach();
  digitalWrite(buzzerPin, LOW);
  analogWrite(ENA, 150);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENB, 150);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  Serial.println("Esquerda");
}

void turnRight() {
  buzzerTicker.detach();
  digitalWrite(buzzerPin, LOW);
  analogWrite(ENA, 150);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENB, 150);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  Serial.println("Direita");
}

void stopCar() {
  buzzerTicker.detach(); // Garante que o buzzer pare de apitar
  digitalWrite(buzzerPin, LOW); // Garante que o buzzer esteja desligado
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  Serial.println("Parado");
}

// Rotas do servidor
void handleRoot() { server.send(200, "text/html", htmlPage); }
void handleForward() { moveForward(); server.send(200,"text/plain","OK"); }
void handleBack() { moveBack(); server.send(200,"text/plain","OK"); }
void handleLeft() { turnLeft(); server.send(200,"text/plain","OK"); }
void handleRight() { turnRight(); server.send(200,"text/plain","OK"); }
void handleStop() { stopCar(); server.send(200,"text/plain","OK"); }

void setup() {
  Serial.begin(115200);
  
  // Pinos como saída
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  stopCar();

  // Conecta Wi-Fi
  WiFi.begin(ssid,password);
  Serial.print("Conectando ao Wi-Fi");
  while(WiFi.status()!=WL_CONNECTED){ delay(500); Serial.print("."); }
  Serial.println();
  Serial.print("Conectado! IP: "); Serial.println(WiFi.localIP());

  // Rotas HTTP
  server.on("/", handleRoot);
  server.on("/FORWARD", handleForward);
  server.on("/BACK", handleBack);
  server.on("/LEFT", handleLeft);
  server.on("/RIGHT", handleRight);
  server.on("/STOP", handleStop);

  server.begin();
  Serial.println("Servidor iniciado!");
}

void loop() {
  server.handleClient();
}
