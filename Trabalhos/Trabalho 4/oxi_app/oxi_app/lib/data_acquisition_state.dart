import 'package:flutter/material.dart';
import 'dart:async';

class DataAcquisitionState with ChangeNotifier {
  final String esp32Ip;
  List<double> dataPoints = [];  // Lista para armazenar os pontos de dados
  double currentValue = 0.0;
  int acquisitionRate = 10;  // Taxa de aquisição ajustada para 10ms
  bool isAcquiring = false;
  Timer? _timer;

  List<double> fftData = [];
  double frequency = 0.0;
  
  int timeDivisions = 100; // Escala de tempo padrão em ms/div
  double maxAmplitude = 5.0;  // Valor máximo inicial de amplitude
  
  double spo2 = 0.0; // Campo para SpO2
  int bpm = 0; // Campo para BPM

  DataAcquisitionState(this.esp32Ip);

  void startAcquisition() async {
    if (!isAcquiring) {
      isAcquiring = true;
      notifyListeners();
      _scheduleNextFetch();
    }
  }

  void stopAcquisition() async {
    if (isAcquiring) {
      isAcquiring = false;
      _timer?.cancel();
      notifyListeners();
    }
  }

  void _scheduleNextFetch() {
    if (isAcquiring) {
      _timer = Timer(Duration(milliseconds: acquisitionRate), () async {
        await _fetchData();
        _scheduleNextFetch();
      });
    }
  }

  Future<void> _fetchData() async {
    // Lógica para buscar os dados
  }

  void updateGraphData(List<double> newDataPoints) {
    // Atualiza a lista de pontos de dados
    dataPoints = newDataPoints;
    _updateMaxAmplitude();  // Atualiza a amplitude máxima
    notifyListeners();
  }

  void _updateMaxAmplitude() {
    if (dataPoints.isNotEmpty) {
      maxAmplitude = dataPoints.reduce((curr, next) => curr.abs() > next.abs() ? curr : next).abs();
      maxAmplitude = maxAmplitude * 1.1; // Adiciona 10% de margem
    }
  }

  void updateSpo2(double newSpo2) {
    spo2 = newSpo2;
    notifyListeners();
  }

  void updateBpm(int newBpm) {
    bpm = newBpm;
    notifyListeners();
  }

  void setTimeDivisions(int divisions) {
    timeDivisions = divisions;
    notifyListeners();
  }
}
