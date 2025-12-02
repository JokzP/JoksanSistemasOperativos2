#!/bin/bash
# system-info.sh
# Muestra información básica del sistema relacionada con GRUB y el entorno.

echo "== System Information =="
echo
echo "Distribución:"
lsb_release -a 2>/dev/null || echo "lsb_release no disponible"

echo
echo "Kernel actual:"
uname -a

echo
echo "Discos y particiones:"
lsblk

echo
echo "Versión de GRUB:"
sudo grub-install --version

echo
echo "Timeout actual en /etc/default/grub:"
grep GRUB_TIMEOUT /etc/default/grub || echo "No se encontró GRUB_TIMEOUT"

echo
echo "Entradas de menú detectadas en grub.cfg:"
grep menuentry /boot/grub/grub.cfg 2>/dev/null | wc -l || echo "No se pudo leer grub.cfg"
