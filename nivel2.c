


//hay que añadir lo de la semana 2 aquí



#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define DEBUGN1 1

// Colores 
#define RESET "\033[0m"
#define NEGRO "\x1b[30m"
#define GRIS "\x1b[38;5;245m"
#define ROJO "\x1b[31m"
#define VERDE "\x1b[32m"
#define AMARILLO "\x1b[33m"
#define AZUL "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define BLANCO "\x1b[97m"
#define NEGRITA "\x1b[1m"

// Declaraciones de funciones del shell
char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);

// Declaraciones de comandos internos
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs();
int internal_fg(char **args);
int internal_bg(char **args);

int main() {
    char line[COMMAND_LINE_SIZE];
    
    while (1) {
        if (read_line(line)) {
            execute_line(line);
        }
    }
    
    return 0;
}

char *read_line(char *line) {
     
    printf("%c ", '$'); // Imprimir el prompt
    fflush(stdout); // vaciar buffer

    // Leer línea desde teclado
    char *ptr = fgets(line, COMMAND_LINE_SIZE, stdin);

    if (ptr != NULL) {
        // Sustituir el salto de línea '\n' por final de cadena '\0'
        char *pos = strchr(line, '\n');
        if (pos != NULL) {
            *pos = '\0';
        }
    } else if (feof(stdin)) {
        // Control de Ctrl+D
        printf("\rBye Bye\n");
        exit(0);
    }

    return ptr;
}

int execute_line(char *line) {
    char *args[ARGS_SIZE];
    
    // Trocear la línea en tokens
    int num_tokens = parse_args(args, line);

    // Si hay tokens, comprobar si es comando interno
    if (num_tokens > 0) {
        check_internal(args);
    }

    return 0;
}

int parse_args(char **args, char *line) {
    int num_tokens = 0;
    char *token = strtok(line, " \t\n\r");

    while (token != NULL && num_tokens < ARGS_SIZE - 1) {
        // Tratar comentarios: si empieza por '#', ignorar resto de la línea
        if (token[0] == '#') {
            break;
        }

        args[num_tokens] = token;

        #if DEBUGN1
        fprintf(stderr, GRIS "[parse_args()→ token %d: %s]\n" RESET, num_tokens, args[num_tokens]);
        #endif

        num_tokens++;
        token = strtok(NULL, " \t\n\r");
    }

    // El último argumento siempre debe terminar en NULL
    args[num_tokens] = NULL;

    #if DEBUGN1
    fprintf(stderr, GRIS "[parse_args()→ token %d: (null)]\n" RESET, num_tokens);
    #endif

    return num_tokens;
}

int check_internal(char **args) {
    if (strcmp(args[0], "exit") == 0) {
        printf("Bye Bye\n");
        exit(0);
    }
    if (strcmp(args[0], "cd") == 0) return internal_cd(args);
    if (strcmp(args[0], "export") == 0) return internal_export(args);
    if (strcmp(args[0], "source") == 0) return internal_source(args);
    if (strcmp(args[0], "jobs") == 0) return internal_jobs();
    if (strcmp(args[0], "fg") == 0) return internal_fg(args);
    if (strcmp(args[0], "bg") == 0) return internal_bg(args);

    return 0;
}

int internal_cd(char **args) {
    #if DEBUGN1
    fprintf(stderr, GRIS "[internal_cd()→ Esta función cambiará de directorio]\n" RESET);
    #endif
    return 1;
}

int internal_export(char **args) {
    #if DEBUGN1
    fprintf(stderr, GRIS "[internal_export()→ Esta función asignará valores a variables de entorno]\n" RESET);
    #endif
    return 1;
}

int internal_source(char **args) {
    #if DEBUGN1
    fprintf(stderr, GRIS "[internal_source()→ Esta función ejecutará un fichero de líneas de comandos]\n" RESET);
    #endif
    return 1;
}

int internal_jobs() {
    #if DEBUGN1
    fprintf(stderr, GRIS "[internal_jobs()→ Esta función mostrará el PID de los procesos que no estén en foreground]\n" RESET);
    #endif
    return 1;
}

int internal_fg(char **args) {
    #if DEBUGN1
    fprintf(stderr, GRIS "[internal_fg()→ Esta función pondrá un trabajo en foreground]\n" RESET);
    #endif
    return 1;
}

int internal_bg(char **args) {
    #if DEBUGN1
    fprintf(stderr, GRIS "[internal_bg()→ Esta función pondrá un trabajo en background]\n" RESET);
    #endif
    return 1;
} 