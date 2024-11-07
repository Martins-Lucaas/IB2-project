import 'package:flutter/material.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_database/firebase_database.dart';
import 'package:firebase_storage/firebase_storage.dart';
import 'package:image_picker/image_picker.dart';
import 'dart:io';

class UserProfilePage extends StatefulWidget {
  const UserProfilePage({Key? key}) : super(key: key);

  @override
  _UserProfilePageState createState() => _UserProfilePageState();
}

class _UserProfilePageState extends State<UserProfilePage> {
  final FirebaseAuth _auth = FirebaseAuth.instance;
  final DatabaseReference _database = FirebaseDatabase.instance.ref();
  final FirebaseStorage _storage = FirebaseStorage.instance;
  final ImagePicker _picker = ImagePicker();

  Map<String, dynamic> _userData = {};
  bool _isLoading = true;
  bool _isEditing = false;
  String? _profileImageUrl;
  bool isDoctor = false;

  // Controladores de texto
  late TextEditingController _nameController;
  late TextEditingController _emailController;
  late TextEditingController _crmController;
  late TextEditingController _specialtyController;
  late TextEditingController _ageController;
  late TextEditingController _weightController;
  late TextEditingController _heightController;
  late TextEditingController _rgController;
  late TextEditingController _addressController;

  @override
  void initState() {
    super.initState();
    _loadUserData();
  }

  Future<void> _loadUserData() async {
    String userId = _auth.currentUser?.uid ?? "";
    if (userId.isEmpty) return;

    // Detectar se o usuário é médico ou paciente
    DatabaseReference userRef = _database.child('users/doctors/$userId');
    final snapshot = await userRef.get();
    if (!snapshot.exists) {
      userRef = _database.child('users/patients/$userId');
      isDoctor = false;
    } else {
      isDoctor = true;
    }

    final userSnapshot = await userRef.get();
    if (userSnapshot.exists) {
      setState(() {
        _userData = Map<String, dynamic>.from(userSnapshot.value as Map);
        _profileImageUrl = _userData['profileImageUrl'] as String?;
        _initializeControllers();
        _isLoading = false;
      });
    } else {
      setState(() {
        _isLoading = false;
      });
    }
  }

  void _initializeControllers() {
    _nameController = TextEditingController(text: _userData['nomeCompleto']);
    _emailController = TextEditingController(text: _userData['email']);
    _crmController = TextEditingController(text: _userData['crm'] ?? '');
    _specialtyController = TextEditingController(text: _userData['especialidade'] ?? '');
    _ageController = TextEditingController(text: _userData['idade'] ?? '');
    _weightController = TextEditingController(text: _userData['peso'] ?? '');
    _heightController = TextEditingController(text: _userData['altura'] ?? '');
    _rgController = TextEditingController(text: _userData['rg'] ?? '');
    _addressController = TextEditingController(text: _userData['endereco'] ?? '');
  }

  Future<void> _pickAndUploadImage() async {
    final pickedFile = await _picker.pickImage(source: ImageSource.gallery);

    if (pickedFile != null) {
      File imageFile = File(pickedFile.path);
      String userId = _auth.currentUser!.uid;

      try {
        final ref = _storage.ref().child('profile_images/$userId.jpg');
        await ref.putFile(imageFile);

        String imageUrl = await ref.getDownloadURL();
        await _database.child(isDoctor ? 'users/doctors/$userId' : 'users/patients/$userId').update({'profileImageUrl': imageUrl});

        setState(() {
          _profileImageUrl = imageUrl;
        });

        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(content: Text('Foto de perfil atualizada com sucesso!')),
        );
      } catch (e) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Erro ao atualizar foto de perfil: $e')),
        );
      }
    }
  }

  Future<void> _saveUserData() async {
    String userId = _auth.currentUser!.uid;
    DatabaseReference userRef = isDoctor
        ? _database.child('users/doctors/$userId')
        : _database.child('users/patients/$userId');

    await userRef.update({
      'nomeCompleto': _nameController.text,
      'email': _emailController.text,
      if (isDoctor) 'crm': _crmController.text,
      if (isDoctor) 'especialidade': _specialtyController.text,
      if (!isDoctor) 'idade': _ageController.text,
      if (!isDoctor) 'peso': _weightController.text,
      if (!isDoctor) 'altura': _heightController.text,
      if (!isDoctor) 'rg': _rgController.text,
      if (!isDoctor) 'endereco': _addressController.text,
    });

    setState(() {
      _isEditing = false;
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Dados do perfil atualizados com sucesso!')),
      );
    });
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
          "Perfil do Usuário",
          style: TextStyle(
            color: Colors.cyanAccent,
            fontSize: 35,
            fontWeight: FontWeight.bold,
          ),
        ),
        actions: [
          IconButton(
            icon: Icon(_isEditing ? Icons.save : Icons.edit),
            color: Colors.cyanAccent,
            onPressed: () {
              if (_isEditing) {
                _saveUserData();
              } else {
                setState(() {
                  _isEditing = true;
                });
              }
            },
          ),
        ],
      ),
      body: _isLoading
          ? const Center(child: CircularProgressIndicator(color: Colors.cyanAccent))
          : Padding(
              padding: const EdgeInsets.all(16.0),
              child: Column(
                children: [
                  GestureDetector(
                    onTap: _pickAndUploadImage,
                    child: CircleAvatar(
                      radius: 60,
                      backgroundImage: _profileImageUrl != null
                          ? NetworkImage(_profileImageUrl!)
                          : const AssetImage('assets/default_profile.png') as ImageProvider,
                      child: _profileImageUrl == null
                          ? const Icon(Icons.camera_alt, size: 30, color: Colors.cyanAccent)
                          : null,
                    ),
                  ),
                  const SizedBox(height: 20),
                  _buildEditableField("Nome Completo", _nameController),
                  _buildEditableField("Email", _emailController),
                  if (isDoctor) ...[
                    _buildEditableField("CRM", _crmController),
                    _buildEditableField("Especialidade", _specialtyController),
                  ] else ...[
                    _buildEditableField("Data de Nascimento", TextEditingController(text: _userData['dataNascimento'])),
                    _buildEditableField("Idade", _ageController),
                    _buildEditableField("Peso (kg)", _weightController),
                    _buildEditableField("Altura (cm)", _heightController),
                    _buildEditableField("RG", _rgController),
                    _buildEditableField("Endereço", _addressController),
                  ],
                ],
              ),
            ),
    );
  }

  Widget _buildEditableField(String label, TextEditingController controller) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 5),
      child: TextField(
        controller: controller,
        enabled: _isEditing,
        style: const TextStyle(color: Colors.white),
        decoration: InputDecoration(
          labelText: label,
          labelStyle: const TextStyle(color: Colors.cyanAccent),
          enabledBorder: const UnderlineInputBorder(
            borderSide: BorderSide(color: Colors.cyanAccent),
          ),
          focusedBorder: const UnderlineInputBorder(
            borderSide: BorderSide(color: Colors.cyanAccent, width: 2),
          ),
        ),
      ),
    );
  }
}
