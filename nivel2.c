#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64

// Control de depuracion (Desactivado N1, Activado N2)
#define DEBUGN1 0
#define DEBUGN2 1

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
    fflush(stdout);     // Vaciar buffer

    // Leer linea desde teclado
    char *ptr = fgets(line, COMMAND_LINE_SIZE, stdin);

    if (ptr != NULL) {
        // Sustituir el salto de linea '\n' por final de cadena '\0'
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
    
    // Trocear la linea en tokens
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
        // Tratar comentarios: si empieza por '#', ignorar resto de la linea
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

    // El ultimo argumento siempre debe terminar en NULL
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
    char cwd[COMMAND_LINE_SIZE];
    char target_dir[COMMAND_LINE_SIZE] = "";

    // Determinar el directorio destino
    if (args[1] == NULL) {
        // Caso sin argumentos (HOME)
        char *home = getenv("HOME");
        if (home == NULL) {
            fprintf(stderr, ROJO "Error: La variable HOME no esta definida\n" RESET);
            return -1;
        }
        strcpy(target_dir, home);
    } else {
        // Caso con 1 o mas argumentos 
        for (int i = 1; args[i] != NULL; i++) {
            if (i > 1) {
                strcat(target_dir, " ");
            }
            strcat(target_dir, args[i]);
        }

        // Eliminar comillas simples, dobles y el caracter escape (\) del directorio destino
        char clean_dir[COMMAND_LINE_SIZE];
        int j = 0;
        for (int i = 0; target_dir[i] != '\0'; i++) {
            if (target_dir[i] != '\'' && target_dir[i] != '"' && target_dir[i] != '\\') {
                clean_dir[j++] = target_dir[i];
            }
        }
        clean_dir[j] = '\0';
        strcpy(target_dir, clean_dir);
    }

    // Cambiar de directorio
    if (chdir(target_dir) != 0) {
        perror(ROJO "chdir() error" RESET);
        return -1;
    }

    // Imprimir el nuevo directorio 
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        #if DEBUGN2
        fprintf(stderr, GRIS "[internal_cd()- PWD: %s]\n" RESET, cwd);
        #endif
    } else {
        perror(ROJO "getcwd() error" RESET);
    }

    return 1;
}

int internal_export(char **args) {
    // Control de sintaxis inicial
    if (args[1] == NULL) {
        fprintf(stderr, ROJO "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return -1;
    }

    // Buscar el simbolo '='
    char *pos_igual = strchr(args[1], '=');
    if (pos_igual == NULL) {
        #if DEBUGN2
        fprintf(stderr, GRIS "[internal_export() nombre: %s]\n" RESET, args[1]);
        fprintf(stderr, GRIS "[internal_export()- valor: (null)]\n" RESET);
        #endif
        fprintf(stderr, ROJO "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return -1;
    }

    // Separar NOMBRE y VALOR
    *pos_igual = '\0';
    char *nombre = args[1];
    char *valor = pos_igual + 1;

    #if DEBUGN2
    fprintf(stderr, GRIS "[internal_export() nombre: %s]\n" RESET, nombre);
    fprintf(stderr, GRIS "[internal_export()- valor: %s]\n" RESET, valor);
    #endif

    // Obtener valor antiguo
    char *old_val = getenv(nombre);
    #if DEBUGN2
    fprintf(stderr, GRIS "[internal_export() antiguo valor para %s: %s]\n" RESET, 
            nombre, old_val ? old_val : "(null)");
    #endif

    // Asignar nuevo valor con setenv()
    if (setenv(nombre, valor, 1) != 0) {
        perror(ROJO "setenv() error" RESET);
        return -1;
    }

    // Obtener y mostrar el nuevo valor
    char *new_val = getenv(nombre);
    #if DEBUGN2
    fprintf(stderr, GRIS "[internal_export()- nuevo valor para %s: %s]\n" RESET, 
            nombre, new_val ? new_val : "(null)");
    #endif

    return 1;
}

int internal_source(char **args) {
    #if DEBUGN1
    fprintf(stderr, GRIS "[internal_source()→ Esta funcion ejecutara un fichero de lineas de comandos]\n" RESET);
    #endif
    return 1;
}

int internal_jobs() {
    #if DEBUGN1
    fprintf(stderr, GRIS "[internal_jobs()→ Esta funcion mostrara el PID de los procesos que no esten en foreground]\n" RESET);
    #endif
    return 1;
}

int internal_fg(char **args) {
    #if DEBUGN1
    fprintf(stderr, GRIS "[internal_fg()→ Esta funcion pondra un trabajo en foreground]\n" RESET);
    #endif
    return 1;
}

int internal_bg(char **args) {
    #if DEBUGN1
    fprintf(stderr, GRIS "[internal_bg()→ Esta funcion pondra un trabajo en background]\n" RESET);
    #endif
    return 1;
}