/* SL - Simple Lang: A minimal stack-based programming language
 * Author: Chris Curl
 * 
 * SL is a simple, Forth-like stack-based language designed for
 * educational purposes and simple scripting tasks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define STACK_SIZE 256
#define RETURN_STACK_SIZE 64
#define DICT_SIZE 1000
#define NAME_LEN 32
#define CODE_SIZE 10000

typedef long cell_t;

/* Stacks */
cell_t stack[STACK_SIZE];
int sp = -1;
cell_t rstack[RETURN_STACK_SIZE];
int rsp = -1;

/* Dictionary entry */
typedef struct {
    char name[NAME_LEN];
    int code_addr;
    int is_immediate;
} word_t;

word_t dict[DICT_SIZE];
int dict_ptr = 0;

/* Code memory */
unsigned char code[CODE_SIZE];
int here = 0;
int ip = 0;

/* Input buffer */
char input_buffer[1024];
int input_pos = 0;

/* State */
int compiling = 0;
int running = 0;

/* Stack operations */
void push(cell_t val) {
    if (sp >= STACK_SIZE - 1) {
        fprintf(stderr, "Stack overflow!\n");
        exit(1);
    }
    stack[++sp] = val;
}

cell_t pop() {
    if (sp < 0) {
        fprintf(stderr, "Stack underflow!\n");
        exit(1);
    }
    return stack[sp--];
}

cell_t peek() {
    if (sp < 0) {
        fprintf(stderr, "Stack empty!\n");
        exit(1);
    }
    return stack[sp];
}

void rpush(cell_t val) {
    if (rsp >= RETURN_STACK_SIZE - 1) {
        fprintf(stderr, "Return stack overflow!\n");
        exit(1);
    }
    rstack[++rsp] = val;
}

cell_t rpop() {
    if (rsp < 0) {
        fprintf(stderr, "Return stack underflow!\n");
        exit(1);
    }
    return rstack[rsp--];
}

/* Bytecodes */
enum {
    OP_NOP = 0,
    OP_LIT,      /* Push literal */
    OP_ADD,      /* + */
    OP_SUB,      /* - */
    OP_MUL,      /* * */
    OP_DIV,      /* / */
    OP_MOD,      /* % */
    OP_EQ,       /* = */
    OP_LT,       /* < */
    OP_GT,       /* > */
    OP_AND,      /* AND */
    OP_OR,       /* OR */
    OP_NOT,      /* NOT */
    OP_DUP,      /* DUP */
    OP_DROP,     /* DROP */
    OP_SWAP,     /* SWAP */
    OP_OVER,     /* OVER */
    OP_ROT,      /* ROT */
    OP_EMIT,     /* EMIT */
    OP_DOT,      /* . */
    OP_CR,       /* CR */
    OP_CALL,     /* Call word */
    OP_RET,      /* Return */
    OP_JMP,      /* Unconditional jump */
    OP_JZ,       /* Jump if zero */
    OP_HALT      /* Stop execution */
};

/* Emit byte to code memory */
void emit_byte(unsigned char b) {
    if (here >= CODE_SIZE) {
        fprintf(stderr, "Code memory full!\n");
        exit(1);
    }
    code[here++] = b;
}

/* Emit cell (as bytes) */
void emit_cell(cell_t val) {
    emit_byte(OP_LIT);
    for (size_t i = 0; i < sizeof(cell_t); i++) {
        emit_byte((val >> (i * 8)) & 0xFF);
    }
}

/* Read cell from code */
cell_t read_cell(int addr) {
    cell_t val = 0;
    for (size_t i = 0; i < sizeof(cell_t); i++) {
        val |= ((cell_t)code[addr + i]) << (i * 8);
    }
    return val;
}

/* Find word in dictionary */
int find_word(const char *name) {
    for (int i = dict_ptr - 1; i >= 0; i--) {
        if (strcmp(dict[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/* Add word to dictionary */
void add_word(const char *name, int code_addr, int immediate) {
    if (dict_ptr >= DICT_SIZE) {
        fprintf(stderr, "Dictionary full!\n");
        exit(1);
    }
    strncpy(dict[dict_ptr].name, name, NAME_LEN - 1);
    dict[dict_ptr].name[NAME_LEN - 1] = '\0';
    dict[dict_ptr].code_addr = code_addr;
    dict[dict_ptr].is_immediate = immediate;
    dict_ptr++;
}

/* Execute bytecode */
void execute() {
    running = 1;
    while (running && ip < CODE_SIZE) {
        unsigned char op = code[ip++];
        cell_t a, b, c;
        
        switch (op) {
            case OP_NOP:
                break;
                
            case OP_LIT:
                push(read_cell(ip));
                ip += sizeof(cell_t);
                break;
                
            case OP_ADD:
                b = pop();
                a = pop();
                push(a + b);
                break;
                
            case OP_SUB:
                b = pop();
                a = pop();
                push(a - b);
                break;
                
            case OP_MUL:
                b = pop();
                a = pop();
                push(a * b);
                break;
                
            case OP_DIV:
                b = pop();
                a = pop();
                if (b == 0) {
                    fprintf(stderr, "Division by zero!\n");
                    exit(1);
                }
                push(a / b);
                break;
                
            case OP_MOD:
                b = pop();
                a = pop();
                push(a % b);
                break;
                
            case OP_EQ:
                b = pop();
                a = pop();
                push(a == b ? -1 : 0);
                break;
                
            case OP_LT:
                b = pop();
                a = pop();
                push(a < b ? -1 : 0);
                break;
                
            case OP_GT:
                b = pop();
                a = pop();
                push(a > b ? -1 : 0);
                break;
                
            case OP_AND:
                b = pop();
                a = pop();
                push(a & b);
                break;
                
            case OP_OR:
                b = pop();
                a = pop();
                push(a | b);
                break;
                
            case OP_NOT:
                a = pop();
                push(~a);
                break;
                
            case OP_DUP:
                a = peek();
                push(a);
                break;
                
            case OP_DROP:
                pop();
                break;
                
            case OP_SWAP:
                b = pop();
                a = pop();
                push(b);
                push(a);
                break;
                
            case OP_OVER:
                a = stack[sp - 1];
                push(a);
                break;
                
            case OP_ROT:
                c = pop();
                b = pop();
                a = pop();
                push(b);
                push(c);
                push(a);
                break;
                
            case OP_EMIT:
                printf("%c", (char)pop());
                fflush(stdout);
                break;
                
            case OP_DOT:
                printf("%ld ", pop());
                fflush(stdout);
                break;
                
            case OP_CR:
                printf("\n");
                break;
                
            case OP_CALL:
                rpush(ip + sizeof(cell_t));
                ip = read_cell(ip);
                break;
                
            case OP_RET:
                if (rsp < 0) {
                    running = 0;
                } else {
                    ip = rpop();
                }
                break;
                
            case OP_JMP:
                ip = read_cell(ip);
                break;
                
            case OP_JZ:
                a = read_cell(ip);
                ip += sizeof(cell_t);
                if (pop() == 0) {
                    ip = a;
                }
                break;
                
            case OP_HALT:
                running = 0;
                break;
                
            default:
                fprintf(stderr, "Unknown opcode: %d at %d\n", op, ip - 1);
                running = 0;
                break;
        }
    }
}

/* Parse and compile/execute a word */
void parse_word(char *word) {
    /* Check if it's a number */
    char *endptr;
    long val = strtol(word, &endptr, 0);
    if (*endptr == '\0') {
        if (compiling) {
            emit_cell(val);
        } else {
            push(val);
        }
        return;
    }
    
    /* Look up in dictionary */
    int idx = find_word(word);
    if (idx >= 0) {
        if (compiling && !dict[idx].is_immediate) {
            emit_byte(OP_CALL);
            for (size_t i = 0; i < sizeof(cell_t); i++) {
                emit_byte((dict[idx].code_addr >> (i * 8)) & 0xFF);
            }
        } else {
            int saved_ip = ip;
            ip = dict[idx].code_addr;
            execute();
            ip = saved_ip;
        }
        return;
    }
    
    fprintf(stderr, "Unknown word: %s\n", word);
}

/* Initialize built-in words */
void init_builtins() {
    /* Arithmetic */
    add_word("+", -OP_ADD, 0);
    add_word("-", -OP_SUB, 0);
    add_word("*", -OP_MUL, 0);
    add_word("/", -OP_DIV, 0);
    add_word("MOD", -OP_MOD, 0);
    
    /* Comparison */
    add_word("=", -OP_EQ, 0);
    add_word("<", -OP_LT, 0);
    add_word(">", -OP_GT, 0);
    
    /* Logic */
    add_word("AND", -OP_AND, 0);
    add_word("OR", -OP_OR, 0);
    add_word("NOT", -OP_NOT, 0);
    
    /* Stack */
    add_word("DUP", -OP_DUP, 0);
    add_word("DROP", -OP_DROP, 0);
    add_word("SWAP", -OP_SWAP, 0);
    add_word("OVER", -OP_OVER, 0);
    add_word("ROT", -OP_ROT, 0);
    
    /* I/O */
    add_word("EMIT", -OP_EMIT, 0);
    add_word(".", -OP_DOT, 0);
    add_word("CR", -OP_CR, 0);
    
    /* Control */
    add_word("BYE", -OP_HALT, 0);
}

/* Get next word from input */
int get_word(char *word, int max_len) {
    while (input_buffer[input_pos] && isspace(input_buffer[input_pos])) {
        input_pos++;
    }
    
    if (!input_buffer[input_pos]) {
        return 0;
    }
    
    int i = 0;
    while (input_buffer[input_pos] && !isspace(input_buffer[input_pos]) && i < max_len - 1) {
        word[i++] = input_buffer[input_pos++];
    }
    word[i] = '\0';
    
    return i > 0;
}

/* Process input line */
void process_line(char *line) {
    strncpy(input_buffer, line, sizeof(input_buffer) - 1);
    input_buffer[sizeof(input_buffer) - 1] = '\0';
    input_pos = 0;
    
    char word[NAME_LEN];
    while (get_word(word, NAME_LEN)) {
        /* Handle comments - skip rest of line if word starts with \ */
        if (word[0] == '\\') {
            break;
        }
        parse_word(word);
    }
}

/* REPL */
void repl() {
    char line[1024];
    
    printf("SL - Simple Lang v1.0\n");
    printf("Type 'BYE' to exit\n\n");
    
    while (1) {
        printf("> ");
        fflush(stdout);
        
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        
        process_line(line);
        
        if (!running) {
            break;
        }
    }
}

/* Load and execute a file */
void load_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        exit(1);
    }
    
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        process_line(line);
    }
    
    fclose(f);
}

int main(int argc, char *argv[]) {
    init_builtins();
    
    /* Set up builtin code addresses */
    for (int i = 0; i < dict_ptr; i++) {
        if (dict[i].code_addr < 0) {
            /* Builtin - create code for it */
            int opcode = -dict[i].code_addr;
            dict[i].code_addr = here;
            emit_byte(opcode);
            emit_byte(OP_RET);
        }
    }
    
    if (argc > 1) {
        load_file(argv[1]);
    } else {
        repl();
    }
    
    return 0;
}
