import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'data_acquisition_state.dart';
import 'chart_widget.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
      create: (context) => DataAcquisitionState('192.168.1.100'), // Substitua pelo IP do seu ESP32
      child: MaterialApp(
        title: 'Data Acquisition System',
        theme: ThemeData(
          primarySwatch: Colors.blue,
        ),
        home: const MyHomePage(),
      ),
    );
  }
}

class MyHomePage extends StatelessWidget {
  const MyHomePage({super.key});

  @override
  Widget build(BuildContext context) {
    final state = Provider.of<DataAcquisitionState>(context);

    return Scaffold(
      appBar: AppBar(
        title: const Text('Data Acquisition System'),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            Text(
              'Current Value: ${state.currentValue.toStringAsFixed(4)} V',
            ),
            const ChartWidget(),
            Slider(
              value: state.acquisitionRate.toDouble(),
              min: 1,
              max: 2000,
              divisions: 2000,
              label: state.acquisitionRate.toString(),
              onChanged: (value) {
                state.updateAcquisitionRate(value.toInt());
              },
            ),
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceAround,
              children: [
                ElevatedButton(
                  onPressed: state.startAcquisition,
                  child: const Text('Start'),
                ),
                ElevatedButton(
                  onPressed: state.stopAcquisition,
                  child: const Text('Stop'),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}
