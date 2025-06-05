#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

// Valores por defecto
#define N_DEFECTO 100000
#define ITERACIONES_DEFECTO 1000
#define HILOS_DEFECTO 4

// Función que implementa el método de Jacobi para resolver ecuaciones diferenciales
void jacobi(int num_iteraciones, int n, double* u, double* f) {
    int iteracion;
    double h = 1.0 / n;
    double h2 = h * h;
    double* u_temp = (double*)malloc((n + 1) * sizeof(double));

    // Condiciones de frontera
    u_temp[0] = u[0];
    u_temp[n] = u[n];

    for (iteracion = 0; iteracion < num_iteraciones; iteracion += 2) {
        // Primera barrida: de u a u_temp
        #pragma omp parallel for
        for (int i = 1; i < n; ++i) {
            u_temp[i] = (u[i - 1] + u[i + 1] + h2 * f[i]) / 2.0;
        }

        // Segunda barrida: de u_temp a u
        #pragma omp parallel for
        for (int i = 1; i < n; ++i) {
            u[i] = (u_temp[i - 1] + u_temp[i + 1] + h2 * f[i]) / 2.0;
        }
    }

    free(u_temp);
}

int main(int argc, char** argv) {
    int i, n, num_iteraciones, num_hilos;
    double* u;   // Solución
    double* f;   // Fuente
    double h;
    double tiempo_inicio, tiempo_fin;

    // Lectura de argumentos o uso de valores por defecto
    n = (argc > 1) ? atoi(argv[1]) : N_DEFECTO;
    num_iteraciones = (argc > 2) ? atoi(argv[2]) : ITERACIONES_DEFECTO;
    num_hilos = (argc > 3) ? atoi(argv[3]) : HILOS_DEFECTO;
    h = 1.0 / n;

    omp_set_num_threads(num_hilos);

    // Asignación de memoria
    u = (double*)malloc((n + 1) * sizeof(double));
    f = (double*)malloc((n + 1) * sizeof(double));

    // Inicialización
    memset(u, 0, (n + 1) * sizeof(double));  // u empieza en cero
    for (i = 0; i <= n; ++i)
        f[i] = i * h;  // función fuente lineal

    // Medición del tiempo de ejecución
    tiempo_inicio = omp_get_wtime();
    jacobi(num_iteraciones, n, u, f);
    tiempo_fin = omp_get_wtime();

    printf("\nTiempo de ejecución: %f segundos\n", tiempo_fin - tiempo_inicio);

    // Liberar memoria
    free(f);
    free(u);

    return 0;
}

