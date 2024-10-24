#include <WiFi.h>
#include <WebServer.h>

// Definição das credenciais de WiFi
const char *ssid = "Martins WiFi6";
const char *password = "17031998";

WebServer server(80);
const int bufferSize = 100;  // Número máximo de pontos no gráfico
float timeElapsed = 0;
int vADCBuffer[bufferSize];
int bufferIndex = 0;
bool updatingData = false;
unsigned long lastUpdateTime = 0;
unsigned long acquisitionRate = 500;
const int pinvADC = 33;
/* hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile bool flag = false; 
// Função para ler o valor do sinal vADC
float readvADCValue() {
  int valorADC = analogRead(pinvADC);
  float tensao = ((valorADC * 3.3) / 4095); // Convertendo para volts
  return tensao;
}

//Função para ser chamada pelo hardware do temporizador quando o temporizador estoura
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux); //Entra na seção critica
  flag = true; //Sinaliza que a interrupção ocorreu
  portEXIT_CRITICAL_ISR(&timerMux); //Sai da seção crítica
}*/

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
      "background-color: #0d0d0d;"
      "color: #e0e0e0;"
      "display: flex;"
      "justify-content: center;"
      "align-items: center;"
      "height: 100vh;"
      "margin: 0;"
      "}"
      ".container {"
      "background-color: #1a1a1a;"
      "border-radius: 10px;"
      "box-shadow: 0 0 15px rgba(0, 255, 255, 0.2);"
      "text-align: center;"
      "padding: 20px;"
      "max-width: 600px;"
      "width: 100%;"
      "}"
      ".header {"
      "display: flex;"
      "justify-content: space-between;"
      "align-items: center;"
      "color: #e0e0e0;"
      "}"
      ".header h2 {"
      "color: #00e676;"
      "margin: 0;"
      "}"
      ".button-large {"
      "width: 120px;"
      "height: 120px;"
      "border-radius: 50%;"
      "background-color: #000;"
      "border: 5px solid #00e676;"
      "color: #00e676;"
      "font-size: 24px;"
      "display: flex;"
      "justify-content: center;"
      "align-items: center;"
      "margin: 20px auto;"
      "box-shadow: 0 0 20px #00e676, 0 0 0 #00e676, 0 0 0 #00e676;"
      "transition: box-shadow 0.2s ease;"
      "user-select: none;"
      "}"
      ".button-large:hover {"
      "box-shadow: 0 0 30px #00e676, 0 0 0 #00e676, 0 0 0 #00e676;"
      "}"
      ".chart-container {"
      "position: relative;"
      "height: 300px;"
      "width: 100%;"
      "margin: 20px 0;"
      "border: 2px solid #00e676;"
      "border-radius: 10px;"
      "padding: 10px;"
      "box-sizing: border-box;"
      "background-color: #222;"
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
      "background: #333;"
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
      "background: #00e676;"
      "cursor: pointer;"
      "border-radius: 50%;"
      "user-select: none;"
      "}"
      ".slider::-moz-range-thumb {"
      "width: 20px;"
      "height: 20px;"
      "background: #00e676;"
      "cursor: pointer;"
      "border-radius: 50%;"
      "user-select: none;"
      "}"
      ".slider-value {"
      "margin-left: 10px;"
      "font-size: 18px;"
      "color: #00e676;"
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
      "background-color: #333;"
      "border: 10px solid #00e676;"
      "display: flex;"
      "justify-content: center;"
      "align-items: center;"
      "color: #00e676;"
      "font-size: 18px;"
      "cursor: pointer;"
      "transition: transform 0.1s ease, box-shadow 0.2s ease;"
      "box-shadow: 0 0 20px #00e676, 0 0 0 #00e676, 0 0 0 #00e676;"
      "user-select: none;"
      "}"
      ".button-start:hover {"
      "transform: scale(1.05);"
      "box-shadow: 0 0 30px #00e676, 0 0 0 #00e676, 0 0 0 #00e676;"
      "}"
      ".button-start:active {"
      "transform: scale(0.95);"
      "}"
      ".button-stop {"
      "width: 100px;"
      "height: 100px;"
      "border-radius: 50%;"
      "background-color: #333;"
      "border: 10px solid #f44336;"
      "display: flex;"
      "justify-content: center;"
      "align-items: center;"
      "color: #f44336;"
      "font-size: 18px;"
      "cursor: pointer;"
      "transition: transform 0.1s ease, box-shadow 0.2s ease;"
      "box-shadow: 0 0 20px #f44336, 0 0 0 #f44336, 0 0 0 #f44336;"
      "user-select: none;"
      "}"
      ".button-stop:hover {"
      "transform: scale(1.05);"
      "box-shadow: 0 0 30px #f44336, 0 0 0 #f44336, 0 0 0 #f44336;"
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
      "transition: background-color 0.1s ease, transform 0.1s ease, box-shadow 0.2s ease;"
      "display: inline-block;"
      "background-color: #2196f3;"
      "box-shadow: 0 0 20px #2196f3, 0 0 0 #2196f3, 0 0 0 #2196f3;"
      "user-select: none;"
      "}"
      ".button:hover {"
      "background-color: #1976d2;"
      "box-shadow: 0 0 30px #2196f3, 0 0 0 #2196f3, 0 0 0 #2196f3;"
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
      "<input type='range' min='1' max='2000' value='500' class='slider' id='vADCRange' oninput='updateSliderValue(this.value)'>"
      "<span class='slider-value' id='sliderValue'>500</span>"
      "</div>"
      "<div class='button-container'>"
      "<div class='button-start' onclick='startAcquisition()'>Iniciar</div>"
      "<div class='button-stop' onclick='stopAcquisition()'>Parar</div>"
      "</div>"
      "<button class='button' onclick='save()'>Salvar</button>"
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
      "borderColor: '#00e676',"
      "backgroundColor: 'rgba(0, 230, 118, 0.2)',"
      "tension: 0.1"
      "}]"
      "},"
      "options: {"
      "animation: {"
      "duration: 0"
      "},"
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
      "if (updatingData) {"
      "clearInterval(intervalId);"
      "intervalId = setInterval(fetchData, acquisitionRate);"
      "}"
      "}"

      "function startAcquisition() {"
      "if (!updatingData) {"
      "intervalId = setInterval(fetchData, acquisitionRate);"
      "updatingData = true;"
      "}"
      "}"

      "function stopAcquisition() {"
      "clearInterval(intervalId);"
      "updatingData = false;"
      "clearChart();"
      "}"

      "function save() {"
      "alert('Dados salvos!');"
      "}"

      "function fetchData() {"
      "fetch('/vADCvalue')"
      ".then(response => response.text())"
      ".then(data => {"
      "var vADCvalue = parseFloat(data);"
      "updateChart(vADCvalue);"
      "updateCurrentValue(vADCvalue);"
      "});"
      "}"

      "function updateChart(vADCvalue) {"
      "vADCChart.data.labels.push(timeElapsed);"
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
      "}"
      "</script>"
      "</body>"
      "</html>";
  server.send(200, "text/html", html);
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

  server.begin();
  Serial.println("Servidor iniciado");
  //Configurando o temporizador do hardware
  timer = timerBegin(0, 80, true); //Timer 0, prescaler 80
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, acquisitionRate, true); 
  timerAlarmEnable(timer).
}

void loop() {
  server.handleClient();
  if (updatingData && (millis() - lastUpdateTime >= acquisitionRate)) {
    lastUpdateTime = millis();
    float vADCvalue = readvADCValue();
    server.send(200, "text/plain", String(vADCvalue, 4));
  }
  // Verifica a flag da interrupção do temporizador
  /*if(flag){
    portENTER_CRITICAL(&timerMux);
    flag=false;
    portEXIT_CRITICAL(&timerMux);
    float vADCvalue = readvADCValue(); 
  }*/
}
