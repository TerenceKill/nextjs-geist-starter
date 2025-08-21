#!/bin/bash

# QEMU Emulator Script für Smart Kühlschrank Firmware
# Startet die Firmware in einer emulierten ARM-Umgebung

set -e  # Bei Fehlern abbrechen

# Konfiguration
FIRMWARE_BINARY="bin/smart_fridge"
QEMU_SYSTEM="qemu-system-arm"
MACHINE_TYPE="versatilepb"
CPU_TYPE="arm1176"
MEMORY_SIZE="256M"
LOG_FILE="logs/qemu_emulation.log"
KERNEL_LOG="logs/kernel.log"

# Farben für Ausgabe
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Funktionen
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Banner anzeigen
print_banner() {
    echo "================================================================"
    echo "  Smart Kühlschrank Firmware - QEMU Emulation"
    echo "================================================================"
    echo "  Ziel-Architektur: ARM (versatilepb)"
    echo "  CPU: $CPU_TYPE"
    echo "  Speicher: $MEMORY_SIZE"
    echo "  Firmware: $FIRMWARE_BINARY"
    echo "================================================================"
    echo ""
}

# Voraussetzungen prüfen
check_prerequisites() {
    print_info "Prüfe Voraussetzungen..."
    
    # QEMU installiert?
    if ! command -v $QEMU_SYSTEM &> /dev/null; then
        print_error "QEMU nicht gefunden! Installieren Sie qemu-system-arm"
        print_info "Ubuntu/Debian: sudo apt-get install qemu-system-arm"
        exit 1
    fi
    
    # Firmware kompiliert?
    if [ ! -f "$FIRMWARE_BINARY" ]; then
        print_warning "Firmware nicht gefunden. Kompiliere..."
        make clean && make all
        if [ ! -f "$FIRMWARE_BINARY" ]; then
            print_error "Firmware-Kompilierung fehlgeschlagen!"
            exit 1
        fi
    fi
    
    # Log-Verzeichnis erstellen
    mkdir -p logs
    
    # Workspace-Verzeichnis prüfen
    if [ ! -d "Workspace" ]; then
        print_warning "Workspace-Verzeichnis nicht gefunden. Erstelle..."
        make test-files
    fi
    
    print_success "Alle Voraussetzungen erfüllt"
}

# ARM-Cross-Compilation (falls nötig)
cross_compile_for_arm() {
    print_info "Prüfe ARM-Cross-Compilation..."
    
    # Prüfen ob ARM-GCC verfügbar ist
    if command -v arm-linux-gnueabihf-gcc &> /dev/null; then
        print_info "ARM-Cross-Compiler gefunden. Kompiliere für ARM..."
        
        # Backup der x86-Version
        if [ -f "$FIRMWARE_BINARY" ]; then
            cp "$FIRMWARE_BINARY" "${FIRMWARE_BINARY}.x86_backup"
        fi
        
        # Für ARM kompilieren
        make clean
        CC=arm-linux-gnueabihf-gcc make all
        
        if [ -f "$FIRMWARE_BINARY" ]; then
            print_success "ARM-Kompilierung erfolgreich"
            file "$FIRMWARE_BINARY"
        else
            print_error "ARM-Kompilierung fehlgeschlagen"
            # Fallback auf x86-Version
            if [ -f "${FIRMWARE_BINARY}.x86_backup" ]; then
                mv "${FIRMWARE_BINARY}.x86_backup" "$FIRMWARE_BINARY"
                print_warning "Verwende x86-Version als Fallback"
            fi
        fi
    else
        print_warning "ARM-Cross-Compiler nicht verfügbar"
        print_info "Installieren mit: sudo apt-get install gcc-arm-linux-gnueabihf"
        print_info "Verwende x86-Version für Emulation"
    fi
}

# Minimales Linux-Kernel-Image erstellen (für vollständige Emulation)
create_minimal_kernel() {
    print_info "Erstelle minimales Kernel-Image..."
    
    KERNEL_DIR="kernel_build"
    KERNEL_IMAGE="kernel_build/zImage"
    
    if [ ! -f "$KERNEL_IMAGE" ]; then
        print_info "Lade minimalen Linux-Kernel..."
        
        mkdir -p "$KERNEL_DIR"
        cd "$KERNEL_DIR"
        
        # Vereinfachtes Kernel-Setup (für Demo-Zwecke)
        # In echter Implementierung würde hier ein vollständiger Kernel gebaut
        
        # Dummy-Kernel erstellen (für Demonstration)
        echo "Erstelle Dummy-Kernel für Demonstration..."
        dd if=/dev/zero of=zImage bs=1M count=1 2>/dev/null
        
        cd ..
        print_warning "Dummy-Kernel erstellt. Für echte Emulation Linux-Kernel kompilieren!"
    fi
}

# Firmware in QEMU starten
start_qemu_emulation() {
    print_info "Starte QEMU-Emulation..."
    
    # Log-Dateien vorbereiten
    echo "=== QEMU Emulation gestartet am $(date) ===" > "$LOG_FILE"
    echo "=== Kernel Log ===" > "$KERNEL_LOG"
    
    # QEMU-Kommando zusammenstellen
    QEMU_CMD="$QEMU_SYSTEM \
        -M $MACHINE_TYPE \
        -cpu $CPU_TYPE \
        -m $MEMORY_SIZE \
        -nographic \
        -no-reboot \
        -serial stdio \
        -monitor none"
    
    # Verschiedene Emulations-Modi
    case "${1:-user}" in
        "system")
            print_info "Starte System-Emulation (benötigt Kernel)..."
            if [ -f "kernel_build/zImage" ]; then
                QEMU_CMD="$QEMU_CMD -kernel kernel_build/zImage -append 'console=ttyAMA0'"
            else
                print_error "Kernel-Image nicht gefunden für System-Emulation"
                print_info "Verwende User-Mode-Emulation stattdessen"
                start_user_mode_emulation
                return
            fi
            ;;
        "user"|*)
            print_info "Starte User-Mode-Emulation..."
            start_user_mode_emulation
            return
            ;;
    esac
    
    print_info "QEMU-Kommando: $QEMU_CMD"
    print_info "Drücken Sie Ctrl+A, dann X zum Beenden"
    print_info "Logs werden in $LOG_FILE gespeichert"
    
    # QEMU starten
    echo "Starte QEMU..." | tee -a "$LOG_FILE"
    $QEMU_CMD 2>&1 | tee -a "$LOG_FILE"
}

# User-Mode-Emulation (einfacher)
start_user_mode_emulation() {
    print_info "Starte User-Mode-Emulation mit qemu-user..."
    
    # Prüfen ob qemu-arm verfügbar ist
    if command -v qemu-arm &> /dev/null; then
        print_info "Führe Firmware mit qemu-arm aus..."
        echo "=== User-Mode Emulation gestartet am $(date) ===" >> "$LOG_FILE"
        
        # Firmware mit QEMU-User ausführen
        qemu-arm "$FIRMWARE_BINARY" 2>&1 | tee -a "$LOG_FILE"
    else
        print_warning "qemu-arm nicht verfügbar. Führe nativ aus..."
        native_execution
    fi
}

# Native Ausführung (Fallback)
native_execution() {
    print_info "Führe Firmware nativ aus (Fallback)..."
    echo "=== Native Ausführung gestartet am $(date) ===" >> "$LOG_FILE"
    
    # Prüfen ob Firmware ausführbar ist
    if [ -x "$FIRMWARE_BINARY" ]; then
        "$FIRMWARE_BINARY" 2>&1 | tee -a "$LOG_FILE"
    else
        print_error "Firmware nicht ausführbar: $FIRMWARE_BINARY"
        exit 1
    fi
}

# Emulation beenden und aufräumen
cleanup() {
    print_info "Beende Emulation und räume auf..."
    
    # QEMU-Prozesse beenden
    pkill -f qemu-system-arm 2>/dev/null || true
    pkill -f qemu-arm 2>/dev/null || true
    
    # Log-Zusammenfassung
    if [ -f "$LOG_FILE" ]; then
        echo "" >> "$LOG_FILE"
        echo "=== Emulation beendet am $(date) ===" >> "$LOG_FILE"
        
        print_info "Log-Datei: $LOG_FILE"
        print_info "Letzte 10 Log-Einträge:"
        tail -10 "$LOG_FILE"
    fi
    
    print_success "Cleanup abgeschlossen"
}

# Signal-Handler für sauberes Beenden
trap cleanup EXIT INT TERM

# Hilfe anzeigen
show_help() {
    echo "Smart Kühlschrank Firmware - QEMU Emulator"
    echo ""
    echo "Verwendung: $0 [MODUS] [OPTIONEN]"
    echo ""
    echo "Modi:"
    echo "  user     - User-Mode-Emulation (Standard)"
    echo "  system   - System-Emulation (benötigt Kernel)"
    echo "  native   - Native Ausführung (kein QEMU)"
    echo ""
    echo "Optionen:"
    echo "  -h, --help    - Diese Hilfe anzeigen"
    echo "  -v, --verbose - Ausführliche Ausgabe"
    echo "  -c, --compile - Vor Start neu kompilieren"
    echo ""
    echo "Beispiele:"
    echo "  $0                    # User-Mode-Emulation"
    echo "  $0 system             # System-Emulation"
    echo "  $0 native             # Native Ausführung"
    echo "  $0 user --compile     # Neu kompilieren und starten"
}

# Hauptfunktion
main() {
    local mode="user"
    local verbose=false
    local recompile=false
    
    # Argumente verarbeiten
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -v|--verbose)
                verbose=true
                set -x  # Debug-Modus
                shift
                ;;
            -c|--compile)
                recompile=true
                shift
                ;;
            user|system|native)
                mode=$1
                shift
                ;;
            *)
                print_error "Unbekannte Option: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # Banner anzeigen
    print_banner
    
    # Neu kompilieren falls gewünscht
    if [ "$recompile" = true ]; then
        print_info "Kompiliere Firmware neu..."
        make clean && make all
    fi
    
    # Voraussetzungen prüfen
    check_prerequisites
    
    # Je nach Modus starten
    case $mode in
        "system")
            create_minimal_kernel
            cross_compile_for_arm
            start_qemu_emulation "system"
            ;;
        "user")
            cross_compile_for_arm
            start_user_mode_emulation
            ;;
        "native")
            native_execution
            ;;
    esac
}

# Script starten
main "$@"
