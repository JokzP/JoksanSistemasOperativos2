# Configuration Changes

## Task 2.1: Modify Boot Timeout

### Edición del archivo principal

Comando:

```bash
sudo nano /etc/default/grub
```

Se localizó la línea:

```bash
GRUB_TIMEOUT=5
```

y se cambió a:

```bash
GRUB_TIMEOUT=30
```

---

## Task 2.2: Additional Customizations

Se dejaron los siguientes valores en el archivo `/etc/default/grub`:

```bash
GRUB_DEFAULT=0
GRUB_TIMEOUT=30
GRUB_DISTRIBUTOR=`lsb_release -i -s 2> /dev/null || echo Debian`
GRUB_CMDLINE_LINUX_DEFAULT="quiet splash"
GRUB_CMDLINE_LINUX=""

GRUB_TIMEOUT_STYLE=menu
```

### Justificación de los cambios

- `GRUB_TIMEOUT=30`: otorga al usuario más tiempo para seleccionar una entrada antes del arranque automático.
- `GRUB_DEFAULT=0`: se asegura que la primera entrada de la lista sea la predeterminada.
- `GRUB_CMDLINE_LINUX_DEFAULT="quiet splash"`: conserva el arranque gráfico amigable.
- `GRUB_CMDLINE_LINUX=""`: se mantiene sin parámetros adicionales para evitar comportamientos inesperados.
- `GRUB_TIMEOUT_STYLE=menu`: fuerza la visualización del menú, incluso si hay una sola entrada de sistema operativo.

---

## Task 2.3: Apply Changes

Comando ejecutado para regenerar la configuración:

```bash
sudo update-grub
```

Salida típica (resumen):

- Detección de sistema operativo instalado.
- Generación de nuevas entradas de menú para el kernel.
- Confirmación de que se escribió un nuevo `/boot/grub/grub.cfg`.

En caso de error, se hubiera utilizado:

```bash
sudo cp /etc/default/grub.backup /etc/default/grub
sudo update-grub
```

---

## Task 2.4: Test Modifications

Después de ejecutar `sudo update-grub`:

1. Se reinició el sistema:
   ```bash
   sudo reboot
   ```
2. Se observó que el menú GRUB ahora mostraba una cuenta regresiva de **30 segundos**.
3. El usuario contó el tiempo disponible para seleccionar opciones en el menú.
4. Se verificó que el sistema arrancara correctamente en la opción predeterminada.

### Evidencia

- `screenshots/03-grub-config-after.png`: configuración modificada.
- `screenshots/04-modified-boot-menu.png`: menú de arranque con nuevo timeout.
- `screenshots/05-update-grub-output.png`: salida del comando `update-grub`.
