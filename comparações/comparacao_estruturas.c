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
    static int initialized = 0;

    if (!initialized) {
        QueryPerformanceFrequency(&freq);
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
    return (int64_t)(end->tv_sec - start->tv_sec) * 1000000000LL + (int64_t)(end->tv_nsec - start->tv_nsec);
}

void shuffle_array(int *arr, size_t n) {
    for (size_t i = n - 1; i > 0; --i) {
        size_t j = (size_t)(rand() % (i + 1));
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

int* gera_vetor(int n) {
    int *v = malloc(n * sizeof(int));
    if (!v) exit(EXIT_FAILURE);
    for (int i = 0; i < n; i++) v[i] = i + 1;
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
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    node->key = key;
    node->left = node->right = NULL;
    node->height = 1;
    return node;
}

AVLNode* avl_rightRotate(AVLNode *y) {
    AVLNode *x = y->left;
    AVLNode *T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = 1 + ((avl_height(y->left) > avl_height(y->right)) ? avl_height(y->left) : avl_height(y->right));
    x->height = 1 + ((avl_height(x->left) > avl_height(x->right)) ? avl_height(x->left) : avl_height(x->right));
    return x;
}

AVLNode* avl_leftRotate(AVLNode *x) {
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
        return avl_rightRotate(node);
    }
    // RR
    if (balance < -1 && key > node->right->key) {
        return avl_leftRotate(node);
    }
    // LR
    if (balance > 1 && key > node->left->key) {
        node->left = avl_leftRotate(node->left);
        return avl_rightRotate(node);
    }
    // RL
    if (balance < -1 && key < node->right->key) {
        node->right = avl_rightRotate(node->right);
        return avl_leftRotate(node);
    }
    return node;
}

// Função auxiliar para balanceamento
static AVLNode* avl_recalc_and_balance(AVLNode* node) {
    if (!node) return NULL;
    
    node->height = 1 + ((avl_height(node->left) > avl_height(node->right)) ? avl_height(node->left) : avl_height(node->right));
    int balance = avl_getBalance(node);
    
    // LL
    if (balance > 1 && avl_getBalance(node->left) >= 0) {
        return avl_rightRotate(node);
    }
    // LR
    if (balance > 1 && avl_getBalance(node->left) < 0) {
        node->left = avl_leftRotate(node->left);
        return avl_rightRotate(node);
    }
    // RR
    if (balance < -1 && avl_getBalance(node->right) <= 0) {
        return avl_leftRotate(node);
    }
    // RL
    if (balance < -1 && avl_getBalance(node->right) > 0) {
        node->right = avl_rightRotate(node->right);
        return avl_leftRotate(node);
    }
    return node;
}

// Função auxiliar para balanceamento completo
static void avl_full_balance(AVLNode** pnode) {
    if (!(*pnode)) return;
    avl_full_balance(&((*pnode)->left));
    avl_full_balance(&((*pnode)->right));
    *pnode = avl_recalc_and_balance(*pnode);
}

AVLNode* avl_minValueNode(AVLNode* node) {
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

    clock_gettime(CLOCK_MONOTONIC, &ts1);
    AVLNode* curr = root;
    AVLNode* parent = NULL;
    while (curr && curr->key != key) {
        parent = curr;
        if (key < curr->key) curr = curr->left;
        else curr = curr->right;
    }
    clock_gettime(CLOCK_MONOTONIC, &ts2);
    result.t_busca_remocao = diff_nsec(&ts1, &ts2);

    if (!curr) {
        result.node = root;
        return result;
    }

    if (curr->left && curr->right) {
        AVLNode* succParent = curr;
        AVLNode* succ = curr->right;
        while (succ->left) {
            succParent = succ;
            succ = succ->left;
        }
        curr->key = succ->key;
        key = succ->key;
        curr = succ;
        parent = succParent;
    }

    AVLNode* child = (curr->left) ? curr->left : curr->right;
    if (!parent) {
        root = child;
    } else if (parent->left == curr) {
        parent->left = child;
    } else {
        parent->right = child;
    }

    free(curr);

    clock_gettime(CLOCK_MONOTONIC, &ts2);
    avl_full_balance(&root);
    clock_gettime(CLOCK_MONOTONIC, &ts3);

    result.t_balance = diff_nsec(&ts2, &ts3);
    result.node = root;
    return result;
}

AVLNode* avl_deleteNode(AVLNode* root, int key, int64_t *out_busca, int64_t *out_bal) {
    AVLDelResult res = avl_deleteNode_internal(root, key);
    *out_busca = res.t_busca_remocao;
    *out_bal = res.t_balance;
    return res.node;
}

void avl_free(AVLNode* node) {
    if (!node) return;
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
    RBNode* node = (RBNode*)malloc(sizeof(RBNode));
    node->key = key;
    node->color = RED;
    node->left = node->right = node->parent = NULL;
    return node;
}

void rb_leftRotate(RBTree *tree, RBNode *x) {
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
    while (z->parent && z->parent->color == RED) {
        RBNode *g = z->parent->parent;
        if (!g) break;
        if (z->parent == g->left) {
            RBNode *y = g->right;
            if (y && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                g->color = RED;
                z = g;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rb_leftRotate(tree, z);
                }
                z->parent->color = BLACK;
                g->color = RED;
                rb_rightRotate(tree, g);
            }
        } else {
            RBNode *y = g->left;
            if (y && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                g->color = RED;
                z = g;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rb_rightRotate(tree, z);
                }
                z->parent->color = BLACK;
                g->color = RED;
                rb_leftRotate(tree, g);
            }
        }
    }
    tree->root->color = BLACK;
}

void rb_insert(RBTree *tree, int key) {
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
    while (x->left) x = x->left;
    return x;
}

void rb_transplant(RBTree *tree, RBNode *u, RBNode *v) {
    if (!u->parent) tree->root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}

void rb_deleteFixup(RBTree *tree, RBNode *x) {

    while (x != tree->root && (!x || x->color == BLACK)) {
        RBNode *xp = x ? x->parent : NULL;
        if (!xp) break;

        if (x == xp->left) {
            RBNode *w = xp->right;
            if (w && w->color == RED) {
                w->color = BLACK;
                xp->color = RED;
                rb_leftRotate(tree, xp);
                w = xp->right;
            }
            if ((!w || !w->left || w->left->color == BLACK) &&
                (!w || !w->right || w->right->color == BLACK)) {
                if (w) w->color = RED;
                x = xp;
            } else {
                if (!w || !w->right || w->right->color == BLACK) {
                    if (w && w->left) w->left->color = BLACK;
                    if (w) w->color = RED;
                    rb_rightRotate(tree, w);
                    w = xp->right;
                }
                if (w) w->color = xp->color;
                xp->color = BLACK;
                if (w && w->right) w->right->color = BLACK;
                rb_leftRotate(tree, xp);
                x = tree->root;
            }
        } else {
            RBNode *w = xp->left;
            if (w && w->color == RED) {
                w->color = BLACK;
                xp->color = RED;
                rb_rightRotate(tree, xp);
                w = xp->left;
            }
            if ((!w || !w->right || w->right->color == BLACK) &&
                (!w || !w->left || w->left->color == BLACK)) {
                if (w) w->color = RED;
                x = xp;
            } else {
                if (!w || !w->left || w->left->color == BLACK) {
                    if (w && w->right) w->right->color = BLACK;
                    if (w) w->color = RED;
                    rb_leftRotate(tree, w);
                    w = xp->left;
                }
                if (w) w->color = xp->color;
                xp->color = BLACK;
                if (w && w->left) w->left->color = BLACK;
                rb_rightRotate(tree, xp);
                x = tree->root;
            }
        }
    }
    if (x) x->color = BLACK;
}

typedef struct {
    RBNode *raiz;
    int64_t t_busca_remocao;
    int64_t t_balance;
} RBDelResult;

RBDelResult rb_delete(RBTree *tree, int key) {
    struct timespec ts1, ts2, ts3;
    RBDelResult result = { tree->root, 0, 0 };
    
    
    // 1) Busca o nó a ser removido
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    RBNode *z = tree->root;
    while (z) {
        if (key < z->key) z = z->left;
        else if (key > z->key) z = z->right;
        else break;
    }
    clock_gettime(CLOCK_MONOTONIC, &ts2);
    result.t_busca_remocao = diff_nsec(&ts1, &ts2);
    
    if (!z) {
        result.raiz = tree->root;
        return result;
    }
    
    
    RBNode *y = z;
    Color y_original_color = y->color;
    RBNode *x = NULL;
    
    if (!z->left) {
        x = z->right;
        rb_transplant(tree, z, z->right);
    } else if (!z->right) {
        x = z->left;
        rb_transplant(tree, z, z->left);
    } else {
        y = rb_minimum(z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            if (x) x->parent = y;
        } else {
            rb_transplant(tree, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        rb_transplant(tree, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }
    free(z);
    
    // 2) Balanceamento
    clock_gettime(CLOCK_MONOTONIC, &ts2);
    if (y_original_color == BLACK) {
        rb_deleteFixup(tree, x);
    }
    clock_gettime(CLOCK_MONOTONIC, &ts3);
    
    result.t_balance = diff_nsec(&ts2, &ts3);
    result.raiz = tree->root;
    return result;
}

void rb_free(RBNode* node) {
    if (!node) return;
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
    return lvl;
}

void sl_insert(SkipList *sl, int key) {
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
        res.found = 1;
        for (int i = 1; i <= sl->level; i++) {
            if (update[i]->forward[i] != x) break;
            update[i]->forward[i] = x->forward[i];
        }
        free(x->forward);
        free(x);
        
        while (sl->level > 1 && !sl->header->forward[sl->level]) {
            sl->level--;
        }
    } 
    res.t_balance = 0;
    return res;
}

void sl_free(SkipList* sl) {
    SLNode* node = sl->header->forward[1];
    while (node) {
        SLNode* next = node->forward[1];
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
    printf("Iniciando experimento para N = %d\n", N);

    int64_t menor_total_avl = INT64_MAX, menor_busca_avl = 0, menor_bal_avl = 0;
    int64_t menor_total_rb = INT64_MAX, menor_busca_rb = 0, menor_bal_rb = 0;
    int64_t menor_total_sl = INT64_MAX, menor_busca_sl = 0, menor_bal_sl = 0;

    for (int tentativa = 0; tentativa < 1; tentativa++) {
        printf("Tentativa %d para N = %d\n", tentativa + 1, N);

        int *vetor = gera_vetor(N);

        // AVL
        AVLNode *root_avl = NULL;
        for (int i = 0; i < N; i++) root_avl = avl_insert(root_avl, vetor[i]);

        int64_t busca_avl = 0, bal_avl = 0;
        for (int i = 0; i < N; i++) {
            int64_t b = 0, t = 0;
            root_avl = avl_deleteNode(root_avl, vetor[i], &b, &t);
            busca_avl += b;
            bal_avl += t;
        }
        avl_free(root_avl);
        printf("AVL: finalizado\n");

        int64_t total_avl = busca_avl + bal_avl;
        if (total_avl < menor_total_avl) {
            menor_total_avl = total_avl;
            menor_busca_avl = busca_avl;
            menor_bal_avl = bal_avl;
        }

        // RB
        RBTree rb_tree = { NULL };
        for (int i = 0; i < N; i++) rb_insert(&rb_tree, vetor[i]);

        int64_t busca_rb = 0, bal_rb = 0;
        for (int i = 0; i < N; i++) {
            RBDelResult res = rb_delete(&rb_tree, vetor[i]);
            busca_rb += res.t_busca_remocao;
            bal_rb += res.t_balance;
        }
        rb_free(rb_tree.root);
        printf("RB: finalizado\n");

        int64_t total_rb = busca_rb + bal_rb;
        if (total_rb < menor_total_rb) {
            menor_total_rb = total_rb;
            menor_busca_rb = busca_rb;
            menor_bal_rb = bal_rb;
        }

        // Skip List
        SkipList *sl = sl_create();
        for (int i = 0; i < N; i++) sl_insert(sl, vetor[i]);

        int64_t busca_sl = 0, bal_sl = 0;
        for (int i = 0; i < N; i++) {
            SLDelResult res = sl_delete(sl, vetor[i]);
            busca_sl += res.t_busca_remocao;
            bal_sl += res.t_balance;
        }
        sl_free(sl);
        printf("SkipList: finalizado\n");

        int64_t total_sl = busca_sl + bal_sl;
        if (total_sl < menor_total_sl) {
            menor_total_sl = total_sl;
            menor_busca_sl = busca_sl;
            menor_bal_sl = bal_sl;
        }

        free(vetor);
    }

    fprintf(csv, "AVL,%d,%" PRId64 ",%" PRId64 ",%" PRId64 "\n", N, menor_busca_avl, menor_bal_avl, menor_total_avl);
    fprintf(csv, "RB,%d,%" PRId64 ",%" PRId64 ",%" PRId64 "\n", N, menor_busca_rb, menor_bal_rb, menor_total_rb);
    fprintf(csv, "SkipList,%d,%" PRId64 ",%" PRId64 ",%" PRId64 "\n", N, menor_busca_sl, menor_bal_sl, menor_total_sl);
    fflush(csv);
    printf("Experimento N = %d finalizado\n", N);
}


int main() {
    srand(12345);
    int tamanhos[] = { 100000, 110000, 120000, 130000, 140000, 150000, 160000, 170000, 180000, 190000, 200000, 210000, 220000, 230000, 240000, 250000, 260000, 270000, 280000, 290000, 300000, 310000, 320000, 330000, 340000, 350000, 360000, 370000, 380000, 390000, 400000, 410000, 420000, 430000, 440000, 450000, 460000, 470000, 480000, 490000, 500000};
    int num_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);

    FILE *csv = fopen("../resultados/resultados.csv", "w");
    if (!csv) return EXIT_FAILURE;

    fprintf(csv, "Estrutura,N,TempoBuscaRemocao(ns),TempoBalanceamento(ns),TempoTotal(ns)\n");
    fflush(csv);

    for (int i = 0; i < num_tamanhos; i++) {
        experimento_para_tamanho(tamanhos[i], csv);
        fflush(csv);
    }
    fclose(csv);
    return 0;
}
