#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Funci�n que implementa el m�todo de Jacobi para resolver ecuaciones diferenciales parciales (1D Poisson)
void jacobi(int nsweeps, int n, double* u, double* f) {
    int i, sweep;
    double h  = 1.0 / n;       // Tama�o de paso en el espacio
    double h2 = h * h;         // Cuadrado del tama�o de paso
    double* utmp = (double*) malloc((n + 1) * sizeof(double)); // Memoria para una copia temporal de u

    // Condiciones de frontera: la primera y �ltima posici�n se mantienen constantes
    utmp[0] = u[0];
    utmp[n] = u[n];

    // Realiza nsweeps iteraciones del m�todo de Jacobi
    for (sweep = 0; sweep < nsweeps; sweep += 2) {
        // Primera pasada: calcula los nuevos valores en utmp usando los valores actuales de u
        for (i = 1; i < n; ++i)
            utmp[i] = (u[i - 1] + u[i + 1] + h2 * f[i]) / 2;

        // Segunda pasada: actualiza u con los valores de utmp
        for (i = 1; i < n; ++i)
            u[i] = (utmp[i - 1] + utmp[i + 1] + h2 * f[i]) / 2;
    }

    free(utmp); // Liberar memoria de la matriz temporal
}

// Funci�n para escribir la soluci�n en un archivo de salida
void write_solution(int n, double* u, const char* fname) {
    int i;
    double h = 1.0 / n; // Tama�o de paso en el espacio
    FILE* fp = fopen(fname, "w+"); // Abre el archivo para escritura
    if (!fp) {
        printf("Error al abrir el archivo %s\n", fname);
        return;
    }

    // Escribe los valores de la soluci�n en el archivo
    for (i = 0; i <= n; ++i)
        fprintf(fp, "%g %g\n", i * h, u[i]);

    fclose(fp); // Cierra el archivo
}

int main(int argc, char** argv) {
    int i;
    int n, nsteps;
    double* u; // Vector soluci�n
    double* f; // Vector del lado derecho de la ecuaci�n
    double h;
    struct timespec start, end;
    double executionTime;
    char* fname;

    // Obtiene los valores de n y nsteps desde los argumentos de la l�nea de comandos
    n = (argc > 1) ? atoi(argv[1]) : 100; // N�mero de puntos en la malla
    nsteps = (argc > 2) ? atoi(argv[2]) : 100; // N�mero de iteraciones
    fname = (argc > 3) ? argv[3] : NULL; // Nombre del archivo de salida (si se proporciona)
    h = 1.0 / n; // Tama�o de paso

    // Asigna memoria para los vectores u y f
    u = (double*) malloc((n + 1) * sizeof(double));
    f = (double*) malloc((n + 1) * sizeof(double));

    // Inicializa u con ceros
    memset(u, 0, (n + 1) * sizeof(double));

    // Inicializa f con valores lineales (f[i] = i * h)
    for (i = 0; i <= n; ++i)
        f[i] = i * h;

    // Mide el tiempo de ejecuci�n del m�todo de Jacobi
    clock_gettime(CLOCK_MONOTONIC, &start);
    jacobi(nsteps, n, u, f);
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calcula el tiempo de ejecuci�n en segundos
    executionTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nExecution time: %f seconds\n", executionTime);

    // Si se proporciona un nombre de archivo, guarda la soluci�n
    if (fname)
        write_solution(n, u, fname);

    // Libera la memoria asignada
    free(f);
    free(u);
    
    return 0;
}

