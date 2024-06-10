import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:fl_chart/fl_chart.dart';
import 'data_acquisition_state.dart';

class ChartWidget extends StatelessWidget {
  const ChartWidget({super.key});

  @override
  Widget build(BuildContext context) {
    final state = Provider.of<DataAcquisitionState>(context);

    return SizedBox(
      height: 300,
      width: double.infinity,
      child: LineChart(
        LineChartData(
          minX: 0,
          maxX: 100,
          minY: 0,
          maxY: 3.3,
          lineBarsData: [
            LineChartBarData(
              spots: state.dataPoints
                  .asMap()
                  .entries
                  .map((e) => FlSpot(e.key.toDouble(), e.value))
                  .toList(),
              isCurved: false,
              color: Colors.blue,  // Ajuste no uso do argumento de cores
              dotData: const FlDotData(show: false),
            ),
          ],
          titlesData: const FlTitlesData(show: false),
          gridData: const FlGridData(show: true),
        ),
      ),
    );
  }
}
