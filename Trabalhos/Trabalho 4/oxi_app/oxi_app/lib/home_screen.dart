import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'dart:async';
import 'dart:convert';
import 'package:provider/provider.dart';
import 'data_acquisition_state.dart';
import 'chart_widget.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  _HomeScreenState createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  Timer? _timer;

  @override
  void initState() {
    super.initState();
    _timer = Timer.periodic(const Duration(milliseconds: 1000), (timer) {
      fetchData();
    });
  }

  @override
  void dispose() {
    _timer?.cancel();
    super.dispose();
  }

  Future<void> fetchData() async {
    final state = Provider.of<DataAcquisitionState>(context, listen: false);
    try {
      // Fetch SpO2 data
      final spo2Response = await http.get(Uri.parse('http://${state.esp32Ip}/spo2'));
      if (spo2Response.statusCode == 200) {
        final spo2Value = double.tryParse(spo2Response.body.trim()) ?? 0.0;
        state.updateSpo2(spo2Value);
      }

      // Fetch BPM data
      final bpmResponse = await http.get(Uri.parse('http://${state.esp32Ip}/bpm'));
      if (bpmResponse.statusCode == 200) {
        final bpmValue = int.tryParse(bpmResponse.body.trim()) ?? 0;
        state.updateBpm(bpmValue);
      }

      // Fetch Graph data
      final graphResponse = await http.get(Uri.parse('http://${state.esp32Ip}/graph'));
      if (graphResponse.statusCode == 200) {
        final graphData = List<double>.from(jsonDecode(graphResponse.body).map((e) => e.toDouble()));
        state.updateGraphData(graphData);
      }
    } catch (e) {
      print('Erro ao fazer a requisição HTTP: $e');
    }
  }

  @override
  Widget build(BuildContext context) {
    final state = Provider.of<DataAcquisitionState>(context);

    return Scaffold(
      backgroundColor: Colors.black,
      appBar: AppBar(
        title: const Text('Oxímetro de Pulso'),
        backgroundColor: Colors.teal,
      ),
      body: Center(
        child: SingleChildScrollView(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Column(
                children: [
                  const Text(
                    'Saturação de Oxigênio (SpO2)',
                    style: TextStyle(
                      color: Colors.white,
                      fontSize: 24,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  const SizedBox(height: 10),
                  Text(
                    '${state.spo2}%',
                    style: TextStyle(
                      color: state.spo2 >= 95 ? Colors.green : Colors.red,
                      fontSize: 48,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 50),
              Column(
                children: [
                  const Text(
                    'Batimentos por Minuto (BPM)',
                    style: TextStyle(
                      color: Colors.white,
                      fontSize: 24,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  const SizedBox(height: 10),
                  Text(
                    '${state.bpm} BPM',
                    style: TextStyle(
                      color: state.bpm >= 60 && state.bpm <= 100 ? Colors.green : Colors.red,
                      fontSize: 48,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 50),
              const ChartWidget(), // Adiciona o gráfico
              const SizedBox(height: 20),
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceAround,
                children: [
                  ElevatedButton(
                    onPressed: () {
                      state.startAcquisition();
                    },
                    style: ElevatedButton.styleFrom(backgroundColor: Colors.yellow),
                    child: const Text('Iniciar Aquisição'),
                  ),
                  ElevatedButton(
                    onPressed: () {
                      state.stopAcquisition();
                    },
                    style: ElevatedButton.styleFrom(backgroundColor: Colors.red),
                    child: const Text('Parar Aquisição'),
                  ),
                ],
              ),
              const SizedBox(height: 20),
              Slider(
                value: state.timeDivisions.toDouble(),
                min: 10,
                max: 1000,
                divisions: 9,
                label: '${state.timeDivisions} ms/div',
                onChanged: (double value) {
                  state.setTimeDivisions(value.toInt());
                },
              ),
              const SizedBox(height: 10),
              if (state.frequency > 0)
                Text(
                  'Frequência: ${state.frequency.toStringAsFixed(2)} Hz',
                  style: const TextStyle(fontSize: 16, color: Colors.white),
                ),
            ],
          ),
        ),
      ),
    );
  }
}
