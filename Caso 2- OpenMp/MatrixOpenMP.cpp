#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

// Estructura para representar una matriz
typedef struct {
    size_t tamano;         // Tamaño de la matriz (NxN)
    int* datos;            // Datos de la matriz en un arreglo unidimensional
} Matriz;

// Función para crear una matriz NxN
Matriz* crear_matriz(size_t tamano, int aleatoria) {
    Matriz* matriz = (Matriz*) malloc(sizeof(Matriz));
    matriz->datos = (int*) malloc(sizeof(int) * tamano * tamano);
    matriz->tamano = tamano;

    // Llenar con valores aleatorios si se indica
    if (aleatoria) {
        for (size_t i = 0; i < tamano * tamano; i++) {
            matriz->datos[i] = rand() % 100;
        }
    }
    return matriz;
}

// Función para imprimir una matriz
void imprimir_matriz(Matriz* matriz) {
    for (size_t i = 0; i < matriz->tamano; i++) {
        for (size_t j = 0; j < matriz->tamano; j++) {
            printf("%d ", matriz->datos[i * matriz->tamano + j]);
        }
        printf("\n");
    }
}

// Función para transponer una matriz
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

// Función para multiplicar matrices utilizando OpenMP y la transpuesta de B
Matriz* multiplicar_matrices(Matriz* matrizA, Matriz* matrizB_T, int numHilos) {
    size_t tamano = matrizA->tamano;
    Matriz* resultado = crear_matriz(tamano, 0);

    // Paralelización con OpenMP
    #pragma omp parallel for num_threads(numHilos)
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
    // Comprobación de argumentos
    if (argc != 4) {
        printf("Uso: %s <tamano_matriz> <num_hilos> <mostrar_matrices (0 o 1)>\n", argv[0]);
        return 1;
    }

    int tamanoMatriz = atoi(argv[1]);
    int numHilos = atoi(argv[2]);
    int mostrarMatrices = atoi(argv[3]);

    if (tamanoMatriz <= 0 || numHilos <= 0) {
        printf("El tamaño de la matriz y el número de hilos deben ser positivos.\n");
        return 1;
    }

    srand(time(NULL));  // Inicializar semilla para números aleatorios

    // Crear matrices A y B con valores aleatorios
    Matriz* matrizA = crear_matriz(tamanoMatriz, 1);
    Matriz* matrizB = crear_matriz(tamanoMatriz, 1);

    if (mostrarMatrices) {
        printf("Matriz A:\n");
        imprimir_matriz(matrizA);
        printf("\nMatriz B:\n");
        imprimir_matriz(matrizB);
    }

    // Transponer la matriz B para optimizar el acceso en la multiplicación
    Matriz* matrizB_T = transponer_matriz(matrizB);

    // Medir el tiempo de ejecución
    struct timespec inicio, fin;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    Matriz* matrizResultado = multiplicar_matrices(matrizA, matrizB_T, numHilos);

    clock_gettime(CLOCK_MONOTONIC, &fin);

    if (mostrarMatrices) {
        printf("\nMatriz C (Resultado):\n");
        imprimir_matriz(matrizResultado);
    }

    double tiempoEjecucion = (fin.tv_sec - inicio.tv_sec) + 
                             (fin.tv_nsec - inicio.tv_nsec) / 1e9;
    printf("\nTiempo de ejecución: %f segundos\n", tiempoEjecucion);

    // Liberar memoria
    eliminar_matriz(&matrizA);
    eliminar_matriz(&matrizB);
    eliminar_matriz(&matrizB_T);
    eliminar_matriz(&matrizResultado);

    return 0;
}

