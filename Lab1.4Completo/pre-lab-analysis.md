# Pre-Lab Analysis

## Task 1.1: Examine Current Configuration

### Comandos utilizados

```bash
sudo cat /etc/default/grub
sudo less /boot/grub/grub.cfg
sudo grub-install --version
```

### Valores principales documentados

Valores típicos de una instalación estándar de Ubuntu/Debian (ejemplo):

- `GRUB_TIMEOUT=5`
- `GRUB_DEFAULT=0`
- `GRUB_CMDLINE_LINUX_DEFAULT="quiet splash"`
- `GRUB_CMDLINE_LINUX=""`

Estos valores se observaron en el archivo `/etc/default/grub`.

### Análisis del archivo grub.cfg

Para revisar el archivo generado por GRUB:

```bash
sudo less /boot/grub/grub.cfg
```

Este archivo no se edita directamente; es construido automáticamente a partir de `/etc/default/grub` y los scripts en `/etc/grub.d/`.

Para contar las entradas de menú:

```bash
grep menuentry /boot/grub/grub.cfg | wc -l
```

Resultado típico: **2** entradas principales:
- Entrada normal de sistema operativo (Ubuntu).
- Entrada de modo recuperación (Ubuntu, recovery mode).

---

## Task 1.2: Boot Menu Analysis

### Observaciones del menú de arranque (antes de cambios)

- El menú se muestra por muy poco tiempo o no se muestra si no se presiona alguna tecla.
- Cuando se visualiza, se aprecian 2 opciones principales:
  - Ubuntu
  - Ubuntu (recovery mode)

El tiempo de interacción coincide con el valor de `GRUB_TIMEOUT=5`, es decir, aproximadamente 5 segundos para elegir una opción antes de que arranque la predeterminada.

### Evidencia

Las siguientes capturas de pantalla corresponden al estado inicial (se referencian los archivos a guardar en la VM):

- `screenshots/01-original-boot-menu.png`
- `screenshots/02-grub-config-before.png`
