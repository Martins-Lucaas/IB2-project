import 'dart:async';
import 'dart:math';
import 'package:flutter/material.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:fl_chart/fl_chart.dart';
import 'package:http/http.dart' as http;
import 'package:firebase_database/firebase_database.dart';
import 'package:intl/intl.dart';
import 'RecentMeasurementsPage.dart';
import 'UserProfilePage.dart';

class PatientPage extends StatefulWidget {
  const PatientPage({super.key});

  @override
  _PatientPageState createState() => _PatientPageState();
}

class _PatientPageState extends State<PatientPage> {
  List<FlSpot> _chartDataPoints = [];
  Timer? _dataAcquisitionTimer;
  bool _isMonitoring = false;
  double _upperPressure = 0.0;
  double _lowerPressure = 0.0;
  bool _upperPressureDetected = false;
  bool _lowerPressureDetected = false;
  bool _isReducing = false;
  final String deviceUrl = 'http://192.168.3.26';
  final int maxChartDataPoints = 1000;
  final FirebaseAuth _auth = FirebaseAuth.instance;
  final DatabaseReference _database = FirebaseDatabase.instance.ref();

  @override
  void initState() {
    super.initState();
  }

  Future<void> _toggleMonitoring() async {
    if (_isMonitoring) {
      await _stopDataAcquisition();
    } else {
      await _startDataAcquisition();
    }
  }

  Future<void> _startDataAcquisition() async {
    await _sendCommandToDevice('startMeasurement'); // Envia comando para ligar o motor
    _beginAcquisition();
    setState(() {
      _isMonitoring = true;
      _upperPressureDetected = false;
      _lowerPressureDetected = false;
      _upperPressure = 0.0;
      _lowerPressure = 0.0;
      _isReducing = false;
    });
  }

  Future<void> _stopDataAcquisition() async {
    await _sendCommandToDevice('stopMeasurement'); // Envia comando para desligar o motor
    _dataAcquisitionTimer?.cancel();
    setState(() {
      _isMonitoring = false;
    });
    _storePressureData();
  }

  Future<void> _beginAcquisition() async {
    const acquisitionRate = Duration(milliseconds: 100);
    int step = 0;
    bool motorDesligado = false; // Flag para monitorar o desligamento do motor
    _dataAcquisitionTimer = Timer.periodic(acquisitionRate, (timer) async {
      double pressureValue = _generatePressureData(step);
      setState(() {
        if (_chartDataPoints.length >= maxChartDataPoints) {
          _chartDataPoints.removeAt(0);
        }
        _chartDataPoints.add(FlSpot(_chartDataPoints.length.toDouble(), pressureValue));

        _identifyPressures(pressureValue, step);
      });

      if (_isReducing && !motorDesligado) {
        await _sendCommandToDevice('stopMeasurement'); // Desliga o motor ao detectar a redução
        motorDesligado = true;
      }

      if (_isReducing && pressureValue <= 30) {
        _stopDataAcquisition();
      }
      step++;
    });
  }

  void _identifyPressures(double currentValue, int step) {
    if (_isReducing) {
      if (!_upperPressureDetected && currentValue >= 110 && currentValue <= 140) {
        _upperPressure = double.parse(currentValue.toStringAsFixed(2));
        _upperPressureDetected = true;
      }

      if (_upperPressureDetected && !_lowerPressureDetected && currentValue >= 70 && currentValue <= 100) {
        _lowerPressure = double.parse(currentValue.toStringAsFixed(2));
        _lowerPressureDetected = true;
      }
    }
  }

  double _generatePressureData(int step) {
    double time = step / 10.0;
    double randomVariation = Random().nextDouble() * 10;

    if (time <= 12 && !_isReducing) {
      double value = min((time / 12) * 300 + randomVariation, 300);
      if (value >= 300) _isReducing = true;
      return value;
    } else {
      double decayTime = time - 12;
      double oscillationIntensity = decayTime > 6 ? 15 : 8;
      randomVariation = (Random().nextDouble() * 20 - 10);
      return max(
        300 * exp(-0.12 * decayTime) + oscillationIntensity * sin(decayTime * pi / 2.0) + randomVariation,
        30,
      );
    }
  }

  Future<void> _sendCommandToDevice(String endpoint) async {
    try {
      final response = await http.get(Uri.parse('$deviceUrl/$endpoint'));
      if (response.statusCode != 200) {
        throw Exception('Failed to execute command $endpoint');
      }
    } catch (e) {
      print("Error sending command: $e");
    }
  }

  void _storePressureData() {
    if (_upperPressureDetected && _lowerPressureDetected) {
      DateTime now = DateTime.now();
      String formattedDate = DateFormat('dd/MM/yyyy - HH:mm').format(now);
      String userId = _auth.currentUser!.uid;

      _database.child('users').child('patients').child(userId).child('monitoramento').push().set({
        'systolicPressure': _upperPressure.toStringAsFixed(2),
        'diastolicPressure': _lowerPressure.toStringAsFixed(2),
        'date': formattedDate,
      });
    }
  }

  void _displayUserOptions() {
    showDialog(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(20.0)),
          backgroundColor: Colors.black87,
          title: const Text(
            'User Options',
            style: TextStyle(color: Colors.cyanAccent),
          ),
          content: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              ListTile(
                leading: const Icon(Icons.person, color: Colors.cyanAccent),
                title: const Text('Perfil', style: TextStyle(color: Colors.white)),
                onTap: () {
                  Navigator.of(context).pop();
                  Navigator.of(context).push(
                    MaterialPageRoute(
                      builder: (context) => const UserProfilePage(),
                    ),
                  );
                },
              ),
              ListTile(
                leading: const Icon(Icons.history, color: Colors.cyanAccent),
                title: const Text('Leituras Recentes', style: TextStyle(color: Colors.white)),
                onTap: () {
                  Navigator.of(context).pop();
                  Navigator.of(context).push(
                    MaterialPageRoute(
                      builder: (context) => const RecentMeasurementsPage(),
                    ),
                  );
                },
              ),
              ListTile(
                leading: const Icon(Icons.logout, color: Colors.redAccent),
                title: const Text('Logout', style: TextStyle(color: Colors.white)),
                onTap: () async {
                  await _auth.signOut();
                  Navigator.of(context).pop();
                  Navigator.of(context).pushReplacementNamed('/login');
                },
              ),
            ],
          ),
        );
      },
    );
  }

  @override
  void dispose() {
    _dataAcquisitionTimer?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.black,
      appBar: AppBar(
        backgroundColor: Colors.black,
        elevation: 0,
        centerTitle: true,
        title: const Text(
          'Blood Pressure Monitoring',
          style: TextStyle(
            color: Colors.cyanAccent,
            fontSize: 35,
            fontWeight: FontWeight.bold,
          ),
          textAlign: TextAlign.center,
        ),
        actions: [
          Padding(
            padding: const EdgeInsets.only(right: 16.0),
            child: GestureDetector(
              onTap: _displayUserOptions,
              child: CircleAvatar(
                backgroundColor: Colors.cyanAccent,
                radius: 20,
                child: const Icon(Icons.person, color: Colors.black),
              ),
            ),
          ),
        ],
      ),
      body: Column(
        children: [
          Expanded(
            child: Padding(
              padding: const EdgeInsets.symmetric(horizontal: 16.0, vertical: 8.0),
              child: Card(
                color: Colors.black87,
                shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20.0)),
                elevation: 8,
                child: Padding(
                  padding: const EdgeInsets.all(16.0),
                  child: LineChart(
                    LineChartData(
                      gridData: FlGridData(
                        show: true,
                        drawVerticalLine: true,
                        horizontalInterval: 50,
                        getDrawingHorizontalLine: (value) {
                          return FlLine(
                            color: Colors.white24,
                            strokeWidth: 1,
                          );
                        },
                        getDrawingVerticalLine: (value) {
                          return FlLine(
                            color: Colors.white24,
                            strokeWidth: 1,
                          );
                        },
                      ),
                      titlesData: FlTitlesData(
                        show: true,
                        leftTitles: AxisTitles(
                          sideTitles: SideTitles(
                            showTitles: true,
                            reservedSize: 40,
                            getTitlesWidget: (value, meta) => Text(
                              value.toString(),
                              style: const TextStyle(color: Colors.white),
                            ),
                          ),
                        ),
                        bottomTitles: AxisTitles(
                          sideTitles: SideTitles(
                            showTitles: true,
                            getTitlesWidget: (value, meta) => Text(
                              value.toString(),
                              style: const TextStyle(color: Colors.white),
                            ),
                          ),
                        ),
                      ),
                      borderData: FlBorderData(
                        show: true,
                        border: Border.all(color: Colors.white24),
                      ),
                      lineBarsData: [
                        LineChartBarData(
                          spots: _chartDataPoints,
                          isCurved: true,
                          color: Colors.cyanAccent,
                          barWidth: 2,
                          belowBarData: BarAreaData(
                              show: true,
                              color: Colors.cyanAccent.withOpacity(0.2)),
                          dotData: FlDotData(show: false),
                        ),
                      ],
                      minX: _chartDataPoints.isNotEmpty ? _chartDataPoints.first.x : 0,
                      maxX: _chartDataPoints.isNotEmpty ? _chartDataPoints.last.x : 300,
                      minY: 0,
                      maxY: 300,
                    ),
                  ),
                ),
              ),
            ),
          ),
          const SizedBox(height: 10),
          Text(
            'Upper Pressure: ${_upperPressure.toStringAsFixed(2)} mmHg',
            style: const TextStyle(color: Colors.cyanAccent, fontSize: 18, fontWeight: FontWeight.bold),
          ),
          Text(
            'Lower Pressure: ${_lowerPressure.toStringAsFixed(2)} mmHg',
            style: const TextStyle(color: Colors.cyanAccent, fontSize: 18, fontWeight: FontWeight.bold),
          ),
          const SizedBox(height: 20),
          ElevatedButton(
            onPressed: _toggleMonitoring,
            style: ElevatedButton.styleFrom(
              backgroundColor: _isMonitoring ? Colors.redAccent : Colors.cyanAccent,
              shape: RoundedRectangleBorder(
                borderRadius: BorderRadius.circular(20.0),
              ),
              padding: const EdgeInsets.symmetric(horizontal: 32, vertical: 12),
              textStyle: const TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
            ),
            child: Text(_isMonitoring ? 'Stop Monitoring' : 'Start Monitoring'),
          ),
          const SizedBox(height: 20),
        ],
      ),
    );
  }
}
