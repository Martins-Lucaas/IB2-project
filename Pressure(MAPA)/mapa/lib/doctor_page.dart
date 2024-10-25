import 'package:flutter/material.dart';

class DoctorPage extends StatelessWidget {
  const DoctorPage({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Página do Médico'),
        backgroundColor: const Color(0xFF149393),
      ),
      body: const Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Text(
              'Bem-vindo, Médico!',
              style: TextStyle(fontSize: 24),
            ),
            SizedBox(height: 20),
            Icon(
              Icons.medical_services,
              size: 100,
              color: Color(0xFF149393),
            ),
          ],
        ),
      ),
    );
  }
}
