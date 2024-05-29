#include <WiFi.h>
#include <WebServer.h>
// Definição das credenciais de WiFi
const char *ssid = "Martins WiFi6";
const char *password = "17031998";

// Inicialização do servidor web na porta 80
WebServer server(80);

// Definição do tamanho do buffer para leitura do sinal EMG
const int bufferSize = 250;

// Variáveis relacionadas à leitura e atualização dos dados do EMG
float timeElapsed = 0;
int emgBuffer[bufferSize];
int bufferIndex = 0;
bool updatingData = false;
unsigned long lastUpdateTime = 0;
unsigned long acquisitionRate = 500; // Taxa de aquisição padrão em milissegundos
const int pinEMG = 33;

// Função para ler o valor do sinal EMG
float readEMGValue() {
  int valorADC = analogRead(pinEMG);
  float tensao = ((valorADC * 3.3) / 4095); // Convertendo para volts
  return tensao; // Retorna em volts
}

// Função para lidar com requisições na rota principal ("/")
void handleRoot() {
  String html =
      "<!DOCTYPE HTML>"
      "<html>"
      "<head>"
      "<title>Projeto IB1</title>"
      "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"
      "<style>"
      "body {"
      "font-family: Arial, sans-serif;"
      "margin: 0;"
      "padding: 0;"
      "background-color: #0d0d0d;"
      "color: #e0e0e0;"
      "}"
      ".container {"
      "margin: 20px;"
      "padding: 20px;"
      "background-color: #1a1a1a;"
      "border-radius: 10px;"
      "box-shadow: 0 0 15px rgba(0, 255, 255, 0.2);"
      "text-align: center;"
      "max-width: 100%;"
      "box-sizing: border-box;"
      "}"
      ".container h2 {"
      "font-size: 32px;"
      "color: #00e676;"
      "}"
      ".container p {"
      "font-size: 18px;"
      "color: #81c784;"
      "}"
      ".container canvas {"
      "width: 100%;"
      "height: 300px;"
      "margin: 20px auto;"
      "border: 1px solid #00e676;"
      "}"
      "#rateBox {"
      "margin: 20px auto;"
      "padding: 10px;"
      "border: 1px solid #00e676;"
      "border-radius: 5px;"
      "background-color: #1a1a1a;"
      "width: 100%;"
      "max-width: 300px;"
      "color: #e0e0e0;"
      "display: flex;"
      "justify-content: center;"
      "align-items: center;"
      "}"
      "#rateBox input {"
      "width: 60%;"
      "padding: 10px;"
      "margin: 5px;"
      "border: none;"
      "border-radius: 5px;"
      "background-color: #333;"
      "color: #fff;"
      "font-size: 16px;"
      "text-align: center;"
      "}"
      "#rateBox button {"
      "padding: 10px 20px;"
      "margin: 5px;"
      "border: none;"
      "border-radius: 5px;"
      "background-color: #00e676;"
      "color: #fff;"
      "font-size: 16px;"
      "cursor: pointer;"
      "}"
      "#rateBox button:hover {"
      "background-color: #00c853;"
      "}"
      ".button {"
      "border-radius: 20px;"
      "padding: 10px 20px;"
      "margin: 10px;"
      "cursor: pointer;"
      "border: none;"
      "color: #fff;"
      "font-size: 16px;"
      "transition: background-color 0.1s ease, transform 0.1s ease;"
      "display: inline-block;"
      "}"
      ".button.start {"
      "background-color: #00e676;"
      "}"
      ".button.start:hover {"
      "background-color: #00c853;"
      "}"
      ".button.start:active {"
      "transform: scale(0.95);"
      "}"
      ".button.stop {"
      "background-color: #f44336;"
      "}"
      ".button.stop:hover {"
      "background-color: #d32f2f;"
      "}"
      ".button.stop:active {"
      "transform: scale(0.95);"
      "}"
      ".button.save {"
      "background-color: #2196f3;"
      "}"
      ".button.save:hover {"
      "background-color: #1976d2;"
      "}"
      ".button.save:active {"
      "transform: scale(0.95);"
      "}"
      "</style>"
      "</head>"
      "<body>"
      "<div class='container'>"
      "<h2>Projeto IB1</h2>"
      "<p>Valor do EMG: <span id='emg_value'>0</span> V</p>" // Exibe o valor do EMG em volts
      "<canvas id='potChart' class='potChart' width='2000' height='300'></canvas>"
      "<div id='rateBox'>"
      "Taxa de Aquisição (ms): <input type='number' id='rateInput' value='500'>"
      "<button onclick='updateRate()'>OK</button>"
      "</div>" // Text box para definir a taxa de aquisição
      "<button id='startButton' class='button start' onclick='start()'>Iniciar</button>"
      "<button class='button stop' onclick='stop()'>Parar</button>"
      "<button class='button save' onclick='save()'>Salvar</button>"
      "</div>"
      "<script>"
      "var ctx = document.getElementById('potChart').getContext('2d');"
      "var potChart;"
      "var timeElapsed = 0;"
      "var intervalId;"
      "var dataArray = [];"
      "var acquisitionRate = 500;" // Taxa de aquisição padrão em milissegundos
      "function updateRate() {"
      "  var rateInput = document.getElementById('rateInput').value;"
      "  if (rateInput > 0) {"
      "    acquisitionRate = rateInput;"
      "    if (updatingData) {"
      "      clearInterval(intervalId);"
      "      intervalId = setInterval(fetchData, acquisitionRate);"
      "    }"
      "  } else {"
      "    alert('Taxa de aquisição deve ser maior que 0');"
      "  }"
      "}"
      "function fetchData() {"
      "  fetch('/emgvalue')"
      "  .then(response => response.text())"
      "  .then(data => {"
      "    document.getElementById('emg_value').innerText = data;"
      "    updatePotChart(parseFloat(data));"
      "  });"
      "}"
      "function updatePotChart(emgvalue) {"
      "  var emgVolts = emgvalue.toFixed(4);"
      "  dataArray.push({x: (timeElapsed * 1000).toFixed(1), y: emgVolts});"
      "  timeElapsed += acquisitionRate / 1000;" // Alteração para tempo em milissegundos"
      "  if (dataArray.length > " + String(bufferSize) + ") {"
      "    dataArray.shift();"
      "  }"
      "  potChart.data.labels.push(dataArray[dataArray.length - 1].x);"
      "  potChart.data.datasets[0].data.push(emgVolts);"
      "  if (potChart.data.labels.length > " + String(bufferSize) + ") {"
      "    potChart.data.labels.shift();"
      "    potChart.data.datasets[0].data.shift();"
      "  }"
      "  potChart.update();"
      "}"
      "function start() {"
      "  if (!updatingData) {"
      "    var potCtx = document.getElementById('potChart').getContext('2d');"
      "    potChart = new Chart(potCtx, {"
      "      type: 'line',"
      "      data: {"
      "        labels: [],"
      "        datasets: [{"
      "          label: 'Amplitude EMG',"
      "          data: [],"
      "          fill: false,"
      "          borderColor: '#00e676',"
      "          backgroundColor: 'rgba(0, 230, 118, 0.2)',"
      "          tension: 0.1"
      "        }]"
      "      },"
      "      options: {"
      "        animation: {"
      "          duration: 0"
      "        },"
      "        scales: {"
      "          x: {"
      "            display: false,"
      "          },"
      "          y: {"
      "            suggestedMin: 0,"
      "            suggestedMax: 3.3" // Máximo de 3.3V
      "          }"
      "        }"
      "      }"
      "    });"
      "    intervalId = setInterval(fetchData, acquisitionRate);"
      "    document.getElementById('startButton').disabled = true;"
      "    updatingData = true;"
      "  }"
      "}"
      "function stop() {"
      "  clearInterval(intervalId);"
      "  document.getElementById('startButton').disabled = false;"
      "  updatingData = false;"
      "  dataArray = [];" // Limpar o array de dados"
      "  timeElapsed = 0;" // Resetar o tempo decorrido"
      "  if (potChart) {"
      "    potChart.data.labels = [];" // Limpar os labels do gráfico"
      "    potChart.data.datasets[0].data = [];" // Limpar os dados do gráfico"
      "    potChart.update();"
      "  }"
      "}"
      "function save() {"
      "  alert('Dados salvos!');"
      "}"
      "</script>"
      "</body>"
      "</html>";
  server.send(200, "text/html", html);
}

void setup() {
  pinMode(pinEMG, INPUT);
  Serial.begin(115200);

  // Conecta-se à rede WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }

  Serial.println("Conectado ao WiFi");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/emgvalue", HTTP_GET, []() {
    server.send(200, "text/plain", String(readEMGValue(), 4)); // Precisão de 4 casas decimais
  });

  server.begin();
  Serial.println("Servidor iniciado");
}

// Função principal que é executada repetidamente
void loop() {
    server.handleClient();
    if (millis() - lastUpdateTime >= acquisitionRate) { // Tempo em milissegundos
    lastUpdateTime = millis();
    if (updatingData) {
      float emgvalue = readEMGValue();
      server.send(200, "text/plain", String(emgvalue, 4)); // Precisão de 4 casas decimais
    }
  }
}
