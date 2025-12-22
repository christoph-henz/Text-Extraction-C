# text-extraction-frontend (C++ MVVM skeleton)

Kurzes Gerüst einer C++ GUI-Anwendung mit Clean Architecture + MVVM.

Build (mit SFML optional):

```bash
mkdir build && cd build
cmake ..
cmake --build .
./text-extraction-frontend
```

Wenn SFML installiert ist (find_package findet es), wird eine einfache GUI gestartet.
Ansonsten läuft eine Console-Fallback-Variante.

Ordnerstruktur (grobe Übersicht):
- include/Core
- include/UseCases
- include/Presentation
- src/ViewModels
- src/Views
