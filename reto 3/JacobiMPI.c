#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_POR_DEFECTO 100000
#define PASOS_POR_DEFECTO 1000

// Función que aplica el método de Jacobi en paralelo
void jacobi(int pasos, int n_local, int n_total, double* u_local, double* f_local, int rango, int num_procesos) {
    double h = 1.0 / n_total;
    double h2 = h * h;
    double* tmp = malloc((n_local + 2) * sizeof(double)); // incluye celdas fantasma

    for (int paso = 0; paso < pasos; ++paso) {
        // Intercambio de bordes con procesos vecinos
        if (rango > 0)
            MPI_Sendrecv(&u_local[1], 1, MPI_DOUBLE, rango - 1, 0,
                         &u_local[0], 1, MPI_DOUBLE, rango - 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (rango < num_procesos - 1)
            MPI_Sendrecv(&u_local[n_local], 1, MPI_DOUBLE, rango + 1, 0,
                         &u_local[n_local + 1], 1, MPI_DOUBLE, rango + 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Cálculo del nuevo valor en cada punto local
        for (int i = 1; i <= n_local; ++i) {
            tmp[i] = (u_local[i - 1] + u_local[i + 1] + h2 * f_local[i]) / 2.0;
        }

        // Intercambio de punteros para la siguiente iteración
        double* aux = u_local;
        u_local = tmp;
        tmp = aux;
    }

    free(tmp);
}

int main(int argc, char** argv) {
    int n = N_POR_DEFECTO;          // Tamaño total del dominio
    int pasos = PASOS_POR_DEFECTO;  // Número de barridos del método Jacobi

    if (argc > 1) n = atoi(argv[1]);
    if (argc > 2) pasos = atoi(argv[2]);

    MPI_Init(&argc, &argv);
    int rango, num_procesos;
    MPI_Comm_rank(MPI_COMM_WORLD, &rango);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procesos);

    // Verifica que n sea divisible por el número de procesos
    if (n % num_procesos != 0) {
        if (rango == 0)
            printf("Error: N debe ser divisible por el número de procesos.\n");
        MPI_Finalize();
        return 1;
    }

    int n_local = n / num_procesos;

    // u_local tiene celdas adicionales en los extremos para los valores fantasma
    double* u_local = calloc(n_local + 2, sizeof(double));
    double* f_local = malloc((n_local + 2) * sizeof(double));

    // Inicialización del vector f_local
    double h = 1.0 / n;
    for (int i = 1; i <= n_local; ++i) {
        int i_global = rango * n_local + i;
        f_local[i] = i_global * h;  // Ejemplo simple de función fuente
    }

    // Medición del tiempo de ejecución
    double tiempo_inicio = MPI_Wtime();
    jacobi(pasos, n_local, n, u_local, f_local, rango, num_procesos);
    double tiempo_fin = MPI_Wtime();

    if (rango == 0)
        printf("Tiempo de ejecución: %f segundos\n", tiempo_fin - tiempo_inicio);

    free(u_local);
    free(f_local);
    MPI_Finalize();
    return 0;
}

