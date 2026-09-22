#include <exprtree.h>
#include <translate.h>
#include <codegen.h>
#include <gsymboltable_utils.h>

FILE* targetFile;
FILE* inputFile;
extern FILE* yyin;
extern int yydebug;
extern int yyparse();

int main(int argc, char* argv[]) {
	yydebug = 0; // toggle for debugging
	int argStart = 1;

	// To print global symbol table 
	if(argc > argStart && argv[argStart][0] == '-' && argv[argStart][1] == 'g') {
		showGlobalSymbolTable = 1;
		argStart++;
	}
	if(argc - argStart != 2) {
		fprintf(stderr, "Usage: %s [-g] <input_file> <output_file>\n", argv[0]);
		exit(1);
	}
    char* intermediateFile = "output_temp.xsm";
	inputFile = fopen(argv[argStart], "r");
    targetFile = fopen(intermediateFile, "w");
	if(!inputFile) {
		fprintf(stderr, "Error: Unable to open input file %s\n", argv[1]);
		exit(1);
	}
    if (!targetFile) {
        fprintf(stderr, "Error: Unable to open target file\n");
        exit(1);
    }
	yyin = inputFile;
    if (yyparse() != 0) {
        fprintf(stderr, "Error: Compilation failed due to parse errors\n");
        exit(1);
    }
	fclose(inputFile);
    fclose(targetFile);
    translate(intermediateFile, argv[argStart + 1]);
    return 0;
}
