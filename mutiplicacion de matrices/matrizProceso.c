#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

// Estructura para representar una matriz cuadrada
typedef struct {
    size_t matrixSize; // Tamaño de la matriz (n x n)
    int* matrixData;   // Puntero a los datos de la matriz
} Matrix;

// Función para crear una matriz y llenarla con valores aleatorios si se solicita
Matrix* create_matrix(size_t matrixSize, int fillWithRandom) {
    Matrix* matrix = (Matrix*) malloc(sizeof(Matrix)); // Reservar memoria para la estructura de la matriz
    matrix->matrixData = (int*) malloc(sizeof(int) * matrixSize * matrixSize); // Reservar memoria para los datos
    matrix->matrixSize = matrixSize;

    // Llenar la matriz con valores aleatorios entre 0 y 99 si se indica
    if (fillWithRandom) {
        for (size_t i = 0; i < matrixSize * matrixSize; i++) {
            matrix->matrixData[i] = rand() % 100;
        }
    }
    return matrix;
}

// Función para imprimir una matriz en formato de tabla
void print_matrix(Matrix* matrix) {
    for (size_t i = 0; i < matrix->matrixSize; i++) {
        for (size_t j = 0; j < matrix->matrixSize; j++) {
            printf("%d ", matrix->matrixData[i * matrix->matrixSize + j]);
        }
        printf("\n");
    }
}

// Función para multiplicar matrices utilizando procesos
void multiply_matrices(Matrix* matrixA, Matrix* matrixB, int* resultMatrix, int numProcesses) {
    size_t size = matrixA->matrixSize;
    size_t rowsPerProcess = size / numProcesses; // Filas que calculará cada proceso
    size_t remainingRows = size % numProcesses; // Filas restantes en caso de que no sea divisible

    for (int i = 0; i < numProcesses; i++) {
        pid_t pid = fork(); // Crear un proceso hijo

        if (pid == 0) { // Código ejecutado por los procesos hijos
            size_t startRow = i * rowsPerProcess;
            size_t endRow = (i + 1) * rowsPerProcess;
            if (i == numProcesses - 1) endRow += remainingRows; // El último proceso maneja las filas restantes

            // Multiplicación de matrices para las filas asignadas a este proceso
            for (size_t i = startRow; i < endRow; i++) {
                for (size_t j = 0; j < size; j++) {
                    resultMatrix[i * size + j] = 0;
                    for (size_t k = 0; k < size; k++) {
                        resultMatrix[i * size + j] += matrixA->matrixData[i * size + k] * matrixB->matrixData[k * size + j];
                    }
                }
            }
            exit(0); // Terminar el proceso hijo después de completar su tarea
        }
    }

    // Esperar a que todos los procesos hijos terminen antes de continuar
    for (int i = 0; i < numProcesses; i++) {
        wait(NULL);
    }
}

int main(int argc, char* argv[]) {
    // Validación de los argumentos ingresados
    if (argc != 4) {
        printf("Uso: %s <tamaño_matriz> <num_procesos> <mostrar_matrices (0 o 1)>\n", argv[0]);
        return 1;
    }

    int matrixSize = atoi(argv[1]); // Tamaño de la matriz
    int numProcesses = atoi(argv[2]); // Número de procesos a utilizar
    int showMatrices = atoi(argv[3]); // Bandera para mostrar matrices

    // Validar que los valores ingresados sean positivos
    if (matrixSize <= 0 || numProcesses <= 0) {
        printf("El tamaño de la matriz y el número de procesos deben ser positivos.\n");
        return 1;
    }

    srand(time(NULL)); // Inicializar la semilla para números aleatorios

    // Crear matrices A y B con valores aleatorios
    Matrix* matrixA = create_matrix(matrixSize, 1);
    Matrix* matrixB = create_matrix(matrixSize, 1);

    // Imprimir matrices si el usuario lo solicita
    if (showMatrices) {
        printf("Matriz A:\n");
        print_matrix(matrixA);
        printf("\nMatriz B:\n");
        print_matrix(matrixB);
    }

    // Crear memoria compartida para almacenar la matriz resultante
    int shm_id = shmget(IPC_PRIVATE, matrixSize * matrixSize * sizeof(int), IPC_CREAT | 0666);
    int* resultMatrix = (int*) shmat(shm_id, NULL, 0);

    // Medir el tiempo de ejecución de la multiplicación de matrices
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    multiply_matrices(matrixA, matrixB, resultMatrix, numProcesses);

    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Calcular el tiempo de ejecución en segundos
    double executionTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nTiempo de ejecución: %f segundos\n", executionTime);

    // Imprimir la matriz resultante si el usuario lo solicita
    if (showMatrices) {
        printf("\nMatriz C (Resultado):\n");
        for (size_t i = 0; i < matrixSize; i++) {
            for (size_t j = 0; j < matrixSize; j++) {
                printf("%d ", resultMatrix[i * matrixSize + j]);
            }
            printf("\n");
        }
    }

    // Liberar recursos de memoria compartida
    shmdt(resultMatrix);
    shmctl(shm_id, IPC_RMID, NULL);
    
    // Liberar memoria de las matrices
    free(matrixA->matrixData);
    free(matrixA);
    free(matrixB->matrixData);
    free(matrixB);

    return 0;
}

