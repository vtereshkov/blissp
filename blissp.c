// BLISSP interpreter
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LEN  8
#define VARS 64

typedef struct {char name[LEN]; int addr, size;} Var;
typedef struct {int mem[VARS]; Var vars[VARS]; int num;} Vars;

static void check(bool ok, const char *msg, char ch) {
    if (!ok) {
        fprintf(stderr, "error: %s %c\n", msg, ch);
        exit(1);
    }
}

static void skipspace(const char **src) {
    while (isspace(**src)) (*src)++;
}

static int getvar(Vars *vars, const char *name, int len) {
    if (len > LEN - 1) len = LEN - 1;

    for (int i = 0; i < vars->num; i++)
        if (memcmp(name, vars->vars[i].name, len) == 0 && vars->vars[i].name[len] == 0)
            return vars->vars[i].addr;

    check(vars->num < VARS, "out of memory", 0);
    Var *var = &vars->vars[vars->num];
    memcpy(var->name, name, len); 
    var->name[len] = 0;
    var->addr = (vars->num > 0) ? (vars->vars[vars->num - 1].addr + vars->vars[vars->num - 1].size) : 0;
    var->size = 1;
    check(var->addr + var->size < VARS, "out of memory", 0);
    return vars->vars[vars->num++].addr;
}

static int expr(const char **src, Vars *vars, bool skip);

static int term(const char **src, Vars *vars, bool skip) {
    int val = 0;
    skipspace(src);
    if (isalpha(**src)) {
        int len = 0;
        while (isalpha((*src)[len]))
            len++;
        val = skip ? -1 : getvar(vars, *src, len);
        (*src) += len;
    } else if (isdigit(**src)) {
        char *tail = NULL;
        val = strtol(*src, &tail, 10);
        *src = tail;
    } else if (**src == '(') {
        (*src)++;
        val = expr(src, vars, skip);
        skipspace(src);
        check(**src == ')', "expected ), found", **src);
        (*src)++;
    } else {
        check(false, "illegal term", **src);
    }
    skipspace(src);
    if (**src == '\'') {val = skip ? -1 : vars->mem[val]; (*src)++;}
    return val;
}

static int expr(const char **src, Vars *vars, bool skip)
{
    skipspace(src);
    int val = (isalnum(**src) || **src == '(') ? term(src, vars, skip) : 0;

    while (skipspace(src), **src && **src != ')') {
        char op = **src;
        const char *loop = NULL;
        switch (op) {
            case '?': (*src)++; term(src, vars, skip || !val); val = val ? -1 : 0; continue;
            case '@': (*src)++; loop = *src; do {*src = loop; val = term(src, vars, skip);} while (!skip && val); val = -1; continue; 
            default: break;           
        }

        (*src)++;
        const int rval = term(src, vars, skip);
        if (!skip) {
            switch (op) {
                case '+': val += rval; continue;
                case '-': val -= rval; continue;
                case '*': val *= rval; continue;
                case '/': check(rval != 0, "division by zero", 0); val /= rval; continue;
                case '%': check(rval != 0, "division by zero", 0); val %= rval; continue;
                case '&': val &= rval; continue;
                case '|': val |= rval; continue;
                case '!': val = ~rval; continue;
                case '=': val = (val == rval) ? -1 : 0; continue;
                case '#': val = (val != rval) ? -1 : 0; continue;
                case '>': val = (val >  rval) ? -1 : 0; continue;
                case '<': val = (val <  rval) ? -1 : 0; continue;
                case ':': vars->mem[val] = rval; val = -1; continue;
                case '~': check(vars->num > 0 && val == vars->vars[vars->num - 1].addr && val + rval < VARS, "cannot resize", 0); vars->vars[vars->num - 1].size = rval; val = -1; continue;
                case '$': printf("%d ", rval); val = -1; continue;  
                case ',': val = rval; continue;
                default: check(false, "illegal operator", op); break;
            }            
        }
    }
    return val;
}

static void eval(const char *src) {
    Vars vars = {0};
    expr(&src, &vars, false);
}

int main(int argc, char **argv)
{
    check(argc > 1, "no input file", 0);
    FILE *f = fopen(argv[1], "rb");
    check(f, "cannot open file", 0);
    char src[512] = {0};
    check(fread(src, 1, sizeof(src), f) > 0, "cannot read file", 0);
    fclose(f);

    eval(src);
    return 0;
}