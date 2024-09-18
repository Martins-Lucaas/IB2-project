import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;

class DataAcquisitionState with ChangeNotifier {
  final String esp32Ip;
  List<double> dataPoints = [];
  double currentValue = 0.0;
  int timeScale = 50; // Escala de tempo
  int fetchDelay = 10; // Novo atributo para atraso do fetchData
  bool isAcquiring = false;

  DataAcquisitionState(this.esp32Ip);

  void startAcquisition() async {
    if (!isAcquiring) {
      isAcquiring = true;
      notifyListeners();
      debugPrint("Starting data acquisition...");
      try {
        final response = await http.get(Uri.parse('http://$esp32Ip/startAcquisition'));
        if (response.statusCode == 200) {
          debugPrint("Acquisition started successfully.");
          _fetchData();
        } else {
          debugPrint("Failed to start acquisition: ${response.statusCode}");
        }
      } catch (e) {
        debugPrint("Error starting acquisition: $e");
      }
    }
  }

  void stopAcquisition() async {
    if (isAcquiring) {
      isAcquiring = false;
      debugPrint("Stopping data acquisition...");
      try {
        final response = await http.get(Uri.parse('http://$esp32Ip/stopAcquisition'));
        if (response.statusCode == 200) {
          debugPrint("Acquisition stopped successfully.");
        } else {
          debugPrint("Failed to stop acquisition: ${response.statusCode}");
        }
      } catch (e) {
        debugPrint("Error stopping acquisition: $e");
      }
      notifyListeners();
    }
  }

  // Atualiza a escala de tempo
  void updateTimeScale(int scale) {
    timeScale = scale;
    notifyListeners();
  }

  // Atualiza o atraso do fetchData
  void updateFetchDelay(int delay) {
    fetchDelay = delay;
    notifyListeners();
  }

  void _fetchData() async {
    while (isAcquiring) {
      try {
        final response = await http.get(Uri.parse('http://$esp32Ip/vADCvalue'));
        if (response.statusCode == 200) {
          currentValue = double.parse(response.body);
          if (dataPoints.length >= 100) {
            dataPoints.removeAt(0);
          }
          dataPoints.add(currentValue);
          debugPrint("Fetched value: $currentValue");
          notifyListeners();
        } else {
          debugPrint("Failed to fetch data. Status code: ${response.statusCode}");
        }
      } catch (e) {
        debugPrint("Error fetching data: $e");
      }
      await Future.delayed(Duration(milliseconds: fetchDelay)); // Usando o novo atraso
    }
  }
}
