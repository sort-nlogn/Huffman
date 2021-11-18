#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define get_code_value(i) *(codes + 128 + (int)nodes[i].symbol)

typedef struct TmpNode{
    struct TmpNode *left, *right, *parent;
    int symbol;
    unsigned int freq;
}Node;

typedef struct{                
    char value[257];
    int length;
}Code;

int cmp(const void *n1, const void *n2){
    int freq1 = ((Node *)n1)->freq;
    int freq2 = ((Node *)n2)->freq;
    if(!freq1)
        return freq2;
    if(!freq2)
        return -freq1;
    return freq1 - freq2;  
}

int set_frequences(FILE *in, Node nodes[513]){
    char c;
    int alphabet_size = 0;
    nodes[128].freq = 1; nodes[128].symbol = 128; //pseudo-EOF
    while((c = fgetc(in)) != EOF){
        nodes[c + 128].freq++;
        if(nodes[c + 128].freq == 1){ // если первый раз встретили этот символ
            alphabet_size++;
            nodes[c + 128].symbol = c;
        }
    }
    qsort(nodes, 257, sizeof(Node), cmp); // первые alpahabet_size + 1(+EOF) нод отсортированы, всё остальное - нули; 
    return alphabet_size + 1; // +EOF
}

void node_init(Node *new_node, Node *left, Node *right){
    new_node->left = left;
    new_node->right = right;
    new_node->freq = left->freq + right->freq;
    new_node->symbol = '$';
    left->parent = new_node;
    right->parent = new_node;
}

int get_freq_sum(Node *arr1, Node *arr2, int i1, int i2, int l1, int l2){
    if(i1 == l1 || i2 == l2){
        return INT_MAX;
    }
    return (arr1 + i1)->freq + (arr2 + i2)->freq;
}

void build_tree(Node arr1[513], int alphabet_size){
    Node *arr2 = arr1 + 257;
    int i = 0, j = 0;
    for(int k = 0; k < alphabet_size - 1; k++){
        unsigned int s1 = get_freq_sum(arr1, arr1, i, i + 1, alphabet_size, alphabet_size);
        unsigned int s2 = get_freq_sum(arr2, arr2, j, j + 1, k, k);
        unsigned int s3 = get_freq_sum(arr1, arr2, i, j, alphabet_size, k);
        if(s1 <= s2 && s1 <= s3){
            node_init(arr2 + k, arr1 + i++, arr1 + i++);
        }else if(s2 <= s1 && s2 <= s3){
            node_init(arr2 + k, arr2 + j++, arr2 + j++);
        }else{
            node_init(arr2 + k, arr2 + j++, arr1 + i++);
        }
    }
}

void write_traverse_string(Node *root){
    if(root->left){
        write_traverse_string(root->left);
        write_traverse_string(root->right);
    }else{
        printf("%c\n", root->symbol);
    }
}

void get_code(Node *leaf, Node *root, Code *code){
    int len = 0;
    do{
        if(leaf->parent->left == leaf){
            code->value[len++] = '0';
        }else{
            code->value[len++] = '1';
        }
        leaf = leaf->parent;
    }while (leaf != root);
    code->length = len;
}

void get_codes(Code codes[257], Node nodes[513], const int alphabet_size){
    Node *root = nodes + 257 + alphabet_size - 2;
    for(int i = 0; i < alphabet_size; i++){
        get_code(nodes + i, root, codes + (int)nodes[i].symbol + 128);
    }
    // get_code(nodes + 256, root, codes + 256);
}


int main(){
    FILE *in = fopen("in.txt", "rb");
    Node nodes[513] = {{.parent = NULL},
                        {.left = NULL},
                        {.right = NULL},
                        {.freq = 0},
                        {.symbol = 0}};
    int alphabet_size = set_frequences(in, nodes);

    fclose(in);

    Code codes[257] = {{.value = {'\0'}},
                        {.length = 0}
                    };

    // for(int i = 0; i < alphabet_size; i++){
    //     printf("Symbol: %d Freq: %d\n", nodes[i].symbol, nodes[i].freq);
    // }

    build_tree(nodes, alphabet_size);

    // for(int i = 0; i < alphabet_size - 1;  i++){
    //     printf("Sum of frequences: %d\n", nodes[257 + i].freq);
    // }

    get_codes(codes, nodes, alphabet_size);
    for(int i = 0; i < alphabet_size; i++){
        if(nodes[i].symbol < 128){
            printf("%c code: %s\n", nodes[i].symbol, get_code_value(i));
        }else{
            printf("EOF code: %s\n", get_code_value(i));
        }
    }

    return 0;
}