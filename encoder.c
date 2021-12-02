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
    while(!feof(in)){
        c = fgetc(in);
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

unsigned int get_freq_sum(Node *arr1, Node *arr2, int i1, int i2, int l1, int l2){
    if(i1 == l1 || i2 == l2){
        return UINT_MAX;
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

void encode_file(FILE *in, FILE *out, Code codes[257]){
    char c = fgetc(in), curr_byte;
    int byte_pos = 0, code_pos = 0;
    Code curr_code = codes[128 + c];
    while(!feof(in)){
        curr_byte |= (int)(curr_code.value[code_pos++] == '0' ? 0: 1) << byte_pos++;
        if(byte_pos == 8){
            fputc(curr_byte, out);
            printf("Curr byte: %d\n", curr_byte);
            curr_byte = 0; byte_pos = 0;
        }
        if(code_pos == curr_code.length){
            c = fgetc(in);
            curr_code = codes[128 + c]; code_pos = 0;
        }
    }
    //write pseudo-EOF
    Code eof_code = codes[256]; code_pos = 0;
    while(code_pos != eof_code.length){
        if(byte_pos == 8){
            fputc(curr_byte, out);
            printf("Curr byte: %d\n", curr_byte);
            curr_byte = 0; byte_pos = 0;
        }
        curr_byte |= (eof_code.value[code_pos++] == '0'? 0: 1) << byte_pos++;
    }
}

void dump_traverse_string(FILE *out, Node *root, int alphabet_size){ // 1 - D, 0 - U
    Node **st = malloc((2 * alphabet_size - 1) * sizeof(Node *));
    size_t head = 0, pos = 0;
    char byte = 0;
    Node *curr = root; int done = 0;
    while(!done){        
        if(curr){
            if(curr != root){
                byte |= 1 << pos++;
           }
           st[head++] = curr;
           curr = curr->left;
       }else{
            pos++;
            if(st[head - 1]->parent->right == st[head - 1]){
                pos++;
            }
            if(pos >= 8){
                pos %= 8; fputc(byte, out); byte = 0;
            } 
            if((head -= 2) == -1){
                done = 1;
            }else{
                curr = st[head]->right;
            }
       }   
    }
    if(byte){
        fputc(byte, out);
    }
}

int main(){
    FILE *in = fopen("in.txt", "rb");
    FILE *out = fopen("out.bin", "wb");
    Node nodes[513] = {{.parent = NULL},
                        {.left = NULL},
                        {.right = NULL},
                        {.freq = 0},
                        {.symbol = 0}};
    int alphabet_size = set_frequences(in, nodes);

    Code codes[257] = {{.value = {'\0'}},
                        {.length = 0}
                    };

    build_tree(nodes, alphabet_size);

    get_codes(codes, nodes, alphabet_size);
    for(int i = 0; i < alphabet_size; i++){
        if(nodes[i].symbol < 128){
            printf("%d code: %s length: %d\n", nodes[i].symbol, get_code_value(i), codes[128 + nodes[i].symbol].length);
        }else{
            printf("EOF code: %s length: %d\n", get_code_value(i), codes[128 + nodes[i].symbol].length);
        }
    }
    fseek(in, 0, SEEK_SET);
    encode_file(in, out, codes);

    Node *root = nodes + 257 + alphabet_size - 2;
    dump_traverse_string(out, root, alphabet_size);
    return 0;
}