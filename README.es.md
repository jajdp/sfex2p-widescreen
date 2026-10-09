# Street Fighter EX2 Plus — 16:9 de verdad

Un mod de pantalla ancha para la recompilación estática
[PSXRecomp](https://github.com/RetroPortingToolKit/psxrecomp) de *Street Fighter EX2 Plus* (PlayStation, 1999,
`SLUS-01105`). Las peleas llenan la pantalla ancha **dibujando el fondo del escenario hasta
los bordes**: sin estirar la imagen, sin tiras repetidas y sin píxeles viejos en los márgenes.

*(English: [README.md](README.md))*

![La primera demo en 16:9, con el fondo completo y a 60 FPS](docs/images/xbox-fondo-arreglado.png)

*En una Xbox Series en modo desarrollador. El texto está en español porque esa compilación
llevaba además la traducción [sfex2p-es](https://github.com/jajdp/sfex2p-es); el mod del 16:9
es independiente de ella.*

## Qué hace

- **16:9 en las peleas.** El 3D del juego se dibuja ancho de verdad: se ensancha el campo de
  visión, no se estira una imagen de 4:3.
- **El fondo se ensancha, no se estira.** La capa de mosaicos del escenario dibuja 17 columnas
  de piezas de 32×32 en el juego original, justo la pantalla de 4:3. Este mod parchea ese
  bucle para que dibuje **23 columnas** (3 más por lado) y muda sus paquetes a otros búferes,
  así que en los márgenes hay escenario de verdad, que se desplaza con el resto.
- **Las pantallas 2D se quedan en 4:3.** Logos, menús, selección de personaje y cargas
  conservan su encuadre con barras laterales: solo la pelea va ancha.
- **Se enciende y se apaga.** Aparece en el lanzador, en **Mods → Visual → Widescreen
  (16:9)**. Apagado, el juego vuelve al 4:3 sin reinstalar nada.

## Qué NO hay en este repositorio

**Ni código del juego, ni imagen del disco, ni BIOS, ni ejecutable compilado.** Hace falta tu
propia copia volcada del juego y un proyecto PSXRecomp funcionando. Aquí está solo el mod: el
código del plugin, el paquete y el script que instala los dos.

## Requisitos

| | |
|---|---|
| [Un proyecto PSXRecomp de Street Fighter EX2 Plus](https://github.com/strider973/Street-Fighter-EX2-Plus-Recompiled) | la carpeta con `CMakeLists.txt` y `game.toml` |
| Tu propio volcado del disco | NTSC-U, `SLUS-01105` |
| Una BIOS retail SCPH-1001 | la pide ese proyecto (`openbios = false`), no este mod |
| Un compilador de C capaz de construir ese proyecto | el plugin se compila dentro del ejecutable |
| Python 3.8 o posterior | solo para el script de instalación |

## Instalación

Se descarga la **[última versión publicada](https://github.com/jajdp/sfex2p-widescreen/releases/latest)**
—un kit de código, no un binario—, se descomprime y, desde esa carpeta:

```sh
python tools/apply_widescreen.py <ruta de la raíz del proyecto del juego>
```

Y se recompila el juego. (Clonar este repositorio vale igual: el kit son estos archivos.) Los
pasos completos, qué cambia el script y cómo deshacerlo están en
**[docs/INSTALL.es.md](docs/INSTALL.es.md)**.

¿Por qué hay que compilar? Porque los paquetes de mods de PSXRecomp no llevan código nativo, a
propósito: una función que cambia el aspecto de la pantalla tiene que ser un *plugin de
confianza del propio juego*, enlazado dentro de su ejecutable. El paquete de `mods/` es lo que
lo hace aparecer —y apagarse— en el lanzador; `plugin/sfex2p_widescreen.c` es lo que trabaja.

## Cómo funciona

En corto: el plugin pide al runtime un aspecto fijo de 16:9 antes de que arranque el
renderizador, reserva dos búferes de paquetes más grandes en la ventana de GPU-DMA de los mods
y parchea 22 palabras de instrucción de la función del fondo para que su bucle recorra 23
columnas en vez de 17. Una vez por VBlank emulado repone los enlaces viejos que la propia
contabilidad del juego deja sueltos en esa cadena más larga.

Eso último es lo delicado, y está contado —con las mediciones y los dos intentos fallidos— en
**[docs/HOW-IT-WORKS.md](docs/HOW-IT-WORKS.md)** (en inglés).

## Estructura

```
mods/packages/sfex2p.widescreen/1.0.0/manifest.toml   el paquete del mod (el interruptor)
plugin/sfex2p_widescreen.c                            el plugin del juego (se compila dentro)
tools/apply_widescreen.py                             instala los dos en un proyecto del juego
docs/INSTALL.es.md         instalar, comprobar y quitar
docs/HOW-IT-WORKS.md       el ensanche del fondo y la reparación de la cadena, en detalle
```

## Estado

Compilado y jugado en Windows y en una Xbox Series en modo desarrollador: 60 FPS, el fondo
completo desde el primer cuadro de la demo de atracción y ninguna costura en los márgenes.

Comprobado además desde cero el 2026-10-08: el instalador aplicado sobre un árbol del juego
recién clonado (exactamente los cinco cambios documentados y ninguno más), recompilado, y
activo al arrancar: `psxrecomp: mod selected fixed display aspect 16:9`.

## Créditos y licencia

Mod de **Recompilaciones**. Publicado bajo la
[PolyForm Noncommercial License 1.0.0](LICENSE), la misma licencia del framework PSXRecomp
en el que se enchufa.

*Street Fighter EX2 Plus* es © Capcom / Arika. Este proyecto no está afiliado a ellos, ni a
Sony, ni al autor de PSXRecomp, y no distribuye nada que les pertenezca. El detalle —qué hay y
qué no hay aquí exactamente, y cómo pedir una retirada— está en
**[NOTICE.es.md](NOTICE.es.md)**. Los titulares de derechos pueden escribir a
**jajdpmail@gmail.com**.
