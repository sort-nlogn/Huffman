#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

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

void restore_tree(RestoredNode nodes[], FILE *archive, char alphabet[], int eof_pos){
    int end = strlen(alphabet) + 1; int read_cnt = 0;
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
                    // printf("curr: %d\n", curr->symbol);
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

        if(char_pos == 8){
            c = fgetc(archive); char_pos = 0;
        }

        if(!curr->left){ // Leaf
            if(curr->symbol == 128){ // EOF
                done = 1;
            }else{
                fputc(curr->symbol, decoded);
                curr = root;
            }
        }
    }
}

//alphabet_size -> eof_pos -> alphabet -> traverse_string -> encoded_file
int main(){
    FILE *archive = fopen("out.bin", "rb");
    FILE *decoded = fopen("decoded.txt", "wb");
    int alphabet_size = fgetc(archive);
    int eof_pos = fgetc(archive);
    printf("Alphabet size: %d EOF pos: %d\n", alphabet_size, eof_pos);
    char alphabet[257] = {'\0'};
    char c;
    for(int i = 0; i < alphabet_size; i++){
        c = fgetc(archive);
        alphabet[i] = c;
    }

    // for(int i = 0; i < alphabet_size; i++){
    //     printf("%d ", alphabet[i]);
    // }
    RestoredNode nodes[257 + 256] = {{.left = NULL,
                                    .right = NULL,
                                    .parent = NULL,
                                    .symbol = '\0'}};
    restore_tree(nodes, archive, alphabet, eof_pos);
    dfs(nodes);
    decode_file(nodes, archive, decoded);
    return 0;
}