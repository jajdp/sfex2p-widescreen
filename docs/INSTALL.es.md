# Instalar el mod de 16:9

*(English: [INSTALL.md](INSTALL.md))*

## Antes de empezar

Hace falta un proyecto PSXRecomp de Street Fighter EX2 Plus que ya compile y funcione en tu
máquina: la carpeta con `CMakeLists.txt`, `game.toml` y tu `disc/`. Este repositorio no trae
el juego, ni la imagen del disco, ni el framework.

Ese proyecto es
[strider973/Street-Fighter-EX2-Plus-Recompiled](https://github.com/strider973/Street-Fighter-EX2-Plus-Recompiled),
construido sobre el framework [PSXRecomp](https://github.com/RetroPortingToolKit/psxrecomp)
(clónalo con `--recurse-submodules`). Dos cosas que necesita y que este mod no puede darte: **tu
propio volcado del disco** y **una BIOS retail SCPH-1001** —ese proyecto va con
`openbios = false`, así que la OpenBIOS incluida no sirve—. Primero compílalo y hazlo arrancar;
este mod es un cambio sobre esa compilación.

El mod está hecho para la edición NTSC-U, `SLUS-01105`. El plugin comprueba cada palabra de
instrucción original antes de escribir nada, así que en otra edición simplemente no hace nada,
en vez de estropear el juego.

## 1. Instalar los archivos

Consigue el kit: descarga `sfex2p.widescreen-1.0.0.zip` de
[la página de releases](https://github.com/jajdp/sfex2p-widescreen/releases/latest) y
descomprímelo, o clona este repositorio —el contenido es el mismo—. Conserva la disposición
tal como viene: el instalador lee el plugin y el paquete de su propia carpeta, relativos a sí
mismo. Después, desde esa carpeta:

```sh
python tools/apply_widescreen.py <ruta de la raíz del proyecto del juego>
```

Con `--dry-run` enseña lo que cambiaría sin escribir nada.

El script hace cuatro cambios, cada uno con su firma, de modo que pasarlo dos veces no cambia
nada:

| Qué | Dónde |
|---|---|
| El código del plugin | `sfex2p_widescreen.c`, copiado a la raíz del proyecto |
| El paquete del mod | `mods/packages/sfex2p.widescreen/1.0.0/manifest.toml` |
| El registro en la compilación | `CMakeLists.txt`: el plugin en `CODEGEN_SETUP_SOURCES` y un paso posterior a la compilación que deja `mods/packages/` junto al ejecutable |
| La configuración del juego | `game.toml`: un bloque `[widescreen]` con `gte_game_mode = true` y `nw_phase_backdrop = false` |

Las escrituras son atómicas y conservan el fin de línea de cada archivo. No se borra nada.

### Por qué `nw_phase_backdrop = false`

Esa opción del framework estira el fondo 2D sobre el cuadro ancho. Este mod, en cambio,
ensancha el bucle del propio juego: dejar las dos encendidas duplica el borde y deja costuras
verticales en los márgenes. Ponla en `true` solo si quitas el plugin y te vale la aproximación
barata.

## 2. Recompilar el juego

Se compila el proyecto como siempre. El plugin se compila dentro del ejecutable: ésa es toda
la razón de que este paso exista.

## 3. Encenderlo

Arranca el juego. En el lanzador, en **Mods**, el grupo **Visual** trae ahora **Widescreen
(16:9)**, encendido de fábrica. Pulsa **PLAY**.

![La lista de Mods del lanzador, con la opción encendida](images/pc-lanzador-mods.png)

Tiene que verse así:

- los menús, los logos y la selección de personaje en 4:3, con barras laterales;
- la pelea ocupando todo el ancho, con escenario hasta los dos bordes;
- ni tiras verticales, ni píxeles congelados, ni costuras en los márgenes; tampoco en la demo
  de atracción que sale justo después de arrancar.

## 4. Apagarlo

Desmarca **Widescreen (16:9)** en el lanzador: el juego vuelve a su 4:3 original y el plugin
se queda dormido. Las partidas guardadas no se ven afectadas
(`save_compatibility = "shared"`).

Para quitarlo del todo: borra del proyecto `mods/packages/sfex2p.widescreen/` y
`sfex2p_widescreen.c`, deshaz los dos añadidos del `CMakeLists.txt` y el bloque
`[widescreen]` del `game.toml` —cada uno lleva el comentario
`sfex2p.widescreen (Recompilaciones)`— y recompila.

## Consolas y otros empaquetados UWP

En una compilación UWP (por ejemplo una Xbox en modo desarrollador) el paquete del mod no se
instala aparte: se compila y se copia dentro del paquete de la aplicación junto al ejecutable,
así que viaja dentro de él. Para actualizar el mod se instala una versión nueva del paquete.

## Si algo no sale bien

| Síntoma | Causa |
|---|---|
| El juego sigue en 4:3 con la opción encendida | El plugin no se compiló. Comprueba que `sfex2p_widescreen.c` está en `CODEGEN_SETUP_SOURCES` y que recompilaste. |
| La opción no aparece en el lanzador | El paquete no quedó junto al ejecutable. Mira el paso posterior a la compilación y que exista `mods/packages/sfex2p.widescreen/1.0.0/manifest.toml`. |
| Sale el 16:9, pero los márgenes repiten tiras | El ensanche del fondo no está activo: la reserva de memoria GPU-DMA del plugin no cayó donde apunta la instrucción parcheada, o el juego no es `SLUS-01105`. El mod se queda en 17 columnas a propósito, antes que dibujar basura. |
| El lanzador no deja arrancar el juego | Dos funciones reclamando el mismo id de plugin, casi siempre una copia vieja del paquete. Deja un solo paquete `sfex2p.widescreen`. |
