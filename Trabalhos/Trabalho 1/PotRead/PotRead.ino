#include <WiFi.h>
#include <WebServer.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Definição das credenciais de WiFi
const char *ssid = "Martins WiFi6";
const char *password = "17031998";

WebServer server(80);
const int bufferSize = 100;  // Número máximo de pontos no gráfico
float timeElapsed = 0;
int vADCBuffer[bufferSize];
int bufferIndex = 0;
bool updatingData = false;
unsigned long acquisitionRate = 500;
const int pinvADC = 33;

TaskHandle_t vADCTaskHandle = NULL;

// Função para ler o valor do sinal vADC
float readvADCValue() {
  int valorADC = analogRead(pinvADC);
  float tensao = ((valorADC * 3.3) / 4095); // Convertendo para volts
  return tensao;
}

void handleRoot() {
  String html =
      "<!DOCTYPE HTML>"
      "<html lang='en'>"
      "<head>"
      "<meta charset='UTF-8'>"
      "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
      "<title>Construção de um sistema de aquisição de dados</title>"
      "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"
      "<style>"
      "body {"
      "font-family: Arial, sans-serif;"
      "background-color: #f0f0f0;"
      "color: #333;"
      "display: flex;"
      "justify-content: center;"
      "align-items: center;"
      "height: 100vh;"
      "margin: 0;"
      "}"
      ".container {"
      "background-color: #ffffff;"
      "border-radius: 10px;"
      "box-shadow: 0 0 0 rgba(0, 0, 0, 0.1);"
      "text-align: center;"
      "padding: 20px;"
      "max-width: 600px;"
      "width: 100%;"
      "}"
      ".header {"
      "display: flex;"
      "justify-content: space-between;"
      "align-items: center;"
      "color: #333;"
      "}"
      ".header h2 {"
      "color: #000000;"
      "margin: 0;"
      "}"
      ".button-large {"
      "width: 120px;"
      "height: 120px;"
      "border-radius: 50%;"
      "background-color: #ffffff;"
      "border: 5px solid #000000;"
      "color: #000000;"
      "font-size: 24px;"
      "display: flex;"
      "justify-content: center;"
      "align-items: center;"
      "margin: 20px auto;"
      "box-shadow: 0 0 0 #000000, 0 0 0 #000000, 0 0 0 #000000;"
      "transition: box-shadow 0.2s ease;"
      "user-select: none;"
      "}"
      ".button-large:hover {"
      "box-shadow: 0 0 0 #000000, 0 0 0 #000000, 0 0 0 #000000;"
      "}"
      ".chart-container {"
      "position: relative;"
      "height: 300px;"
      "width: 100%;"
      "margin: 20px 0;"
      "border: 2px solid #000000;"
      "border-radius: 10px;"
      "padding: 10px;"
      "box-sizing: border-box;"
      "background-color: #fafafa;"
      "}"
      ".slider-container {"
      "display: flex;"
      "justify-content: center;"
      "align-items: center;"
      "margin: 20px 0;"
      "}"
      ".slider {"
      "width: 80%;"
      "-webkit-appearance: none;"
      "appearance: none;"
      "height: 10px;"
      "background: #ccc;"
      "outline: none;"
      "opacity: 0.7;"
      "transition: opacity 0.2s;"
      "border-radius: 5px;"
      "user-select: none;"
      "}"
      ".slider:hover {"
      "opacity: 1;"
      "}"
      ".slider::-webkit-slider-thumb {"
      "-webkit-appearance: none;"
      "appearance: none;"
      "width: 20px;"
      "height: 20px;"
      "background: #000000;"
      "cursor: pointer;"
      "border-radius: 50%;"
      "user-select: none;"
      "}"
      ".slider::-moz-range-thumb {"
      "width: 20px;"
      "height: 20px;"
      "background: #000000;"
      "cursor: pointer;"
      "border-radius: 50%;"
      "user-select: none;"
      "}"
      ".slider-value {"
      "margin-left: 10px;"
      "font-size: 18px;"
      "color: #000000;"
      "}"
      ".button-container {"
      "display: flex;"
      "justify-content: space-around;"
      "margin: 20px 0;"
      "}"
      ".button-start {"
      "width: 100px;"
      "height: 100px;"
      "border-radius: 50%;"
      "background-color: #ffffff;"
      "border: 10px solid #2196f3;"
      "display: flex;"
      "justify-content: center;"
      "align-items: center;"
      "color: #2196f3;"
      "font-size: 18px;"
      "cursor: pointer;"
      "transition: transform 0.1s ease, box-shadow 0.2s ease;"
      "box-shadow: 0 0 0 #2196f3, 0 0 0 #2196f3, 0 0 0 #2196f3;"
      "user-select: none;"
      "}"
      ".button-start:hover {"
      "transform: scale(1.05);"
      "box-shadow: 0 0 0 #2196f3, 0 0 0 #2196f3, 0 0 0 #2196f3;"
      "}"
      ".button-start:active {"
      "transform: scale(0.95);"
      "}"
      ".button-stop {"
      "width: 100px;"
      "height: 100px;"
      "border-radius: 50%;"
      "background-color: #ffffff;"
      "border: 10px solid #f44336;"
      "display: flex;"
      "justify-content: center;"
      "align-items: center;"
      "color: #f44336;"
      "font-size: 18px;"
      "cursor: pointer;"
      "transition: transform 0.1s ease, box-shadow 0.2s ease;"
      "box-shadow: 0 0 0 #f44336, 0 0 0 #f44336, 0 0 0 #f44336;"
      "user-select: none;"
      "}"
      ".button-stop:hover {"
      "transform: scale(1.05);"
      "box-shadow: 0 0 0 #f44336, 0 0 0 #f44336, 0 0 0 #f44336;"
      "}"
      ".button-stop:active {"
      "transform: scale(0.95);"
      "}"
      ".button {"
      "border-radius: 20px;"
      "padding: 10px 20px;"
      "margin: 10px auto;"
      "cursor: pointer;"
      "border: none;"
      "color: #fff;"
      "font-size: 16px;"
      "transition: background-color 0.1s ease, transform 0.1s ease, box-shadow 0.1s ease;"
      "display: inline-block;"
      "background-color: #2196f3;"
      "box-shadow: 0 0 0 #2196f3, 0 0 0 #2196f3, 0 0 0 #2196f3;"
      "user-select: none;"
      "}"
      ".button:hover {"
      "background-color: #1976d2;"
      "box-shadow: 0 0 0 #2196f3, 0 0 0 #2196f3, 0 0 0 #2196f3;"
      "}"
      ".button:active {"
      "transform: scale(0.95);"
      "}"
      "</style>"
      "</head>"
      "<body>"
      "<div class='container'>"
      "<div class='header'>"
      "<h2>Construção de um sistema de aquisição de dados</h2>"
      "</div>"
      "<div class='button-large'>"
      "<span id='currentValue'>Trabalho 1</span>"
      "</div>"
      "<div class='chart-container'>"
      "<canvas id='vADCChart'></canvas>"
      "</div>"
      "<div class='slider-container'>"
      "<input type='range' min='1' max='2000' value='500' class='slider' id='vADCRange' onchange='updateSliderValue(this.value)'>"
      "<span class='slider-value' id='sliderValue'>500</span>"
      "</div>"
      "<div class='button-container'>"
      "<div class='button-start' onclick='startAcquisition()'>Iniciar</div>"
      "<div class='button-stop' onclick='stopAcquisition()'>Parar</div>"
      "</div>"
      "</div>"
      "<script>"
      "var ctx = document.getElementById('vADCChart').getContext('2d');"
      "var vADCChart = new Chart(ctx, {"
      "type: 'line',"
      "data: {"
      "labels: [],"
      "datasets: [{"
      "label: 'Valor de Tensão',"
      "data: [],"
      "fill: false,"
      "borderColor: '#000000',"
      "backgroundColor: 'rgba(0, 0, 0, 0.2)',"
      "tension: 0.1,"
      "pointStyle: false," // Remover o estilo dos pontos
      "pointRadius: 0" // Remover os círculos em volta dos pontos
      "}]"
      "},"
      "options: {"
      "animation: false," // Desativando a animação
      "scales: {"
      "x: {"
      "display: false,"
      "},"
      "y: {"
      "suggestedMin: 0,"
      "suggestedMax: 3.3"
      "}"
      "}"
      "}"
      "});"

      "var intervalId;"
      "var updatingData = false;"
      "var acquisitionRate = 500;" // Valor inicial do slider
      "var timeElapsed = 0;"
      "var bufferSize = " + String(bufferSize) + ";"
      "var currentValueElement = document.getElementById('currentValue');"

      "function updateSliderValue(value) {"
      "document.getElementById('sliderValue').innerText = value;"
      "acquisitionRate = value;"
      "fetch('/updateAcquisitionRate?rate=' + value);"
      "}"

      "function startAcquisition() {"
      "if (!updatingData) {"
      "updatingData = true;"
      "fetch('/startAcquisition');"
      "intervalId = requestAnimationFrame(fetchData);"
      "}"
      "}"

      "function stopAcquisition() {"
      "if (updatingData) {"
      "updatingData = false;"
      "fetch('/stopAcquisition');"
      "cancelAnimationFrame(intervalId);"
      "clearChart();"
      "}"
      "}"

      "function fetchData() {"
      "if (updatingData) {"
      "fetch('/vADCvalue')"
      ".then(response => response.text())"
      ".then(data => {"
      "var vADCvalue = parseFloat(data);"
      "updateChart(vADCvalue);"
      "updateCurrentValue(vADCvalue);"
      "intervalId = setTimeout(fetchData, acquisitionRate);"
      "});"
      "}"
      "}"

      "function updateChart(vADCvalue) {"
      "vADCChart.data.labels.push(timeElapsed.toFixed(2));"
      "vADCChart.data.datasets[0].data.push(vADCvalue);"
      "timeElapsed += acquisitionRate / 1000;"
      "if (vADCChart.data.labels.length > bufferSize) {"
      "vADCChart.data.labels.shift();"
      "vADCChart.data.datasets[0].data.shift();"
      "}"
      "vADCChart.update();"
      "}"

      "function updateCurrentValue(vADCvalue) {"
      "currentValueElement.innerText = vADCvalue.toFixed(4) + ' V';"
      "}"

      "function clearChart() {"
      "vADCChart.data.labels = [];"
      "vADCChart.data.datasets[0].data = [];"
      "timeElapsed = 0;"
      "vADCChart.update();"
      "clearBuffer();"
      "}"

      "function clearBuffer() {"
      "fetch('/clearBuffer');"
      "}"
      "</script>"
      "</body>"
      "</html>";
  server.send(200, "text/html", html);
}

void vADCTask(void *pvParameters) {
  while (1) {
    if (updatingData) {
      float vADCvalue = readvADCValue();
      vADCBuffer[bufferIndex] = vADCvalue;
      bufferIndex = (bufferIndex + 1) % bufferSize;
      vTaskDelay(acquisitionRate / portTICK_PERIOD_MS);
    } else {
      vTaskDelay(100 / portTICK_PERIOD_MS);  // Aguarda 100ms antes de verificar novamente
    }
  }
}

void startAcquisition() {
  updatingData = true;
}

void stopAcquisition() {
  updatingData = false;
}

void clearBuffer() {
  memset(vADCBuffer, 0, sizeof(vADCBuffer));  // Limpa o buffer
  bufferIndex = 0;
}

void setup() {
  pinMode(pinvADC, INPUT);
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }

  Serial.println("Conectado ao WiFi");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/vADCvalue", HTTP_GET, []() {
    server.send(200, "text/plain", String(readvADCValue(), 4)); // Precisão de 4 casas decimais
  });
  server.on("/updateAcquisitionRate", HTTP_GET, []() {
    if (server.hasArg("rate")) {
      acquisitionRate = server.arg("rate").toInt();
      clearBuffer();  // Limpa o buffer quando a taxa de aquisição é alterada
    }
    server.send(200, "text/plain", "Taxa de aquisição atualizada e buffer limpo");
  });
  server.on("/startAcquisition", HTTP_GET, []() {
    startAcquisition();
    server.send(200, "text/plain", "Aquisição iniciada");
  });
  server.on("/stopAcquisition", HTTP_GET, []() {
    stopAcquisition();
    server.send(200, "text/plain", "Aquisição parada");
  });
  server.on("/clearBuffer", HTTP_GET, []() {
    clearBuffer();
    server.send(200, "text/plain", "Buffer limpo");
  });

  server.begin();
  Serial.println("Servidor iniciado");

  xTaskCreate(vADCTask, "vADCTask", 2048, NULL, 1, &vADCTaskHandle);
}

void loop() {
  server.handleClient();
}
