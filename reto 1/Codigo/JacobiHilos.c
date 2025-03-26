#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

// Definimos valores por defecto para el tamaño del problema, número de iteraciones y número de hilos
#define DEFAULT_N 100000
#define DEFAULT_NSTEPS 1000
#define DEFAULT_THREADS 4

// Estructura que almacena los datos de cada hilo
typedef struct {
    int start, end; // Rango de índices que procesará el hilo
    int n;
    double h2;
    double* u;
    double* f;
    double* utmp;
} ThreadData;

pthread_barrier_t barrier; // Barrera para sincronizar los hilos

// Función que ejecuta cada hilo para actualizar una porción del arreglo
void* jacobi_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    for (int i = data->start; i < data->end; ++i) {
        data->utmp[i] = (data->u[i-1] + data->u[i+1] + data->h2 * data->f[i]) / 2;
    }

    // Sincronización de los hilos antes de proceder con la siguiente iteración
    pthread_barrier_wait(&barrier);
    return NULL;
}

// Función que ejecuta el método de Jacobi en paralelo
void jacobi(int nsweeps, int n, int num_threads, double* u, double* f) {
    int i, sweep;
    double h = 1.0 / n;
    double h2 = h * h;
    double* utmp = (double*)malloc((n + 1) * sizeof(double)); // Arreglo temporal

    utmp[0] = u[0]; // Condiciones de frontera
    utmp[n] = u[n];

    pthread_t threads[num_threads]; // Arreglo de hilos
    ThreadData thread_data[num_threads]; // Datos para cada hilo
    pthread_barrier_init(&barrier, NULL, num_threads); // Inicialización de la barrera
    int chunk_size = n / num_threads; // Tamaño de la porción de datos para cada hilo

    for (sweep = 0; sweep < nsweeps; sweep += 2) {
        // Crear hilos para realizar el cálculo en paralelo
        for (i = 0; i < num_threads; i++) {
            thread_data[i].start = 1 + i * chunk_size;
            thread_data[i].end = (i == num_threads - 1) ? n : (i + 1) * chunk_size;
            thread_data[i].n = n;
            thread_data[i].h2 = h2;
            thread_data[i].u = u;
            thread_data[i].f = f;
            thread_data[i].utmp = utmp;
            pthread_create(&threads[i], NULL, jacobi_thread, &thread_data[i]);
        }

        // Esperar a que todos los hilos terminen
        for (i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
        }

        // Intercambiar los punteros para la siguiente iteración
        double* tmp = u;
        u = utmp;
        utmp = tmp;
    }

    pthread_barrier_destroy(&barrier); // Destruir la barrera al final del proceso
    free(utmp); // Liberar memoria
}

int main(int argc, char** argv) {
    int i, n, nsteps, num_threads;
    double* u;
    double* f;
    double h;
    struct timespec start, end;

    // Leer los parámetros de entrada o usar valores por defecto
    n = (argc > 1) ? atoi(argv[1]) : DEFAULT_N;
    nsteps = (argc > 2) ? atoi(argv[2]) : DEFAULT_NSTEPS;
    num_threads = (argc > 3) ? atoi(argv[3]) : DEFAULT_THREADS;
    h = 1.0 / n;

    // Reservar memoria para los arreglos
    u = (double*)malloc((n + 1) * sizeof(double));
    f = (double*)malloc((n + 1) * sizeof(double));
    memset(u, 0, (n + 1) * sizeof(double)); // Inicializar el arreglo u con ceros
    for (i = 0; i <= n; ++i)
        f[i] = i * h; // Inicialización del arreglo de términos fuente

    // Medir el tiempo de ejecución
    clock_gettime(CLOCK_MONOTONIC, &start);
    jacobi(nsteps, n, num_threads, u, f);
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calcular y mostrar el tiempo de ejecución
    double executionTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nExecution time: %f seconds\n", executionTime);

    // Liberar la memoria utilizada
    free(f);
    free(u);
    return 0;
}

