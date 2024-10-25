import 'package:firebase_core/firebase_core.dart';

/// Classe que fornece as opções padrão de configuração do Firebase.
class DefaultFirebaseOptions {
  static FirebaseOptions get currentPlatform {
   return const FirebaseOptions(
  apiKey: "AIzaSyAOS3EzkvCVMPAc6S3YepwDuLeK38SHGIs",
  authDomain: "mapa-pressure.firebaseapp.com",
  databaseURL: "https://mapa-pressure-default-rtdb.firebaseio.com",
  projectId: "mapa-pressure",
  storageBucket: "mapa-pressure.appspot.com",
  messagingSenderId: "1022177397587",
  appId: "1:1022177397587:web:4ac35ed92eeacf5f296fa6",
  measurementId: "G-Q1DHJDYYBH",
);

  }
}
