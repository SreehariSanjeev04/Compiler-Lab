#include <binding.h>
#include <gsymboltable.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int returnStaticBindAddress(const char *varname)
{
    if (!varname || strlen(varname) <= 0)
    {
        fprintf(stderr, "Error: Variable name is invalid. Only single-letter variable names are allowed.\n");
        exit(1);
    }
    struct Gsymbol *symbol = GLookup(varname);
    if (!symbol)
    {
        fprintf(stderr, "Error: Variable '%s' not defined\n", varname);
        exit(1);
    }
    return symbol->binding;
}
