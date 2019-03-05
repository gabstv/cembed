#include <stdio.h>
#include <strings.h>
#include <stdlib.h>

typedef struct {
    char **files;
    int files_n;
    int files_cap;
    char * output;
} args_t;

void free_args(args_t * v) {
    free(v->files);
    free(v);
}

void print_usage(const char * progname) {
    printf("usage: %s $FILE\t| put contents of $FILE into $FILE.h\n", progname);
    printf("       %s -o embedded.h $FILE\t| put contents of $FILE into embedded.h\n", progname);
    printf("       %s -o embedded.h $FILE1 $FILE2\t| put contents of $FILE1 and $FILE2 into embedded.h\n", progname);
}

void help(const char * progname) {
    printf("cembed is a tool to embed file contents into a c header file.\n\n");
    print_usage(progname);
}

char * safe_var_name(const char * inputn, const char * suffix) {
    char * outp = (char *)malloc(strlen(inputn)+sizeof(char)*5);
    for(int i = 0; inputn[i] != '\0'; i++){
        if(inputn[i] >= '0' && inputn[i] <= '9'){
            outp[i] = inputn[i];
            continue;
        }
        if(inputn[i] == 0){
            outp[i] = 0;
            continue;
        }
        if(inputn[i] >= 'A' && inputn[i] <= 'Z'){
            outp[i] = inputn[i];
            continue;
        }
        if(inputn[i] >= 'a' && inputn[i] <= 'z'){
            outp[i] = inputn[i];
            continue;
        }
        outp[i] = '_';
    }
    if(suffix != NULL){
        strcat(outp, suffix);
    }
    return outp;
}

char parse(int argc, char *argv[], args_t * result) {
    result->files = NULL;
    result->output = NULL;
    if(argc < 2){
        print_usage(argv[0]);
        return 1;
    }
    if(strcmp(argv[1], "--help") == 0){
        help(argv[0]);
        return 2;
    }
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0){
            // needs next arg
            if(argc<=i+1){
                // error
                printf("invalid arg after -o\n");
                return 1;
            }
            result->output = argv[i+1];
            i++;
            continue;
        }
        //if(strcmp(argv[i], "--t"))
        // no other match, it's an input file then
        if(result->files == NULL){
            result->files = (char **)malloc(10);
            result->files_cap = 10;
            result->files_n = 0;
        }
        result->files[result->files_n] = argv[i];
        result->files_n++;
    };
    if(result->files_n == 0){
        printf("no input files specified\n");
        return 1;
    }
    // set a file output if none is specified
    if(result->output == NULL){
        result->output = safe_var_name(result->files[0], ".h");
    }
    return 0;
}

int main(int argc, char *argv[]) {
    args_t * pargs = (args_t *)malloc(sizeof(args_t));
    int z = parse(argc, argv, pargs);
    if(z == 2){
        // is help
        free(pargs->files);
        free(pargs);
        return 0;
    }
    if(z == 1){
        free(pargs->files);
        free(pargs);
        return 1;
    }
    // build output file
    printf("build output\n");
    FILE *f;
    f = fopen(pargs->output, "w");
    printf("will write file '%s'\n", pargs->output);
    fprintf(f, "// AUTO GENERATED FILE\n");
    printf("will write l2\n");
    fprintf(f, "\n");
    fprintf(f, "#pragma once\n\n");
    // loop through all
    int i, j;
    char buffer[1024];
    for(i=0;i<pargs->files_n;i++) {
        FILE * rf = fopen(pargs->files[i], "r");
        if(rf == NULL){
            fclose(f);
            remove(pargs->output);
            printf("could not open %s\n", pargs->files[i]);
            free_args(pargs);
            return 1;
        }
        fprintf(f, "static const char %s[] = {", safe_var_name(pargs->files[i], NULL));
        int first = 1;
        while(1){
            int nr = fread(buffer, 1, 1024, rf);
            if(nr>0){
                // write into the new header file
                for(j=0;j<nr;j++){
                    if(first){
                        first = 0;
                    }else{
                        fprintf(f,",");
                    }
                    fprintf(f,"%#x", buffer[j]);
                }
            }
            if(nr < 1024){
                if(nr==0){
                    printf("%d\n", feof(rf));
                }
                break;
            }
        }
        fprintf(f,"};\n");
        fclose(rf);
    }
    fclose(f);
    printf("will free file\n");
    f = NULL;

    return 0;
}