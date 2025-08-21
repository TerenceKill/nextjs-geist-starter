#ifndef CONFIG_H
#define CONFIG_H

// Konfigurationsdatei für Smart Kühlschrank Firmware
// Definiert alle wichtigen Konstanten und Einstellungen

// Dateipfade für Sensor-Dateien im Workspace Verzeichnis
#define TEMPERATURE_FILE "Workspace/temperatur.txt"
#define DOOR_FILE "Workspace/tuer.txt"
#define ENERGY_FILE "Workspace/energie.txt"
#define BUTTON_FILE "Workspace/taster.txt"

// Schwellenwerte für Alarme und Warnungen
#define MAX_TEMP_THRESHOLD 8.0f     // Maximale Innentemperatur in °C
#define MIN_TEMP_THRESHOLD -2.0f    // Minimale Innentemperatur in °C
#define DOOR_OPEN_THRESHOLD 30      // Maximale Türöffnungszeit in Sekunden
#define MAX_ENERGY_THRESHOLD 200.0f // Maximaler Energieverbrauch in Watt

// Soll-Werte für den Kühlschrank
#define TARGET_TEMPERATURE 4.0f     // Zieltemperatur in °C
#define TARGET_ENERGY 120.0f        // Normaler Energieverbrauch in Watt

// I2C Display Konfiguration
#define DISPLAY_ROWS 2              // Anzahl der Zeilen
#define DISPLAY_COLS 40             // Anzahl der Spalten pro Zeile

// Timing-Konfiguration
#define SENSOR_UPDATE_INTERVAL 1    // Sensor-Überprüfung alle 1 Sekunde
#define SENSOR_WRITE_INTERVAL 5     // Sensor-Werte schreiben alle 5 Sekunden

// Logging-Level Definitionen
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3
} LogLevel;

// Sensor-Datenstruktur
typedef struct {
    float temperatur;               // Aktuelle Temperatur in °C
    int tuer_offen;                // 1 = offen, 0 = geschlossen
    float energie_verbrauch;        // Aktueller Energieverbrauch in Watt
    long tuer_offen_seit;          // Zeitstempel wann Tür geöffnet wurde
    int gueltig;                   // 1 = Daten gültig, 0 = Fehler beim Lesen
} SensorDaten;

// Display-Status Struktur
typedef struct {
    char zeile1[DISPLAY_COLS + 1]; // Erste Zeile + Null-Terminator
    char zeile2[DISPLAY_COLS + 1]; // Zweite Zeile + Null-Terminator
    int aktualisiert;              // Flag für Display-Update
} DisplayStatus;

// Globale Konfigurationswerte
extern int aktuelle_log_stufe;     // Aktuelle Logging-Stufe
extern int debug_modus;            // Debug-Modus aktiviert/deaktiviert

#endif // CONFIG_H
