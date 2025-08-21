#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"

// I2C Display Treiber für Smart Kühlschrank
// Simuliert ein 2x40 Zeichen I2C Display

// Display-Puffer Struktur
typedef struct {
    char zeile1[DISPLAY_COLS + 1];  // Erste Zeile + Null-Terminator
    char zeile2[DISPLAY_COLS + 1];  // Zweite Zeile + Null-Terminator
    int aktualisiert;               // Flag für Änderungen
    int initialisiert;              // Flag für Initialisierung
} DisplayPuffer;

// Globaler Display-Puffer
extern DisplayPuffer display_puffer;

// Funktionsdeklarationen

/**
 * Initialisiert das I2C Display System
 * Setzt Display-Puffer zurück und zeigt Startmeldung
 */
void display_initialisieren(void);

/**
 * Aktualisiert das Display mit aktuellen Sensor-Daten
 * @param daten Aktuelle Sensor-Daten
 * @param log_level Aktuelles Log-Level (wird als erste Ziffer angezeigt)
 */
void display_aktualisieren(const SensorDaten* daten, int log_level);

/**
 * Zeigt eine Warnung auf dem Display an
 * @param warnung_text Text der Warnung (max. 39 Zeichen)
 */
void display_warnung_anzeigen(const char* warnung_text);

/**
 * Zeigt eine Fehlermeldung auf dem Display an
 * @param fehler_text Text der Fehlermeldung (max. 39 Zeichen)
 */
void display_fehler_anzeigen(const char* fehler_text);

/**
 * Löscht das Display (beide Zeilen)
 */
void display_loeschen(void);

/**
 * Formatiert die erste Zeile mit Sensor-Daten
 * @param zeile Puffer für die formatierte Zeile (min. 41 Zeichen)
 * @param daten Sensor-Daten
 * @param log_level Aktuelles Log-Level
 */
void display_zeile1_formatieren(char* zeile, const SensorDaten* daten, int log_level);

/**
 * Formatiert die zweite Zeile mit Status-Informationen
 * @param zeile Puffer für die formatierte Zeile (min. 41 Zeichen)
 * @param daten Sensor-Daten
 */
void display_zeile2_formatieren(char* zeile, const SensorDaten* daten);

/**
 * Gibt das Display auf der Konsole aus (für Debugging/Simulation)
 */
void display_ausgeben(void);

/**
 * Schreibt Display-Inhalt in eine Datei (für externe Überwachung)
 * @param dateiname Pfad zur Ausgabe-Datei
 */
void display_in_datei_schreiben(const char* dateiname);

/**
 * Zentriert einen Text in einer Zeile
 * @param ziel Ziel-Puffer (min. 41 Zeichen)
 * @param text Zu zentrierender Text
 */
void text_zentrieren(char* ziel, const char* text);

/**
 * Füllt eine Zeile mit Leerzeichen auf
 * @param zeile Zu füllende Zeile
 */
void zeile_auffuellen(char* zeile);

/**
 * Konvertiert Log-Level zu Anzeige-Zeichen
 * @param log_level Log-Level (0-3)
 * @return Zeichen für Display ('0'-'3')
 */
char log_level_zu_zeichen(int log_level);

/**
 * Erstellt eine Fortschrittsbalken-Darstellung
 * @param wert Aktueller Wert
 * @param minimum Minimum-Wert
 * @param maximum Maximum-Wert
 * @param breite Breite des Balkens in Zeichen
 * @param puffer Ausgabe-Puffer
 */
void fortschrittsbalken_erstellen(float wert, float minimum, float maximum, 
                                 int breite, char* puffer);

/**
 * Zeigt Startbildschirm an
 */
void display_startbildschirm_anzeigen(void);

/**
 * Zeigt Systeminformationen an
 */
void display_systeminfo_anzeigen(void);

/**
 * Beendet das Display-System ordnungsgemäß
 */
void display_beenden(void);

// Hilfsmakros für Display-Formatierung
#define DISPLAY_DATEI "Workspace/display.txt"
#define LEER_ZEILE "                                        " // 40 Leerzeichen

// Spezielle Zeichen für Display (ASCII)
#define GRAD_ZEICHEN 'C'        // °C wird als C dargestellt
#define PFEIL_RECHTS '>'        // Pfeil für Richtung
#define PFEIL_LINKS '<'         // Pfeil für Richtung
#define BALKEN_ZEICHEN '#'      // Zeichen für Fortschrittsbalken
#define LEER_BALKEN '-'         // Leerer Balken

#endif // DISPLAY_H
