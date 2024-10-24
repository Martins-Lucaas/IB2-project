import 'dart:async';
import 'dart:math';
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'package:provider/provider.dart';
import 'package:fl_chart/fl_chart.dart'; // Para exibir gráficos no Flutter

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
      create: (context) => DataAcquisitionState(),
      child: MaterialApp(
        debugShowCheckedModeBanner: false,
        title: 'Oxímetro de Pulso',
        theme: ThemeData(
          brightness: Brightness.light,
          primaryColor: Colors.teal,
          scaffoldBackgroundColor: Colors.white,
          textTheme: const TextTheme(
            bodyMedium: TextStyle(fontSize: 16.0, fontFamily: 'Roboto'),
          ),
        ),
        home: const HomeScreen(),
      ),
    );
  }
}

class DataAcquisitionState extends ChangeNotifier {
  String ipAddress = '192.168.3.20'; // IP do ESP32
  List<double> irValues = []; // Sinal IR
  List<double> redValues = []; // Sinal Red
  List<double> timestamps = []; // Armazenar os timestamps
  final int maxDataPoints = 100; // Limite de 100 pontos
  double currentBPM = 0; // Armazenar o valor atual de BPM
  double currentSpO2 = 0; // Armazenar o valor atual de SpO2
  bool sensorWarning = false; // Indica se o sensor está com problema

  DataAcquisitionState() {
    fetchData();
    calculatePeriodicBPMAndSpO2(); // Iniciar o cálculo periódico de BPM e SpO2
  }

  Future<void> fetchData() async {
    Timer.periodic(const Duration(milliseconds: 50), (timer) async {
      try {
        final response = await http.get(Uri.parse('http://$ipAddress/dados'));
        if (response.statusCode == 200) {
          final data = response.body.split(',');

          // Parse dos valores IR e Red
          double irValue = double.parse(data[0].split(':')[1]);
          double redValue = double.parse(data[1].split(':')[1]);
          double timestamp = DateTime.now().millisecondsSinceEpoch / 1000;

          irValues.add(irValue);
          redValues.add(redValue);
          timestamps.add(timestamp);

          // Verificar se os valores estão abaixo de 40.000
          if (irValue < 40000 || redValue < 40000) {
            sensorWarning = true; // Ativar o aviso de sensor
          } else {
            sensorWarning = false; // Desativar o aviso se os valores forem normais
          }

          // Remover os dados antigos quando o limite de pontos for ultrapassado
          if (irValues.length > maxDataPoints) {
            irValues.removeAt(0);
            redValues.removeAt(0);
            timestamps.removeAt(0);
          }

          notifyListeners();
        } else {
          print('Erro na resposta HTTP: ${response.statusCode}');
        }
      } catch (e) {
        print('Erro ao fazer requisição HTTP: $e');
      }
    });
  }

  // Função para calcular o BPM e SpO2 a cada 10 segundos
  void calculatePeriodicBPMAndSpO2() {
    Timer.periodic(const Duration(seconds: 5), (timer) {
      if (irValues.isNotEmpty && redValues.isNotEmpty) {
        currentBPM = calculateBPM();
        currentSpO2 = calculateSpO2();
        notifyListeners(); // Atualizar os valores exibidos na interface
      }
    });
  }

  // Função para calcular o BPM com base nos picos do sinal IR
  double calculateBPM() {
    if (timestamps.length < 2) return 0;

    // Detectar picos no sinal IR para identificar batimentos
    List<int> peakIndices = [];
    for (int i = 1; i < irValues.length - 1; i++) {
      if (irValues[i] > irValues[i - 1] &&
          irValues[i] > irValues[i + 1] &&
          irValues[i] > 40000) {
        peakIndices.add(i);
      }
    }

    // Calcular BPM com base nos intervalos de tempo entre os picos
    if (peakIndices.length >= 2) {
      List<double> intervals = [];
      for (int i = 1; i < peakIndices.length; i++) {
        double interval = timestamps[peakIndices[i]] - timestamps[peakIndices[i - 1]];
        intervals.add(interval);
      }

      double averageInterval = intervals.reduce((a, b) => a + b) / intervals.length;
      double bpm = 60 / averageInterval; // Converter intervalo para BPM
      return bpm;
    } else {
      return 0; // Não há batimentos suficientes para calcular BPM
    }
  }

  // Função para calcular o SpO2
  double calculateSpO2() {
    if (irValues.isEmpty || redValues.isEmpty) return 0;

    // Calcular a razão R = (AC_red/DC_red) / (AC_ir/DC_ir)
    double irAC = irValues.reduce((a, b) => max(a - irValues[0], b - irValues[0]));
    double irDC = irValues.reduce((a, b) => a + b) / irValues.length;

    double redAC = redValues.reduce((a, b) => max(a - redValues[0], b - redValues[0]));
    double redDC = redValues.reduce((a, b) => a + b) / redValues.length;

    if (irDC == 0 || redDC == 0) return 0;

    double R = (redAC / redDC) / (irAC / irDC);

    // Fórmula aproximada para calcular SpO2 a partir de R
    double spO2 = 110 - 25 * R;
  
    // Ajuste: Limitar o SpO2 entre 95% e 100%
    return spO2.clamp(95, 100); // Modificação aqui para garantir valores entre 95 e 100
  }

  // Normalizar os valores do sinal entre 0 e 100 para o gráfico
  List<double> normalizeValues(List<double> values) {
    if (values.isEmpty) return [];

    double minValue = values.reduce((a, b) => a < b ? a : b);
    double maxValue = values.reduce((a, b) => a > b ? a : b);

    if (maxValue == minValue) return List<double>.filled(values.length, 50.0);

    return values.map((value) {
      return 100 * (value - minValue) / (maxValue - minValue);
    }).toList();
  }

  List<double> get normalizedIrValues => normalizeValues(irValues);
  List<double> get normalizedRedValues => normalizeValues(redValues);
}

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  @override
  Widget build(BuildContext context) {
    final dataState = Provider.of<DataAcquisitionState>(context);

    return Scaffold(
      appBar: AppBar(
        title: const Text('Oxímetro de Pulso'),
        backgroundColor: Colors.teal[600],
        centerTitle: true,
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Container(
              decoration: BoxDecoration(
                borderRadius: BorderRadius.circular(16),
                color: Colors.teal.withOpacity(0.1),
                boxShadow: [
                  BoxShadow(
                    color: Colors.black12,
                    blurRadius: 8.0,
                  ),
                ],
              ),
              child: SizedBox(
                height: 200,
                child: Padding(
                  padding: const EdgeInsets.all(8.0),
                  child: LineChart(
                    LineChartData(
                      minY: 0,  // Limitar o valor mínimo do eixo Y a 0
                      maxY: 100,  // Limitar o valor máximo do eixo Y a 100
                      titlesData: const FlTitlesData(
                        show: true,
                        bottomTitles: AxisTitles(
                          sideTitles: SideTitles(showTitles: false), // Remover completamente as legendas do eixo X
                        ),
                        leftTitles: AxisTitles(
                        ),
                        topTitles: AxisTitles(
                          sideTitles: SideTitles(showTitles: false), // Remover qualquer título superior
                        ),
                        rightTitles: AxisTitles(
                        ),
                      ),
                      lineBarsData: [
                        // Gráfico do sinal IR
                        LineChartBarData(
                          spots: List.generate(
                            dataState.normalizedIrValues.length,
                            (index) => FlSpot(
                              dataState.timestamps[index],
                              dataState.normalizedIrValues[index],
                            ),
                          ),
                          isCurved: true,
                          color: Colors.red, // Sinal IR em vermelho
                          barWidth: 3,
                          belowBarData: BarAreaData(show: false),
                          dotData: FlDotData(show: false), // Desabilitar pontos no gráfico
                        ),
                        // Gráfico do sinal Red
                        LineChartBarData(
                          spots: List.generate(
                            dataState.normalizedRedValues.length,
                            (index) => FlSpot(
                              dataState.timestamps[index],
                              dataState.normalizedRedValues[index],
                            ),
                          ),
                          isCurved: true,
                          color: Colors.blue, // Sinal Red em azul
                          barWidth: 3,
                          belowBarData: BarAreaData(show: false),
                          dotData: FlDotData(show: false), // Desabilitar pontos no gráfico
                        ),
                      ],
                    ),
                  ),
                ),
              ),
            ),
            const SizedBox(height: 20),
            
            // Exibir mensagem se o sensor não detectar o dedo
            if (dataState.sensorWarning)
              const Text(
                'Por favor, coloque o dedo sobre o sensor.',
                style: TextStyle(
                  fontSize: 20,
                  color: Colors.red,
                  fontWeight: FontWeight.bold,
                ),
              ),
            const SizedBox(height: 20),
            Text(
              'BPM: ${dataState.currentBPM.toStringAsFixed(2)}',
              style: const TextStyle(
                fontSize: 26,
                fontWeight: FontWeight.bold,
                color: Colors.teal,
              ),
            ),
            const SizedBox(height: 10),
            Text(
              'SpO2: ${dataState.currentSpO2.toStringAsFixed(2)}%',
              style: const TextStyle(
                fontSize: 26,
                fontWeight: FontWeight.bold,
                color: Colors.teal,
              ),
            ),
          ],
        ),
      ),
    );
  }
}
