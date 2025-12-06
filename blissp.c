// BLISSP interpreter
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LEN  8
#define VARS 64

typedef struct {char name[LEN]; int val;} Var;
typedef struct {Var vars[VARS]; int num;} Vars;

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
        if (memcmp(name, vars->vars[i].name, len) == 0)
            return i;

    check(vars->num < VARS, "too many variables", 0);
    Var *var = &vars->vars[vars->num++];
    memcpy(var->name, name, len);
    var->name[len] = 0;
    return vars->num - 1;
}

static int expr(const char **src, Vars *vars, bool skip);

static int term(const char **src, Vars *vars, bool skip) {
    skipspace(src);
    if (isalpha(**src)) {
        int len = 0;
        while (isalpha((*src)[len]))
            len++;
        int var = skip ? -1 : getvar(vars, *src, len);
        (*src) += len;
        skipspace(src);
        if (**src == '\'') {var = skip ? -1 : vars->vars[var].val; (*src)++;}
        return var;
    } else if (isdigit(**src)) {
        char *tail = NULL;
        const int num = strtol(*src, &tail, 10);
        *src = tail;
        return num;
    } else if (**src == '(') {
        (*src)++;
        const int val = expr(src, vars, skip);
        skipspace(src);
        check(**src == ')', "expected", ')');
        (*src)++;
        return val;
    } else {
        check(false, "illegal term", **src);
        return 0;
    }
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
                case ':': vars->vars[val].val = rval; val = -1; continue;
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
    check(argc >= 0, "no input file", 0);
    FILE *f = fopen(argv[1], "rb");
    check(f, "cannot open file", 0);

    char src[512] = {0};
    check(fread(src, 1, sizeof(src), f) > 0, "cannot read file", 0);
    fclose(f);

    eval(src);
    return 0;
}
