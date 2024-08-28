import 'package:flutter/material.dart';
import 'package:http/http.dart' as http; // Para fazer requisições HTTP
import 'package:provider/provider.dart';
import 'dart:async';

import 'login_page.dart'; // Importe a tela de login

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
      create: (context) => DataAcquisitionState(),
      child: MaterialApp(
        debugShowCheckedModeBanner: false,
        title: 'Oxímetro de Pulso',
        theme: ThemeData.dark().copyWith(
          primaryColor: Colors.teal,
          scaffoldBackgroundColor: Colors.black,
        ),
        initialRoute: '/', // Definindo a rota inicial
        routes: {
          '/': (context) => const LoginPage(), // Rota para a página de login
          '/home': (context) => const HomeScreen(), // Rota para a página inicial
        },
      ),
    );
  }
}

class DataAcquisitionState extends ChangeNotifier {
  String ipAddress = '192.168.144.131'; // IP do ESP32
  double spo2 = 0.0;

  DataAcquisitionState() {
    fetchData();
  }

  Future<void> fetchData() async {
    Timer.periodic(const Duration(seconds: 1), (timer) async {
      try {
        final response = await http.get(Uri.parse('http://$ipAddress/dados'));
        if (response.statusCode == 200) {
          final data = response.body.split(',');
          spo2 = double.parse(data[0].split(':')[1]);
          notifyListeners();
        } else {
          print('Erro na resposta HTTP: ${response.statusCode}');
        }
      } catch (e) {
        print('Erro ao fazer requisição HTTP: $e');
      }
    });
  }
}

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  bool _showSpO2 = false; // Estado para controlar a visibilidade do SpO2

  @override
  Widget build(BuildContext context) {
    final dataState = Provider.of<DataAcquisitionState>(context);

    // Determina a cor do texto do SpO2 com base no valor
    Color spo2Color;
    if (dataState.spo2 < 90) {
      spo2Color = Colors.red;
    } else if (dataState.spo2 >= 95) {
      spo2Color = Colors.green;
    } else {
      spo2Color = Colors.yellow;
    }

    return Scaffold(
      appBar: AppBar(
        title: Row(
          children: [
            Image.asset(
              'assets/images/45 Anos UFU-19.png', // Caminho da imagem pequena
              width: 160,
              height: 160,
            ),
            const SizedBox(width: 10),
            const Expanded(
              child: Center(
                child: Text('Oxímetro de Pulso'),
              ),
            ),
          ],
        ),
        centerTitle: true,
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            ElevatedButton(
              onPressed: () {
                setState(() {
                  _showSpO2 = !_showSpO2; // Alterna a visibilidade do SpO2
                });
              },
              child: Text(_showSpO2 ? 'Esconder SpO2' : 'Mostrar SpO2'),
            ),
            const SizedBox(height: 16),
            if (_showSpO2) // Exibe o SpO2 apenas se _showSpO2 for verdadeiro
              Text(
                'SpO2: ${dataState.spo2.toStringAsFixed(2)}%',
                style: TextStyle(
                  fontSize: 24,
                  color: spo2Color, // Define a cor com base no valor
                ),
              ),
          ],
        ),
      ),
    );
  }
}
