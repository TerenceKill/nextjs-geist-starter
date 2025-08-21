#include "sensor.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

// Globale Variablen
SensorDaten aktuelle_sensordaten = {0};
DateiInfo datei_infos[4] = {0};

// Statische Variablen für Simulation
static time_t letzter_schreibvorgang = 0;
static float basis_temperatur = 4.0f;  // Basis für Temperaturschwankungen

/**
 * Initialisiert das Sensor-System
 */
void sensor_system_initialisieren(void) {
    LOG_INFO_MSG("Sensor-System wird initialisiert...");
    
    // Standard-Sensor-Dateien erstellen falls nicht vorhanden
    standard_sensor_dateien_erstellen();
    
    // Initiale Sensor-Werte lesen
    if (sensor_werte_lesen(&aktuelle_sensordaten)) {
        LOG_INFO_MSG("Sensor-System erfolgreich initialisiert");
        LOG_DEBUG_F("Initiale Temperatur: %.1f°C", aktuelle_sensordaten.temperatur);
        LOG_DEBUG_F("Initiale Tür-Status: %s", 
                   aktuelle_sensordaten.tuer_offen ? "offen" : "geschlossen");
        LOG_DEBUG_F("Initialer Energieverbrauch: %.1fW", aktuelle_sensordaten.energie_verbrauch);
    } else {
        LOG_ERROR_MSG("Fehler beim Initialisieren des Sensor-Systems");
    }
    
    letzter_schreibvorgang = time(NULL);
}

/**
 * Liest alle Sensor-Werte aus den Dateien
 */
int sensor_werte_lesen(SensorDaten* daten) {
    if (daten == NULL) {
        LOG_ERROR_MSG("Ungültiger Zeiger für Sensor-Daten");
        return 0;
    }
    
    int erfolg = 1;
    
    // Temperatur lesen
    if (!temperatur_lesen(&daten->temperatur)) {
        LOG_WARNING_MSG("Fehler beim Lesen der Temperatur");
        daten->temperatur = TARGET_TEMPERATURE; // Fallback-Wert
        erfolg = 0;
    }
    
    // Tür-Status lesen
    if (!tuer_status_lesen(&daten->tuer_offen, &daten->tuer_offen_seit)) {
        LOG_WARNING_MSG("Fehler beim Lesen des Tür-Status");
        daten->tuer_offen = 0; // Fallback: Tür geschlossen
        daten->tuer_offen_seit = 0;
        erfolg = 0;
    }
    
    // Energieverbrauch lesen
    if (!energie_lesen(&daten->energie_verbrauch)) {
        LOG_WARNING_MSG("Fehler beim Lesen des Energieverbrauchs");
        daten->energie_verbrauch = TARGET_ENERGY; // Fallback-Wert
        erfolg = 0;
    }
    
    // Daten validieren
    if (!sensor_werte_validieren(daten)) {
        LOG_WARNING_MSG("Sensor-Werte sind nicht plausibel");
        erfolg = 0;
    }
    
    daten->gueltig = erfolg;
    return erfolg;
}

/**
 * Schreibt simulierte Sensor-Werte (alle 5 Sekunden)
 */
void sensor_werte_simulieren_und_schreiben(void) {
    time_t jetzt = time(NULL);
    
    // Prüfen ob 5 Sekunden vergangen sind
    if (jetzt - letzter_schreibvorgang < SENSOR_WRITE_INTERVAL) {
        return;
    }
    
    LOG_DEBUG_MSG("Generiere neue Sensor-Werte...");
    
    SensorDaten neue_daten;
    zufaellige_sensor_werte_generieren(&neue_daten);
    
    // Temperatur schreiben
    FILE* temp_datei = fopen(TEMPERATURE_FILE, "w");
    if (temp_datei != NULL) {
        fprintf(temp_datei, "%.2f\n", neue_daten.temperatur);
        fclose(temp_datei);
        LOG_DEBUG_F("Neue Temperatur geschrieben: %.2f°C", neue_daten.temperatur);
    }
    
    // Tür-Status schreiben
    FILE* tuer_datei = fopen(DOOR_FILE, "w");
    if (tuer_datei != NULL) {
        fprintf(tuer_datei, "%d %ld\n", neue_daten.tuer_offen, neue_daten.tuer_offen_seit);
        fclose(tuer_datei);
        LOG_DEBUG_F("Neuer Tür-Status geschrieben: %s", 
                   neue_daten.tuer_offen ? "offen" : "geschlossen");
    }
    
    // Energieverbrauch schreiben
    FILE* energie_datei = fopen(ENERGY_FILE, "w");
    if (energie_datei != NULL) {
        fprintf(energie_datei, "%.2f\n", neue_daten.energie_verbrauch);
        fclose(energie_datei);
        LOG_DEBUG_F("Neuer Energieverbrauch geschrieben: %.2fW", neue_daten.energie_verbrauch);
    }
    
    letzter_schreibvorgang = jetzt;
}

/**
 * Prüft ob sich eine Datei geändert hat
 */
int datei_wurde_geaendert(const char* dateiname, int datei_index) {
    struct stat datei_stat;
    
    if (stat(dateiname, &datei_stat) != 0) {
        // Datei existiert nicht
        if (datei_infos[datei_index].datei_existiert) {
            LOG_WARNING_F("Datei %s ist verschwunden", dateiname);
            datei_infos[datei_index].datei_existiert = 0;
            return 1; // Änderung erkannt
        }
        return 0;
    }
    
    // Datei existiert
    if (!datei_infos[datei_index].datei_existiert) {
        LOG_INFO_F("Datei %s wurde erstellt", dateiname);
        datei_infos[datei_index].datei_existiert = 1;
        datei_infos[datei_index].letzte_aenderung = datei_stat.st_mtime;
        return 1;
    }
    
    // Prüfen ob Änderungszeit unterschiedlich ist
    if (datei_stat.st_mtime != datei_infos[datei_index].letzte_aenderung) {
        LOG_DEBUG_F("Datei %s wurde geändert", dateiname);
        datei_infos[datei_index].letzte_aenderung = datei_stat.st_mtime;
        return 1;
    }
    
    return 0;
}

/**
 * Liest Temperatur aus Datei
 */
int temperatur_lesen(float* temperatur) {
    FILE* datei = fopen(TEMPERATURE_FILE, "r");
    if (datei == NULL) {
        return 0;
    }
    
    int erfolg = (fscanf(datei, "%f", temperatur) == 1);
    fclose(datei);
    
    return erfolg;
}

/**
 * Liest Tür-Status aus Datei
 */
int tuer_status_lesen(int* tuer_offen, long* offen_seit) {
    FILE* datei = fopen(DOOR_FILE, "r");
    if (datei == NULL) {
        return 0;
    }
    
    int erfolg = (fscanf(datei, "%d %ld", tuer_offen, offen_seit) == 2);
    fclose(datei);
    
    return erfolg;
}

/**
 * Liest Energieverbrauch aus Datei
 */
int energie_lesen(float* energie) {
    FILE* datei = fopen(ENERGY_FILE, "r");
    if (datei == NULL) {
        return 0;
    }
    
    int erfolg = (fscanf(datei, "%f", energie) == 1);
    fclose(datei);
    
    return erfolg;
}

/**
 * Validiert Sensor-Werte auf Plausibilität
 */
int sensor_werte_validieren(const SensorDaten* daten) {
    // Temperatur-Bereich prüfen (-50°C bis +50°C)
    if (daten->temperatur < -50.0f || daten->temperatur > 50.0f) {
        LOG_WARNING_F("Temperatur außerhalb des gültigen Bereichs: %.2f°C", daten->temperatur);
        return 0;
    }
    
    // Tür-Status prüfen (0 oder 1)
    if (daten->tuer_offen != 0 && daten->tuer_offen != 1) {
        LOG_WARNING_F("Ungültiger Tür-Status: %d", daten->tuer_offen);
        return 0;
    }
    
    // Energieverbrauch prüfen (0W bis 1000W)
    if (daten->energie_verbrauch < 0.0f || daten->energie_verbrauch > 1000.0f) {
        LOG_WARNING_F("Energieverbrauch außerhalb des gültigen Bereichs: %.2fW", daten->energie_verbrauch);
        return 0;
    }
    
    return 1;
}

/**
 * Erstellt Standard-Sensor-Dateien
 */
void standard_sensor_dateien_erstellen(void) {
    // Temperatur-Datei
    if (access(TEMPERATURE_FILE, F_OK) != 0) {
        FILE* datei = fopen(TEMPERATURE_FILE, "w");
        if (datei != NULL) {
            fprintf(datei, "%.2f\n", TARGET_TEMPERATURE);
            fclose(datei);
            LOG_INFO_F("Standard-Temperatur-Datei erstellt: %s", TEMPERATURE_FILE);
        }
    }
    
    // Tür-Datei
    if (access(DOOR_FILE, F_OK) != 0) {
        FILE* datei = fopen(DOOR_FILE, "w");
        if (datei != NULL) {
            fprintf(datei, "0 0\n"); // Tür geschlossen, seit 0
            fclose(datei);
            LOG_INFO_F("Standard-Tür-Datei erstellt: %s", DOOR_FILE);
        }
    }
    
    // Energie-Datei
    if (access(ENERGY_FILE, F_OK) != 0) {
        FILE* datei = fopen(ENERGY_FILE, "w");
        if (datei != NULL) {
            fprintf(datei, "%.2f\n", TARGET_ENERGY);
            fclose(datei);
            LOG_INFO_F("Standard-Energie-Datei erstellt: %s", ENERGY_FILE);
        }
    }
    
    // Taster-Datei
    if (access(BUTTON_FILE, F_OK) != 0) {
        FILE* datei = fopen(BUTTON_FILE, "w");
        if (datei != NULL) {
            fprintf(datei, "0\n"); // Taster nicht gedrückt
            fclose(datei);
            LOG_INFO_F("Standard-Taster-Datei erstellt: %s", BUTTON_FILE);
        }
    }
}

/**
 * Generiert realistische Zufallswerte
 */
void zufaellige_sensor_werte_generieren(SensorDaten* daten) {
    // Temperatur: Schwankung um Basis-Temperatur ±2°C
    float temp_schwankung = ((float)rand() / RAND_MAX - 0.5f) * 4.0f;
    daten->temperatur = basis_temperatur + temp_schwankung;
    
    // Tür: 90% geschlossen, 10% offen
    daten->tuer_offen = (rand() % 10 == 0) ? 1 : 0;
    daten->tuer_offen_seit = daten->tuer_offen ? time(NULL) : 0;
    
    // Energie: Basis-Verbrauch ±30W
    float energie_schwankung = ((float)rand() / RAND_MAX - 0.5f) * 60.0f;
    daten->energie_verbrauch = TARGET_ENERGY + energie_schwankung;
    
    // Bei offener Tür höherer Energieverbrauch
    if (daten->tuer_offen) {
        daten->energie_verbrauch += 50.0f;
    }
    
    daten->gueltig = 1;
}

/**
 * Überprüft kritische Sensor-Zustände
 */
int sensor_alarme_pruefen(const SensorDaten* daten) {
    int probleme = 0;
    
    // Temperatur zu hoch
    if (daten->temperatur > MAX_TEMP_THRESHOLD) {
        LOG_WARNING_F("ALARM: Temperatur zu hoch! %.2f°C (Max: %.2f°C)", 
                     daten->temperatur, MAX_TEMP_THRESHOLD);
        probleme++;
    }
    
    // Temperatur zu niedrig
    if (daten->temperatur < MIN_TEMP_THRESHOLD) {
        LOG_WARNING_F("ALARM: Temperatur zu niedrig! %.2f°C (Min: %.2f°C)", 
                     daten->temperatur, MIN_TEMP_THRESHOLD);
        probleme++;
    }
    
    // Tür zu lange offen
    if (daten->tuer_offen) {
        long offen_dauer = tuer_offen_dauer_berechnen(daten->tuer_offen_seit);
        if (offen_dauer > DOOR_OPEN_THRESHOLD) {
            LOG_WARNING_F("ALARM: Tür zu lange offen! %ld Sekunden (Max: %d)", 
                         offen_dauer, DOOR_OPEN_THRESHOLD);
            probleme++;
        }
    }
    
    // Energieverbrauch zu hoch
    if (daten->energie_verbrauch > MAX_ENERGY_THRESHOLD) {
        LOG_WARNING_F("ALARM: Energieverbrauch zu hoch! %.2fW (Max: %.2fW)", 
                     daten->energie_verbrauch, MAX_ENERGY_THRESHOLD);
        probleme++;
    }
    
    return probleme;
}

/**
 * Berechnet Tür-Öffnungsdauer
 */
long tuer_offen_dauer_berechnen(long offen_seit) {
    if (offen_seit == 0) {
        return 0;
    }
    return time(NULL) - offen_seit;
}

/**
 * Beendet das Sensor-System
 */
void sensor_system_beenden(void) {
    LOG_INFO_MSG("Sensor-System wird beendet");
}
