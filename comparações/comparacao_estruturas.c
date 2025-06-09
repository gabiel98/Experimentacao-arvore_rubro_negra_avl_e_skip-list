#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#ifdef _WIN32
#include <windows.h>

int clock_gettime(int type, struct timespec *spec) {
    static LARGE_INTEGER freq;
    static BOOL use_qpc;
    static int initialized = 0;
    
    if (!initialized) {
        printf("[TIMER] Inicializando contador de performance...\n");
        use_qpc = QueryPerformanceFrequency(&freq);
        initialized = 1;
    }
    
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    spec->tv_sec = count.QuadPart / freq.QuadPart;
    spec->tv_nsec = (long)((count.QuadPart % freq.QuadPart) * 1000000000LL / freq.QuadPart);
    return 0;
}

#define CLOCK_MONOTONIC 1
#endif

// ====================== Funções de temporização ==========================
static inline int64_t diff_nsec(struct timespec *start, struct timespec *end) {
    return (int64_t)(end->tv_sec - start->tv_sec) * 1000000000LL 
         + (int64_t)(end->tv_nsec - start->tv_nsec);
}

// ====================== Geração e embaralhamento de vetores ==========================
void shuffle_array(int *arr, size_t n) {
    printf("[SHUFFLE] Embaralhando array de tamanho %zu...\n", n);
    for (size_t i = n - 1; i > 0; --i) {
        size_t j = (size_t)(rand() % (i + 1));
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

int* gera_vetor(int n) {
    printf("[GERA_VETOR] Criando vetor de tamanho %d...\n", n);
    int *v = malloc(n * sizeof(int));
    if (!v) {
        fprintf(stderr, "Falha ao alocar vetor de tamanho %d\n", n);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++) {
        v[i] = i + 1;
    }
    shuffle_array(v, n);
    return v;
}

// ====================== Árvore AVL ==========================
typedef struct AVLNode {
    int key;
    struct AVLNode *left, *right;
    int height;
} AVLNode;

static inline int avl_height(AVLNode *N) {
    return N ? N->height : 0;
}

static inline int avl_getBalance(AVLNode *N) {
    if (!N) return 0;
    return avl_height(N->left) - avl_height(N->right);
}

AVLNode* avl_newNode(int key) {
    printf("[AVL] Criando novo nó com chave %d\n", key);
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    node->key = key;
    node->left = node->right = NULL;
    node->height = 1;
    return node;
}

AVLNode* avl_rightRotate(AVLNode *y) {
    printf("[AVL] Rotação direita no nó %d\n", y->key);
    AVLNode *x = y->left;
    AVLNode *T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = 1 + ((avl_height(y->left) > avl_height(y->right)) ? avl_height(y->left) : avl_height(y->right));
    x->height = 1 + ((avl_height(x->left) > avl_height(x->right)) ? avl_height(x->left) : avl_height(x->right));
    return x;
}

AVLNode* avl_leftRotate(AVLNode *x) {
    printf("[AVL] Rotação esquerda no nó %d\n", x->key);
    AVLNode *y = x->right;
    AVLNode *T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = 1 + ((avl_height(x->left) > avl_height(x->right)) ? avl_height(x->left) : avl_height(x->right));
    y->height = 1 + ((avl_height(y->left) > avl_height(y->right)) ? avl_height(y->left) : avl_height(y->right));
    return y;
}

AVLNode* avl_insert(AVLNode* node, int key) {
    if (!node) return avl_newNode(key);
    
    printf("[AVL] Inserindo chave %d (nó atual: %d)\n", key, node->key);
    
    if (key < node->key)
        node->left = avl_insert(node->left, key);
    else if (key > node->key)
        node->right = avl_insert(node->right, key);
    else
        return node;
        
    node->height = 1 + ((avl_height(node->left) > avl_height(node->right)) ? avl_height(node->left) : avl_height(node->right));
    int balance = avl_getBalance(node);
    
    // LL
    if (balance > 1 && key < node->left->key) {
        printf("[AVL] Caso LL na chave %d\n", key);
        return avl_rightRotate(node);
    }
    // RR
    if (balance < -1 && key > node->right->key) {
        printf("[AVL] Caso RR na chave %d\n", key);
        return avl_leftRotate(node);
    }
    // LR
    if (balance > 1 && key > node->left->key) {
        printf("[AVL] Caso LR na chave %d\n", key);
        node->left = avl_leftRotate(node->left);
        return avl_rightRotate(node);
    }
    // RL
    if (balance < -1 && key < node->right->key) {
        printf("[AVL] Caso RL na chave %d\n", key);
        node->right = avl_rightRotate(node->right);
        return avl_leftRotate(node);
    }
    return node;
}

// Função auxiliar para balanceamento
static AVLNode* avl_recalc_and_balance(AVLNode* node) {
    if (!node) return NULL;
    
    printf("[AVL] Recalculando balanceamento para nó %d\n", node->key);
    
    node->height = 1 + ((avl_height(node->left) > avl_height(node->right)) ? avl_height(node->left) : avl_height(node->right));
    int balance = avl_getBalance(node);
    
    // LL
    if (balance > 1 && avl_getBalance(node->left) >= 0) {
        printf("[AVL] Balanceamento LL para nó %d\n", node->key);
        return avl_rightRotate(node);
    }
    // LR
    if (balance > 1 && avl_getBalance(node->left) < 0) {
        printf("[AVL] Balanceamento LR para nó %d\n", node->key);
        node->left = avl_leftRotate(node->left);
        return avl_rightRotate(node);
    }
    // RR
    if (balance < -1 && avl_getBalance(node->right) <= 0) {
        printf("[AVL] Balanceamento RR para nó %d\n", node->key);
        return avl_leftRotate(node);
    }
    // RL
    if (balance < -1 && avl_getBalance(node->right) > 0) {
        printf("[AVL] Balanceamento RL para nó %d\n", node->key);
        node->right = avl_rightRotate(node->right);
        return avl_leftRotate(node);
    }
    return node;
}

// Função auxiliar para balanceamento completo
static void avl_full_balance(AVLNode** pnode) {
    if (!(*pnode)) return;
    printf("[AVL] Balanceamento completo iniciado na subárvore de %d\n", (*pnode)->key);
    avl_full_balance(&((*pnode)->left));
    avl_full_balance(&((*pnode)->right));
    *pnode = avl_recalc_and_balance(*pnode);
}

AVLNode* avl_minValueNode(AVLNode* node) {
    printf("[AVL] Buscando valor mínimo na subárvore de %d\n", node->key);
    AVLNode* current = node;
    while (current->left)
        current = current->left;
    return current;
}

typedef struct {
    AVLNode *node; 
    int64_t t_busca_remocao; 
    int64_t t_balance; 
} AVLDelResult;

AVLDelResult avl_deleteNode_internal(AVLNode* root, int key) {
    struct timespec ts1, ts2, ts3;
    AVLDelResult result = { NULL, 0, 0 };
    
    printf("[AVL] Iniciando remoção da chave %d\n", key);
    
    // 1) Busca o nó
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    AVLNode* parent = root;
    AVLNode* curr = root;
    AVLNode* to_delete = NULL;
    AVLNode* parent_of_del = NULL;
    
    while (curr) {
        printf("[AVL] Buscando chave %d (atual: %d)\n", key, curr ? curr->key : -1);
        if (key < curr->key) {
            parent = curr;
            curr = curr->left;
        } else if (key > curr->key) {
            parent = curr;
            curr = curr->right;
        } else {
            to_delete = curr;
            parent_of_del = parent;
            break;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &ts2);
    result.t_busca_remocao = diff_nsec(&ts1, &ts2);
    
    if (!to_delete) {
        printf("[AVL] Chave %d não encontrada\n", key);
        result.node = root;
        return result;
    }
    
    printf("[AVL] Nó %d encontrado para remoção\n", to_delete->key);
    
    // 2) Remove o nó
    if (to_delete->left && to_delete->right) {
        printf("[AVL] Nó com dois filhos, buscando sucessor\n");
        AVLNode* succ = avl_minValueNode(to_delete->right);
        int succ_key = succ->key;
        printf("[AVL] Sucessor encontrado: %d\n", succ_key);
        AVLDelResult sub = avl_deleteNode_internal(root, succ_key);
        result.t_busca_remocao += sub.t_busca_remocao;
        root->key = succ_key;
        result.node = root;
        result.t_balance = sub.t_balance;
        return result;
    }
    
    AVLNode* child = to_delete->left ? to_delete->left : to_delete->right;
    if (!parent_of_del) {
        printf("[AVL] Removendo raiz da árvore\n");
        root = child;
    } else if (parent_of_del->left == to_delete) {
        printf("[AVL] Removendo filho esquerdo de %d\n", parent_of_del->key);
        parent_of_del->left = child;
    } else {
        printf("[AVL] Removendo filho direito de %d\n", parent_of_del->key);
        parent_of_del->right = child;
    }
    printf("[AVL] Liberando nó %d\n", to_delete->key);
    free(to_delete);
    
    // 3) Balanceamento
    printf("[AVL] Iniciando balanceamento pós-remoção\n");
    clock_gettime(CLOCK_MONOTONIC, &ts2);
    avl_full_balance(&root);
    clock_gettime(CLOCK_MONOTONIC, &ts3);
    
    result.t_balance = diff_nsec(&ts2, &ts3);
    result.node = root;
    printf("[AVL] Remoção completa da chave %d\n", key);
    return result;
}

AVLNode* avl_deleteNode(AVLNode* root, int key, int64_t *out_busca, int64_t *out_bal) {
    printf("[AVL] Chamada para deletar chave %d\n", key);
    AVLDelResult res = avl_deleteNode_internal(root, key);
    *out_busca = res.t_busca_remocao;
    *out_bal = res.t_balance;
    return res.node;
}

void avl_free(AVLNode* node) {
    if (!node) return;
    printf("[AVL] Liberando nó %d\n", node->key);
    avl_free(node->left);
    avl_free(node->right);
    free(node);
}

// ====================== Árvore Rubro-Negra ==========================
typedef enum { RED, BLACK } Color;

typedef struct RBNode {
    int key;
    Color color;
    struct RBNode *left, *right, *parent;
} RBNode;

typedef struct {
    RBNode *root;
} RBTree;

RBNode* rb_newNode(int key) {
    printf("[RB] Criando novo nó com chave %d\n", key);
    RBNode* node = (RBNode*)malloc(sizeof(RBNode));
    node->key = key;
    node->color = RED;
    node->left = node->right = node->parent = NULL;
    return node;
}

void rb_leftRotate(RBTree *tree, RBNode *x) {
    printf("[RB] Rotação esquerda no nó %d\n", x->key);
    RBNode *y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) tree->root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
}

void rb_rightRotate(RBTree *tree, RBNode *y) {
    printf("[RB] Rotação direita no nó %d\n", y->key);
    RBNode *x = y->left;
    y->left = x->right;
    if (x->right) x->right->parent = y;
    x->parent = y->parent;
    if (!y->parent) tree->root = x;
    else if (y == y->parent->left) y->parent->left = x;
    else y->parent->right = x;
    x->right = y;
    y->parent = x;
}

void rb_insertFixup(RBTree *tree, RBNode *z) {
    printf("[RB] Iniciando fixup para inserção da chave %d\n", z->key);
    while (z->parent && z->parent->color == RED) {
        RBNode *g = z->parent->parent;
        if (!g) break;
        if (z->parent == g->left) {
            RBNode *y = g->right;
            if (y && y->color == RED) {
                printf("[RB] Caso 1: tio vermelho (chave %d)\n", z->key);
                z->parent->color = BLACK;
                y->color = BLACK;
                g->color = RED;
                z = g;
            } else {
                if (z == z->parent->right) {
                    printf("[RB] Caso 2: forma de triângulo (chave %d)\n", z->key);
                    z = z->parent;
                    rb_leftRotate(tree, z);
                }
                printf("[RB] Caso 3: forma de linha (chave %d)\n", z->key);
                z->parent->color = BLACK;
                g->color = RED;
                rb_rightRotate(tree, g);
            }
        } else {
            RBNode *y = g->left;
            if (y && y->color == RED) {
                printf("[RB] Caso 1 (espelhado): tio vermelho (chave %d)\n", z->key);
                z->parent->color = BLACK;
                y->color = BLACK;
                g->color = RED;
                z = g;
            } else {
                if (z == z->parent->left) {
                    printf("[RB] Caso 2 (espelhado): forma de triângulo (chave %d)\n", z->key);
                    z = z->parent;
                    rb_rightRotate(tree, z);
                }
                printf("[RB] Caso 3 (espelhado): forma de linha (chave %d)\n", z->key);
                z->parent->color = BLACK;
                g->color = RED;
                rb_leftRotate(tree, g);
            }
        }
    }
    tree->root->color = BLACK;
    printf("[RB] Fixup concluído para chave %d\n", z->key);
}

void rb_insert(RBTree *tree, int key) {
    printf("[RB] Inserindo chave %d\n", key);
    RBNode *z = rb_newNode(key);
    RBNode *y = NULL;
    RBNode *x = tree->root;
    while (x) {
        y = x;
        if (z->key < x->key) x = x->left;
        else x = x->right;
    }
    z->parent = y;
    if (!y) tree->root = z;
    else if (z->key < y->key) y->left = z;
    else y->right = z;
    z->left = z->right = NULL;
    z->color = RED;
    rb_insertFixup(tree, z);
}

RBNode* rb_minimum(RBNode* x) {
    printf("[RB] Buscando mínimo a partir do nó %d\n", x->key);
    while (x->left) x = x->left;
    return x;
}

void rb_transplant(RBTree *tree, RBNode *u, RBNode *v) {
    printf("[RB] Transplantando nó %d por %d\n", u ? u->key : -1, v ? v->key : -1);
    if (!u->parent) tree->root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}

void rb_deleteFixup(RBTree *tree, RBNode *x) {
    printf("[RB] Iniciando fixup para remoção\n");

    while (x != tree->root && (!x || x->color == BLACK)) {
        RBNode *xp = x ? x->parent : NULL;
        if (!xp) break;

        if (x == xp->left) {
            RBNode *w = xp->right;
            if (w && w->color == RED) {
                printf("[RB] Caso 1: irmão vermelho\n");
                w->color = BLACK;
                xp->color = RED;
                rb_leftRotate(tree, xp);
                w = xp->right;
            }
            if ((!w || !w->left || w->left->color == BLACK) &&
                (!w || !w->right || w->right->color == BLACK)) {
                printf("[RB] Caso 2: irmão preto com filhos pretos\n");
                if (w) w->color = RED;
                x = xp;
            } else {
                if (!w || !w->right || w->right->color == BLACK) {
                    printf("[RB] Caso 3: irmão preto com filho esquerdo vermelho\n");
                    if (w && w->left) w->left->color = BLACK;
                    if (w) w->color = RED;
                    rb_rightRotate(tree, w);
                    w = xp->right;
                }
                printf("[RB] Caso 4: irmão preto com filho direito vermelho\n");
                if (w) w->color = xp->color;
                xp->color = BLACK;
                if (w && w->right) w->right->color = BLACK;
                rb_leftRotate(tree, xp);
                x = tree->root;
            }
        } else {
            RBNode *w = xp->left;
            if (w && w->color == RED) {
                printf("[RB] Caso 1 (espelhado): irmão vermelho\n");
                w->color = BLACK;
                xp->color = RED;
                rb_rightRotate(tree, xp);
                w = xp->left;
            }
            if ((!w || !w->right || w->right->color == BLACK) &&
                (!w || !w->left || w->left->color == BLACK)) {
                printf("[RB] Caso 2 (espelhado): irmão preto com filhos pretos\n");
                if (w) w->color = RED;
                x = xp;
            } else {
                if (!w || !w->left || w->left->color == BLACK) {
                    printf("[RB] Caso 3 (espelhado): irmão preto com filho direito vermelho\n");
                    if (w && w->right) w->right->color = BLACK;
                    if (w) w->color = RED;
                    rb_leftRotate(tree, w);
                    w = xp->left;
                }
                printf("[RB] Caso 4 (espelhado): irmão preto com filho esquerdo vermelho\n");
                if (w) w->color = xp->color;
                xp->color = BLACK;
                if (w && w->left) w->left->color = BLACK;
                rb_rightRotate(tree, xp);
                x = tree->root;
            }
        }
    }
    if (x) x->color = BLACK;
    printf("[RB] Fixup de remoção concluído\n");
}

typedef struct {
    RBNode *raiz;
    int64_t t_busca_remocao;
    int64_t t_balance;
} RBDelResult;

RBDelResult rb_delete(RBTree *tree, int key) {
    struct timespec ts1, ts2, ts3;
    RBDelResult result = { tree->root, 0, 0 };
    
    printf("[RB] Iniciando remoção da chave %d\n", key);
    
    // 1) Busca o nó a ser removido
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    RBNode *z = tree->root;
    while (z) {
        printf("[RB] Buscando chave %d (atual: %d)\n", key, z ? z->key : -1);
        if (key < z->key) z = z->left;
        else if (key > z->key) z = z->right;
        else break;
    }
    clock_gettime(CLOCK_MONOTONIC, &ts2);
    result.t_busca_remocao = diff_nsec(&ts1, &ts2);
    
    if (!z) {
        printf("[RB] Chave %d não encontrada\n", key);
        result.raiz = tree->root;
        return result;
    }
    
    printf("[RB] Nó %d encontrado para remoção\n", z->key);
    
    RBNode *y = z;
    Color y_original_color = y->color;
    RBNode *x = NULL;
    
    if (!z->left) {
        printf("[RB] Nó sem filho esquerdo\n");
        x = z->right;
        rb_transplant(tree, z, z->right);
    } else if (!z->right) {
        printf("[RB] Nó sem filho direito\n");
        x = z->left;
        rb_transplant(tree, z, z->left);
    } else {
        printf("[RB] Nó com dois filhos\n");
        y = rb_minimum(z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            printf("[RB] Sucessor é filho direito\n");
            if (x) x->parent = y;
        } else {
            printf("[RB] Sucessor não é filho direito\n");
            rb_transplant(tree, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        rb_transplant(tree, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }
    printf("[RB] Liberando nó %d\n", z->key);
    free(z);
    
    // 2) Balanceamento
    printf("[RB] Iniciando fixup pós-remoção\n");
    clock_gettime(CLOCK_MONOTONIC, &ts2);
    if (y_original_color == BLACK) {
        rb_deleteFixup(tree, x);
    }
    clock_gettime(CLOCK_MONOTONIC, &ts3);
    
    result.t_balance = diff_nsec(&ts2, &ts3);
    result.raiz = tree->root;
    printf("[RB] Remoção da chave %d concluída\n", key);
    return result;
}

void rb_free(RBNode* node) {
    if (!node) return;
    printf("[RB] Liberando nó %d\n", node->key);
    rb_free(node->left);
    rb_free(node->right);
    free(node);
}

// ====================== Skip List ==========================
#define SKIPLIST_MAX_LEVEL 32
#define SKIPLIST_P 0.5

typedef struct SLNode {
    int key;
    struct SLNode **forward;
    int nodeLevel;
} SLNode;

typedef struct {
    int level;
    SLNode *header;
} SkipList;

SkipList* sl_create() {
    printf("[SL] Criando nova SkipList\n");
    SkipList *sl = (SkipList*)malloc(sizeof(SkipList));
    sl->level = 1;
    SLNode *header = (SLNode*)malloc(sizeof(SLNode));
    header->key = 0;
    header->nodeLevel = SKIPLIST_MAX_LEVEL;
    header->forward = (SLNode**)malloc(sizeof(SLNode*) * (SKIPLIST_MAX_LEVEL + 1));
    for (int i = 0; i <= SKIPLIST_MAX_LEVEL; i++)
        header->forward[i] = NULL;
    sl->header = header;
    return sl;
}

int sl_randomLevel() {
    int lvl = 1;
    while ((rand() / (double)RAND_MAX) < SKIPLIST_P && lvl < SKIPLIST_MAX_LEVEL)
        lvl++;
    printf("[SL] Nível aleatório gerado: %d\n", lvl);
    return lvl;
}

void sl_insert(SkipList *sl, int key) {
    printf("[SL] Inserindo chave %d\n", key);
    SLNode *update[SKIPLIST_MAX_LEVEL + 1];
    SLNode *x = sl->header;
    for (int i = sl->level; i >= 1; i--) {
        while (x->forward[i] && x->forward[i]->key < key)
            x = x->forward[i];
        update[i] = x;
    }
    x = x->forward[1];
    if (!x || x->key != key) {
        int lvl = sl_randomLevel();
        if (lvl > sl->level) {
            printf("[SL] Aumentando nível da SkipList para %d\n", lvl);
            for (int i = sl->level + 1; i <= lvl; i++)
                update[i] = sl->header;
            sl->level = lvl;
        }
        SLNode *newNode = (SLNode*)malloc(sizeof(SLNode));
        newNode->key = key;
        newNode->nodeLevel = lvl;
        newNode->forward = (SLNode**)malloc(sizeof(SLNode*) * (lvl + 1));
        for (int i = 1; i <= lvl; i++) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
        printf("[SL] Chave %d inserida no nível %d\n", key, lvl);
    }
}

typedef struct {
    int found;
    int64_t t_busca_remocao;
    int64_t t_balance;
} SLDelResult;

SLDelResult sl_delete(SkipList *sl, int key) {
    SLDelResult res = { 0, 0, 0 };
    struct timespec ts1, ts2;
    SLNode *update[SKIPLIST_MAX_LEVEL + 1];
    SLNode *x = sl->header;
    
    printf("[SL] Removendo chave %d\n", key);
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    for (int i = sl->level; i >= 1; i--) {
        while (x->forward[i] && x->forward[i]->key < key)
            x = x->forward[i];
        update[i] = x;
    }
    x = x->forward[1];
    clock_gettime(CLOCK_MONOTONIC, &ts2);
    res.t_busca_remocao = diff_nsec(&ts1, &ts2);
    
    if (x && x->key == key) {
        printf("[SL] Chave %d encontrada para remoção\n", key);
        res.found = 1;
        for (int i = 1; i <= sl->level; i++) {
            if (update[i]->forward[i] != x) break;
            update[i]->forward[i] = x->forward[i];
        }
        printf("[SL] Liberando nó %d\n", x->key);
        free(x->forward);
        free(x);
        
        while (sl->level > 1 && !sl->header->forward[sl->level]) {
            printf("[SL] Reduzindo nível da SkipList para %d\n", sl->level-1);
            sl->level--;
        }
    } else {
        printf("[SL] Chave %d não encontrada\n", key);
    }
    res.t_balance = 0;
    printf("[SL] Remoção concluída para chave %d\n", key);
    return res;
}

void sl_free(SkipList* sl) {
    printf("[SL] Liberando SkipList\n");
    SLNode* node = sl->header->forward[1];
    while (node) {
        SLNode* next = node->forward[1];
        printf("[SL] Liberando nó %d\n", node->key);
        free(node->forward);
        free(node);
        node = next;
    }
    free(sl->header->forward);
    free(sl->header);
    free(sl);
}

// ====================== Função de experimento ==========================
void experimento_para_tamanho(int N, FILE *csv) {
    printf("\n===== INICIANDO EXPERIMENTO PARA N = %d =====\n", N);
    
    printf("[EXPERIMENTO] Gerando vetor...\n");
    int *vetor = gera_vetor(N);

    // AVL
    printf("\n[EXPERIMENTO] TESTE AVL\n");
    printf("[AVL] Construindo árvore...\n");
    AVLNode *root_avl = NULL;
    for (int i = 0; i < N; i++) {
        root_avl = avl_insert(root_avl, vetor[i]);
    }
    
    printf("[AVL] Iniciando remoções...\n");
    int64_t soma_busca_avl = 0, soma_bal_avl = 0, soma_total_avl = 0;
    for (int i = 0; i < N; i++) {
        printf("[AVL] Removendo chave %d (%d/%d)\n", vetor[i], i+1, N);
        int64_t t_busca = 0, t_bal = 0;
        root_avl = avl_deleteNode(root_avl, vetor[i], &t_busca, &t_bal);
        soma_busca_avl += t_busca;
        soma_bal_avl += t_bal;
        soma_total_avl += (t_busca + t_bal);
    }
    fprintf(csv, "AVL,%d,%" PRId64 ",%" PRId64 ",%" PRId64 "\n",
            N, soma_busca_avl, soma_bal_avl, soma_total_avl);
    fflush(csv);
    printf("[AVL] Liberando memória...\n");
    avl_free(root_avl);

    // RB-Tree
    printf("\n[EXPERIMENTO] TESTE RB-TREE\n");
    printf("[RB] Construindo árvore...\n");
    RBTree rb_tree = { NULL };
    for (int i = 0; i < N; i++) {
        rb_insert(&rb_tree, vetor[i]);
    }
    
    printf("[RB] Iniciando remoções...\n");
    int64_t soma_busca_rb = 0, soma_bal_rb = 0, soma_total_rb = 0;
    for (int i = 0; i < N; i++) {
        printf("[RB] Removendo chave %d (%d/%d)\n", vetor[i], i+1, N);
        RBDelResult res = rb_delete(&rb_tree, vetor[i]);
        soma_busca_rb += res.t_busca_remocao;
        soma_bal_rb += res.t_balance;
        soma_total_rb += (res.t_busca_remocao + res.t_balance);
    }
    fprintf(csv, "RB,%d,%" PRId64 ",%" PRId64 ",%" PRId64 "\n",
            N, soma_busca_rb, soma_bal_rb, soma_total_rb);
    fflush(csv);
    printf("[RB] Liberando memória...\n");
    rb_free(rb_tree.root);

    // Skip List
    printf("\n[EXPERIMENTO] TESTE SKIP LIST\n");
    printf("[SL] Construindo SkipList...\n");
    SkipList *sl = sl_create();
    for (int i = 0; i < N; i++) {
        sl_insert(sl, vetor[i]);
    }
    
    printf("[SL] Iniciando remoções...\n");
    int64_t soma_busca_sl = 0, soma_bal_sl = 0, soma_total_sl = 0;
    for (int i = 0; i < N; i++) {
        printf("[SL] Removendo chave %d (%d/%d)\n", vetor[i], i+1, N);
        SLDelResult res = sl_delete(sl, vetor[i]);
        soma_busca_sl += res.t_busca_remocao;
        soma_bal_sl += res.t_balance;
        soma_total_sl += (res.t_busca_remocao + res.t_balance);
    }
    fprintf(csv, "SkipList,%d,%" PRId64 ",%" PRId64 ",%" PRId64 "\n",
            N, soma_busca_sl, soma_bal_sl, soma_total_sl);
    fflush(csv);
    printf("[SL] Liberando memória...\n");
    sl_free(sl);

    printf("[EXPERIMENTO] Liberando vetor...\n");
    free(vetor);
    printf("===== EXPERIMENTO PARA N = %d CONCLUÍDO =====\n\n", N);
}

// ====================== main ==========================
int main() {
    srand(12345);
    int tamanhos[] = {10, 20, 30, 40, 50};

    FILE *csv = fopen("../resultados/resultados_ate_50.csv", "w");
    if (!csv) {
        perror("Não foi possível criar resultados.csv");
        return EXIT_FAILURE;
    }
    fprintf(csv, "Estrutura,N,TempoBuscaRemocao(ns),TempoBalanceamento(ns),TempoTotal(ns)\n");
    fflush(csv);

    for (int i = 0; i < 5; i++) {
        printf("Iniciando experimento para N = %d...\n", tamanhos[i]);
        experimento_para_tamanho(tamanhos[i], csv);
        fflush(csv);
        printf("Concluído N = %d\n", tamanhos[i]);
    }
    fclose(csv);
    printf("Todas as medições gravadas em resultados.csv\n");
    return 0;
}
