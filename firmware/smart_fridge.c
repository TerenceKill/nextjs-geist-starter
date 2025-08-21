// Für usleep() unter C99
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "config.h"
#include "logging.h"
#include "sensor.h"
#include "display.h"

// Globale Variablen für Programmsteuerung
static volatile int programm_laeuft = 1;
static time_t letzter_sensor_check = 0;
static time_t letzte_taster_pruefung = 0;
static int system_initialisiert = 0;

// Funktionsdeklarationen
void signal_handler(int signal);
void system_initialisieren(void);
void hauptschleife(void);
void sensor_daten_verarbeiten(void);
void system_status_pruefen(void);
void taster_verarbeiten(void);
void system_beenden(void);
void hilfe_anzeigen(void);
void version_anzeigen(void);

/**
 * Signal-Handler für sauberes Beenden
 */
void signal_handler(int signal) {
    switch (signal) {
        case SIGINT:
            LOG_INFO_MSG("SIGINT empfangen - Programm wird beendet");
            break;
        case SIGTERM:
            LOG_INFO_MSG("SIGTERM empfangen - Programm wird beendet");
            break;
        default:
            LOG_WARNING_F("Unbekanntes Signal empfangen: %d", signal);
            break;
    }
    programm_laeuft = 0;
}

/**
 * Initialisiert alle Systemkomponenten
 */
void system_initialisieren(void) {
    printf("=== Smart Kühlschrank Firmware v1.0 ===\n");
    printf("Initialisierung wird gestartet...\n\n");
    
    // Signal-Handler registrieren
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Zufallsgenerator initialisieren
    srand(time(NULL));
    
    // Logging-System initialisieren
    logging_initialisieren();
    LOG_INFO_MSG("=== SYSTEM START ===");
    
    // Display-System initialisieren
    display_initialisieren();
    
    // Kurze Pause für Startbildschirm
    sleep(2);
    
    // Sensor-System initialisieren
    sensor_system_initialisieren();
    
    // Systeminformationen auf Display anzeigen
    display_systeminfo_anzeigen();
    sleep(3);
    
    system_initialisiert = 1;
    LOG_INFO_MSG("Alle Systeme erfolgreich initialisiert");
    
    printf("\nSystem bereit! Drücken Sie Ctrl+C zum Beenden.\n");
    printf("Log-Level ändern: echo '1' > %s\n", BUTTON_FILE);
    printf("Sensor-Werte ändern: Dateien in %s bearbeiten\n\n", "Workspace/");
}

/**
 * Hauptschleife des Programms
 */
void hauptschleife(void) {
    LOG_INFO_MSG("Hauptschleife gestartet");
    
    while (programm_laeuft) {
        time_t jetzt = time(NULL);
        
        // Sensor-Simulation (alle 5 Sekunden neue Werte schreiben)
        sensor_werte_simulieren_und_schreiben();
        
        // Sensor-Daten verarbeiten (jede Sekunde prüfen)
        if (jetzt - letzter_sensor_check >= SENSOR_UPDATE_INTERVAL) {
            sensor_daten_verarbeiten();
            letzter_sensor_check = jetzt;
        }
        
        // Taster-Eingabe prüfen (alle 2 Sekunden)
        if (jetzt - letzte_taster_pruefung >= 2) {
            taster_verarbeiten();
            letzte_taster_pruefung = jetzt;
        }
        
        // System-Status prüfen
        system_status_pruefen();
        
        // Kurze Pause um CPU-Last zu reduzieren
        usleep(100000); // 100ms
    }
    
    LOG_INFO_MSG("Hauptschleife beendet");
}

/**
 * Verarbeitet Sensor-Daten und aktualisiert Display
 */
void sensor_daten_verarbeiten(void) {
    SensorDaten neue_daten;
    
    // Prüfen ob sich Sensor-Dateien geändert haben
    int aenderungen = 0;
    aenderungen += datei_wurde_geaendert(TEMPERATURE_FILE, TEMP_DATEI_INDEX);
    aenderungen += datei_wurde_geaendert(DOOR_FILE, TUER_DATEI_INDEX);
    aenderungen += datei_wurde_geaendert(ENERGY_FILE, ENERGIE_DATEI_INDEX);
    
    // Sensor-Werte lesen (immer, auch ohne Änderungen für Zeitstempel-Updates)
    if (sensor_werte_lesen(&neue_daten)) {
        // Daten mit vorherigen vergleichen für Änderungslog
        if (memcmp(&aktuelle_sensordaten, &neue_daten, sizeof(SensorDaten)) != 0 || aenderungen > 0) {
            LOG_DEBUG_MSG("Sensor-Daten aktualisiert");
            
            // Temperatur-Änderung loggen
            if (aktuelle_sensordaten.temperatur != neue_daten.temperatur) {
                LOG_INFO_F("Temperatur geändert: %.2f°C -> %.2f°C", 
                          aktuelle_sensordaten.temperatur, neue_daten.temperatur);
            }
            
            // Tür-Status-Änderung loggen
            if (aktuelle_sensordaten.tuer_offen != neue_daten.tuer_offen) {
                LOG_INFO_F("Tür-Status geändert: %s -> %s",
                          aktuelle_sensordaten.tuer_offen ? "offen" : "geschlossen",
                          neue_daten.tuer_offen ? "offen" : "geschlossen");
            }
            
            // Energie-Änderung loggen (nur bei größeren Änderungen)
            float energie_diff = neue_daten.energie_verbrauch - aktuelle_sensordaten.energie_verbrauch;
            if (energie_diff > 10.0f || energie_diff < -10.0f) {
                LOG_INFO_F("Energieverbrauch geändert: %.1fW -> %.1fW (Δ%.1fW)",
                          aktuelle_sensordaten.energie_verbrauch, 
                          neue_daten.energie_verbrauch, energie_diff);
            }
            
            // Aktuelle Daten aktualisieren
            aktuelle_sensordaten = neue_daten;
        }
        
        // Alarme prüfen
        int probleme = sensor_alarme_pruefen(&neue_daten);
        if (probleme > 0) {
            LOG_WARNING_F("Sensor-Alarme erkannt: %d Problem(e)", probleme);
        }
        
        // Display aktualisieren
        display_aktualisieren(&neue_daten, log_level_abfragen());
        
    } else {
        LOG_ERROR_MSG("Fehler beim Lesen der Sensor-Daten");
        display_fehler_anzeigen("Sensor-Lesefehler");
    }
}

/**
 * Verarbeitet Taster-Eingaben für Log-Level-Änderung
 */
void taster_verarbeiten(void) {
    int altes_level = log_level_abfragen();
    
    // Taster prüfen und Log-Level ggf. erhöhen
    taster_pruefen_und_log_level_erhoehen();
    
    int neues_level = log_level_abfragen();
    
    // Bei Änderung Display aktualisieren
    if (altes_level != neues_level) {
        LOG_INFO_F("Log-Level durch Taster geändert: %d -> %d", altes_level, neues_level);
        display_aktualisieren(&aktuelle_sensordaten, neues_level);
        
        // Kurze Bestätigung auf Display
        char meldung[40];
        snprintf(meldung, sizeof(meldung), "Log-Level: %s", log_level_zu_string(neues_level));
        display_warnung_anzeigen(meldung);
        
        // Nach 2 Sekunden normales Display wiederherstellen
        sleep(2);
        display_aktualisieren(&aktuelle_sensordaten, neues_level);
    }
}

/**
 * Überprüft allgemeinen System-Status
 */
void system_status_pruefen(void) {
    static time_t letzter_status_check = 0;
    time_t jetzt = time(NULL);
    
    // Nur alle 30 Sekunden prüfen
    if (jetzt - letzter_status_check < 30) {
        return;
    }
    
    LOG_DEBUG_MSG("System-Status wird geprüft");
    
    // Speicher-Status prüfen (vereinfacht)
    // In echter Implementierung würde hier malloc_stats() o.ä. verwendet
    
    // Datei-System prüfen
    if (access("Workspace", F_OK) != 0) {
        LOG_ERROR_MSG("Workspace-Verzeichnis nicht zugänglich!");
        display_fehler_anzeigen("Workspace-Fehler");
    }
    
    // Log-Datei-Größe prüfen
    FILE* log_datei = fopen("kuehlschrank.log", "r");
    if (log_datei != NULL) {
        fseek(log_datei, 0, SEEK_END);
        long log_groesse = ftell(log_datei);
        fclose(log_datei);
        
        // Warnung bei großer Log-Datei (>1MB)
        if (log_groesse > 1024 * 1024) {
            LOG_WARNING_F("Log-Datei wird groß: %ld Bytes", log_groesse);
        }
    }
    
    letzter_status_check = jetzt;
    LOG_DEBUG_MSG("System-Status OK");
}

/**
 * Beendet das System ordnungsgemäß
 */
void system_beenden(void) {
    LOG_INFO_MSG("System-Shutdown wird eingeleitet");
    
    if (system_initialisiert) {
        // Display-Abschiedsmeldung
        display_beenden();
        sleep(2);
        
        // Systeme herunterfahren
        sensor_system_beenden();
        logging_beenden();
    }
    
    printf("\nSmart Kühlschrank Firmware beendet.\n");
    printf("Auf Wiedersehen!\n");
}

/**
 * Zeigt Hilfe-Informationen an
 */
void hilfe_anzeigen(void) {
    printf("Smart Kühlschrank Firmware v1.0\n");
    printf("================================\n\n");
    printf("Verwendung: %s [Optionen]\n\n", "smart_fridge");
    printf("Optionen:\n");
    printf("  -h, --help     Zeigt diese Hilfe an\n");
    printf("  -v, --version  Zeigt Versionsinformationen an\n\n");
    printf("Steuerung während der Laufzeit:\n");
    printf("  Ctrl+C         Programm beenden\n");
    printf("  echo '1' > %s  Log-Level erhöhen\n", BUTTON_FILE);
    printf("\nSensor-Dateien (manuell editierbar):\n");
    printf("  %s  Temperatur in °C\n", TEMPERATURE_FILE);
    printf("  %s       Tür-Status (0=zu, 1=offen) und Zeitstempel\n", DOOR_FILE);
    printf("  %s     Energieverbrauch in Watt\n", ENERGY_FILE);
    printf("\nDisplay-Ausgabe:\n");
    printf("  %s     Aktueller Display-Inhalt\n", DISPLAY_DATEI);
    printf("  kuehlschrank.log        System-Log-Datei\n");
}

/**
 * Zeigt Versionsinformationen an
 */
void version_anzeigen(void) {
    printf("Smart Kühlschrank Firmware\n");
    printf("Version: 1.0\n");
    printf("Build: %s %s\n", __DATE__, __TIME__);
    printf("Compiler: %s\n", __VERSION__);
    printf("\nKonfiguration:\n");
    printf("  Max. Temperatur: %.1f°C\n", MAX_TEMP_THRESHOLD);
    printf("  Min. Temperatur: %.1f°C\n", MIN_TEMP_THRESHOLD);
    printf("  Soll-Temperatur: %.1f°C\n", TARGET_TEMPERATURE);
    printf("  Max. Tür-Öffnungszeit: %d Sekunden\n", DOOR_OPEN_THRESHOLD);
    printf("  Max. Energieverbrauch: %.0fW\n", MAX_ENERGY_THRESHOLD);
    printf("  Display: %dx%d Zeichen\n", DISPLAY_ROWS, DISPLAY_COLS);
}

/**
 * Hauptfunktion
 */
int main(int argc, char* argv[]) {
    // Kommandozeilen-Argumente verarbeiten
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            hilfe_anzeigen();
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            version_anzeigen();
            return 0;
        } else {
            printf("Unbekannte Option: %s\n", argv[i]);
            printf("Verwenden Sie -h für Hilfe.\n");
            return 1;
        }
    }
    
    // System initialisieren
    system_initialisieren();
    
    // Hauptschleife ausführen
    hauptschleife();
    
    // System ordnungsgemäß beenden
    system_beenden();
    
    return 0;
}
