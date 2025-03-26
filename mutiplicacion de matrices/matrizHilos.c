#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Estructura para representar una matriz
typedef struct {
    size_t matrixSize; // Dimensión de la matriz (n x n)
    int* matrixData;   // Puntero a los datos de la matriz almacenados en un arreglo 1D
} Matrix;

// Estructura para los datos que se pasan a los hilos
typedef struct {
    Matrix* matrixA;
    Matrix* matrixB;
    Matrix* resultMatrix;
    size_t startRow;  // Fila inicial para este hilo
    size_t endRow;    // Fila final para este hilo
} ThreadData;

// Función para crear una matriz
Matrix* create_matrix(size_t matrixSize, int fillWithRandom) {
    Matrix* matrix = (Matrix*) malloc(sizeof(Matrix));
    matrix->matrixData = (int*) malloc(sizeof(int) * matrixSize * matrixSize);
    matrix->matrixSize = matrixSize;

    // Si fillWithRandom es 1, se llena la matriz con valores aleatorios
    if (fillWithRandom) {
        for (size_t i = 0; i < matrixSize * matrixSize; i++) {
            matrix->matrixData[i] = rand() % 100; // Números entre 0 y 99
        }
    }
    return matrix;
}

// Función para imprimir una matriz en la consola
void print_matrix(Matrix* matrix) {
    for (size_t i = 0; i < matrix->matrixSize; i++) {
        for (size_t j = 0; j < matrix->matrixSize; j++) {
            printf("%d ", matrix->matrixData[i * matrix->matrixSize + j]);
        }
        printf("\n");
    }
}

// Función que ejecutan los hilos para multiplicar la matriz en filas asignadas
void* multiply_matrices_thread(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    size_t size = data->matrixA->matrixSize;

    // Iterar sobre las filas asignadas al hilo actual
    for (size_t i = data->startRow; i < data->endRow; i++) {
        for (size_t j = 0; j < size; j++) {
            data->resultMatrix->matrixData[i * size + j] = 0;
            for (size_t k = 0; k < size; k++) {
                data->resultMatrix->matrixData[i * size + j] += 
                    data->matrixA->matrixData[i * size + k] * data->matrixB->matrixData[k * size + j];
            }
        }
    }
    return NULL;
}

// Función para multiplicar dos matrices utilizando hilos
Matrix* multiply_matrices(Matrix* matrixA, Matrix* matrixB, int numThreads) {
    size_t size = matrixA->matrixSize;
    Matrix* resultMatrix = create_matrix(size, 0); // Crear matriz resultado
    
    pthread_t threads[numThreads];
    ThreadData threadData[numThreads];
    size_t rowsPerThread = size / numThreads;
    size_t remainingRows = size % numThreads;

    // Crear hilos y asignarles porciones de la matriz
    for (int i = 0; i < numThreads; i++) {
        threadData[i].matrixA = matrixA;
        threadData[i].matrixB = matrixB;
        threadData[i].resultMatrix = resultMatrix;
        threadData[i].startRow = i * rowsPerThread;
        threadData[i].endRow = (i + 1) * rowsPerThread;
        
        // El último hilo toma las filas restantes
        if (i == numThreads - 1) threadData[i].endRow += remainingRows;

        pthread_create(&threads[i], NULL, multiply_matrices_thread, &threadData[i]);
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    return resultMatrix;
}

// Función para liberar la memoria de una matriz
void delete_matrix(Matrix** matrix) {
    if (matrix == NULL || *matrix == NULL) return;
    free((*matrix)->matrixData);
    free(*matrix);
    *matrix = NULL;
}

int main(int argc, char* argv[]) {
    // Verificar que los argumentos sean correctos
    if (argc != 4) {
        printf("Uso: %s <tamaño_matriz> <num_hilos> <mostrar_matrices (0 o 1)>\n", argv[0]);
        return 1;
    }

    int matrixSize = atoi(argv[1]); // Tamaño de la matriz
    int numThreads = atoi(argv[2]); // Número de hilos a usar
    int showMatrices = atoi(argv[3]); // Bandera para mostrar o no las matrices

    // Validación de parámetros
    if (matrixSize <= 0 || numThreads <= 0) {
        printf("El tamaño de la matriz y el número de hilos deben ser positivos.\n");
        return 1;
    }

    srand(time(NULL)); // Inicializar generador de números aleatorios
    
    // Crear matrices A y B
    Matrix* matrixA = create_matrix(matrixSize, 1);
    Matrix* matrixB = create_matrix(matrixSize, 1);

    // Si se activó la opción, imprimir las matrices originales
    if (showMatrices) {
        printf("Matriz A:\n");
        print_matrix(matrixA);
        printf("\nMatriz B:\n");
        print_matrix(matrixB);
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start); // Iniciar medición de tiempo

    // Multiplicar matrices
    Matrix* resultMatrix = multiply_matrices(matrixA, matrixB, numThreads);
    clock_gettime(CLOCK_MONOTONIC, &end);  // Finalizar medición de tiempo

    // Si se activó la opción, imprimir la matriz resultado
    if (showMatrices) {
        printf("\nMatriz C (Resultado):\n");
        print_matrix(resultMatrix);
    }

    // Calcular y mostrar el tiempo de ejecución
    double executionTime = (end.tv_sec - start.tv_sec) + 
                         (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nTiempo de ejecución: %f segundos\n", executionTime);

    // Liberar memoria de las matrices
    delete_matrix(&matrixA);
    delete_matrix(&matrixB);
    delete_matrix(&resultMatrix);

    return 0;
}

