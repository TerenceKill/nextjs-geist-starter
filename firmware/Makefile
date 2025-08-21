# Makefile für Smart Kühlschrank Firmware
# Kompiliert alle C-Dateien zu einem ausführbaren Programm

# Compiler und Flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic -g -O2
LDFLAGS = -lm

# Verzeichnisse
SRCDIR = .
OBJDIR = obj
BINDIR = bin
WORKSPACE = Workspace

# Quelldateien und Objektdateien
SOURCES = smart_fridge.c logging.c sensor.c display.c
OBJECTS = $(SOURCES:%.c=$(OBJDIR)/%.o)
TARGET = $(BINDIR)/smart_fridge

# Hauptziel
all: directories $(TARGET)

# Verzeichnisse erstellen
directories:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)
	@mkdir -p $(WORKSPACE)

# Hauptprogramm linken
$(TARGET): $(OBJECTS)
	@echo "Linke Programm: $@"
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build erfolgreich abgeschlossen!"

# Objektdateien kompilieren
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo "Kompiliere: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Abhängigkeiten (vereinfacht)
$(OBJDIR)/smart_fridge.o: smart_fridge.c config.h logging.h sensor.h display.h
$(OBJDIR)/logging.o: logging.c logging.h config.h
$(OBJDIR)/sensor.o: sensor.c sensor.h config.h logging.h
$(OBJDIR)/display.o: display.c display.h config.h logging.h

# Debug-Build mit zusätzlichen Debug-Informationen
debug: CFLAGS += -DDEBUG -g3 -O0
debug: clean all

# Release-Build mit Optimierungen
release: CFLAGS += -DNDEBUG -O3 -s
release: clean all

# Programm ausführen
run: $(TARGET)
	@echo "Starte Smart Kühlschrank Firmware..."
	cd $(BINDIR) && ./smart_fridge

# Programm mit Valgrind auf Memory-Leaks prüfen
memcheck: $(TARGET)
	@echo "Führe Memory-Check mit Valgrind durch..."
	cd $(BINDIR) && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./smart_fridge

# Statische Code-Analyse mit cppcheck
analyze:
	@echo "Führe statische Code-Analyse durch..."
	cppcheck --enable=all --std=c99 --platform=unix64 $(SOURCES)

# Dokumentation mit Doxygen generieren (falls installiert)
docs:
	@if command -v doxygen >/dev/null 2>&1; then \
		echo "Generiere Dokumentation..."; \
		doxygen Doxyfile 2>/dev/null || echo "Doxyfile nicht gefunden"; \
	else \
		echo "Doxygen nicht installiert"; \
	fi

# Test-Sensor-Dateien erstellen
test-files:
	@echo "Erstelle Test-Sensor-Dateien..."
	@echo "5.5" > $(WORKSPACE)/temperatur.txt
	@echo "0 0" > $(WORKSPACE)/tuer.txt
	@echo "125.0" > $(WORKSPACE)/energie.txt
	@echo "0" > $(WORKSPACE)/taster.txt
	@echo "Test-Dateien erstellt in $(WORKSPACE)/"

# Sensor-Werte für Tests setzen
test-temp-high:
	@echo "15.0" > $(WORKSPACE)/temperatur.txt
	@echo "Temperatur auf 15°C gesetzt (Alarm-Test)"

test-temp-low:
	@echo "-5.0" > $(WORKSPACE)/temperatur.txt
	@echo "Temperatur auf -5°C gesetzt (Alarm-Test)"

test-door-open:
	@echo "1 $$(date +%s)" > $(WORKSPACE)/tuer.txt
	@echo "Tür als offen markiert"

test-door-close:
	@echo "0 0" > $(WORKSPACE)/tuer.txt
	@echo "Tür als geschlossen markiert"

test-energy-high:
	@echo "250.0" > $(WORKSPACE)/energie.txt
	@echo "Energieverbrauch auf 250W gesetzt (Alarm-Test)"

test-button-press:
	@echo "1" > $(WORKSPACE)/taster.txt
	@echo "Taster-Druck simuliert (Log-Level wird erhöht)"

# Alle Test-Dateien zurücksetzen
reset-tests:
	@echo "Setze alle Test-Werte zurück..."
	@echo "4.0" > $(WORKSPACE)/temperatur.txt
	@echo "0 0" > $(WORKSPACE)/tuer.txt
	@echo "120.0" > $(WORKSPACE)/energie.txt
	@echo "0" > $(WORKSPACE)/taster.txt
	@echo "Alle Werte auf Standardwerte zurückgesetzt"

# Log-Dateien anzeigen
show-logs:
	@echo "=== Aktuelle Log-Datei ==="
	@if [ -f $(BINDIR)/kuehlschrank.log ]; then \
		tail -20 $(BINDIR)/kuehlschrank.log; \
	else \
		echo "Keine Log-Datei gefunden"; \
	fi

# Display-Status anzeigen
show-display:
	@echo "=== Aktueller Display-Inhalt ==="
	@if [ -f $(WORKSPACE)/display.txt ]; then \
		cat $(WORKSPACE)/display.txt; \
	else \
		echo "Keine Display-Datei gefunden"; \
	fi

# Aufräumen
clean:
	@echo "Räume auf..."
	rm -rf $(OBJDIR)
	rm -rf $(BINDIR)
	rm -f *.log
	rm -f core

# Alles löschen (inklusive Workspace)
distclean: clean
	@echo "Lösche alle generierten Dateien..."
	rm -rf $(WORKSPACE)
	rm -rf docs/

# Installation (für System-weite Installation)
install: $(TARGET)
	@echo "Installiere Smart Kühlschrank Firmware..."
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/smart_fridge
	@echo "Installation abgeschlossen. Aufruf mit: smart_fridge"

# Deinstallation
uninstall:
	@echo "Deinstalliere Smart Kühlschrank Firmware..."
	sudo rm -f /usr/local/bin/smart_fridge
	@echo "Deinstallation abgeschlossen"

# Hilfe anzeigen
help:
	@echo "Smart Kühlschrank Firmware - Makefile Hilfe"
	@echo "============================================"
	@echo ""
	@echo "Build-Targets:"
	@echo "  all          - Kompiliert das Programm (Standard)"
	@echo "  debug        - Debug-Build mit zusätzlichen Informationen"
	@echo "  release      - Release-Build mit Optimierungen"
	@echo "  clean        - Löscht kompilierte Dateien"
	@echo "  distclean    - Löscht alle generierten Dateien"
	@echo ""
	@echo "Ausführung:"
	@echo "  run          - Startet das Programm"
	@echo "  memcheck     - Führt Memory-Check mit Valgrind durch"
	@echo "  analyze      - Statische Code-Analyse mit cppcheck"
	@echo ""
	@echo "Test-Funktionen:"
	@echo "  test-files   - Erstellt Standard-Test-Dateien"
	@echo "  test-temp-high   - Setzt hohe Temperatur (Alarm-Test)"
	@echo "  test-temp-low    - Setzt niedrige Temperatur (Alarm-Test)"
	@echo "  test-door-open   - Öffnet Tür (Alarm-Test)"
	@echo "  test-door-close  - Schließt Tür"
	@echo "  test-energy-high - Setzt hohen Energieverbrauch (Alarm-Test)"
	@echo "  test-button-press - Simuliert Taster-Druck"
	@echo "  reset-tests  - Setzt alle Werte zurück"
	@echo ""
	@echo "Monitoring:"
	@echo "  show-logs    - Zeigt aktuelle Log-Einträge"
	@echo "  show-display - Zeigt aktuellen Display-Inhalt"
	@echo ""
	@echo "Installation:"
	@echo "  install      - Installiert das Programm system-weit"
	@echo "  uninstall    - Deinstalliert das Programm"
	@echo ""
	@echo "Sonstiges:"
	@echo "  docs         - Generiert Dokumentation (falls Doxygen verfügbar)"
	@echo "  help         - Zeigt diese Hilfe"

# Phony-Targets (keine Dateien)
.PHONY: all debug release clean distclean run memcheck analyze docs test-files \
        test-temp-high test-temp-low test-door-open test-door-close \
        test-energy-high test-button-press reset-tests show-logs show-display \
        install uninstall help directories
