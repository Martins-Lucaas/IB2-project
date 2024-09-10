import 'dart:async';
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
        theme: ThemeData.dark().copyWith(
          primaryColor: Colors.teal,
          scaffoldBackgroundColor: Colors.black,
        ),
        home: const HomeScreen(),
      ),
    );
  }
}

class DataAcquisitionState extends ChangeNotifier {
  String ipAddress = '192.168.3.15'; // IP do ESP32
  List<double> irValues = [];
  List<double> redValues = [];
  List<double> timestamps = [];
  final int maxDataPoints = 100; // Limite de 100 pontos
  Timer? _dataTimer; // Timer para controlar a coleta de dados
  bool isFingerOnSensor = true; // Para verificar se o dedo está no sensor

  DataAcquisitionState() {
    fetchData();
  }

  void fetchData() {
    _dataTimer = Timer.periodic(const Duration(milliseconds: 50), (timer) async {
      try {
        final response = await http.get(Uri.parse('http://$ipAddress/dados'));
        if (response.statusCode == 200) {
          final data = response.body.split(',');

          double irValue = double.parse(data[0].split(':')[1]);
          double redValue = double.parse(data[1].split(':')[1]);
          double timestamp = DateTime.now().millisecondsSinceEpoch / 1000;

          irValues.add(irValue);
          redValues.add(redValue);
          timestamps.add(timestamp);

          // Remover os dados antigos quando o limite de pontos for ultrapassado
          if (irValues.length > maxDataPoints) {
            irValues.removeAt(0);
            redValues.removeAt(0);
            timestamps.removeAt(0);
          }

          // Verificar se o dedo está fora do sensor
          if (irValue < 40000 && redValue < 40000) {
            isFingerOnSensor = false;
            stopSystem(); // Parar o sistema se o dedo não estiver no sensor
          } else {
            isFingerOnSensor = true;
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

  // Função para parar o sistema
  void stopSystem() {
    if (_dataTimer != null) {
      _dataTimer!.cancel();
      _dataTimer = null;
      print("Sistema desligado: dedo fora do sensor.");
    }
  }

  // Função para calcular SpO2
  double calculateSpO2() {
    if (irValues.isEmpty || redValues.isEmpty) return 0;

    // Calculando valores AC e DC para IR e RED
    double acRed = redValues.reduce((a, b) => (b - a).abs()) / redValues.length;
    double dcRed = redValues.reduce((a, b) => a + b) / redValues.length;

    double acIr = irValues.reduce((a, b) => (b - a).abs()) / irValues.length;
    double dcIr = irValues.reduce((a, b) => a + b) / irValues.length;

    // Razão R
    double ratio = (acRed / dcRed) / (acIr / dcIr);

    // Estimar SpO2
    double spo2 = 110 - 25 * ratio;
    return spo2.clamp(0, 100); // Limitar SpO2 entre 0 e 100%
  }

  // Função para calcular BPM
  double calculateBPM() {
    if (timestamps.length < 2 || irValues.isEmpty) return 0;

    // Detectar picos no sinal IR
    List<int> peakIndexes = _detectPeaksAndValleys(irValues);

    if (peakIndexes.length < 2) return 0;

    // Calcular o intervalo de tempo entre picos consecutivos
    List<double> peakIntervals = [];
    for (int i = 1; i < peakIndexes.length; i++) {
      peakIntervals.add(timestamps[peakIndexes[i]] - timestamps[peakIndexes[i - 1]]);
    }

    // Média dos intervalos entre picos
    double avgPeakInterval = peakIntervals.reduce((a, b) => a + b) / peakIntervals.length;

    // Calcular BPM
    double bpm = 45 / avgPeakInterval;
    return bpm;
  }

  // Função para detectar picos e vales (melhorada)
  List<int> _detectPeaksAndValleys(List<double> data) {
    List<int> peakIndexes = [];
    bool isRising = false;

    for (int i = 1; i < data.length - 1; i++) {
      // Detectar um vale (mínimo local)
      if (data[i] < data[i - 1] && data[i] < data[i + 1]) {
        isRising = true;  // Após um vale, esperamos que o sinal suba
      }

      // Detectar um pico (máximo local), que só é considerado após um vale
      if (isRising && data[i] > data[i - 1] && data[i] > data[i + 1]) {
        peakIndexes.add(i);
        isRising = false;  // Após o pico, aguardamos novamente por um vale
      }
    }

    return peakIndexes;
  }

  // Função para calcular o valor mínimo dos dois sinais
  double getMinY() {
    if (irValues.isEmpty || redValues.isEmpty) return 0;
    return (irValues + redValues).reduce((a, b) => a < b ? a : b);
  }

  // Função para calcular o valor máximo dos dois sinais
  double getMaxY() {
    if (irValues.isEmpty || redValues.isEmpty) return 0;
    return (irValues + redValues).reduce((a, b) => a > b ? a : b);
  }
}

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  bool _showData = false;

  @override
  Widget build(BuildContext context) {
    final dataState = Provider.of<DataAcquisitionState>(context);

    double spo2 = dataState.calculateSpO2();

    // Determinar a cor do SpO₂ com base no valor
    Color spo2Color;
    if (spo2 >= 95) {
      spo2Color = Colors.green; // Bom nível de SpO₂
    } else {
      spo2Color = Colors.red; // Nível de SpO₂ baixo
    }

    return Scaffold(
      appBar: AppBar(
        title: const Text('Oxímetro de Pulso'),
        centerTitle: true,
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            ElevatedButton(
              onPressed: () {
                setState(() {
                  _showData = !_showData;
                });
              },
              child: Text(_showData ? 'Esconder Dados' : 'Mostrar Dados'),
            ),
            const SizedBox(height: 16),
            if (_showData)
              Column(
                children: [
                  const SizedBox(height: 16),
                  // Exibir SpO₂ com cor indicativa
                  Text(
                    'SpO₂: ${spo2.toStringAsFixed(2)}%',
                    style: TextStyle(fontSize: 24, color: spo2Color),
                  ),
                  // Exibir BPM
                  Text(
                    'BPM: ${dataState.calculateBPM().toStringAsFixed(2)}',
                    style: const TextStyle(fontSize: 24),
                  ),
                  const SizedBox(height: 16),
                  // Exibir mensagem se o dedo não estiver no sensor
                  if (!dataState.isFingerOnSensor)
                    const Text(
                      'Dedo fora do sensor! Sistema desligado.',
                      style: TextStyle(fontSize: 18, color: Colors.red),
                    ),
                  if (dataState.isFingerOnSensor)
                    SizedBox(
                      height: 200,
                      child: LineChart(LineChartData(
                        minY: dataState.getMinY(), // Amplitude mínima dinâmica
                        maxY: dataState.getMaxY(), // Amplitude máxima dinâmica
                        lineBarsData: [
                          LineChartBarData(
                            spots: List.generate(dataState.irValues.length,
                                (index) => FlSpot(dataState.timestamps[index], dataState.irValues[index])),
                            isCurved: true,
                            color: Colors.red,
                            barWidth: 2,
                            belowBarData: BarAreaData(show: false),
                            dotData: FlDotData(show: false), // Desabilitar pontos no gráfico
                          ),
                          LineChartBarData(
                            spots: List.generate(dataState.redValues.length,
                                (index) => FlSpot(dataState.timestamps[index], dataState.redValues[index])),
                            isCurved: true,
                            color: Colors.blue,
                            barWidth: 2,
                            belowBarData: BarAreaData(show: false),
                            dotData: FlDotData(show: false), // Desabilitar pontos no gráfico
                          ),
                        ],
                      )),
                    ),
                ],
              ),
          ],
        ),
      ),
    );
  }
}
