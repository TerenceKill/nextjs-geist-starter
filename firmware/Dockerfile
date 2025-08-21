# Dockerfile für Smart Kühlschrank Firmware
# Erstellt eine Linux-Umgebung mit allen notwendigen Tools für Entwicklung und Testing

# Basis-Image: Ubuntu 22.04 LTS
FROM ubuntu:22.04

# Metadaten
LABEL maintainer="Smart Kühlschrank Entwicklungsteam"
LABEL description="Entwicklungs- und Test-Umgebung für Smart Kühlschrank Firmware"
LABEL version="1.0"

# Umgebungsvariablen setzen
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Berlin
ENV LANG=de_DE.UTF-8
ENV LC_ALL=de_DE.UTF-8

# System aktualisieren und grundlegende Pakete installieren
RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y \
    # Compiler und Build-Tools
    gcc \
    make \
    libc6-dev \
    build-essential \
    # Debugging und Analyse-Tools
    gdb \
    valgrind \
    cppcheck \
    strace \
    ltrace \
    # QEMU für Hardware-Emulation
    qemu-system-arm \
    qemu-user-static \
    # Nützliche Utilities
    vim \
    nano \
    htop \
    tree \
    file \
    hexdump \
    # Netzwerk-Tools für erweiterte Tests
    netcat-openbsd \
    curl \
    wget \
    # Lokalisierung
    locales \
    tzdata \
    # Dokumentations-Tools (optional)
    doxygen \
    graphviz \
    # Zusätzliche Entwicklungstools
    git \
    cmake \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Deutsche Lokalisierung konfigurieren
RUN locale-gen de_DE.UTF-8 && \
    update-locale LANG=de_DE.UTF-8

# Zeitzone setzen
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Arbeitsverzeichnis erstellen
WORKDIR /app/firmware

# Benutzer für sicherere Ausführung erstellen
RUN groupadd -r kuehlschrank && \
    useradd -r -g kuehlschrank -d /app -s /bin/bash kuehlschrank && \
    chown -R kuehlschrank:kuehlschrank /app

# Firmware-Quellcode kopieren
COPY --chown=kuehlschrank:kuehlschrank . .

# Verzeichnisse für Build und Logs erstellen
RUN mkdir -p obj bin Workspace logs && \
    chown -R kuehlschrank:kuehlschrank .

# Zu Benutzer wechseln
USER kuehlschrank

# Firmware kompilieren
RUN make clean && make all

# Standard-Sensor-Dateien erstellen
RUN make test-files

# Gesundheitscheck definieren
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD test -f bin/smart_fridge || exit 1

# Port für eventuelle Netzwerk-Erweiterungen
EXPOSE 8080

# Volumes für persistente Daten
VOLUME ["/app/firmware/Workspace", "/app/firmware/logs"]

# Umgebungsvariablen für Laufzeit
ENV FIRMWARE_HOME=/app/firmware
ENV PATH=$PATH:/app/firmware/bin

# Startup-Script erstellen
RUN echo '#!/bin/bash' > startup.sh && \
    echo 'echo "=== Smart Kühlschrank Firmware Container ==="' >> startup.sh && \
    echo 'echo "Firmware-Verzeichnis: $FIRMWARE_HOME"' >> startup.sh && \
    echo 'echo "Verfügbare Befehle:"' >> startup.sh && \
    echo 'echo "  make run          - Firmware starten"' >> startup.sh && \
    echo 'echo "  make memcheck     - Memory-Check durchführen"' >> startup.sh && \
    echo 'echo "  make test-*       - Verschiedene Tests ausführen"' >> startup.sh && \
    echo 'echo "  make show-logs    - Log-Ausgabe anzeigen"' >> startup.sh && \
    echo 'echo "  make show-display - Display-Status anzeigen"' >> startup.sh && \
    echo 'echo "  make help         - Alle verfügbaren Befehle"' >> startup.sh && \
    echo 'echo ""' >> startup.sh && \
    echo 'echo "Sensor-Dateien befinden sich in: Workspace/"' >> startup.sh && \
    echo 'echo "Log-Dateien werden in: logs/ gespeichert"' >> startup.sh && \
    echo 'echo ""' >> startup.sh && \
    echo 'echo "Container bereit! Verwenden Sie make run zum Starten."' >> startup.sh && \
    echo 'exec "$@"' >> startup.sh && \
    chmod +x startup.sh

# Standard-Kommando
CMD ["./startup.sh", "bash"]

# Build-Anweisungen als Kommentar
# 
# Container bauen:
#   docker build -t smart-kuehlschrank-firmware .
#
# Container ausführen (interaktiv):
#   docker run -it --rm smart-kuehlschrank-firmware
#
# Container mit persistenten Daten:
#   docker run -it --rm \
#     -v $(pwd)/workspace:/app/firmware/Workspace \
#     -v $(pwd)/logs:/app/firmware/logs \
#     smart-kuehlschrank-firmware
#
# Firmware im Container starten:
#   docker run -it --rm smart-kuehlschrank-firmware make run
#
# Memory-Check im Container:
#   docker run -it --rm smart-kuehlschrank-firmware make memcheck
#
# Container mit QEMU-Emulation (erweitert):
#   docker run -it --rm --privileged \
#     -v $(pwd)/workspace:/app/firmware/Workspace \
#     smart-kuehlschrank-firmware ./run_emulator.sh
