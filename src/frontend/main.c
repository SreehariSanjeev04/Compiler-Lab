#include <exprtree.h>
#include <translate.h>
#include <codegen.h>
extern FILE* inputFile;
extern FILE* targetFile;
extern FILE* yyin;
extern tnode* root;
extern int yyparse();

int main(int argc, char* argv[]) {
	if(argc != 3) {
		fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
		exit(1);
	}
    char* intermediateFile = "output_temp.xsm";
	inputFile = fopen(argv[1], "r");
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
    yyparse();
	generateCode(root, targetFile);
	fclose(inputFile);
    fclose(targetFile);
    translate(intermediateFile, argv[2]);
    return 0;
}