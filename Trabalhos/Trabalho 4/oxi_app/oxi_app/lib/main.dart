import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'data_acquisition_state.dart';
import 'home_screen.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MultiProvider(
      providers: [
        ChangeNotifierProvider(
          create: (context) => DataAcquisitionState('192.168.82.131'), // IP do ESP32
        ),
      ],
      child: MaterialApp(
        debugShowCheckedModeBanner: false,
        title: 'Oxímetro de Pulso',
        theme: ThemeData(
          primarySwatch: Colors.teal,
        ),
        home: const HomeScreen(),
      ),
    );
  }
}
