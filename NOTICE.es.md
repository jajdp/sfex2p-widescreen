# Aviso

*(English: [NOTICE.md](NOTICE.md))*

## Sin afiliación

Esto es un proyecto de aficionados, no oficial y sin ánimo de lucro. **No está afiliado a
Capcom ni a Arika**, ni a ninguna de sus filiales, ni cuenta con su respaldo o su aprobación;
tampoco está afiliado a los autores del framework
[PSXRecomp](https://github.com/RetroPortingToolKit/psxrecomp) ni a los del
[proyecto del juego](https://github.com/strider973/Street-Fighter-EX2-Plus-Recompiled) sobre el
que corre este mod. *Street Fighter*, *Street Fighter EX2 Plus* y todos los nombres, personajes
y marcas relacionados son propiedad de sus respectivos titulares, y aquí se usan únicamente
para identificar el juego con el que este mod es compatible.

Aquí no se vende nada y no se monetiza nada.

## Lo que este repositorio no distribuye

Ni ROM. Ni imagen del disco. Ni BIOS. Ni ejecutable compilado. Ni código del juego decompilado
o recompilado. Ni arte, ni audio, ni música, ni las letras del juego. Este repositorio es un
**kit de código**: el plugin lo compila dentro del ejecutable del juego quien lo construye, y
ese ejecutable no sale nunca de su máquina. Para jugar hace falta **tu propia copia legítima**
del juego y una compilación del proyecto del juego que funcione, y este repositorio no da
ninguna de las dos, ni puede darlas.

## Lo que sí contiene de la obra original

El plugin tiene que saber qué instrucciones está reemplazando, así que lleva una tabla de
**22 palabras de instrucción —88 bytes—** de la función de dibujo del fondo del propio juego,
cada una emparejada con el valor que el plugin escribe en su lugar. El plugin **comprueba cada
una de esas palabras originales antes de escribir nada**: en otra edición del juego la
comprobación falla y el plugin no hace absolutamente nada, en vez de estropearlo. Esos 88 bytes
son todo el código del programa original que se cita en cualquier parte de este repositorio, y
se citan para esa verificación.

`docs/HOW-IT-WORKS.md` describe en prosa el comportamiento del juego —cómo funcionan su bucle de
mosaicos y su tabla de ordenación—, como tiene que hacer la documentación de cualquier mod. No
contiene código del juego más allá de esas palabras.

Las capturas de `docs/images/` muestran el juego en marcha, para documentar qué hace el mod.
Siguen siendo propiedad de sus respectivos titulares.

## Retirada y contacto

Si tienes derechos sobre este material y quieres que algo de aquí se retire o se cambie,
escribe a **jajdpmail@gmail.com** diciendo a qué te opones y en calidad de qué escribes.

Las peticiones de los titulares de derechos se atienden: se quita la parte en disputa, o se
retira el repositorio entero, sin discutirlo, y recibirás una respuesta confirmándolo. No hace
falta ningún aviso ni trámite legal más allá de ese correo para llegar a quien mantiene esto.
