#ifndef LOGGING_H
#define LOGGING_H

#include "config.h"
#include <stdio.h>
#include <time.h>

// Logging-System für Smart Kühlschrank Firmware
// Unterstützt verschiedene Log-Level und dynamische Anpassung

// Globale Variable für aktuelles Log-Level (wird in config.h deklariert)
// extern LogLevel aktuelle_log_stufe; // Bereits in config.h definiert

// Funktionsdeklarationen

/**
 * Initialisiert das Logging-System
 * Setzt Standard-Log-Level und öffnet Log-Dateien
 */
void logging_initialisieren(void);

/**
 * Schreibt eine Log-Nachricht mit angegebenem Level
 * @param level Log-Level (DEBUG, INFO, WARNING, ERROR)
 * @param nachricht Die zu loggende Nachricht
 */
void log_nachricht(LogLevel level, const char* nachricht);

/**
 * Schreibt eine formatierte Log-Nachricht
 * @param level Log-Level
 * @param format Printf-ähnliches Format
 * @param ... Variable Argumente für Format
 */
void log_formatiert(LogLevel level, const char* format, ...);

/**
 * Setzt das aktuelle Log-Level
 * @param neues_level Neues Log-Level (0-3)
 */
void log_level_setzen(int neues_level);

/**
 * Gibt das aktuelle Log-Level zurück
 * @return Aktuelles Log-Level
 */
int log_level_abfragen(void);

/**
 * Überprüft Taster-Eingabe und erhöht Log-Level bei Bedarf
 * Liest BUTTON_FILE und erhöht Log-Level stufenweise
 */
void taster_pruefen_und_log_level_erhoehen(void);

/**
 * Konvertiert Log-Level zu String für Anzeige
 * @param level Log-Level
 * @return String-Darstellung des Levels
 */
const char* log_level_zu_string(LogLevel level);

/**
 * Beendet das Logging-System ordnungsgemäß
 * Schließt offene Dateien und gibt Ressourcen frei
 */
void logging_beenden(void);

// Makros für einfache Verwendung
#define LOG_DEBUG_MSG(msg) log_nachricht(LOG_DEBUG, msg)
#define LOG_INFO_MSG(msg) log_nachricht(LOG_INFO, msg)
#define LOG_WARNING_MSG(msg) log_nachricht(LOG_WARNING, msg)
#define LOG_ERROR_MSG(msg) log_nachricht(LOG_ERROR, msg)

// Formatierte Logging-Makros
#define LOG_DEBUG_F(fmt, ...) log_formatiert(LOG_DEBUG, fmt, __VA_ARGS__)
#define LOG_INFO_F(fmt, ...) log_formatiert(LOG_INFO, fmt, __VA_ARGS__)
#define LOG_WARNING_F(fmt, ...) log_formatiert(LOG_WARNING, fmt, __VA_ARGS__)
#define LOG_ERROR_F(fmt, ...) log_formatiert(LOG_ERROR, fmt, __VA_ARGS__)

#endif // LOGGING_H
