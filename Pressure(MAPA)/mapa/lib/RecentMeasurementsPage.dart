import 'package:flutter/material.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_database/firebase_database.dart';

class RecentMeasurementsPage extends StatefulWidget {
  const RecentMeasurementsPage({Key? key}) : super(key: key);

  @override
  _RecentMeasurementsPageState createState() => _RecentMeasurementsPageState();
}

class _RecentMeasurementsPageState extends State<RecentMeasurementsPage> {
  final FirebaseAuth _auth = FirebaseAuth.instance;
  final DatabaseReference _database = FirebaseDatabase.instance.ref();
  List<Map<String, dynamic>> _measurements = [];
  bool _isLoading = true;

  @override
  void initState() {
    super.initState();
    _loadMeasurements();
  }

  Future<void> _loadMeasurements() async {
  String userId = _auth.currentUser?.uid ?? "";
  if (userId.isEmpty) return;

  DatabaseReference monitoramentoRef = _database.child('users/patients/$userId/monitoramento');

  try {
    final snapshot = await monitoramentoRef.limitToLast(10).get();

    if (snapshot.exists) {
      final data = snapshot.value as Map<dynamic, dynamic>;

      final List<Map<String, dynamic>> measurements = [];
      data.forEach((key, value) {
        measurements.add({
          'date': value['date'] as String? ?? 'Data não disponível',
          'systolicPressure': double.tryParse(value['diastolicPressure'].toString()) ?? 0.0,
          'diastolicPressure': double.tryParse(value['systolicPressure'].toString()) ?? 0.0,
        });
      });

      // Ordena manualmente a lista por data decrescente (caso a data esteja em um formato que permita isso)
      measurements.sort((a, b) => b['date'].compareTo(a['date']));

      setState(() {
        _measurements = measurements;
        _isLoading = false;
      });
    } else {
      setState(() {
        _isLoading = false;
      });
    }
  } catch (error) {
    print("Erro ao carregar medições: $error");
    setState(() {
      _isLoading = false;
    });
  }
}



  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.black,
      appBar: AppBar(
        backgroundColor: Colors.black,
        elevation: 0,
        centerTitle: true,
        iconTheme: const IconThemeData(color: Colors.cyanAccent),
        title: const Text(
          "Últimas Medições",
          style: TextStyle(
            color: Colors.cyanAccent,
            fontSize: 35,
            fontWeight: FontWeight.bold,
          ),
          textAlign: TextAlign.center,
        ),
      ),
      body: _isLoading
          ? const Center(child: CircularProgressIndicator(color: Colors.cyanAccent))
          : Padding(
              padding: const EdgeInsets.symmetric(horizontal: 16.0, vertical: 8.0),
              child: _measurements.isEmpty
                  ? const Center(
                      child: Text(
                        "Nenhuma medição encontrada.",
                        style: TextStyle(fontSize: 18, color: Colors.white70),
                      ),
                    )
                  : ListView.builder(
                      itemCount: _measurements.length,
                      itemBuilder: (context, index) {
                        final measurement = _measurements[index];
                        return Card(
                          color: Colors.blue,
                          shape: RoundedRectangleBorder(
                            borderRadius: BorderRadius.circular(20),
                          ),
                          elevation: 8,
                          margin: const EdgeInsets.symmetric(vertical: 8.0),
                          child: Padding(
                            padding: const EdgeInsets.all(16.0),
                            child: Column(
                              crossAxisAlignment: CrossAxisAlignment.start,
                              children: [
                                Text(
                                  "Data: ${measurement['date']}",
                                  style: const TextStyle(
                                    color: Colors.cyanAccent,
                                    fontSize: 18,
                                    fontWeight: FontWeight.bold,
                                  ),
                                ),
                                const SizedBox(height: 8),
                                Text(
                                  "Pressão Sistólica: ${measurement['systolicPressure'].toStringAsFixed(2)} mmHg",
                                  style: const TextStyle(
                                    color: Colors.white,
                                    fontSize: 16,
                                  ),
                                ),
                                const SizedBox(height: 4),
                                Text(
                                  "Pressão Diastólica: ${measurement['diastolicPressure'].toStringAsFixed(2)} mmHg",
                                  style: const TextStyle(
                                    color: Colors.white,
                                    fontSize: 16,
                                  ),
                                ),
                              ],
                            ),
                          ),
                        );
                      },
                    ),
            ),
    );
  }
}
