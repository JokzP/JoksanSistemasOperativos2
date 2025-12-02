# Troubleshooting Checklist

Lista de verificación rápida en caso de problemas con GRUB:

- [ ] ¿El archivo `/etc/default/grub` tiene sintaxis válida?
- [ ] ¿Se ejecutó `sudo update-grub` después de los cambios?
- [ ] ¿Se escribió correctamente el disco objetivo en `grub-install` (por ejemplo, `/dev/sda` y no `/dev/sda1`)?
- [ ] ¿Las particiones de sistema y `/boot` están presentes y montadas?
- [ ] ¿Hay suficiente espacio en `/boot` para nuevos kernels e initrd?
- [ ] ¿Se cuenta con un Live USB de la distribución para reparación?
- [ ] ¿Existe un archivo de respaldo `/etc/default/grub.backup` disponible?
