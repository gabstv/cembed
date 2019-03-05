#include <stdio.h>
#include <strings.h>
#include <stdlib.h>

typedef struct {
    char **files;
    int files_n;
    int files_cap;
    char * output;
} parsedargs;

void freepargs(parsedargs * v) {
    free(v->files);
    free(v);
}

void help() {
    //TODO: print help
    printf("TODO: print help\n");
}

char * normalizefn(const char * inputn, const char * suffix) {
    printf("normalizefn\n");
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
    strcat(outp, suffix);
    printf("normalizefn 2\n");
    return outp;
}

char parse(int argc, char *argv[], parsedargs * result) {
    result->files = NULL;
    result->output = NULL;
    if(argc < 2){
        printf("usage %s file\n", argv[0]);
        return 1;
    }
    if(strcmp(argv[1], "--help") == 0){
        help();
        return 2;
    }
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-o") == 0){
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
        result->output = normalizefn(result->files[0], ".h");
    }
    return 0;
}

int main(int argc, char *argv[]) {
    parsedargs * pargs = (parsedargs *)malloc(sizeof(parsedargs));
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
            freepargs(pargs);
            return 1;
        }
        fprintf(f, "static char %s[] = {", normalizefn(pargs->files[i], "_z"));
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