#include "display.h"
#include "logging.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Globaler Display-Puffer
DisplayPuffer display_puffer = {0};

/**
 * Initialisiert das I2C Display System
 */
void display_initialisieren(void) {
    LOG_INFO_MSG("Display-System wird initialisiert...");
    
    // Display-Puffer zurücksetzen
    memset(&display_puffer, 0, sizeof(DisplayPuffer));
    
    // Startbildschirm anzeigen
    display_startbildschirm_anzeigen();
    
    display_puffer.initialisiert = 1;
    display_puffer.aktualisiert = 1;
    
    // Initial auf Konsole ausgeben
    display_ausgeben();
    
    LOG_INFO_MSG("Display-System erfolgreich initialisiert");
}

/**
 * Aktualisiert das Display mit aktuellen Sensor-Daten
 */
void display_aktualisieren(const SensorDaten* daten, int log_level) {
    if (!display_puffer.initialisiert) {
        LOG_WARNING_MSG("Display nicht initialisiert!");
        return;
    }
    
    if (daten == NULL) {
        LOG_ERROR_MSG("Ungültige Sensor-Daten für Display-Update");
        return;
    }
    
    // Neue Zeilen formatieren
    char neue_zeile1[DISPLAY_COLS + 1];
    char neue_zeile2[DISPLAY_COLS + 1];
    
    display_zeile1_formatieren(neue_zeile1, daten, log_level);
    display_zeile2_formatieren(neue_zeile2, daten);
    
    // Prüfen ob sich etwas geändert hat
    if (strcmp(display_puffer.zeile1, neue_zeile1) != 0 || 
        strcmp(display_puffer.zeile2, neue_zeile2) != 0) {
        
        // Puffer aktualisieren
        strcpy(display_puffer.zeile1, neue_zeile1);
        strcpy(display_puffer.zeile2, neue_zeile2);
        display_puffer.aktualisiert = 1;
        
        // Display ausgeben
        display_ausgeben();
        display_in_datei_schreiben(DISPLAY_DATEI);
        
        LOG_DEBUG_MSG("Display aktualisiert");
    }
}

/**
 * Zeigt eine Warnung auf dem Display an
 */
void display_warnung_anzeigen(const char* warnung_text) {
    if (warnung_text == NULL) return;
    
    // Einfache Warnung
    strncpy(display_puffer.zeile2, "WARNUNG: ", DISPLAY_COLS);
    strncat(display_puffer.zeile2, warnung_text, DISPLAY_COLS - 9);
    zeile_auffuellen(display_puffer.zeile2);
    
    display_puffer.aktualisiert = 1;
    display_ausgeben();
    display_in_datei_schreiben(DISPLAY_DATEI);
    
    LOG_DEBUG_F("Warnung auf Display angezeigt: %s", warnung_text);
}

/**
 * Zeigt eine Fehlermeldung auf dem Display an
 */
void display_fehler_anzeigen(const char* fehler_text) {
    if (fehler_text == NULL) return;
    
    // Einfache Fehlermeldung
    strncpy(display_puffer.zeile1, "FEHLER: ", DISPLAY_COLS);
    strncat(display_puffer.zeile1, fehler_text, DISPLAY_COLS - 8);
    strncpy(display_puffer.zeile2, "System pruefen!", DISPLAY_COLS);
    
    zeile_auffuellen(display_puffer.zeile1);
    zeile_auffuellen(display_puffer.zeile2);
    
    display_puffer.aktualisiert = 1;
    display_ausgeben();
    display_in_datei_schreiben(DISPLAY_DATEI);
    
    LOG_ERROR_F("Fehler auf Display angezeigt: %s", fehler_text);
}

/**
 * Löscht das Display
 */
void display_loeschen(void) {
    strcpy(display_puffer.zeile1, LEER_ZEILE);
    strcpy(display_puffer.zeile2, LEER_ZEILE);
    display_puffer.aktualisiert = 1;
    
    display_ausgeben();
    LOG_DEBUG_MSG("Display gelöscht");
}

/**
 * Formatiert die erste Zeile mit Sensor-Daten
 */
void display_zeile1_formatieren(char* zeile, const SensorDaten* daten, int log_level) {
    char log_char = log_level_zu_zeichen(log_level);
    
    // Einfaches Format ohne komplexe snprintf
    char temp_str[10], energie_str[10];
    sprintf(temp_str, "%.1f", daten->temperatur);
    sprintf(energie_str, "%.0f", daten->energie_verbrauch);
    
    strcpy(zeile, "");
    strncat(zeile, &log_char, 1);
    strcat(zeile, " T:");
    strcat(zeile, temp_str);
    strcat(zeile, "C D:");
    strcat(zeile, daten->tuer_offen ? "OFFEN" : "ZU");
    strcat(zeile, " E:");
    strcat(zeile, energie_str);
    strcat(zeile, "W");
    
    zeile_auffuellen(zeile);
}

/**
 * Formatiert die zweite Zeile mit Status-Informationen
 */
void display_zeile2_formatieren(char* zeile, const SensorDaten* daten) {
    // Einfache Status-Meldungen
    if (!daten->gueltig) {
        strcpy(zeile, "SENSOR-FEHLER!");
    }
    else if (daten->temperatur > MAX_TEMP_THRESHOLD) {
        strcpy(zeile, "TEMPERATUR ZU HOCH!");
    }
    else if (daten->temperatur < MIN_TEMP_THRESHOLD) {
        strcpy(zeile, "TEMPERATUR ZU NIEDRIG!");
    }
    else if (daten->tuer_offen) {
        long offen_dauer = time(NULL) - daten->tuer_offen_seit;
        if (offen_dauer > DOOR_OPEN_THRESHOLD) {
            strcpy(zeile, "TUER ZU LANGE OFFEN!");
        } else {
            strcpy(zeile, "Tuer ist offen");
        }
    }
    else if (daten->energie_verbrauch > MAX_ENERGY_THRESHOLD) {
        strcpy(zeile, "ENERGIEVERBRAUCH ZU HOCH!");
    }
    else {
        strcpy(zeile, "Status: OK - Alle Werte normal");
    }
    
    zeile_auffuellen(zeile);
}

/**
 * Gibt das Display auf der Konsole aus
 */
void display_ausgeben(void) {
    printf("\n");
    printf("┌──────────────────────────────────────────┐\n");
    printf("│%s│\n", display_puffer.zeile1);
    printf("│%s│\n", display_puffer.zeile2);
    printf("└──────────────────────────────────────────┘\n");
    fflush(stdout);
}

/**
 * Schreibt Display-Inhalt in eine Datei
 */
void display_in_datei_schreiben(const char* dateiname) {
    FILE* datei = fopen(dateiname, "w");
    if (datei == NULL) {
        LOG_WARNING_F("Konnte Display-Datei nicht schreiben: %s", dateiname);
        return;
    }
    
    fprintf(datei, "Smart Kühlschrank Display (2x40 Zeichen)\n");
    fprintf(datei, "========================================\n");
    fprintf(datei, "Zeile 1: %s\n", display_puffer.zeile1);
    fprintf(datei, "Zeile 2: %s\n", display_puffer.zeile2);
    fprintf(datei, "========================================\n");
    
    time_t jetzt = time(NULL);
    fprintf(datei, "Letzte Aktualisierung: %s", ctime(&jetzt));
    
    fclose(datei);
}

/**
 * Zentriert einen Text in einer Zeile
 */
void text_zentrieren(char* ziel, const char* text) {
    int text_laenge = strlen(text);
    int padding = (DISPLAY_COLS - text_laenge) / 2;
    
    // Zeile mit Leerzeichen füllen
    memset(ziel, ' ', DISPLAY_COLS);
    ziel[DISPLAY_COLS] = '\0';
    
    // Text in die Mitte kopieren
    if (text_laenge <= DISPLAY_COLS) {
        memcpy(ziel + padding, text, text_laenge);
    }
}

/**
 * Füllt eine Zeile mit Leerzeichen auf
 */
void zeile_auffuellen(char* zeile) {
    int laenge = strlen(zeile);
    
    // Mit Leerzeichen auffüllen bis DISPLAY_COLS
    for (int i = laenge; i < DISPLAY_COLS; i++) {
        zeile[i] = ' ';
    }
    zeile[DISPLAY_COLS] = '\0';
}

/**
 * Konvertiert Log-Level zu Anzeige-Zeichen
 */
char log_level_zu_zeichen(int log_level) {
    switch (log_level) {
        case LOG_DEBUG:   return '0';
        case LOG_INFO:    return '1';
        case LOG_WARNING: return '2';
        case LOG_ERROR:   return '3';
        default:          return '?';
    }
}

/**
 * Erstellt eine Fortschrittsbalken-Darstellung
 */
void fortschrittsbalken_erstellen(float wert, float minimum, float maximum, 
                                 int breite, char* puffer) {
    if (breite <= 0 || puffer == NULL) return;
    
    // Wert normalisieren (0.0 bis 1.0)
    float normalisiert = (wert - minimum) / (maximum - minimum);
    if (normalisiert < 0.0f) normalisiert = 0.0f;
    if (normalisiert > 1.0f) normalisiert = 1.0f;
    
    // Anzahl gefüllter Zeichen berechnen
    int gefuellt = (int)(normalisiert * breite);
    
    // Balken erstellen
    for (int i = 0; i < breite; i++) {
        if (i < gefuellt) {
            puffer[i] = BALKEN_ZEICHEN;
        } else {
            puffer[i] = LEER_BALKEN;
        }
    }
    puffer[breite] = '\0';
}

/**
 * Zeigt Startbildschirm an
 */
void display_startbildschirm_anzeigen(void) {
    text_zentrieren(display_puffer.zeile1, "Smart Kuehlschrank v1.0");
    text_zentrieren(display_puffer.zeile2, "System wird gestartet...");
    
    display_ausgeben();
    display_in_datei_schreiben(DISPLAY_DATEI);
    
    LOG_INFO_MSG("Startbildschirm angezeigt");
}

/**
 * Zeigt Systeminformationen an
 */
void display_systeminfo_anzeigen(void) {
    strcpy(display_puffer.zeile1, "Soll: 4.0C | Max: 8.0C | Min: -2.0C     ");
    strcpy(display_puffer.zeile2, "Energie Soll: 120W | Max: 200W         ");
    
    display_ausgeben();
    display_in_datei_schreiben(DISPLAY_DATEI);
}

/**
 * Beendet das Display-System
 */
void display_beenden(void) {
    text_zentrieren(display_puffer.zeile1, "System wird beendet...");
    text_zentrieren(display_puffer.zeile2, "Auf Wiedersehen!");
    
    display_ausgeben();
    display_in_datei_schreiben(DISPLAY_DATEI);
    
    LOG_INFO_MSG("Display-System beendet");
}
