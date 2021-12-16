/*
   ===================      =====================     
   |ЖЁСТКО РАСПАКОВАЛ|      |ПОЛНЫЙ АРХИВ ФАЙЛОВ|
   ===================      =====================
*/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>

typedef struct RTnode{
    struct RTnode *left, *right, *parent;
    int symbol; 
}RestoredNode;

void link_nodes(RestoredNode *parent, RestoredNode *child, int is_left){
    if(is_left){
        parent->left = child;
    }else{
        parent->right = child;
    }
    child->parent = parent;
}

void dfs(RestoredNode *root){
    if(root->left){
        dfs(root->left);
        dfs(root->right);
    }else{
        printf("%d ", root->symbol);
    }
}

void restore_tree(RestoredNode nodes[], FILE *archive, char alphabet[], short eof_pos, int alphabet_size){
    int end = alphabet_size; int read_cnt = 0;
    int abc_pos = 0, head = 0; int read_eof = 0;
    RestoredNode *curr = nodes;
    char curr_bit = -1, prev_bit = -1, char_pos = 0;
    char c = fgetc(archive);
    while(read_cnt != end){
        curr_bit = (c >> char_pos++) & 1;
        if(curr_bit){ // D
            link_nodes(curr, nodes + ++head, prev_bit);
            curr = prev_bit? curr->left: curr->right;
        }else{ // U
            if(prev_bit){ // leaf
                if(read_cnt++ == eof_pos){
                    curr->symbol = 128;
                }else{
                    curr->symbol = alphabet[abc_pos++];
                }
            }
            curr = curr->parent;
        }
        prev_bit = curr_bit;
        if(char_pos == 8 && read_cnt != end){
            c = fgetc(archive); char_pos = 0;
        }
    }
}

void decode_file(RestoredNode *root, FILE *archive, FILE *decoded){
    char c = fgetc(archive);
    char curr_bit = 0, char_pos = 0;
    int done = 0;
    RestoredNode *curr = root;
    while(!done){
        curr_bit = (c >> char_pos++) & 1;
        curr = curr_bit? curr->right: curr->left;

        if(!curr->left){ // Leaf
            if(curr->symbol == 128){ // EOF
                done = 1;
            }else{
                fputc(curr->symbol, decoded);
                curr = root;
            }
        }

        if(char_pos == 8 && !done){
            c = fgetc(archive); char_pos = 0;
        }

    }
}

void unpack_file(FILE *archive, FILE *f){
    short alphabet_size, eof_pos;
    fread(&alphabet_size, sizeof(short), 1, archive);
    fread(&eof_pos, sizeof(short), 1, archive);
    char alphabet[257] = {'\0'}; char c;
    for(int i = 0; i < alphabet_size - 1; i++){
        c = fgetc(archive);
        alphabet[i] = c;
    } 
    RestoredNode nodes[257 + 256] = {{.left = NULL,
                                    .right = NULL,
                                    .parent = NULL,
                                    .symbol = '\0'}};  
    restore_tree(nodes, archive, alphabet, eof_pos, alphabet_size);
    decode_file(nodes, archive, f);
}

void unpack_archive(FILE *archive, const char *base){
    while(!feof(archive)){
        char type = fgetc(archive);
        char buf[1024] = {'\0'};
        fgets(buf, sizeof(buf), archive); buf[strlen(buf) - 1] = '\0';
        char cmnd[1024] = {'\0'};
        snprintf(cmnd, sizeof(cmnd), "mkdir %s%.*s", base, strchr(buf, ' ')-buf, buf);

        if(!opendir(strchr(cmnd, ' ') + 1)){
            system(cmnd);
        }
        if(type == '0' || type == '1'){// regular file || empty file
            char fp[1024] = {'\0'};
            snprintf(fp, sizeof(fp), "%s%s", strchr(cmnd, ' ')+1, strchr(buf, ' ')+1);
            printf("%s\n", fp);
            FILE *f = fopen(fp, "wb");
            if(type == '0'){
                unpack_file(archive, f);
            }
            fclose(f);
        }
    }
}

//alphabet_size -> eof_pos -> alphabet -> traverse_string -> encoded_file
int main(int argc, char *argv[]){

    if(argc < 3){
        printf("Too few command arguments");
        exit(0);
    }


    while(argv[1][strlen(argv[1])-1]=='\\'){argv[1][strlen(argv[1])-1] = '\0';}
    FILE *archive = fopen(argv[1], "rb");
    if(!archive){
        printf("Incorrect archive path");
        exit(0);
    }

    if(!opendir(argv[2])){
        printf("Incorrect save path");
        exit(0);
    }
    char cmnd[1024] = {'\0'};
    snprintf(cmnd, sizeof(cmnd), "mkdir %s\\%.*s", argv[2], 
            strrchr(argv[1], '.') - strrchr(argv[1], '\\')-1,
            strrchr(argv[1], '\\') + 1);

    printf("%s\n", cmnd);
            
    if(opendir(strrchr(cmnd, ' ') + 1)){
        printf("%.*s already exist in %s", strrchr(argv[1], '.')-strrchr(argv[1], '\\')-1,
                        strrchr(argv[1], '\\')+1,
                        argv[2]);
        exit(0);
    }
    if(system(cmnd)){
        printf("Incorrect save path");
        exit(0);
    }
    unpack_archive(archive, strrchr(cmnd, ' ') + 1);
    return 0;
}