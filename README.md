# Proyecto I - Arreglos Paginados

## Descripción del proyecto

Este repositorio contiene una implementación de un sistema de ordenamiento de archivos binarios grandes en C++. El objetivo principal es comparar el desempeño de distintos algoritmos de ordenamiento bajo dos condiciones:

1. Cuando todos los datos se cargan completamente en memoria (baseline).
2. Cuando se trabaja con memoria limitada mediante una estructura paginada (`PagedArray`).

El proyecto permite analizar cómo influyen factores como el tamaño de página y la cantidad de páginas en memoria sobre el tiempo de ejecución y el número de accesos a disco (page hits y page faults).

Incluye herramientas para generar datos de prueba, ordenar archivos y convertirlos a un formato legible.

---

## Compilación

Compila todos los programas necesarios:

```bash
g++ generator.cpp -o generator
g++ sorter.cpp -o sorter
g++ Baseline.cpp -o baseline_sorter
g++ makeReadable.cpp -o makeReadable
```

---

## Ejecución

### generator

Genera un archivo binario con enteros aleatorios que será usado como entrada para los algoritmos de ordenamiento, en small generara 32MB, con medium 64MB y con large 128MB.

```bash
./generator -size SMALL|MEDIUM|LARGE -output archivo.bin
```

**Argumentos:**
- `-size`: define el tamaño del archivo a generar (SMALL, MEDIUM o LARGE).
- `-output`: ruta y nombre del archivo binario que se va a crear.

---

### sorter

Ordena un archivo binario utilizando memoria paginada (`PagedArray`). Solo una parte del archivo se mantiene en memoria y el resto se accede desde disco.

```bash
./sorter -input input.bin -output output.bin -alg QUICK|RADIX|TIM|MERGE|HEAP -pageSize N -pageCount N
```

**Argumentos:**
- `-input`: archivo binario de entrada que contiene los datos desordenados.
- `-output`: archivo binario donde se guardará el resultado ordenado.
- `-alg`: algoritmo de ordenamiento a utilizar.
- `-pageSize`: cantidad de enteros que puede almacenar cada página.
- `-pageCount`: número de páginas disponibles en memoria.

---

### baseline_sorter

Ordena un archivo binario cargándolo completamente en memoria RAM. Se utiliza como referencia para comparar con la versión paginada.

```bash
./baseline_sorter -input input.bin -output output.bin -alg QUICK|RADIX|TIM|MERGE|HEAP
```

**Argumentos:**
- `-input`: archivo binario de entrada.
- `-output`: archivo binario de salida ordenado.
- `-alg`: algoritmo de ordenamiento a utilizar.

---

### makeReadable

Convierte un archivo binario en un archivo de texto legible, donde los enteros están separados por comas.

```bash
./makeReadable input.bin output.txt
```

**Argumentos:**
- `input.bin`: archivo binario de entrada.
- `output.txt`: archivo de texto donde se escribirá el contenido en formato legible.

---

## Flujo típico de uso

1. Generar archivo de datos:
```bash
./generator -size SMALL -output data.bin
```

2. Ordenar con paginación:
```bash
./sorter -input data.bin -output sorted.bin -alg QUICK -pageSize 1024 -pageCount 8
```

3. Ordenar con baseline:
```bash
./baseline_sorter -input data.bin -output sorted_base.bin -alg QUICK
```

4. Convertir a formato legible:
```bash
./makeReadable sorted.bin readable.txt
```
