import 'package:flutter/material.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_database/firebase_database.dart';
import 'register_page.dart';
import 'patient_page.dart';
import 'doctor_page.dart';

class LoginPage extends StatefulWidget {
  const LoginPage({super.key});

  @override
  _LoginPageState createState() => _LoginPageState();
}

class _LoginPageState extends State<LoginPage> {
  final _emailController = TextEditingController();
  final _passwordController = TextEditingController();
  final FirebaseAuth _auth = FirebaseAuth.instance;
  final DatabaseReference _databaseReference = FirebaseDatabase.instance.ref();

  final bool _isHoveringLogin = false;
  final bool _isHoveringRegister = false;
  bool _isPressedLogin = false;
  bool _isPressedRegister = false;

  Future<void> _login() async {
    try {
      await _auth.signInWithEmailAndPassword(
        email: _emailController.text.trim(),
        password: _passwordController.text.trim(),
      );
      User? user = _auth.currentUser;

      if (user != null) {
        DatabaseReference patientRef = _databaseReference.child('users/patients').child(user.uid);
        DatabaseEvent event = await patientRef.once();

        if (event.snapshot.exists) {
          Navigator.pushReplacement(
            context,
            MaterialPageRoute(builder: (context) => const PatientPage()),
          );
        } else {
          Navigator.pushReplacement(
            context,
            MaterialPageRoute(builder: (context) => const DoctorPage()),
          );
        }
      }
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Falha ao fazer login: $e')),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.black,
      body: LayoutBuilder(
        builder: (context, constraints) {
          return Stack(
            children: [
              // Background gradient
              Positioned.fill(
                child: Container(
                  decoration: const BoxDecoration(
                    gradient: LinearGradient(
                      colors: [Colors.black, Color(0xFF1C1C1E)],
                      begin: Alignment.topCenter,
                      end: Alignment.bottomCenter,
                    ),
                  ),
                ),
              ),
              Center(
                child: SingleChildScrollView(
                  child: Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      Image.asset(
                        'assets/images/logo.png',
                        width: 120.0,
                      ),
                      const SizedBox(height: 20),
                      Container(
                        width: 250.0,
                        padding: const EdgeInsets.symmetric(vertical: 15),
                        decoration: BoxDecoration(
                          color: Colors.grey[900],
                          borderRadius: BorderRadius.circular(20),
                          boxShadow: [
                            BoxShadow(
                              color: Colors.blueAccent.withOpacity(0.5),
                              blurRadius: 15,
                              spreadRadius: 2,
                              offset: const Offset(0, 4),
                            ),
                          ],
                        ),
                        alignment: Alignment.center,
                        child: const Text(
                          'MedPress',
                          style: TextStyle(
                            color: Colors.blueAccent,
                            fontSize: 30,
                            fontWeight: FontWeight.bold,
                            letterSpacing: 2.0,
                          ),
                        ),
                      ),
                      const SizedBox(height: 40),
                      // E-mail Field with Icon
                      _buildInputField(
                        controller: _emailController,
                        hintText: 'Insira seu e-mail',
                        icon: Icons.email,
                      ),
                      const SizedBox(height: 20),
                      // Password Field with Icon
                      _buildInputField(
                        controller: _passwordController,
                        hintText: 'Insira sua senha',
                        icon: Icons.lock,
                        obscureText: true,
                      ),
                      const SizedBox(height: 30),
                      // Login Button
                      _buildAnimatedButton(
                        label: 'Entrar',
                        isHovering: _isHoveringLogin,
                        isPressed: _isPressedLogin,
                        onTapDown: (_) => setState(() => _isPressedLogin = true),
                        onTapUp: (_) {
                          setState(() => _isPressedLogin = false);
                          _login();
                        },
                      ),
                      const SizedBox(height: 20),
                      // Register Button
                      _buildAnimatedButton(
                        label: 'Registrar',
                        isHovering: _isHoveringRegister,
                        isPressed: _isPressedRegister,
                        onTapDown: (_) => setState(() => _isPressedRegister = true),
                        onTapUp: (_) {
                          setState(() => _isPressedRegister = false);
                          Navigator.push(
                            context,
                            MaterialPageRoute(builder: (context) => const RegisterPage()),
                          );
                        },
                      ),
                    ],
                  ),
                ),
              ),
            ],
          );
        },
      ),
    );
  }

  // Function to build input fields with consistent styling
  Widget _buildInputField({
    required TextEditingController controller,
    required String hintText,
    required IconData icon,
    bool obscureText = false,
  }) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 10),
      width: 500.0,
      height: 50.0,
      decoration: BoxDecoration(
        color: Colors.grey[850],
        borderRadius: BorderRadius.circular(10),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withOpacity(0.3),
            spreadRadius: 2,
            blurRadius: 8,
            offset: const Offset(0, 3),
          ),
        ],
      ),
      child: Row(
        children: [
          Icon(icon, color: Colors.blueAccent),
          const SizedBox(width: 10),
          Expanded(
            child: TextField(
              controller: controller,
              obscureText: obscureText,
              style: const TextStyle(color: Colors.white),
              decoration: InputDecoration(
                hintText: hintText,
                hintStyle: TextStyle(color: Colors.grey[400]),
                border: InputBorder.none,
              ),
            ),
          ),
        ],
      ),
    );
  }

  // Function to build animated buttons with hover and press effects
  Widget _buildAnimatedButton({
    required String label,
    required bool isHovering,
    required bool isPressed,
    required void Function(TapDownDetails) onTapDown,
    required void Function(TapUpDetails) onTapUp,
  }) {
    return MouseRegion(
      onEnter: (_) => setState(() => isHovering = true),
      onExit: (_) => setState(() => isHovering = false),
      child: GestureDetector(
        onTapDown: onTapDown,
        onTapUp: onTapUp,
        child: AnimatedContainer(
          duration: const Duration(milliseconds: 200),
          width: isPressed ? 480.0 : 500.0,
          height: isPressed ? 45.0 : 50.0,
          decoration: BoxDecoration(
            color: isHovering ? Colors.grey[700] : Colors.grey[850],
            borderRadius: BorderRadius.circular(34),
            boxShadow: [
              BoxShadow(
                color: Colors.blueAccent.withOpacity(0.3),
                spreadRadius: 2,
                blurRadius: 8,
                offset: const Offset(0, 4),
              ),
            ],
          ),
          alignment: Alignment.center,
          child: Text(
            label,
            style: const TextStyle(
              color: Colors.blueAccent,
              fontSize: 25,
              fontWeight: FontWeight.w900,
            ),
          ),
        ),
      ),
    );
  }
}
