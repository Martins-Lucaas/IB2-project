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
      create: (context) => DataAcquisitionState('192.168.3.20'), // Substitua pelo IP do seu ESP32
      child: MaterialApp(
        title: 'Sistemas de aquisição de dados',
        theme: ThemeData(
          brightness: Brightness.light, // Mudar para tema claro
          primaryColor: Colors.blue,
          textTheme: Theme.of(context).textTheme.apply(
                bodyColor: Colors.black,
                displayColor: Colors.black,
              ),
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
        title: const Center(child: Text('Sistemas de aquisição de dados')),
      ),
      body: Center(
        child: SingleChildScrollView(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: <Widget>[
              // Adicionar a imagem com tamanho ajustado
              Image.asset(
                'assets/images/45 Anos UFU-19.png', // Atualizado para o novo caminho da imagem
                width: 200, // Defina a largura desejada
                height: 200, // Defina a altura desejada
              ),
              const SizedBox(height: 20),
              const ChartWidget(),
              const SizedBox(height: 20),
              // Título para o Slider de escala de tempo
              const Text(
                'Ajuste da Escala de Tempo',
                style: TextStyle(fontSize: 16),
              ),
              // Slider para ajustar a escala de tempo
              Slider(
                value: state.timeScale.toDouble(),
                min: 1,
                max: 100, // Defina um valor máximo adequado para a escala de tempo
                divisions: 100,
                label: state.timeScale.toString(),
                onChanged: (value) {
                  state.updateTimeScale(value.toInt());
                },
              ),
              const SizedBox(height: 20),
              // Título para o Slider de velocidade de aquisição
              const Text(
                'Velocidade de Aquisição (ms)',
                style: TextStyle(fontSize: 16),
              ),
              // Novo Slider para ajustar o atraso do fetchData
              Slider(
                value: state.fetchDelay.toDouble(),
                min: 1,
                max: 1000, // Ajuste conforme necessário para controlar a velocidade
                divisions: 100,
                label: state.fetchDelay.toString(),
                onChanged: (value) {
                  state.updateFetchDelay(value.toInt());
                },
              ),
              const SizedBox(height: 20),
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceAround,
                children: [
                  ElevatedButton(
                    onPressed: () {
                      debugPrint("Start button pressed");
                      state.startAcquisition();
                    },
                    child: const Text('Iniciar'),
                  ),
                  ElevatedButton(
                    onPressed: () {
                      debugPrint("Stop button pressed");
                      state.stopAcquisition();
                    },
                    child: const Text('Parar'),
                  ),
                ],
              ),
              const SizedBox(height: 20),
              if (state.isAcquiring) ...[
                const Text('Adquirindo Dados...', style: TextStyle(fontSize: 16)),
              ],
              const SizedBox(height: 20),
              // Container para mostrar o valor atual da aquisição
              Container(
                padding: const EdgeInsets.all(16.0),
                decoration: BoxDecoration(
                  color: Colors.white,
                  borderRadius: BorderRadius.circular(10.0),
                  border: Border.all(color: Colors.black),
                ),
                child: Text(
                  'Valor Atual: ${state.currentValue.toStringAsFixed(4)} V',
                  style: const TextStyle(fontSize: 24, color: Colors.black),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
