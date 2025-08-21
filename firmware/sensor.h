#ifndef SENSOR_H
#define SENSOR_H

#include "config.h"
#include <time.h>

// Sensor-System für Smart Kühlschrank
// Verwaltet das Lesen und Schreiben von Sensor-Daten aus/in Dateien

// Struktur für Datei-Metadaten (zur Änderungserkennung)
typedef struct {
    time_t letzte_aenderung;       // Zeitstempel der letzten Dateiänderung
    int datei_existiert;           // Flag ob Datei existiert
} DateiInfo;

// Globale Sensor-Daten
extern SensorDaten aktuelle_sensordaten;
extern DateiInfo datei_infos[4]; // Für alle 4 Sensor-Dateien

// Funktionsdeklarationen

/**
 * Initialisiert das Sensor-System
 * Erstellt Standard-Sensor-Dateien falls sie nicht existieren
 */
void sensor_system_initialisieren(void);

/**
 * Liest alle Sensor-Werte aus den entsprechenden Dateien
 * @param daten Zeiger auf SensorDaten Struktur zum Füllen
 * @return 1 bei Erfolg, 0 bei Fehler
 */
int sensor_werte_lesen(SensorDaten* daten);

/**
 * Schreibt simulierte Sensor-Werte in die Dateien
 * Wird alle 5 Sekunden aufgerufen um neue Zufallswerte zu generieren
 */
void sensor_werte_simulieren_und_schreiben(void);

/**
 * Prüft ob sich eine Sensor-Datei geändert hat
 * @param dateiname Pfad zur zu prüfenden Datei
 * @param datei_index Index in datei_infos Array
 * @return 1 wenn geändert, 0 wenn nicht
 */
int datei_wurde_geaendert(const char* dateiname, int datei_index);

/**
 * Liest Temperatur aus der Temperatur-Datei
 * @param temperatur Zeiger zum Speichern der gelesenen Temperatur
 * @return 1 bei Erfolg, 0 bei Fehler
 */
int temperatur_lesen(float* temperatur);

/**
 * Liest Tür-Status aus der Tür-Datei
 * @param tuer_offen Zeiger zum Speichern des Tür-Status
 * @param offen_seit Zeiger zum Speichern der Öffnungszeit
 * @return 1 bei Erfolg, 0 bei Fehler
 */
int tuer_status_lesen(int* tuer_offen, long* offen_seit);

/**
 * Liest Energieverbrauch aus der Energie-Datei
 * @param energie Zeiger zum Speichern des Energieverbrauchs
 * @return 1 bei Erfolg, 0 bei Fehler
 */
int energie_lesen(float* energie);

/**
 * Validiert Sensor-Werte auf Plausibilität
 * @param daten Zeiger auf zu validierende SensorDaten
 * @return 1 wenn gültig, 0 wenn ungültig
 */
int sensor_werte_validieren(const SensorDaten* daten);

/**
 * Erstellt Standard-Sensor-Dateien mit Anfangswerten
 */
void standard_sensor_dateien_erstellen(void);

/**
 * Generiert zufällige aber realistische Sensor-Werte
 * @param daten Zeiger auf SensorDaten zum Füllen
 */
void zufaellige_sensor_werte_generieren(SensorDaten* daten);

/**
 * Überprüft kritische Sensor-Zustände und gibt Warnungen aus
 * @param daten Aktuelle Sensor-Daten
 * @return Anzahl der erkannten Probleme
 */
int sensor_alarme_pruefen(const SensorDaten* daten);

/**
 * Berechnet wie lange die Tür bereits offen ist
 * @param offen_seit Zeitstempel wann Tür geöffnet wurde
 * @return Anzahl Sekunden seit Öffnung
 */
long tuer_offen_dauer_berechnen(long offen_seit);

/**
 * Beendet das Sensor-System ordnungsgemäß
 */
void sensor_system_beenden(void);

// Hilfsmakros für Sensor-Dateien
#define TEMP_DATEI_INDEX 0
#define TUER_DATEI_INDEX 1
#define ENERGIE_DATEI_INDEX 2
#define TASTER_DATEI_INDEX 3

#endif // SENSOR_H
