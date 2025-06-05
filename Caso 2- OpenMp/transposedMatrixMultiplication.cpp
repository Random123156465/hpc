#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Estructura para representar una matriz cuadrada
typedef struct {
    size_t tamano;   // Tamaño de la matriz (N x N)
    int* datos;      // Datos de la matriz en formato fila mayor (row-major)
} Matriz;

// Función para crear una matriz y llenarla opcionalmente con valores aleatorios
Matriz* crear_matriz(size_t tamano, int llenarAleatorio) {
    Matriz* matriz = (Matriz*) malloc(sizeof(Matriz));
    matriz->datos = (int*) malloc(sizeof(int) * tamano * tamano);
    matriz->tamano = tamano;

    if (llenarAleatorio) {
        for (size_t i = 0; i < tamano * tamano; i++) {
            matriz->datos[i] = rand() % 100;  // Valores entre 0 y 99
        }
    }
    return matriz;
}

// Función para imprimir una matriz en la consola
void imprimir_matriz(Matriz* matriz) {
    for (size_t i = 0; i < matriz->tamano; i++) {
        for (size_t j = 0; j < matriz->tamano; j++) {
            printf("%d ", matriz->datos[i * matriz->tamano + j]);
        }
        printf("\n");
    }
}

// Función que devuelve la transpuesta de una matriz
Matriz* transponer_matriz(Matriz* matriz) {
    size_t tamano = matriz->tamano;
    Matriz* transpuesta = crear_matriz(tamano, 0);

    for (size_t i = 0; i < tamano; i++) {
        for (size_t j = 0; j < tamano; j++) {
            transpuesta->datos[j * tamano + i] = matriz->datos[i * tamano + j];
        }
    }
    return transpuesta;
}

// Multiplicación de matrices utilizando la transpuesta de B
Matriz* multiplicar_con_transpuesta(Matriz* matrizA, Matriz* matrizB_T) {
    size_t tamano = matrizA->tamano;
    Matriz* resultado = crear_matriz(tamano, 0);

    for (size_t i = 0; i < tamano; i++) {
        for (size_t j = 0; j < tamano; j++) {
            int suma = 0;
            for (size_t k = 0; k < tamano; k++) {
                suma += matrizA->datos[i * tamano + k] * matrizB_T->datos[j * tamano + k];
            }
            resultado->datos[i * tamano + j] = suma;
        }
    }
    return resultado;
}

// Función para liberar la memoria de una matriz
void eliminar_matriz(Matriz** matriz) {
    if (matriz == NULL || *matriz == NULL) return;
    free((*matriz)->datos);
    free(*matriz);
    *matriz = NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Uso: %s <tamano_matriz> <mostrar_matrices (0 o 1)>\n", argv[0]);
        return 1;
    }

    int tamano = atoi(argv[1]);
    int mostrarMatrices = atoi(argv[2]);

    srand(time(NULL));  // Inicializar semilla aleatoria

    // Crear matrices A y B con valores aleatorios
    Matriz* matrizA = crear_matriz(tamano, 1);
    Matriz* matrizB = crear_matriz(tamano, 1);

    if (mostrarMatrices) {
        printf("Matriz A:\n");
        imprimir_matriz(matrizA);
        printf("\nMatriz B:\n");
        imprimir_matriz(matrizB);
    }

    // Transponer B para mejorar la localidad de memoria
    Matriz* matrizB_T = transponer_matriz(matrizB);

    // Medición del tiempo de ejecución
    clock_t inicio = clock();
    Matriz* matrizResultado = multiplicar_con_transpuesta(matrizA, matrizB_T);
    clock_t fin = clock();

    if (mostrarMatrices) {
        printf("\nMatriz C (Resultado):\n");
        imprimir_matriz(matrizResultado);
    }

    double tiempo = ((double)(fin - inicio)) / CLOCKS_PER_SEC;
    printf("\nTiempo de ejecución: %f segundos\n", tiemp

