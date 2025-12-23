# Text Extraction Frontend

Eine vollständige C++ SFML-basierte Desktop-GUI-Anwendung für die Texterkennung und -verwaltung mit REST API Integration.

## 📋 Übersicht

Die Anwendung ist ein Frontend für ein Text-Extraktionssystem mit folgenden Hauptfeatures:

- **Benutzer-Authentifizierung**: Login mit verschlüsselter Speicherung von Anmeldedaten
- **Datei-Upload**: Hochladen von Dokumenten mit Fortschrittsanzeige
- **Text-Extraktion**: Extrahieren von Text aus hochgeladenen Dokumenten
- **Dokumentenverwaltung**: Ansicht und Verwaltung persönlicher Uploads
- **Admin-Dashboard**: Umfassende Verwaltung für Administrator
  - Statistiken (Benutzer, Dokumente, Extraktionen)
  - Benutzerverwaltung (CRUD-Operationen, Aktivierung/Deaktivierung)
  - Dokumente-Übersicht (alle Uploads des Systems)
  - Extraktionen-Übersicht (alle Extraktionsvorgänge)
- **UTF-8 Support**: Vollständige Unterstützung für deutsche Umlaute (ä, ö, ü)

## 🏗️ Architektur

Das Projekt folgt **Clean Architecture + MVVM** Prinzipien:

```
include/
├── Core/              # Business Logic & Entities
│   ├── Entity.h       # Entität-Definitionen
│   └── IRepository.h  # Repository Interfaces
├── Services/          # API-Integration & Services
│   ├── ApiService.h   # REST API Client
│   └── LoginService.h # Authentifizierung
├── UseCases/          # Use-Case Implementierungen
│   └── ExtractTextUseCase.h
└── Presentation/      # GUI & ViewModels
    ├── Views.h
    ├── View/
    │   └── MainView.hpp    # Hauptansicht (2000+ Zeilen)
    └── ViewModel/
        └── MainViewModel.h # View Model

src/
├── main.cpp           # Einstiegspunkt
├── Services/          # Service Implementierungen
├── ViewModels/        # ViewModel Implementierungen
└── Views/             # View Implementierungen
```

## 🔧 Technologie Stack

- **GUI Framework**: SFML 3.0.2
- **HTTP Client**: libcurl
- **Authentifizierung**: OpenSSL (SHA256 Hashing)
- **C++ Standard**: C++17
- **Build System**: CMake 3.28+
- **Compiler**: GCC 13.3+

## 📦 Installation & Build

### Voraussetzungen

```bash
# Ubuntu/Debian
sudo apt-get install libsfml-dev libcurl4-openssl-dev libssl-dev cmake
```

### Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
./text-extraction-frontend
```

## 🎯 Verwendung

### Login

1. Beim Start wird ein Login-Dialog angezeigt
2. Standard-Anmeldedaten eingeben
3. Anmeldedaten werden verschlüsselt in `~/.text-extraction-login` gespeichert
4. Bei erneutem Start werden diese automatisch geladen

### Tabs & Funktionen

#### 1. **Home** 🏠
- Willkommensseite mit Benutzerinformationen
- Status angezeigt (Admin/User)

#### 2. **Upload** 📤
- Datei auswählen (nur PDF, TXT, DOCX erlaubt)
- Fortschrittsbalken beim Upload
- Erfolgs-/Fehlermeldungen angezeigt

#### 3. **Extraction** 🔍
- Liste eigener hochgeladener Dokumente
- Dokumentdetails anzeigen
- Text-Extraktion starten
- Extrahierten Text in scrollbarer Box ansehen

#### 4. **Admin Panel** ⚙️ (nur für Admin)

**Statistiken**
- Gesamtbenutzer
- Gesamtdokumente
- Gesamtextraktionen
- Letzte Aktivitäten

**Benutzer**
- Liste aller Benutzer mit Details
- Neuen Benutzer erstellen (Benutzername, Email, Passwort, Rolle)
- Benutzer bearbeiten (Email, Rolle, optional Passwort ändern)
- Benutzer aktivieren/deaktivieren

**Dokumente**
- Alle Uploads aus dem System (von allen Benutzern)
- Zeigt: Dateiname, Uploader, Upload-Datum, Größe

**Extraktionen**
- Alle Extraktionsvorgänge des Systems
- Status-Anzeige:
  - 0 = Unbearbeitet (grau)
  - 2 = Completed (grün)
  - 3 = Fehlgeschlagen (rot)
- Zeigt: Dateiname, Methode, Abschluss-Datum, Status, Uploader

#### 5. **Settings** ⚙️
- API Base URL konfigurieren
- Wert speichern und laden

#### 6. **Profile** 👤
- Aktuelle Benutzerinformationen
- Logout-Button

## 🔐 Sicherheit

### Authentifizierung
- **BasicAuth** mit SHA256 gehashten Passwörtern
- Authorization Header: `Authorization: Basic base64(username:password)`

### Credential Storage
- Passwörter werden **XOR-verschlüsselt** in `~/.text-extraction-login` gespeichert
- Datei-Format: `username|base64(xor_encrypted_password)`

### HTTPS Support
- Zertifikatsvalidierung kann deaktiviert werden für Entwicklung

## 📡 API Integration

Base URL: `http://127.0.0.1:5000/api`

### Wichtige Endpoints

| Endpoint | Methode | Beschreibung |
|----------|---------|-------------|
| `/User` | GET | Login validieren |
| `/Upload` | POST | Datei hochladen (multipart) |
| `/Upload/my-documents` | GET | Eigene Dokumente laden |
| `/Extraction/{documentId}` | POST | Text-Extraktion starten |
| `/Extraction/result/{documentId}` | GET | Extraktionsergebnis abrufen |
| `/Admin/statistics` | GET | Admin-Statistiken |
| `/Admin/users` | GET/POST | Benutzer-Liste & Erstellen |
| `/Admin/users/{userId}` | PUT | Benutzer bearbeiten |
| `/Admin/users/{userId}/activate` | POST | Benutzer aktivieren |
| `/Admin/users/{userId}/deactivate` | POST | Benutzer deaktivieren |
| `/Admin/documents` | GET | Alle Dokumente des Systems |
| `/Admin/extractions` | GET | Alle Extraktionen des Systems |

## 🎨 GUI Layout

### Window Größe
- **1400x750px** (Standardgröße)

### Layout-Struktur
```
┌─────────────────────────────────────────────┐
│  SFML Window (1400x750)                     │
├──────────┬──────────────────────────────────┤
│ Sidebar  │  Main Content Area               │
│ (200px)  │  (1200px)                        │
│          │                                  │
│ - Home   │  [Tabs/Forms/Lists]             │
│ - Upload │                                  │
│ - Extract│                                  │
│ - Admin* │                                  │
│ - Settings                                  │
│ - Profile│                                  │
└──────────┴──────────────────────────────────┘
```

### Farbschema
- **Sidebar**: Dunkelblau (sf::Color(40, 45, 50))
- **Text-Hintergrund**: Weiß (sf::Color::White)
- **Akzente**: Steelblue (sf::Color(70, 130, 180))
- **Erfolgsmeldung**: Grün (sf::Color(50, 150, 50))
- **Fehlermeldung**: Rot (sf::Color(200, 50, 50))

## 📝 Wichtige Code-Patterns

### JSON-Parsing
```cpp
// Komplette Objekte extrahieren zwischen { und }
// Validiert mit Marker-Feld um sicher zu gehen
// Extrahiert alle Felder unabhängig von Reihenfolge
std::string value = ExtractJsonField(jsonStr, "fieldName");
```

### UTF-8 zu SFML String Konvertierung
```cpp
// Konvertiert UTF-8 zu UTF-32 für SFML
sf::Text text(ToSFMLString("Äpfel"), font, 14u);
```

### HTTP Requests
```cpp
// GET
Services::HttpResponse resp = Services::ApiService::Get("endpoint");

// POST mit JSON Body
Services::HttpResponse resp = Services::ApiService::Post("endpoint", jsonBody);

// PUT
Services::HttpResponse resp = Services::ApiService::Put("endpoint", jsonBody);
```

### Event Handling
```cpp
// Alle Eingaben werden im Event Loop verarbeitet (nicht in Render Loop)
while (window.pollEvent(event)) {
    // Text Input, Mouse Clicks, etc.
}
```

## 🐛 Bekannte Probleme & Lösungen

### Text-Input Verdopplung (GELÖST)
- **Problem**: Text wurde 10x pro Keystroke eingegeben
- **Lösung**: Alle Input-Handler in Event Loop verschieben (nicht Render Loop)

### JSON Parsing Fehler (GELÖST)
- **Problem**: userId Extraktion schlug fehl bei unterschiedlicher JSON-Feldordnung
- **Lösung**: Komplette Objekte zwischen { und } extrahieren, mit Marker-Feld validieren

## 🔄 State Management

### Main View States (MainView.hpp)

```cpp
// Tab Navigation
int activeTab = 0;  // 0=Home, 1=Upload, 2=Extraction, 3=Admin, 4=Settings, 5=Profile

// Authentication
std::string loggedInUsername;
std::string loggedInRole;
bool isLoggedIn = false;

// Documents
std::vector<std::pair<std::string, std::string>> myDocuments;  // (fileId, fileName)
bool documentsLoaded = false;

// Admin States
std::vector<UserInfo> adminUsers;
std::vector<DocumentInfo> adminDocuments;
std::vector<ExtractionInfo> adminExtractions;

// UI States
bool showCreateUserForm = false;
bool showEditUserForm = false;
std::string extractionStatus = "";
bool extractionCompleted = false;
```

## 📊 Datenstrukturen

### UserInfo
```cpp
struct UserInfo {
    std::string userId;
    std::string username;
    std::string email;
    std::string role;
    std::string createdAt;
    std::string lastLogin;
    bool isActive;
};
```

### DocumentInfo
```cpp
struct DocumentInfo {
    std::string fileId;
    std::string fileName;
    std::string uploadedBy;
    std::string uploadedAt;
    std::string fileSize;
};
```

### ExtractionInfo
```cpp
struct ExtractionInfo {
    std::string extractionId;
    std::string fileName;
    std::string extractionMethod;
    std::string completedAt;
    std::string status;  // "0"=Unbearbeitet, "2"=Completed, "3"=Failed
    std::string uploadedBy;
};
```

## 🛠️ Development

### Kompilieren mit Debugging
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Console-Output
- `std::cout` wird verwendet für Debugging
- Zeigt API-Responses, Fehler, Erfolgs-Meldungen

### Schreiben Sie neue Features
1. Erstellen Sie eine neue UseCase in `include/UseCases/`
2. Implementieren Sie den UseCase in `src/UseCases/`
3. Verwenden Sie ihn in MainViewModel
4. Rendern Sie UI in MainView.hpp

## 📄 Lizenz

MIT License - siehe LICENSE Datei

## 👥 Kontakt

Entwickler: Christoph

---

**Letzte Aktualisierung**: Dezember 2025
**Version**: 2.0 - Admin Dashboard komplett
