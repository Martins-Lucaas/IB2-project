import 'dart:async';
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'package:fl_chart/fl_chart.dart';

class PatientPage extends StatefulWidget {
  const PatientPage({super.key});

  @override
  _PatientPageState createState() => _PatientPageState();
}

class _PatientPageState extends State<PatientPage> {
  List<FlSpot> _dataPoints = [];
  Timer? _timer;
  bool _isAcquiring = false;
  final String esp32Url = 'http://192.168.3.20'; // Substitua pelo IP do seu ESP32

  @override
  void initState() {
    super.initState();
    _startAcquisition();
  }

  Future<void> _startAcquisition() async {
    const acquisitionRate = Duration(milliseconds: 50); // 50ms
    _timer = Timer.periodic(acquisitionRate, (timer) async {
      if (_isAcquiring) {
        final value = await _fetchVADCValue();
        setState(() {
          if (_dataPoints.length > 100) _dataPoints.removeAt(0);
          _dataPoints.add(FlSpot(_dataPoints.length.toDouble(), value));
        });
      }
    });
    _isAcquiring = true;
  }

  Future<double> _fetchVADCValue() async {
    final response = await http.get(Uri.parse('$esp32Url/vADCvalue'));
    if (response.statusCode == 200) {
      return double.parse(response.body);
    } else {
      throw Exception('Failed to fetch vADC value');
    }
  }

  Future<void> _sendCommand(String endpoint) async {
    final response = await http.get(Uri.parse('$esp32Url/$endpoint'));
    if (response.statusCode != 200) {
      throw Exception('Failed to execute $endpoint command');
    }
  }

  void _onStartPressed() async {
    await _sendCommand('startAcquisition');
    setState(() {
      _isAcquiring = true;
    });
  }

  void _onStopPressed() async {
    await _sendCommand('stopAcquisition');
    setState(() {
      _isAcquiring = false;
    });
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
        title: const Text('Monitoramento de vADC'),
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
                      barWidth: 4,
                      dotData: FlDotData(show: false),
                    ),
                  ],
                  minX: 0,
                  maxX: 100,
                  minY: 0,
                  maxY: 3.3,
                ),
              ),
            ),
          ),
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceEvenly,
            children: [
              ElevatedButton(
                onPressed: _onStartPressed,
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.green,
                ),
                child: const Text('Iniciar'),
              ),
              ElevatedButton(
                onPressed: _onStopPressed,
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.red,
                ),
                child: const Text('Parar'),
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
        ],
      ),
    );
  }
}
