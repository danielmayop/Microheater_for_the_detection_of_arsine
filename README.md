# Microcalentador para la detección de arsina

Microcalentador con control de temperatura para un sensor de arsina. El repositorio reúne el firmware y los scripts, el diseño de la placa, las simulaciones y las mediciones.

| Carpeta | Contenido |
|---|---|
| `Codigos/` | Firmware del controlador, scripts de MATLAB para identificar el modelo y scripts de Python para el análisis de curvas |
| `Kicad/` | Esquemático y placa del controlador de temperatura |
| `LTspice/` | Simulaciones |
| `Analisis/` | Informe completo, figuras y tablas del análisis de las curvas |
| `Imagenes/`, `Hoja de datos/`, `paper/` | Fotos, gráficas y documentación |

## Respuesta transitoria del microcalentador (encendido y apagado)

Se midió la temperatura del microcalentador con duty de 30, 40, 50, 60, 70, 80, 90 y 100 %: unos 10 min encendido y luego el enfriamiento, en dos configuraciones:

- **Aire libre:** sensor sobre la parte resistiva.
- **Dentro de la caja:** el microcalentador dentro de una caja, midiendo la temperatura de su interior.

**Parte resistiva, aire libre**

![Curvas al aire libre](Imagenes/grafica_temp.png)

**Dentro de la caja**

![Curvas dentro de la caja](Imagenes/caja.jpg)

### Conclusiones del transitorio

| | Aire libre | Caja |
|---|---|---|
| **Encendido:** tiempo al 63 % del valor final | ≈ 63 s | ≈ 114 s |
| **Encendido:** subida de 10 a 90 % | 141–161 s | 287–343 s |
| **Encendido:** tiempo al 95 % del valor final | 190–238 s | 413–457 s |
| **Apagado:** tiempo en que la temperatura cae a la mitad | 36–48 s | 72–79 s |
| **Apagado:** tiempo hasta que queda el 10 % | 127–176 s | no se alcanza en el registro (260–353 s), salvo en una corrida |

Encendido: valores para 30–90 % de duty (el 100 % al aire libre es más lento, ver más abajo). Apagado: valores para 30–100 %.

- **Encendido.** Las curvas suben a distintas velocidades porque el valor final es distinto: la pendiente inicial y el aumento final de temperatura son proporcionales al duty (≈ 0,40 °C por cada 1 % de duty al aire libre y ≈ 0,26 °C en la caja). El tiempo de respuesta no depende del duty. Al aire libre se comporta como un sistema de primer orden con τ ≈ 60–66 s. En la caja hay dos escalas de tiempo (≈ 60–80 s y ≈ 300–550 s) y a los 10 min todavía no llegó al régimen estacionario.
- **Apagado.** En valores absolutos lo más caliente se enfría más rápido, pero eso también ocurre en un sistema lineal. Normalizando, al aire libre el enfriamiento es más rápido cuanto mayor es la temperatura: el tiempo a la mitad baja de 48 s (ΔT de 12 °C) a 36 s (ΔT de 37 °C). En la caja no se detecta esa dependencia, y el enfriamiento tiene una cola lenta: tras unos 5 min la temperatura sigue 1,2–4,5 °C por encima de la inicial.
- **¿Es no lineal?** Levemente. Entre 30 y 90 % de duty el calentamiento es prácticamente lineal y el enfriamiento al aire libre muestra una no linealidad leve y sistemática. En la caja no se detecta no linealidad con la resolución de estos datos, aunque no se puede afirmar que sea lineal.
- **A tener en cuenta.** La corrida de 100 % al aire libre se aparta del resto (es más lenta que la de 90 % durante los primeros 3 min) y hay una sola corrida por duty, así que falta repetirla para saber si es real. Tampoco se midió la potencia: se supone proporcional al duty.

El análisis completo, con el método, las tablas por duty y las limitaciones, está en [`Analisis/curvas_microcalentador.md`](Analisis/curvas_microcalentador.md). Para regenerar figuras y tablas:

```
python "Codigos/Python Analisis/figuras_analisis.py" . Analisis
```
