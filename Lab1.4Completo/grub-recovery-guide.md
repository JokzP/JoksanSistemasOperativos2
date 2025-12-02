# GRUB Recovery Guide

Este documento describe varios métodos para recuperar GRUB en caso de fallas.

---

## 1. GRUB Rescue Mode

### 1.1 ¿Cuándo aparece?

El modo `grub rescue>` aparece cuando GRUB no puede encontrar sus módulos o el directorio `/boot/grub`, por ejemplo cuando se borra o mueve la partición de arranque.

### 1.2 Comandos básicos

En el prompt `grub rescue>`:

```text
ls
set
set prefix
set root
insmod normal
normal
```

Ejemplo de procedimiento:

```text
grub rescue> ls
(hd0) (hd0,msdos1) (hd0,msdos2)

grub rescue> set root=(hd0,msdos1)
grub rescue> set prefix=(hd0,msdos1)/boot/grub
grub rescue> insmod normal
grub rescue> normal
```

Con esto se intenta cargar el modo normal de GRUB.

### 1.3 Arranque manual del sistema

En el modo `grub>` (normal):

```text
grub> ls (hd0,msdos1)/
grub> set root=(hd0,msdos1)
grub> linux /boot/vmlinuz-*-generic root=/dev/sda1 ro
grub> initrd /boot/initrd.img-*-generic
grub> boot
```

---

## 2. Recovery from Live USB/DVD

Este método utiliza un Live USB de la misma distribución (por ejemplo Ubuntu).

### 2.1 Pasos básicos

1. Iniciar el sistema desde el Live USB.
2. Abrir una terminal.
3. Identificar la partición raíz:

```bash
lsblk
```

4. Montar la partición:

```bash
sudo mount /dev/sda1 /mnt
```

(Sustituir `/dev/sda1` por la partición correcta.)

5. Montar pseudo-sistemas:

```bash
sudo mount --bind /dev /mnt/dev
sudo mount --bind /proc /mnt/proc
sudo mount --bind /sys /mnt/sys
```

6. Entrar en chroot:

```bash
sudo chroot /mnt
```

7. Reinstalar GRUB:

```bash
grub-install /dev/sda
update-grub
```

8. Salir y desmontar:

```bash
exit
sudo umount /mnt/dev
sudo umount /mnt/proc
sudo umount /mnt/sys
sudo umount /mnt
```

9. Reiniciar el sistema sin el USB.

---

## 3. Backup Restoration

Este método se basa en haber creado previamente un respaldo de `/etc/default/grub`.

### 3.1 Restauración del archivo de configuración

```bash
sudo cp /etc/default/grub.backup /etc/default/grub
sudo update-grub
```

### 3.2 Verificación

- Revisar que no haya errores en la salida de `update-grub`.
- Comprobar los valores clave:

```bash
grep GRUB_TIMEOUT /etc/default/grub
grep GRUB_DEFAULT /etc/default/grub
```

- Reiniciar el sistema y validar que el menú de GRUB funcione:

```bash
sudo reboot
```

---

## 4. Fuentes de consulta recomendadas

- Documentación oficial de GNU GRUB.
- Ubuntu Community Help: Grub2.
- Debian Wiki: GRUB.
- Arch Linux Wiki: GRUB (gran referencia técnica).
