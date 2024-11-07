import 'package:flutter/material.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_database/firebase_database.dart';
import 'UserProfilePage.dart'; // Certifique-se de ter uma página de perfil para o médico ou reusar a existente.

class DoctorPage extends StatefulWidget {
  const DoctorPage({super.key});

  @override
  _DoctorPageState createState() => _DoctorPageState();
}

class _DoctorPageState extends State<DoctorPage> {
  final DatabaseReference _database = FirebaseDatabase.instance.ref();
  final FirebaseAuth _auth = FirebaseAuth.instance;
  List<Map<String, dynamic>> _patients = [];
  bool _isLoading = true;

  @override
  void initState() {
    super.initState();
    _loadPatients();
  }

  Future<void> _loadPatients() async {
    DatabaseReference patientsRef = _database.child('users/patients');
    final snapshot = await patientsRef.get();

    if (snapshot.exists) {
      final data = snapshot.value as Map<dynamic, dynamic>;
      final List<Map<String, dynamic>> patients = [];

      data.forEach((key, value) {
        patients.add({
          'id': key,
          'name': value['nomeCompleto'] ?? 'Nome não disponível',
        });
      });

      setState(() {
        _patients = patients;
        _isLoading = false;
      });
    } else {
      setState(() {
        _isLoading = false;
      });
    }
  }

  Future<void> _logout() async {
    await _auth.signOut();
    Navigator.of(context).pushReplacementNamed('/login');
  }

  void _showUserOptions() {
    showDialog(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20.0)),
          backgroundColor: Colors.black87,
          title: const Text(
            'Opções do Usuário',
            style: TextStyle(color: Colors.cyanAccent),
          ),
          content: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              ListTile(
                leading: const Icon(Icons.person, color: Colors.cyanAccent),
                title: const Text('Perfil', style: TextStyle(color: Colors.white)),
                onTap: () {
                  Navigator.of(context).pop();
                  Navigator.of(context).push(
                    MaterialPageRoute(
                      builder: (context) => const UserProfilePage(),
                    ),
                  );
                },
              ),
              ListTile(
                leading: const Icon(Icons.logout, color: Colors.redAccent),
                title: const Text('Sair', style: TextStyle(color: Colors.white)),
                onTap: () async {
                  await _logout();
                  Navigator.of(context).pop();
                },
              ),
            ],
          ),
        );
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.black,
      appBar: AppBar(
        title: const Text('Página do Médico', style: TextStyle(color: Colors.cyanAccent)),
        backgroundColor: Colors.black,
        centerTitle: true,
        iconTheme: const IconThemeData(color: Colors.cyanAccent),
        actions: [
          IconButton(
            icon: const Icon(Icons.person),
            color: Colors.cyanAccent,
            onPressed: _showUserOptions,
          ),
        ],
      ),
      body: _isLoading
          ? const Center(child: CircularProgressIndicator(color: Colors.cyanAccent))
          : Padding(
              padding: const EdgeInsets.all(16.0),
              child: Column(
                children: [
                  const Text(
                    'Bem-vindo, Médico!',
                    style: TextStyle(fontSize: 24, color: Colors.cyanAccent, fontWeight: FontWeight.bold),
                  ),
                  const SizedBox(height: 20),
                  Expanded(
                    child: ListView.builder(
                      itemCount: _patients.length,
                      itemBuilder: (context, index) {
                        final patient = _patients[index];
                        return Card(
                          color: Colors.black87,
                          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20)),
                          child: ListTile(
                            title: Text(
                              patient['name'],
                              style: const TextStyle(color: Colors.white),
                            ),
                            trailing: const Icon(Icons.arrow_forward, color: Colors.cyanAccent),
                            onTap: () {
                              Navigator.push(
                                context,
                                MaterialPageRoute(
                                  builder: (context) => PatientMeasurementsPage(
                                    patientId: patient['id'],
                                    patientName: patient['name'],
                                  ),
                                ),
                              );
                            },
                          ),
                        );
                      },
                    ),
                  ),
                ],
              ),
            ),
    );
  }
}

class PatientMeasurementsPage extends StatelessWidget {
  final String patientId;
  final String patientName;

  const PatientMeasurementsPage({required this.patientId, required this.patientName, Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.black,
      appBar: AppBar(
        title: Text('$patientName - Medições', style: const TextStyle(color: Colors.cyanAccent)),
        backgroundColor: Colors.black,
        iconTheme: const IconThemeData(color: Colors.cyanAccent),
        centerTitle: true,
      ),
      body: FutureBuilder(
        future: FirebaseDatabase.instance.ref('users/patients/$patientId/monitoramento').orderByKey().limitToLast(10).get(),
        builder: (context, snapshot) {
          if (snapshot.connectionState == ConnectionState.waiting) {
            return const Center(child: CircularProgressIndicator(color: Colors.cyanAccent));
          }

          if (!snapshot.hasData || (snapshot.data! as DataSnapshot).value == null) {
            return const Center(
              child: Text("Nenhuma medição encontrada.", style: TextStyle(color: Colors.white, fontSize: 18)),
            );
          }

          final data = (snapshot.data! as DataSnapshot).value as Map<dynamic, dynamic>;
          final measurements = data.entries.map((entry) {
            final value = entry.value as Map<dynamic, dynamic>;
            return {
              'date': value['date'] ?? 'Data não disponível',
              'systolicPressure': value['systolicPressure'] ?? 'N/A',
              'diastolicPressure': value['diastolicPressure'] ?? 'N/A',
            };
          }).toList()
            ..sort((a, b) => b['date'].compareTo(a['date']));

          return ListView.builder(
            padding: const EdgeInsets.all(16.0),
            itemCount: measurements.length,
            itemBuilder: (context, index) {
              final measurement = measurements[index];
              return Card(
                color: Colors.black87,
                shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20)),
                elevation: 8,
                margin: const EdgeInsets.symmetric(vertical: 8.0),
                child: Padding(
                  padding: const EdgeInsets.all(16.0),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        "Data: ${measurement['date']}",
                        style: const TextStyle(color: Colors.cyanAccent, fontSize: 18, fontWeight: FontWeight.bold),
                      ),
                      const SizedBox(height: 8),
                      Text(
                        "Pressão Sistólica: ${measurement['systolicPressure']} mmHg",
                        style: const TextStyle(color: Colors.white, fontSize: 16),
                      ),
                      const SizedBox(height: 4),
                      Text(
                        "Pressão Diastólica: ${measurement['diastolicPressure']} mmHg",
                        style: const TextStyle(color: Colors.white, fontSize: 16),
                      ),
                    ],
                  ),
                ),
              );
            },
          );
        },
      ),
    );
  }
}
