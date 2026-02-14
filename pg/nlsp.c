#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CELLS 8192
#define MAX_TOKEN 64
#define MAX_ENV 256

typedef struct cell {
    int type; // 0=atom, 1=list, 2=function
    union { 
        char atom[MAX_TOKEN]; 
        struct { struct cell *car, *cdr; } list; 
        struct { struct cell *params, *body; } func;
    } v;
} cell_t;

typedef struct env {
    char name[MAX_TOKEN];
    cell_t *value;
} env_t;

static cell_t cells[MAX_CELLS];
static env_t env[MAX_ENV];
static int next_cell = 0, env_size = 0;
static char token[MAX_TOKEN], *input_ptr;

// Forward declarations
cell_t *parse();
cell_t *eval(cell_t *expr);

cell_t *new_cell() { 
    if (next_cell >= MAX_CELLS) { 
        fprintf(stderr, "Out of memory\n"); 
        exit(1); 
    } 
    return &cells[next_cell++]; 
}
cell_t *make_atom(const char *s) {
    cell_t *c = new_cell();
    c->type = 0;
    if (s) strcpy(c->v.atom, s);
    return c;
}
cell_t *make_list(cell_t *car, cell_t *cdr) { 
    cell_t *c = new_cell(); 
    c->type = 1; 
    c->v.list.car = car; 
    c->v.list.cdr = cdr; 
    return c; 
}
cell_t *make_func(cell_t *params, cell_t *body) { 
    cell_t *c = new_cell(); 
    c->type = 2; 
    c->v.func.params = params; 
    c->v.func.body = body; 
    return c; 
}
int is_number(const char *s) { 
    if (*s == '-' || *s == '+') s++; 
    if (!*s) return 0; 
    while (*s) if (!isdigit(*s++)) return 0; 
    return 1; 
}

cell_t *lookup(const char *name) { 
    for (int i = env_size - 1; i >= 0; i--) 
        if (strcmp(env[i].name, name) == 0) return env[i].value; 
    return NULL; 
}
void bind(const char *name, cell_t *value) { 
    if (env_size < MAX_ENV) { 
        strcpy(env[env_size].name, name); 
        env[env_size].value = value; 
        env_size++; 
    } 
}

char *next_token() {
    while (*input_ptr && isspace(*input_ptr)) input_ptr++; 
    if (!*input_ptr) return NULL;
    char *start = token;
    if (*input_ptr == '(' || *input_ptr == ')') *start++ = *input_ptr++;
    else while (*input_ptr && !isspace(*input_ptr) && *input_ptr != '(' && *input_ptr != ')') *start++ = *input_ptr++;
    *start = '\0'; 
    return token;
}

cell_t *parse_list() {
    char *tok = next_token(); 
    if (!tok || strcmp(tok, ")") == 0) return NULL;
    cell_t *car, *cdr;
    if (strcmp(tok, "(") == 0) { 
        car = parse_list();
        // next_token(); // consume closing ')'
    } else { 
        car = make_atom(tok); 
    }
    cdr = parse_list(); 
    return make_list(car, cdr);
}

cell_t *parse() { 
    char *tok = next_token(); 
    if (!tok) return NULL; 
    if (strcmp(tok, "(") == 0) { 
        cell_t *result = parse_list(); 
        next_token(); 
        return result; 
    } 
    return make_atom(tok); 
}

cell_t *eval_args(cell_t *args) { 
    if (!args) return NULL; 
    return make_list(eval(args->v.list.car), eval_args(args->v.list.cdr)); 
}

cell_t *apply_func(cell_t *func, cell_t *args) {
    if (!func || func->type != 2) return NULL; 
    int old_env_size = env_size; 
    cell_t *params = func->v.func.params;
    while (params && args) { 
        bind(params->v.list.car->v.atom, args->v.list.car); 
        params = params->v.list.cdr; 
        args = args->v.list.cdr; 
    }
    cell_t *result = eval(func->v.func.body); 
    env_size = old_env_size; 
    return result;
}

cell_t *eval(cell_t *expr) {
    if (!expr) return NULL;
    if (expr->type == 0) { // atom
        if (is_number(expr->v.atom)) return expr;
        if (strcmp(expr->v.atom, "nil") == 0) return NULL;
        if (strcmp(expr->v.atom, "t") == 0) return make_atom("t");
        cell_t *val = lookup(expr->v.atom); // variable lookup
        return val ? val : expr;
    }
    
    cell_t *op = expr->v.list.car;
    if (!op || op->type != 0) return NULL;
    cell_t *args = expr->v.list.cdr;
    
    if (!strcmp(op->v.atom, "quote")) return args ? args->v.list.car : NULL;
    
    if (!strcmp(op->v.atom, "car")) { 
        if (!args) return NULL;
        cell_t *arg = eval(args->v.list.car);
        if (!arg) return NULL;
        return (arg->type == 1) ? arg->v.list.car : NULL;
    }
    
    if (!strcmp(op->v.atom, "cdr")) { 
        if (!args) return NULL;
        cell_t *arg = eval(args->v.list.car);
        if (!arg) return NULL;
        return (arg->type == 1) ? arg->v.list.cdr : NULL;
    }
    
    if (!strcmp(op->v.atom, "cons")) {
        if (!args || !args->v.list.cdr) return NULL;
        cell_t *car_val = eval(args->v.list.car);
        cell_t *cdr_val = eval(args->v.list.cdr->v.list.car);
        return make_list(car_val, cdr_val);
    }
    
    if (!strcmp(op->v.atom, "atom")) { 
        if (!args) return NULL;
        cell_t *arg = eval(args->v.list.car);
        if (!arg) return make_atom("t");  // nil is considered atomic
        return (arg->type == 0) ? make_atom("t") : NULL; 
    }
    
    if (!strcmp(op->v.atom, "eq")) {
        if (!args || !args->v.list.cdr) return NULL;
        cell_t *a1 = eval(args->v.list.car);
        cell_t *a2 = eval(args->v.list.cdr->v.list.car);
        if (!a1 && !a2) return make_atom("t");
        if (!a1 || !a2) return NULL;
        if (a1->type == 0 && a2->type == 0) {
            return strcmp(a1->v.atom, a2->v.atom) == 0 ? make_atom("t") : NULL;
        }
        return (a1 == a2) ? make_atom("t") : NULL;
    }
    
    if (!strcmp(op->v.atom, "+") || !strcmp(op->v.atom, "-") || !strcmp(op->v.atom, "*") || !strcmp(op->v.atom, "/")) {
        if (!args || !args->v.list.cdr) return NULL;
        cell_t *a1 = eval(args->v.list.car);
        cell_t *a2 = eval(args->v.list.cdr->v.list.car);
        if (!a1 || !a2) return NULL;
        if (a1->type != 0 || a2->type != 0) return NULL;
        if (!is_number(a1->v.atom) || !is_number(a2->v.atom)) return NULL;
        int n1 = atoi(a1->v.atom), n2 = atoi(a2->v.atom), result = 0;
        if (op->v.atom[0] == '+') result = n1 + n2;
        else if (op->v.atom[0] == '-') result = n1 - n2;
        else if (op->v.atom[0] == '*') result = n1 * n2;
        else if (op->v.atom[0] == '/') result = n2 ? n1 / n2 : 0;
        static char buf[32]; sprintf(buf, "%d", result); 
        return make_atom(buf);
    }
    
    if (!strcmp(op->v.atom, "if")) {
        if (!args || !args->v.list.cdr || !args->v.list.cdr->v.list.cdr) return NULL;
        cell_t *cond = eval(args->v.list.car);
        return cond ? eval(args->v.list.cdr->v.list.car) : eval(args->v.list.cdr->v.list.cdr->v.list.car);
    }
    
    if (!strcmp(op->v.atom, "defun")) {
        if (!args || !args->v.list.cdr || !args->v.list.cdr->v.list.cdr) return NULL;
        char *name = args->v.list.car->v.atom;
        cell_t *params = args->v.list.cdr->v.list.car;
        cell_t *body = args->v.list.cdr->v.list.cdr->v.list.car;
        cell_t *func = make_func(params, body);
        bind(name, func);
        return make_atom(name);
    }
    
    if (!strcmp(op->v.atom, "lambda")) {
        if (!args || !args->v.list.cdr) return NULL;
        return make_func(args->v.list.car, args->v.list.cdr->v.list.car);
    }
    
    if (!strcmp(op->v.atom, "let")) {
        if (!args || !args->v.list.cdr) return NULL;
        int old_env_size = env_size;
        cell_t *bindings = args->v.list.car;
        while (bindings) {
            cell_t *binding = bindings->v.list.car;
            if (binding && binding->type == 1) {
                bind(binding->v.list.car->v.atom, eval(binding->v.list.cdr->v.list.car));
            }
            bindings = bindings->v.list.cdr;
        }
        cell_t *result = eval(args->v.list.cdr->v.list.car);
        env_size = old_env_size;
        return result;
    }
    
    // Built-in predicates and functions
    if (!strcmp(op->v.atom, "null")) {
        if (!args) return NULL;
        cell_t *arg = eval(args->v.list.car);
        return (!arg) ? make_atom("t") : NULL;
    }
    
    if (!strcmp(op->v.atom, "list")) {
        return NULL;
    }
    
    // Function application
    cell_t *func = eval(op);
    if (func && func->type == 2) {
        return apply_func(func, eval_args(args));
    }
    
    return NULL;
}

void print_expr(cell_t *expr) {
    if (!expr) { printf("nil"); return; }
    if (expr->type == 0) { printf("%s", expr->v.atom); return; }
    if (expr->type == 2) { printf("#<function>"); return; }
    printf("("); 
    cell_t *current = expr; 
    int first = 1;
    while (current && current->type == 1) { 
        if (!first) printf(" "); 
        print_expr(current->v.list.car); 
        current = current->v.list.cdr; 
        first = 0; 
        if (current && current->type != 1) {
            printf(" . ");
            print_expr(current);
            break;
        }
    }
    printf(")");
}

int main(int argc, char **argv) {
    FILE *fp = stdin;
    if (argc > 1) { 
        fp = fopen(argv[1], "r"); 
        if (!fp) { 
            fprintf(stderr, "Cannot open file %s\n", argv[1]); 
            return 1; 
        } 
    }
    static char input[8192];
    size_t len = fread(input, 1, sizeof(input) - 1, fp);
    input[len] = '\0'; 
    input_ptr = input;
    if (fp != stdin) fclose(fp);
    
    cell_t *expr;
    while ((expr = parse()) != NULL) { 
        cell_t *result = eval(expr); 
        print_expr(result); 
        printf("\n"); 
    }
    return 0;
}
