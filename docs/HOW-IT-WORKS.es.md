# Cómo funciona el mod de 16:9

Todo lo de aquí vale para Street Fighter EX2 Plus NTSC-U (`SLUS-01105`), cuyo ejecutable carga en
`0x80130000` (desplazamiento `0x800` en el archivo). El mismo motor mueve *Street Fighter EX Plus
Alpha*, así que las formas —aunque no las direcciones— se trasladan.

*(English: [HOW-IT-WORKS.md](HOW-IT-WORKS.md))*

## 1. El ancho lo decide el mod

PSXRecomp sabe mostrar en ancho nativo, pero no deja que lo encienda un ajuste cualquiera: la opción
`[video] aspect_ratio` se queda sujeta a 4:3 salvo que un **plugin de confianza, propio del juego**,
pida otro aspecto de pantalla *antes de que se cree el renderizador*. El razonamiento es sensato: en
un juego de PS1, si un frustum más ancho es siquiera correcto es una pregunta que se responde juego
a juego, y a menudo pantalla a pantalla.

Así que el mod tiene dos mitades:

- `mods/packages/sfex2p.widescreen/…/manifest.toml` — un paquete `format_version = 5` cuya única
  característica reclama el identificador de plugin `sfex2p.widescreen`. No lleva código: eso es lo
  que hace que la característica aparezca en el lanzador y que el jugador pueda apagarla.
- `plugin/sfex2p_widescreen.c` — se compila dentro del ejecutable del juego y registra, bajo ese
  mismo identificador, un callback de **activación** y otro de **VBlank**. La activación corre
  después de que el lanzador confirme el plan de mods y antes de que arranque el renderizador, que
  es exactamente donde hay que llamar a `psx_mod_set_fixed_display_aspect(16, 9)`.

Al `game.toml` se le añade entonces un bloque `[widescreen]` con `gte_game_mode = true`: las peleas
son 3D, así que el detector de juego por GTE del runtime decide cuándo se aplica el frustum ancho y
deja en 4:3 las pantallas 2D (logos, menús, selección de personaje, cargas).

## 2. El problema de los márgenes

Pasar a ancho descubre unos 85 px a cada lado de la pantalla de 512 px. El 3D los rellena solo. El
**fondo 2D del escenario no**: ahí no dibuja nadie, así que esas columnas conservan lo que hubiera
en el búfer de cuadro de antes — en la práctica, tiras verticales repetidas que parecen un borde en
espejo.

Estirar el fondo sobre el cuadro ancho es el arreglo barato (`nw_phase_backdrop = true` en el
framework), y se nota que es un estirado. El arreglo honesto es hacer que el juego dibuje más
escenario.

## 3. El bucle del fondo, desensamblado

`func_80137AD0`, llamada una vez por cuadro con `s1 = 0x801E7F00`:

- `[s1+4]` apunta al mapa de tiles del escenario: **64 columnas × 14 filas**, 8 bytes por celda,
  indexado `fila + columna × 14`;
- el número de filas es `6 − (scrollY >> 5)`, con tope 9, y la primera fila es
  `clamp((scrollY >> 5) + 8, 0, 13)` — el índice de fila **satura**;
- la columna es `(scrollX >> 5 + c) & 63` — el índice de columna **da la vuelta**;
- el bucle recorre **17 columnas** empezando en `x = −(scrollX & 31)`, dibujando tiles `POLY_FT4` de
  32×32 píxeles (16×32 téxeles) en dos búferes de paquetes preenlazados de 153 entradas en
  `0x800D0000`;
- en `0x80137C40`: **una celda cuyo valor es 0 no se dibuja**, y tampoco consume un paquete. El
  cielo no está en el mapa, así que un fondo sano emite bastantes menos tiles que el máximo. (Contar
  paquetes emitidos *no* es, por tanto, una medida de salud: eso costó un diagnóstico equivocado.)

## 4. Ensancharlo

El plugin parchea **22 palabras de instrucción** de esa función, cada una verificada contra su valor
original antes de escribir nada, y con todo o nada:

- el registro base de los búferes de paquetes (`lui s7, 0x800D` → `0x80F0`), apuntando a dos búferes
  más grandes de 9 filas × 23 columnas reservados con `psx_mod_alloc_gpu_dma_memory`. Los búferes de
  fábrica no se pueden agrandar en el sitio: hay otros datos del juego justo detrás;
- la aritmética del índice dentro de cada búfer, sustituida por un desplazamiento (`sll …, 14`) que
  encaja con el nuevo paso;
- el inicio del bucle, movido 3 columnas (96 px) a la izquierda, y el índice de columna del mapa
  sesgado en −3;
- el límite del bucle, `slti v0, t5, 17` → `23`;
- la comprobación de búfer lleno, `inicio + 162×40` → `inicio + 207×40`.

Los tiles de más caen fuera del área de dibujo 4:3, así que en 4:3 la imagen no cambia. Y si la
reserva de memoria GPU-DMA no cae en la dirección que el `lui` parcheado espera, el plugin deja el
juego en 17 columnas en vez de ponerse a escribir donde no debe.

A la función parcheada se llega por el camino de RAM ejecutable del runtime, así que el parche se
aplica desde el callback de VBlank cada vez que se ve el código original en RAM: el juego carga su
ejecutable mucho después de que el plugin se active, y puede recargarlo o restaurarlo de un estado.

## 5. La parte difícil: el enlace huérfano

Con 23 columnas el bloque del fondo se hizo lo bastante largo como para destapar un fallo en la
propia contabilidad de la tabla de ordenación del juego. El síntoma: en la primerísima pelea del
modo atracción tras arrancar, el escenario aparecía **a medio dibujar** un par de segundos y luego
parpadeaba. Apagando el mod salía entero desde el primer cuadro.

El mecanismo, que se generaliza a cualquier juego de este motor:

1. `AddPrims` (`0x801357A4`) encadena el bloque del fondo con el resto de la lista de dibujo
   (personajes, HUD) escribiendo, **en el último paquete que el fondo usó de verdad**, el enlace al
   resto de la lista.
2. El juego repara ese enlace la próxima vez que entra en la función (`jal 0x801C54B8`,
   `SetNextPrim`), pero lee qué paquete hay que reparar de **su propia tabla en `0x801E4CE8`**, una
   ranura por búfer.
3. Esa tabla se pone a −1 en cuanto un cuadro no dibuja ningún fondo, que es lo que pasa en las
   transiciones alrededor de una pelea. La rotura queda entonces olvidada: un salto rancio a una
   tabla de ordenación vieja se queda en mitad de la cadena, y a partir de ahí la GPU se sale del
   fondo.

Con 17 columnas el bucle casi nunca llegaba a ese paquete. Con 23, sí.

**El arreglo:** una vez por VBlank, recorrer el búfer y restaurar solo los enlaces *anteriores* al
paquete al que apunta la tabla del juego. Esos son de cuadros que ya pasaron.

### Dos trampas que conviene conocer

- **No reenlaces el búfer entero.** El enlace al que apunta la tabla es precisamente el que une el
  fondo con todo lo demás. Reescribirlo deja al juego mostrando el escenario y nada más: ni
  personajes, ni siquiera las letras del título. Se intentó dos veces y se revirtió dos veces.
- **No midas la salud de la cadena contando tiles emitidos**, por el motivo del §3: las celdas
  vacías se saltan, así que un cuadro sano emite legítimamente muchos menos paquetes que el máximo
  del bucle.

## 6. Cómo se midió

El servidor de depuración de PSXRecomp ofrece `read_ram`, `set_snapshot` + `read_frame_ram` (cuatro
ventanas de 128 bytes por cuadro), `gpu_frame_dump` y los trazadores `wtrace` / `rtrace` /
`fntrace`. Una salvedad que costó tiempo: **los trazadores no ven los accesos a RAM que hace el
código recompilado del juego**, solo la E/S.

Lo que de verdad lo resolvió fue leer el búfer de paquetes entero con `read_ram` y **recorrer la
cadena como la recorre la GPU**. Eso imprimió, en una línea: la cadena se rompe en el paquete 68 y
salta a `0x801FC2EC`, con 138 paquetes escritos — en la demo de Dhalsim contra Blanka, con solo 69
tiles llegando a la pantalla y la mitad inferior del escenario en negro.
