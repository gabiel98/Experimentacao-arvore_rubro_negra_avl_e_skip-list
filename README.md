# Comparative Analysis of Data Structures

## Introdução

Neste trabalho, vamos explorar na prática o comportamento de três estruturas de dados bastante usadas em aplicações que exigem inserção, busca e remoção eficientes:

- **Árvore AVL**
- **Árvore Rubro-Negra (RB)**
- **Skip List**

Para cada estrutura, executamos as seguintes etapas:

1. Geramos um vetor de chaves em ordem aleatória, com tamanhos variando de **100.000** a **500.000** (passo de 10.000), totalizando 41 conjuntos de entradas.
2. Inserimos todas as chaves na estrutura.
3. Realizamos operações de busca e remoção numa sequência completa.
4. Medimos o tempo gasto em cada fase:
   - **Busca + Remoção**: tempo para localizar e remover cada nó.
   - **Balanceamento**: rebalanceamentos necessários (válido para AVL e RB; Skip List não faz balanceamento explícito).
   - **Total**: soma dos tempos anteriores.

Todos os resultados são salvos em arquivos CSV e, em seguida, usamos um script em Python para gerar gráficos que mostram como o tempo cresce conforme aumentamos o número de elementos.

## Estrutura de Diretórios

```
├── comparacoes
│   ├── comparacao_estruturas.c      # Código-fonte em C
│   └── comparacao_estruturas.exe    
├── graficos
│   ├── grafico_balanceamento.png    # Tempo de balanceamento para AVL/RB
│   ├── grafico_busca_remocao.png    # Tempo de busca + remoção
│   └── grafico_total.png            # Tempo total de cada estrutura
├── plotar_graficos
│   └── plotar_graficos.py           # Python para criar os gráficos
└── resultados
    └── resultados.csv               # Dados completos (100k a 500k)
```

## Como Compilar e Executar

Siga estes passos para reproduzir os testes em sua máquina.

### 1. Preparar o ambiente

- Certifique‑se de ter instalado um compilador C (GCC, Clang etc.)
- Para os gráficos, instale `pandas` e `matplotlib` em Python:
  ```bash
  pip install pandas matplotlib
  ```

### 2. Compilar o programa em C

```bash
cd comparacoes
gcc comparacao_estruturas.c  -o comparacao_estruturas
cd ..
```

### 3. Executar os testes

```bash
./comparacoes/comparacao_estruturas
```

Isso vai gerar (ou atualizar) `resultados/resultados.csv` com os tempos medidos.

### 4. Gerar os gráficos

```bash
cd plotar_graficos
python plotar_graficos.py
```

Os arquivos PNG serão salvos na pasta `graficos/`.

## Pseudocódigos das Estruturas

### Árvore AVL

```text
function AVL_Insert(node, key):
    if node é NULO:
        return novo_node(key)
    if key < node.key:
        node.left = AVL_Insert(node.left, key)
    else if key > node.key:
        node.right = AVL_Insert(node.right, key)
    atualizar_altura(node)
    balance = altura(node.left) - altura(node.right)
    if balance > 1 and key < node.left.key:
        return rotacao_direita(node)
    if balance < -1 and key > node.right.key:
        return rotacao_esquerda(node)
    if balance > 1 and key > node.left.key:
        node.left = rotacao_esquerda(node.left)
        return rotacao_direita(node)
    if balance < -1 and key < node.right.key:
        node.right = rotacao_direita(node.right)
        return rotacao_esquerda(node)
    return node
```

### Árvore Rubro-Negra (Red-Black Tree)

```text
function RB_Insert(tree, key):
    z = novo_node_RED(key)
    inserir_como_em_BST(tree, z)
    while z.parent é RED:
        if parent é filho_esquerdo de grandparent:
            y = tio(z)
            if y.color == RED:
                recolore pai, tio e avô
                z = grandparent
            else:
                se z é filho_direito:
                    rotacao_esquerda(pai)
                    z = pai
                recolore pai e avô
                rotacao_direita(grandparent)
        else:
    tree.root.color = BLACK
```

### Skip List

```text
function SL_Insert(skiplist, key):
    update[1..maxLevel]
    x = skiplist.header
    for i from nível_atual down to 1:
        while x.forward[i] and x.forward[i].key < key:
            x = x.forward[i]
        update[i] = x
    lvl = randomLevel()
    if lvl > skiplist.nível_atual:
        ajusta update[] para novos níveis
        skiplist.nível_atual = lvl
    novo = novo_node(key, lvl)
    for i from 1 to lvl:
        novo.forward[i] = update[i].forward[i]
        update[i].forward[i] = novo

function SL_Delete(skiplist, key):
```

## Ambiente de Execução
Os testes foram executados em um ambiente contendo:

* **Sistema Operacional**: Windows 11 Home 64 bits
* **Especificações**:

  * Processador: AMD Ryzen 5 5600, 3.5GHz (4.4GHz Turbo), 6-Cores 12-Threads
  * Placa de Vídeo: MSI NVIDIA GeForce RTX 3060 VENTUS 2X OC, LHR, 12GB GDDR6, DLSS, Ray Tracing
  * Memória RAM:  DDR4 16GB 3733Mhz
  * SSD: SSD 500GB Leitura 4800MBs e Gravação 2700MBs

## Insights dos Resultados

### Tempo Balanceamento – AVL x RB
  
  <img src="https://github.com/gabiel98/Experimentacao-arvore_rubro_negra_avl_e_skip-list/blob/main/graficos/grafico_balanceamento.png" alt="Gráfico 1" width="80%" style="display: center;">

---

### Tempo Busca + Remoção

  <img src="https://github.com/gabiel98/Experimentacao-arvore_rubro_negra_avl_e_skip-list/blob/main/graficos/grafico_busca_remocao.png" alt="Gráfico 2" width="80%" style="display: center;">

---

### Tempo Total

<img src="https://github.com/gabiel98/Experimentacao-arvore_rubro_negra_avl_e_skip-list/blob/main/graficos/grafico_total.png" alt="Gráfico 3" width="80%" style="display: center;">


---
