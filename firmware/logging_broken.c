#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

// Globale Variablen für das Logging-System
int aktuelle_log_stufe = LOG_INFO;  // Standard Log-Level
static FILE* log_datei = NULL;      // Log-Datei Handle
static int letzter_taster_zustand = 0; // Für Taster-Entprellung

/**
 * Initialisiert das Logging-System
 */
void logging_initialisieren(void) {
    // Log-Datei öffnen (append mode)
    log_datei = fopen("kuehlschrank.log", "a");
    if (log_datei == NULL) {
        fprintf(stderr, "FEHLER: Konnte Log-Datei nicht öffnen!\n");
        log_datei = stderr; // Fallback auf stderr
    }
    
    // Initialisierungs-Nachricht loggen
    log_nachricht(LOG_INFO, "=== Kühlschrank Firmware gestartet ===");
    log_formatiert(LOG_INFO, "Log-Level initialisiert auf: %s", 
                   log_level_zu_string(aktuelle_log_stufe));
}

/**
 * Schreibt eine Log-Nachricht mit Zeitstempel
 */
void log_nachricht(LogLevel level, const char* nachricht) {
    // Prüfen ob Nachricht geloggt werden soll
    if (level < aktuelle_log_stufe) {
        return;
    }
    
    // Zeitstempel erstellen
    time_t jetzt;
    struct tm* zeitinfo;
    char zeitstempel[64];
    
    time(&jetzt);
    zeitinfo = localtime(&jetzt);
    strftime(zeitstempel, sizeof(zeitstempel), "%Y-%m-%d %H:%M:%S", zeitinfo);
    
    // Log-Nachricht formatieren und ausgeben
    const char* level_str = log_level_zu_string(level);
    
    // In Datei schreiben
    if (log_datei != NULL) {
        fprintf(log_datei, "[%s] %s: %s\n", zeitstempel, level_str, nachricht);
        fflush(log_datei); // Sofort schreiben für Debugging
    }
    
    // Auch auf Konsole ausgeben für Entwicklung
    printf("[%s] %s: %s\n", zeitstempel, level_str, nachricht);
}

/**
 * Schreibt eine formatierte Log-Nachricht
 */
void log_formatiert(LogLevel level, const char* format, ...) {
    if (level < aktuelle_log_stufe) {
        return;
    }
    
    // Variable Argumente verarbeiten
    va_list args;
    char puffer[512];
    
    va_start(args, format);
    vsnprintf(puffer, sizeof(puffer), format, args);
    va_end(args);
    
    // An normale Log-Funktion weiterleiten
    log_nachricht(level, puffer);
}

/**
 * Setzt das Log-Level mit Validierung
 */
void log_level_setzen(int neues_level) {
    // Gültigkeit prüfen
    if (neues_level < LOG_DEBUG || neues_level > LOG_ERROR) {
        log_formatiert(LOG_WARNING, "Ungültiges Log-Level: %d. Behalte aktuelles Level bei.", neues_level);
        return;
    }
    
    LogLevel altes_level = aktuelle_log_stufe;
    aktuelle_log_stufe = neues_level;
    
    log_formatiert(LOG_INFO, "Log-Level geändert von %s zu %s", 
                   log_level_zu_string(altes_level), 
                   log_level_zu_string(aktuelle_log_stufe));
}

/**
 * Gibt aktuelles Log-Level zurück
 */
int log_level_abfragen(void) {
    return aktuelle_log_stufe;
}

/**
 * Überprüft Taster und erhöht Log-Level stufenweise
 */
void taster_pruefen_und_log_level_erhoehen(void) {
    FILE* taster_datei = fopen(BUTTON_FILE, "r");
    if (taster_datei == NULL) {
        // Taster-Datei existiert nicht - erstelle sie mit Standardwert
        taster_datei = fopen(BUTTON_FILE, "w");
        if (taster_datei != NULL) {
            fprintf(taster_datei, "0\n");
            fclose(taster_datei);
        }
        return;
    }
    
    int taster_zustand = 0;
    if (fscanf(taster_datei, "%d", &taster_zustand) == 1) {
        // Taster-Entprellung: nur bei steigender Flanke reagieren
        if (taster_zustand == 1 && letzter_taster_zustand == 0) {
            // Log-Level erhöhen (zyklisch)
            int neues_level = (aktuelle_log_stufe + 1) % 4;
            log_level_setzen(neues_level);
            
            // Taster zurücksetzen
            fclose(taster_datei);
            taster_datei = fopen(BUTTON_FILE, "w");
            if (taster_datei != NULL) {
                fprintf(taster_datei, "0\n");
            }
        }
        letzter_taster_zustand = taster_zustand;
    }
    
    if (taster_datei != NULL) {
        fclose(taster_datei);
    }
}

/**
 * Konvertiert Log-Level zu lesbarem String
 */
const char* log_level_zu_string(LogLevel level) {
    switch (level) {
        case LOG_DEBUG:   return "DEBUG";
        case LOG_INFO:    return "INFO";
        case LOG_WARNING: return "WARNUNG";
        case LOG_ERROR:   return "FEHLER";
        default:          return "UNBEKANNT";
    }
}

/**
 * Beendet das Logging-System ordnungsgemäß
 */
void logging_beenden(void) {
    log_nachricht(LOG_INFO, "=== Kühlschrank Firmware beendet ===");
    
    if (log_datei != NULL && log_datei != stderr) {
        fclose(log_datei);
        log_datei = NULL;
    }
}
