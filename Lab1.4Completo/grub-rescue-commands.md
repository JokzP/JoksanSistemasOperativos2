# GRUB Rescue Commands

Ejemplos de comandos útiles en el modo `grub rescue>` y `grub>`:

## Descubrir discos y particiones

```text
ls
ls (hd0,msdos1)/
```

## Establecer root y prefix

```text
set root=(hd0,msdos1)
set prefix=(hd0,msdos1)/boot/grub
```

## Cargar módulo normal y cambiar al modo normal

```text
insmod normal
normal
```

## Arranque manual del kernel

```text
linux /boot/vmlinuz-*-generic root=/dev/sda1 ro
initrd /boot/initrd.img-*-generic
boot
```
