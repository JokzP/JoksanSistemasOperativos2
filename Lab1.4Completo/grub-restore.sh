#!/bin/bash
# grub-restore.sh
# Restaura el archivo /etc/default/grub desde /etc/default/grub.backup y ejecuta update-grub.

set -e

BACKUP="/etc/default/grub.backup"
TARGET="/etc/default/grub"

echo "== GRUB Restore Script =="

if [[ ! -f "$BACKUP" ]]; then:
    echo "ERROR: No se encuentra el archivo de respaldo $BACKUP"
    exit 1
fi

sudo cp "$BACKUP" "$TARGET"
echo "Archivo de configuración restaurado desde el respaldo."

echo "Ejecutando update-grub..."
sudo update-grub

echo "Restauración completada. Reinicia el sistema para probar."
