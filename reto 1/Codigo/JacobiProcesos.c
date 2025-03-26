#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Estructura para representar una matriz
typedef struct {
    size_t matrixSize; // Dimensión de la matriz (n x n)
    int* matrixData;   // Puntero a los datos de la matriz almacenados en un array 1D
} Matrix;

// Estructura para los argumentos de los hilos
typedef struct {
    Matrix* matrixA;
    Matrix* matrixB;
    Matrix* resultMatrix;
    size_t startRow;  // Fila inicial asignada al hilo
    size_t endRow;    // Fila final asignada al hilo
} ThreadData;

// Función para crear una matriz
dMatrix* create_matrix(size_t matrixSize, int fillWithRandom) {
    Matrix* matrix = (Matrix*) malloc(sizeof(Matrix));
    matrix->matrixData = (int*) malloc(sizeof(int) * matrixSize * matrixSize);
    matrix->matrixSize = matrixSize;

    // Llenar la matriz con valores aleatorios si se indica
    if (fillWithRandom) {
        for (size_t i = 0; i < matrixSize * matrixSize; i++) {
            matrix->matrixData[i] = rand() % 100; // Valores entre 0 y 99
        }
    }
    return matrix;
}

// Función para imprimir una matriz
void print_matrix(Matrix* matrix) {
    for (size_t i = 0; i < matrix->matrixSize; i++) {
        for (size_t j = 0; j < matrix->matrixSize; j++) {
            printf("%d ", matrix->matrixData[i * matrix->matrixSize + j]);
        }
        printf("\n");
    }
}

// Función que ejecutará cada hilo para multiplicar una parte de la matriz
void* multiply_matrices_thread(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    size_t size = data->matrixA->matrixSize;

    // Multiplicación de matrices por filas asignadas a este hilo
    for (size_t i = data->startRow; i < data->endRow; i++) {
        for (size_t j = 0; j < size; j++) {
            data->resultMatrix->matrixData[i * size + j] = 0; // Inicializa el valor
            for (size_t k = 0; k < size; k++) {
                data->resultMatrix->matrixData[i * size + j] += 
                    data->matrixA->matrixData[i * size + k] * data->matrixB->matrixData[k * size + j];
            }
        }
    }
    return NULL;
}

// Función para multiplicar dos matrices utilizando múltiples hilos
Matrix* multiply_matrices(Matrix* matrixA, Matrix* matrixB, int numThreads) {
    size_t size = matrixA->matrixSize;
    Matrix* resultMatrix = create_matrix(size, 0);
    
    pthread_t threads[numThreads];
    ThreadData threadData[numThreads];
    size_t rowsPerThread = size / numThreads;
    size_t remainingRows = size % numThreads; // Para manejar casos donde el tamaño no sea divisible

    for (int i = 0; i < numThreads; i++) {
        threadData[i].matrixA = matrixA;
        threadData[i].matrixB = matrixB;
        threadData[i].resultMatrix = resultMatrix;
        threadData[i].startRow = i * rowsPerThread;
        threadData[i].endRow = (i + 1) * rowsPerThread;
        
        // Asigna las filas restantes al último hilo
        if (i == numThreads - 1) threadData[i].endRow += remainingRows;

        // Crear el hilo
        pthread_create(&threads[i], NULL, multiply_matrices_thread, &threadData[i]);
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    return resultMatrix;
}

// Función para liberar la memoria asignada a una matriz
void delete_matrix(Matrix** matrix) {
    if (matrix == NULL || *matrix == NULL) return;
    free((*matrix)->matrixData);
    free(*matrix);
    *matrix = NULL;
}

int main(int argc, char* argv[]) {
    // Validar los argumentos del programa
    if (argc != 4) {
        printf("Uso: %s <matrix_size> <num_threads> <show_matrices (0 o 1)>\n", argv[0]);
        return 1;
    }

    int matrixSize = atoi(argv[1]);
    int numThreads = atoi(argv[2]);
    int showMatrices = atoi(argv[3]);

    if (matrixSize <= 0 || numThreads <= 0) {
        printf("El tamaño de la matriz y el número de hilos deben ser positivos.\n");
        return 1;
    }

    srand(time(NULL));
    
    // Crear las matrices A y B con valores aleatorios
    Matrix* matrixA = create_matrix(matrixSize, 1);
    Matrix* matrixB = create_matrix(matrixSize, 1);

    // Mostrar las matrices originales si se solicita
    if (showMatrices) {
        printf("Matriz A:\n");
        print_matrix(matrixA);
        printf("\nMatriz B:\n");
        print_matrix(matrixB);
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Multiplicar las matrices
    Matrix* resultMatrix = multiply_matrices(matrixA, matrixB, numThreads);
    clock_gettime(CLOCK_MONOTONIC, &end); 

    // Mostrar la matriz resultante si se solicita
    if (showMatrices) {
        printf("\nMatriz C (Resultado):\n");
        print_matrix(resultMatrix);
    }

    // Calcular y mostrar el tiempo de ejecución
    double executionTime = (end.tv_sec - start.tv_sec) + 
                         (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nTiempo de ejecución: %f segundos\n", executionTime);

    // Liberar la memoria asignada a las matrices
    delete_matrix(&matrixA);
    delete_matrix(&matrixB);
    delete_matrix(&resultMatrix);

    return 0;
}

