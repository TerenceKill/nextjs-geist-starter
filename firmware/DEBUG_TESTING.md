# Debug- und Test-Strategie für Smart Kühlschrank Firmware

## Übersicht

Dieses Dokument beschreibt umfassende Strategien für das Debugging und Testing der Smart Kühlschrank Firmware. Es behandelt sowohl Software- als auch Hardware-Fehler und bietet systematische Ansätze zur Fehlererkennung und -behebung.

## Inhaltsverzeichnis

1. [Debugging-Strategien](#debugging-strategien)
2. [Testing-Methoden](#testing-methoden)
3. [Memory-Management](#memory-management)
4. [Fehlererkennung](#fehlererkennung)
5. [Hardware-Simulation](#hardware-simulation)
6. [Automatisierte Tests](#automatisierte-tests)
7. [Performance-Analyse](#performance-analyse)
8. [Troubleshooting-Guide](#troubleshooting-guide)

---

## Debugging-Strategien

### 1. Logging-basiertes Debugging

#### Multi-Level-Logging
```bash
# Debug-Level 0 (DEBUG) - Alle Nachrichten
echo "0" > Workspace/taster.txt

# Debug-Level 1 (INFO) - Informationen und höher
echo "1" > Workspace/taster.txt

# Debug-Level 2 (WARNING) - Warnungen und Fehler
echo "1" > Workspace/taster.txt  # Mehrfach drücken für Level 2

# Debug-Level 3 (ERROR) - Nur Fehler
echo "1" > Workspace/taster.txt  # Mehrfach drücken für Level 3
```

#### Log-Analyse
```bash
# Aktuelle Logs anzeigen
make show-logs

# Log-Datei kontinuierlich verfolgen
tail -f bin/kuehlschrank.log

# Fehler-Logs filtern
grep "FEHLER\|ERROR" bin/kuehlschrank.log

# Warnungen der letzten 10 Minuten
grep "$(date -d '10 minutes ago' '+%Y-%m-%d %H:%M')" bin/kuehlschrank.log | grep "WARNUNG\|WARNING"
```

### 2. GDB-Debugging

#### Vorbereitung
```bash
# Debug-Build erstellen
make debug

# GDB starten
gdb bin/smart_fridge
```

#### Wichtige GDB-Kommandos
```gdb
# Breakpoints setzen
(gdb) break main
(gdb) break sensor_werte_lesen
(gdb) break display_aktualisieren

# Programm starten
(gdb) run

# Variablen inspizieren
(gdb) print aktuelle_sensordaten
(gdb) print display_puffer

# Call-Stack anzeigen
(gdb) backtrace

# Speicher-Dump
(gdb) x/10x &aktuelle_sensordaten

# Watchpoints für Variablen-Änderungen
(gdb) watch aktuelle_sensordaten.temperatur
```

### 3. Valgrind Memory-Debugging

#### Memory-Leak-Erkennung
```bash
# Vollständiger Memory-Check
make memcheck

# Detaillierte Analyse
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes bin/smart_fridge

# Heap-Profiling
valgrind --tool=massif bin/smart_fridge
```

#### Häufige Memory-Probleme
- **Buffer-Overflows**: Prüfung der Array-Grenzen
- **Use-after-free**: Zugriff auf freigegebenen Speicher
- **Memory-Leaks**: Nicht freigegebener Speicher
- **Double-free**: Mehrfache Freigabe desselben Speichers

---

## Testing-Methoden

### 1. Unit-Tests

#### Sensor-Modul Tests
```bash
# Temperatur-Tests
echo "5.0" > Workspace/temperatur.txt    # Normal
echo "15.0" > Workspace/temperatur.txt   # Zu hoch
echo "-10.0" > Workspace/temperatur.txt  # Zu niedrig
echo "abc" > Workspace/temperatur.txt    # Ungültig

# Tür-Tests
echo "0 0" > Workspace/tuer.txt          # Geschlossen
echo "1 $(date +%s)" > Workspace/tuer.txt # Gerade geöffnet
echo "1 $(($(date +%s) - 60))" > Workspace/tuer.txt # 60 Sek offen

# Energie-Tests
echo "120.0" > Workspace/energie.txt     # Normal
echo "300.0" > Workspace/energie.txt     # Zu hoch
echo "-50.0" > Workspace/energie.txt     # Ungültig
```

#### Display-Tests
```bash
# Display-Ausgabe prüfen
make show-display

# Display-Datei überwachen
watch -n 1 cat Workspace/display.txt
```

### 2. Integrationstests

#### Vollständiger Systemtest
```bash
# 1. System starten
make run &
FIRMWARE_PID=$!

# 2. Warten bis System bereit
sleep 5

# 3. Test-Sequenz ausführen
make test-temp-high
sleep 10
make test-door-open
sleep 35  # Warten bis Tür-Alarm
make test-energy-high
sleep 10

# 4. System beenden
kill $FIRMWARE_PID
```

#### Stress-Tests
```bash
# Schnelle Sensor-Änderungen
for i in {1..100}; do
    echo "$((RANDOM % 20 - 5))" > Workspace/temperatur.txt
    sleep 0.1
done

# Log-Level-Cycling
for i in {1..20}; do
    echo "1" > Workspace/taster.txt
    sleep 1
done
```

### 3. Automatisierte Test-Suite

#### Test-Script erstellen
```bash
#!/bin/bash
# test_suite.sh

echo "=== Smart Kühlschrank Test-Suite ==="

# Test 1: Kompilierung
echo "Test 1: Kompilierung..."
make clean && make all
if [ $? -eq 0 ]; then
    echo "✓ Kompilierung erfolgreich"
else
    echo "✗ Kompilierung fehlgeschlagen"
    exit 1
fi

# Test 2: Sensor-Dateien
echo "Test 2: Sensor-Dateien..."
make test-files
if [ -f "Workspace/temperatur.txt" ]; then
    echo "✓ Sensor-Dateien erstellt"
else
    echo "✗ Sensor-Dateien fehlen"
    exit 1
fi

# Test 3: Kurzer Lauf
echo "Test 3: Kurzer Systemlauf..."
timeout 10s make run
if [ $? -eq 124 ]; then
    echo "✓ System läuft stabil"
else
    echo "✗ System-Crash erkannt"
    exit 1
fi

echo "=== Alle Tests bestanden ==="
```

---

## Memory-Management

### 1. Statische Analyse

#### Cppcheck-Analyse
```bash
# Vollständige statische Analyse
make analyze

# Spezifische Checks
cppcheck --enable=all --std=c99 --platform=unix64 \
         --check-config --verbose \
         *.c
```

#### Compiler-Warnungen
```bash
# Strenge Compiler-Flags aktivieren
CFLAGS="-Wall -Wextra -Werror -Wpedantic -Wformat=2 -Wconversion" make
```

### 2. Laufzeit-Überwachung

#### AddressSanitizer
```bash
# Mit AddressSanitizer kompilieren
CFLAGS="-fsanitize=address -g" make debug

# Ausführen
./bin/smart_fridge
```

#### Stack-Overflow-Erkennung
```bash
# Stack-Schutz aktivieren
CFLAGS="-fstack-protector-strong" make
```

### 3. Memory-Patterns

#### Sichere String-Operationen
```c
// Unsicher
strcpy(buffer, input);

// Sicher
strncpy(buffer, input, sizeof(buffer) - 1);
buffer[sizeof(buffer) - 1] = '\0';

// Noch sicherer
snprintf(buffer, sizeof(buffer), "%s", input);
```

#### Puffer-Overflow-Schutz
```c
// Array-Grenzen prüfen
if (index >= 0 && index < ARRAY_SIZE) {
    array[index] = value;
}

// Sichere Datei-Operationen
FILE* datei = fopen(filename, "r");
if (datei != NULL) {
    // Operationen
    fclose(datei);
    datei = NULL;  // Dangling-Pointer vermeiden
}
```

---

## Fehlererkennung

### 1. Sensor-Fehler

#### Plausibilitätsprüfung
```c
// Temperatur-Bereich prüfen
if (temperatur < -50.0f || temperatur > 50.0f) {
    LOG_ERROR_F("Temperatur außerhalb Bereich: %.2f°C", temperatur);
    return SENSOR_ERROR_INVALID_VALUE;
}

// Zeitstempel-Validierung
time_t jetzt = time(NULL);
if (tuer_offen_seit > jetzt) {
    LOG_ERROR_MSG("Tür-Zeitstempel in der Zukunft!");
    return SENSOR_ERROR_INVALID_TIMESTAMP;
}
```

#### Sensor-Ausfälle simulieren
```bash
# Datei löschen (Sensor-Ausfall)
rm Workspace/temperatur.txt

# Ungültige Daten schreiben
echo "FEHLER" > Workspace/temperatur.txt

# Datei-Berechtigungen entziehen
chmod 000 Workspace/energie.txt
```

### 2. System-Fehler

#### Datei-System-Überwachung
```c
// Verfügbaren Speicherplatz prüfen
struct statvfs stat;
if (statvfs(".", &stat) == 0) {
    unsigned long free_space = stat.f_bavail * stat.f_frsize;
    if (free_space < 1024 * 1024) {  // < 1MB
        LOG_WARNING_F("Wenig Speicherplatz: %lu Bytes", free_space);
    }
}
```

#### Prozess-Überwachung
```bash
# CPU-Verbrauch überwachen
top -p $(pgrep smart_fridge)

# Memory-Verbrauch verfolgen
watch -n 1 'ps aux | grep smart_fridge'

# Datei-Handles prüfen
lsof -p $(pgrep smart_fridge)
```

---

## Hardware-Simulation

### 1. QEMU-Emulation

#### ARM-Emulation starten
```bash
# User-Mode-Emulation
./run_emulator.sh user

# System-Emulation (mit Kernel)
./run_emulator.sh system

# Native Ausführung
./run_emulator.sh native
```

#### Hardware-Fehler simulieren
```bash
# Speicher-Knappheit simulieren
ulimit -v 50000  # 50MB virtueller Speicher

# Datei-Handle-Limit
ulimit -n 10     # Nur 10 offene Dateien

# CPU-Limit
cpulimit -l 50 ./bin/smart_fridge  # 50% CPU-Limit
```

### 2. I2C-Display-Simulation

#### Display-Verhalten testen
```bash
# Display-Updates verfolgen
watch -n 0.5 cat Workspace/display.txt

# Display-Formatierung prüfen
# Zeile sollte exakt 40 Zeichen haben
cat Workspace/display.txt | sed -n '3p' | wc -c
```

---

## Performance-Analyse

### 1. Profiling

#### gprof-Profiling
```bash
# Mit Profiling kompilieren
CFLAGS="-pg" make debug

# Ausführen und Profiling-Daten sammeln
./bin/smart_fridge
# Nach Beendigung: gmon.out erstellt

# Profiling-Report generieren
gprof bin/smart_fridge gmon.out > profiling_report.txt
```

#### Timing-Messungen
```c
#include <time.h>

clock_t start = clock();
// Zu messende Operation
sensor_werte_lesen(&daten);
clock_t end = clock();

double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
LOG_DEBUG_F("Sensor-Lesung dauerte: %.3f Sekunden", cpu_time);
```

### 2. Ressourcen-Überwachung

#### System-Ressourcen
```bash
# Kontinuierliche Überwachung
htop -p $(pgrep smart_fridge)

# I/O-Statistiken
iostat -x 1

# Netzwerk-Aktivität (falls relevant)
netstat -i
```

---

## Troubleshooting-Guide

### Häufige Probleme und Lösungen

#### Problem: Programm startet nicht
```bash
# Lösung 1: Berechtigungen prüfen
chmod +x bin/smart_fridge

# Lösung 2: Abhängigkeiten prüfen
ldd bin/smart_fridge

# Lösung 3: Neu kompilieren
make clean && make all
```

#### Problem: Sensor-Werte werden nicht gelesen
```bash
# Lösung 1: Dateien prüfen
ls -la Workspace/

# Lösung 2: Berechtigungen setzen
chmod 644 Workspace/*.txt

# Lösung 3: Dateien neu erstellen
make test-files
```

#### Problem: Display zeigt falsche Werte
```bash
# Lösung 1: Display-Datei prüfen
cat Workspace/display.txt

# Lösung 2: Log-Level erhöhen
echo "1" > Workspace/taster.txt

# Lösung 3: Debug-Modus
make debug && gdb bin/smart_fridge
```

#### Problem: Memory-Leaks
```bash
# Lösung 1: Valgrind-Analyse
make memcheck

# Lösung 2: AddressSanitizer
CFLAGS="-fsanitize=address" make debug

# Lösung 3: Code-Review für malloc/free-Paare
grep -n "malloc\|free" *.c
```

### Notfall-Debugging

#### Core-Dump-Analyse
```bash
# Core-Dumps aktivieren
ulimit -c unlimited

# Nach Crash: Core-Dump analysieren
gdb bin/smart_fridge core

# Backtrace anzeigen
(gdb) bt
(gdb) info registers
```

#### Live-Debugging
```bash
# An laufenden Prozess anhängen
gdb -p $(pgrep smart_fridge)

# Prozess-Status prüfen
cat /proc/$(pgrep smart_fridge)/status
```

---

## Automatisierte Überwachung

### Monitoring-Script
```bash
#!/bin/bash
# monitor.sh - Kontinuierliche Überwachung

while true; do
    PID=$(pgrep smart_fridge)
    if [ -n "$PID" ]; then
        # CPU und Memory
        ps -p $PID -o pid,pcpu,pmem,time
        
        # Log-Fehler zählen
        ERROR_COUNT=$(grep -c "ERROR\|FEHLER" bin/kuehlschrank.log)
        echo "Fehler-Anzahl: $ERROR_COUNT"
        
        # Display-Status
        echo "Display-Update: $(stat -c %Y Workspace/display.txt)"
    else
        echo "Firmware läuft nicht!"
    fi
    
    sleep 10
done
```

### Automatische Tests
```bash
# Cron-Job für regelmäßige Tests
# 0 */6 * * * /path/to/test_suite.sh >> /var/log/firmware_tests.log 2>&1
```

---

## Fazit

Diese umfassende Debug- und Test-Strategie ermöglicht:

1. **Proaktive Fehlererkennung** durch kontinuierliches Monitoring
2. **Systematische Problemlösung** durch strukturierte Debugging-Ansätze
3. **Qualitätssicherung** durch automatisierte Tests
4. **Performance-Optimierung** durch Profiling und Analyse
5. **Robuste Fehlerbehandlung** durch umfassende Validierung

Die Kombination aus statischer Analyse, Laufzeit-Überwachung und systematischen Tests gewährleistet eine stabile und zuverlässige Firmware für den Smart Kühlschrank.
