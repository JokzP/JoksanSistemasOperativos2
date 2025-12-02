#!/bin/bash
# grub-backup.sh
# Crea un respaldo del archivo /etc/default/grub con validaciones básicas.

set -e

BACKUP="/etc/default/grub.backup"
SOURCE="/etc/default/grub"

echo "== GRUB Backup Script =="

if [[ ! -f "$SOURCE" ]]; then
    echo "ERROR: No se encuentra $SOURCE"
    exit 1
fi

if [[ -f "$BACKUP" ]]; then
    echo "ADVERTENCIA: El archivo de respaldo $BACKUP ya existe."
    echo "No se sobrescribirá para evitar pérdida de información."
    exit 1
fi

sudo cp "$SOURCE" "$BACKUP"
echo "Respaldo creado en: $BACKUP"
