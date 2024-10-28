import 'dart:async';
import 'package:flutter/material.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_database/firebase_database.dart';
import 'package:fl_chart/fl_chart.dart';
import 'package:http/http.dart' as http;

class PatientPage extends StatefulWidget {
  const PatientPage({super.key});

  @override
  _PatientPageState createState() => _PatientPageState();
}

class _PatientPageState extends State<PatientPage> {
  List<FlSpot> _dataPoints = [];
  Timer? _timer;
  bool _isAcquiring = false;
  final String esp32Url = 'http://192.168.3.20'; // URL do ESP32
  final double _systolicPressure = 0.0;
  final double _diastolicPressure = 0.0;
  final int maxDataPoints = 1000; // Permite até 1000 pontos no gráfico

  final DatabaseReference _databaseReference = FirebaseDatabase.instance.ref();

  @override
  void initState() {
    super.initState();
  }

  Future<void> _toggleAcquisition() async {
    if (_isAcquiring) {
      await _sendCommand('stopMeasurement');
      setState(() {
        _isAcquiring = false;
      });
    } else {
      await _sendCommand('startMeasurement');
      _startAcquisition();
      setState(() {
        _isAcquiring = true;
      });
    }
  }

  Future<void> _startAcquisition() async {
    const acquisitionRate = Duration(milliseconds: 50); // 50ms
    _timer = Timer.periodic(acquisitionRate, (timer) async {
      if (_isAcquiring) {
        final value = await _fetchVADCValue();
        setState(() {
          if (_dataPoints.length >= maxDataPoints) {
            _dataPoints.removeAt(0);
            _dataPoints = _dataPoints
                .asMap()
                .entries
                .map((entry) => FlSpot(entry.key.toDouble(), entry.value.y))
                .toList();
          }
          _dataPoints.add(FlSpot(_dataPoints.length.toDouble(), value));
        });
      }
    });
  }

  Future<double> _fetchVADCValue() async {
    final response = await http.get(Uri.parse('$esp32Url/vADCvalue'));
    if (response.statusCode == 200) {
      double valor = double.parse(response.body);
      print("Valor recebido do ESP32: $valor");
      return valor;
    } else {
      throw Exception('Falha ao buscar valor vADC');
    }
  }

  Future<void> _sendCommand(String endpoint) async {
    final response = await http.get(Uri.parse('$esp32Url/$endpoint'));
    if (response.statusCode != 200) {
      throw Exception('Falha ao executar comando $endpoint');
    }
  }

  Future<void> _savePressureToFirebase() async {
    User? user = FirebaseAuth.instance.currentUser;
    if (user != null) {
      await _databaseReference.child('users/pacientes').child(user.uid).update({
        'systolicPressure': _systolicPressure,
        'diastolicPressure': _diastolicPressure,
      });
    }
  }

  void _onMotorOnPressed() async {
    await _sendCommand('turnOnMotor');
  }

  void _onMotorOffPressed() async {
    await _sendCommand('turnOffMotor');
  }

  void _onClearBufferPressed() async {
    await _sendCommand('clearBuffer');
    setState(() {
      _dataPoints.clear();
    });
  }

  @override
  void dispose() {
    _timer?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Monitoramento de Pressão Arterial'),
        backgroundColor: Colors.teal,
      ),
      body: Column(
        children: [
          Expanded(
            child: Padding(
              padding: const EdgeInsets.all(16.0),
              child: LineChart(
                LineChartData(
                  gridData: FlGridData(show: true),
                  titlesData: FlTitlesData(show: true),
                  borderData: FlBorderData(show: true),
                  lineBarsData: [
                    LineChartBarData(
                      spots: _dataPoints,
                      isCurved: true,
                      color: Colors.blueAccent,
                      barWidth: 2,
                      dotData: FlDotData(show: false),
                    ),
                  ],
                  minX: _dataPoints.isNotEmpty ? _dataPoints.first.x : 0,
                  maxX: _dataPoints.isNotEmpty ? _dataPoints.last.x : 1000,
                  minY: 0,
                  maxY: 4100,
                ),
              ),
            ),
          ),
          const SizedBox(height: 10),
          Text(
            'Pressão Sistólica: $_systolicPressure mmHg',
            style: Theme.of(context).textTheme.headlineSmall,
          ),
          Text(
            'Pressão Diastólica: $_diastolicPressure mmHg',
            style: Theme.of(context).textTheme.headlineSmall,
          ),
          const SizedBox(height: 10),
          ElevatedButton(
            onPressed: _savePressureToFirebase,
            child: const Text('Salvar Pressão no Firebase'),
          ),
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceEvenly,
            children: [
              ElevatedButton(
                onPressed: _onMotorOnPressed,
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.green,
                ),
                child: const Text('Ligar Motor e Fechar Válvula'),
              ),
              ElevatedButton(
                onPressed: _onMotorOffPressed,
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.red,
                ),
                child: const Text('Desligar Motor e Abrir Válvula'),
              ),
              ElevatedButton(
                onPressed: _onClearBufferPressed,
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.orange,
                ),
                child: const Text('Limpar Buffer'),
              ),
            ],
          ),
          ElevatedButton(
            onPressed: _toggleAcquisition,
            style: ElevatedButton.styleFrom(
              backgroundColor: _isAcquiring ? Colors.red : Colors.blue,
            ),
            child: Text(_isAcquiring ? 'Parar Medição' : 'Iniciar Medição'),
          ),
        ],
      ),
    );
  }
}
