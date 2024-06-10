import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;

class DataAcquisitionState with ChangeNotifier {
  final String esp32Ip;
  List<double> dataPoints = [];
  double currentValue = 0.0;
  int acquisitionRate = 500;
  bool isAcquiring = false;

  DataAcquisitionState(this.esp32Ip);

  void startAcquisition() async {
    if (!isAcquiring) {
      isAcquiring = true;
      notifyListeners();
      await http.get(Uri.parse('http://$esp32Ip/startAcquisition'));
      _fetchData();
    }
  }

  void stopAcquisition() async {
    if (isAcquiring) {
      isAcquiring = false;
      await http.get(Uri.parse('http://$esp32Ip/stopAcquisition'));
      notifyListeners();
    }
  }

  void updateAcquisitionRate(int rate) async {
    acquisitionRate = rate;
    await http.get(Uri.parse('http://$esp32Ip/updateAcquisitionRate?rate=$rate'));
    notifyListeners();
  }

  void _fetchData() async {
    while (isAcquiring) {
      final response = await http.get(Uri.parse('http://$esp32Ip/vADCvalue'));
      if (response.statusCode == 200) {
        currentValue = double.parse(response.body);
        if (dataPoints.length >= 100) {
          dataPoints.removeAt(0);
        }
        dataPoints.add(currentValue);
        notifyListeners();
      }
      await Future.delayed(Duration(milliseconds: acquisitionRate));
    }
  }
}
