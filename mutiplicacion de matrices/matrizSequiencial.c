#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Estructura para representar una matriz cuadrada
typedef struct {
    size_t matrixSize; // Dimensión de la matriz (n x n)
    int* matrixData;   // Puntero a los datos de la matriz almacenados en un arreglo unidimensional
} Matrix;

// Función para crear una matriz
Matrix* create_matrix(size_t matrixSize, int fillWithRandom) {
    Matrix* matrix = (Matrix*) malloc(sizeof(Matrix)); // Reservar memoria para la estructura
    matrix->matrixData = (int*) malloc(sizeof(int) * matrixSize * matrixSize); // Reservar memoria para los datos
    matrix->matrixSize = matrixSize;

    // Si se indica, llenar la matriz con valores aleatorios entre 0 y 99
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

// Función para multiplicar dos matrices cuadradas
Matrix* multiply_matrices(Matrix* matrixA, Matrix* matrixB) {
    size_t size = matrixA->matrixSize;
    Matrix* resultMatrix = create_matrix(size, 0); // Crear matriz vacía para el resultado

    // Algoritmo estándar de multiplicación de matrices
    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < size; j++) {
            resultMatrix->matrixData[i * size + j] = 0;
            for (size_t k = 0; k < size; k++) {
                resultMatrix->matrixData[i * size + j] += 
                    matrixA->matrixData[i * size + k] * matrixB->matrixData[k * size + j];
            }
        }
    }
    return resultMatrix;
}

// Función para liberar la memoria reservada para una matriz
void delete_matrix(Matrix** matrix) {
    if (matrix == NULL || *matrix == NULL) return;
    free((*matrix)->matrixData); // Liberar memoria de los datos de la matriz
    free(*matrix); // Liberar memoria de la estructura matriz
    *matrix = NULL;
}

int main(int argc, char* argv[]) {
    // Verificar los argumentos de entrada
    if (argc != 3) {
        printf("Uso: %s <tamaño_matriz> <mostrar_matrices (0 o 1)>\n", argv[0]);
        return 1;
    }

    int matrixSize = atoi(argv[1]); // Convertir argumento a entero
    if (matrixSize <= 0) {
        printf("El tamaño de la matriz debe ser un número positivo.\n");
        return 1;
    }

    int showMatrices = atoi(argv[2]); // Indicar si se deben mostrar las matrices

    srand(time(NULL)); // Inicializar la semilla para la generación de números aleatorios

    // Crear matrices A y B con valores aleatorios
    Matrix* matrixA = create_matrix(matrixSize, 1);
    Matrix* matrixB = create_matrix(matrixSize, 1);

    // Imprimir las matrices si el usuario lo solicita
    if (showMatrices) {
        printf("Matriz A:\n");
        print_matrix(matrixA);
        printf("\nMatriz B:\n");
        print_matrix(matrixB);
    }

    // Medir el tiempo de ejecución de la multiplicación de matrices
    clock_t start = clock();
    Matrix* resultMatrix = multiply_matrices(matrixA, matrixB);
    clock_t end = clock();

    // Imprimir la matriz resultante si es necesario
    if (showMatrices) {
        printf("\nMatriz C (Resultado):\n");
        print_matrix(resultMatrix);
    }

    // Calcular y mostrar el tiempo de ejecución
    double executionTime = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("\nTiempo de ejecución: %f segundos\n", executionTime);

    // Liberar la memoria reservada para las matrices
    delete_matrix(&matrixA);
    delete_matrix(&matrixB);
    delete_matrix(&resultMatrix);

    return 0;
}

