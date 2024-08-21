import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:fl_chart/fl_chart.dart';
import 'data_acquisition_state.dart';

class ChartWidget extends StatelessWidget {
  const ChartWidget({super.key});

  @override
  Widget build(BuildContext context) {
    final state = Provider.of<DataAcquisitionState>(context);

    return Column(
      children: [
        SizedBox(
          height: 300,
          width: double.infinity,
          child: LineChart(
            LineChartData(
              minY: -state.maxAmplitude, // Amplitude negativa
              maxY: state.maxAmplitude,  // Amplitude positiva ajustável
              minX: 0,
              maxX: state.timeDivisions.toDouble(), // Tamanho gráfico eixo X ajustável
              titlesData: FlTitlesData(
                leftTitles: AxisTitles(
                  sideTitles: SideTitles(
                    showTitles: true,
                    reservedSize: 40,
                    getTitlesWidget: (value, meta) {
                      return Text(
                        value.toStringAsFixed(1),
                        style: const TextStyle(color: Colors.white, fontSize: 12),
                      );
                    },
                  ),
                ),
                bottomTitles: const AxisTitles(
                  sideTitles: SideTitles(
                    showTitles: false,
                  ),
                ),
                topTitles: const AxisTitles(
                  sideTitles: SideTitles(
                    showTitles: false,
                  ),
                ),
                rightTitles: const AxisTitles(
                  sideTitles: SideTitles(
                    showTitles: false,
                  ),
                ),
              ),
              gridData: FlGridData(
                show: true,
                drawVerticalLine: true,
                getDrawingHorizontalLine: (value) {
                  return const FlLine(
                    color: Colors.white24,
                    strokeWidth: 1,
                  );
                },
                getDrawingVerticalLine: (value) {
                  return const FlLine(
                    color: Colors.white24,
                    strokeWidth: 1,
                  );
                },
              ),
              borderData: FlBorderData(
                show: true,
                border: Border.all(color: Colors.white24, width: 1),
              ),
              lineBarsData: [
                LineChartBarData(
                  spots: state.dataPoints
                      .asMap()
                      .entries
                      .map((e) => FlSpot(e.key.toDouble(), e.value))
                      .toList(),
                  color: Colors.yellow,
                  barWidth: 2,
                  dotData: const FlDotData(show: false),
                ),
              ],
            ),
          ),
        ),
        const SizedBox(height: 10),
        Text(
          'Escala de Tempo: ${state.timeDivisions} ms/div',
          style: const TextStyle(color: Colors.white, fontSize: 16),
        ),
      ],
    );
  }
}
