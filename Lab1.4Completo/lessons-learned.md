# Lessons Learned

## Aspectos técnicos

- GRUB es una pieza crítica del sistema; pequeñas modificaciones pueden impedir que el sistema arranque.
- El archivo `/etc/default/grub` actúa como una plantilla de configuración de alto nivel.
- Cada cambio relevante requiere ejecutar `sudo update-grub` para regenerar `grub.cfg`.
- Es muy útil conocer los comandos básicos de GRUB para arrancar un sistema de forma manual cuando el menú falla.
- El uso de `chroot` desde un Live USB permite “entrar” al sistema como si hubiera arrancado normalmente y hacer reparaciones profundas.

## Buenas prácticas

- Siempre crear un respaldo del archivo de configuración antes de modificarlo.
- Documentar cuidadosamente los valores antes y después del cambio.
- Probar los cambios de forma controlada (por ejemplo, con snapshots de la VM).
- Mantener scripts simples de respaldo y restauración ayuda a recuperarse más rápido ante errores.

## Aprendizaje personal

- Se reforzó la comprensión del proceso de arranque de Linux.
- Se entendió la importancia de los tiempos de espera del menú para la usabilidad.
- Se practicó el uso de herramientas de línea de comandos para diagnóstico y reparación.
