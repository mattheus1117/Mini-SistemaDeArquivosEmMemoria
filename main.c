#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// -------------------- ENUM E DEFINIÇÕES --------------------

typedef enum {
    TIPO_PASTA,
    TIPO_ARQUIVO
} TipoNo;

typedef enum {
    TIPO_NUMERICO,
    TIPO_CARACTERE,
    TIPO_BINARIO,
    TIPO_PROGRAMA
} TipoArquivo;

// -------------------- ESTRUTURAS --------------------

typedef struct {
    char nome[100];
    int tamanho;
    TipoArquivo tipoArquivo;
    time_t criado, modificado, acessado;
    int id, permissao;
    char *conteudo;
} Arquivo;

typedef struct No {
    TipoNo tipo;
    struct No *anterior, *proximo;

    union {
        struct {
            char nome[100];
            struct No *diretorio;
            struct No *pai;
        } pasta;

        Arquivo arquivo;
    };
} No;

// -------------------- FUNÇÕES DE CRIAÇÃO --------------------

// Cria um arquivo com metadados e ponteiro nulo para conteúdo
No* criarArquivo(const char *nome, int tamanho, TipoArquivo tipo, int id, int permissao) {
    No *novo = (No*) malloc(sizeof(No));
    novo->tipo = TIPO_ARQUIVO;
    novo->anterior = novo->proximo = NULL;

    strcpy(novo->arquivo.nome, nome);
    novo->arquivo.tamanho = tamanho;
    novo->arquivo.tipoArquivo = tipo;
    novo->arquivo.criado = novo->arquivo.modificado = novo->arquivo.acessado = time(NULL);
    novo->arquivo.id = id;
    novo->arquivo.permissao = permissao;
    novo->arquivo.conteudo = NULL;

    return novo;
}

// Cria uma pasta com ponteiro para o pai
No* criarPasta(const char *nome, No *pai) {
    No *nova = (No*) malloc(sizeof(No));
    nova->tipo = TIPO_PASTA;
    nova->anterior = nova->proximo = NULL;
    strcpy(nova->pasta.nome, nome);
    nova->pasta.diretorio = NULL;
    nova->pasta.pai = pai;
    return nova;
}

// -------------------- INSERÇÃO E REMOÇÃO --------------------

void inserirNoFinal(No **inicio, No *novo) {
    if (*inicio == NULL) {
        *inicio = novo;
        return;
    }
    No *atual = *inicio;
    while (atual->proximo) atual = atual->proximo;
    atual->proximo = novo;
    novo->anterior = atual;
}

void inserirEmPasta(No *pasta, No *novoConteudo) {
    if (!pasta || pasta->tipo != TIPO_PASTA) {
        printf("Erro: destino não é uma pasta.\n");
        return;
    }
    inserirNoFinal(&(pasta->pasta.diretorio), novoConteudo);
}

// Libera recursivamente estrutura de arquivos e pastas
void liberarEstrutura(No *inicio) {
    if (!inicio) return;

    if (inicio->tipo == TIPO_PASTA && inicio->pasta.diretorio)
        liberarEstrutura(inicio->pasta.diretorio);

    liberarEstrutura(inicio->proximo);

    if (inicio->tipo == TIPO_ARQUIVO && inicio->arquivo.conteudo)
        free(inicio->arquivo.conteudo);

    free(inicio);
}

// Remove arquivo ou pasta da lista encadeada
void removerNo(No **inicio, const char *nome) {
    No *atual = *inicio;
    while (atual) {
        const char *nomeAtual = (atual->tipo == TIPO_PASTA) ? atual->pasta.nome : atual->arquivo.nome;
        if (strcmp(nomeAtual, nome) == 0) {
            if (atual->anterior)
                atual->anterior->proximo = atual->proximo;
            else
                *inicio = atual->proximo;

            if (atual->proximo)
                atual->proximo->anterior = atual->anterior;

            if (atual->tipo == TIPO_PASTA && atual->pasta.diretorio)
                liberarEstrutura(atual->pasta.diretorio);
            if (atual->tipo == TIPO_ARQUIVO && atual->arquivo.conteudo)
                free(atual->arquivo.conteudo);

            free(atual);
            printf("'%s' removido com sucesso.\n", nome);
            return;
        }
        atual = atual->proximo;
    }
    printf("Arquivo ou pasta '%s' não encontrado.\n", nome);
}

// Move ou renomeia arquivos ou pastas
void moverOuRenomear(No *pastaAtual, const char *nome1, const char *nome2) {
    No *atual = pastaAtual->pasta.diretorio, *origem = NULL;
    while (atual) {
        const char *nome = (atual->tipo == TIPO_PASTA) ? atual->pasta.nome : atual->arquivo.nome;
        if (strcmp(nome, nome1) == 0) { origem = atual; break; }
        atual = atual->proximo;
    }
    if (!origem) { printf("Erro: '%s' não encontrado no diretório atual.\n", nome1); return; }
    if (strcmp(nome1, nome2) == 0) { printf("Erro: origem e destino são iguais.\n"); return; }

    // Verifica se destino é o pai ou subpasta
    No *destino = NULL;
    if (strcmp(nome2, "..") == 0 && pastaAtual->pasta.pai)
        destino = pastaAtual->pasta.pai;
    else {
        atual = pastaAtual->pasta.diretorio;
        while (atual) {
            if (atual->tipo == TIPO_PASTA && strcmp(atual->pasta.nome, nome2) == 0) {
                destino = atual;
                break;
            }
            atual = atual->proximo;
        }
    }

    if (destino) {
        const char *nomeOrigem = (origem->tipo == TIPO_PASTA) ? origem->pasta.nome : origem->arquivo.nome;
        No *checar = destino->pasta.diretorio;
        while (checar) {
            const char *nomeCheck = (checar->tipo == TIPO_PASTA) ? checar->pasta.nome : checar->arquivo.nome;
            if (strcmp(nomeCheck, nomeOrigem) == 0) {
                printf("Erro: já existe '%s' em '%s'.\n", nomeOrigem, destino->pasta.nome);
                return;
            }
            checar = checar->proximo;
        }

        if (origem->anterior)
            origem->anterior->proximo = origem->proximo;
        else
            pastaAtual->pasta.diretorio = origem->proximo;

        if (origem->proximo)
            origem->proximo->anterior = origem->anterior;

        origem->anterior = origem->proximo = NULL;
        inserirEmPasta(destino, origem);
        printf("'%s' movido para '%s'.\n", nome1, destino->pasta.nome);
        return;
    }

    // Renomear
    atual = pastaAtual->pasta.diretorio;
    while (atual) {
        const char *nomeExistente = (atual->tipo == TIPO_PASTA) ? atual->pasta.nome : atual->arquivo.nome;
        if (strcmp(nomeExistente, nome2) == 0) {
            printf("Erro: já existe um item chamado '%s'.\n", nome2);
            return;
        }
        atual = atual->proximo;
    }

    if (origem->tipo == TIPO_PASTA)
        strcpy(origem->pasta.nome, nome2);
    else
        strcpy(origem->arquivo.nome, nome2);

    printf("'%s' renomeado para '%s'.\n", nome1, nome2);
}


// Implementar comando cp
void copiarArquivo(No *pastaAtual, const char *origemNome, const char *destinoNome) {
    // procura arquivo origem na pasta atual
    No *aux = pastaAtual->pasta.diretorio;
    No *origem = NULL;
    while (aux != NULL) {
        if (aux->tipo == TIPO_ARQUIVO && strcmp(aux->arquivo.nome, origemNome) == 0) {
            origem = aux;
            break;
        }
        aux = aux->proximo;
    }

    if (!origem) {
        printf("Erro: arquivo '%s' não encontrado.\n", origemNome);
        return;
    }

    // verificar se já existe arquivo destino com o nome
    aux = pastaAtual->pasta.diretorio;
    while (aux != NULL) {
        if (aux->tipo == TIPO_ARQUIVO && strcmp(aux->arquivo.nome, destinoNome) == 0) {
            printf("Erro: arquivo '%s' já existe.\n", destinoNome);
            return;
        }
        aux = aux->proximo;
    }

    // criar cópia
    No *novo = criarArquivo(destinoNome, origem->arquivo.tamanho, origem->arquivo.tipoArquivo, rand() % 1000, origem->arquivo.permissao);
    if (origem->arquivo.conteudo != NULL) {
        novo->arquivo.conteudo = strdup(origem->arquivo.conteudo);
    }

    inserirEmPasta(pastaAtual, novo);
    printf("Arquivo '%s' copiado para '%s'.\n", origemNome, destinoNome);
}


// -------------------- TERMINAL --------------------

const char* tipoArquivoStr(TipoArquivo tipo) {
    switch (tipo) {
        case TIPO_NUMERICO: return "Numérico";
        case TIPO_CARACTERE: return "Caractere";
        case TIPO_BINARIO: return "Binário";
        case TIPO_PROGRAMA: return "Programa";
        default: return "Desconhecido";
    }
}

void lsSimples(No *diretorio) {
    No *atual = diretorio;
    while (atual) {
        if (atual->tipo == TIPO_PASTA)
            printf("[Pasta] %s\n", atual->pasta.nome);
        else {
            Arquivo *arq = &atual->arquivo;
            printf("[Arquivo] %s | Tamanho: %d | Tipo: %s | ID: %d | Permissão: %d\n",
                   arq->nome, arq->tamanho, tipoArquivoStr(arq->tipoArquivo), arq->id, arq->permissao);
        }
        atual = atual->proximo;
    }
}

// Laço principal de comandos interativos
void loopComandos(No *raiz) {
    No *pastaAtual = raiz;
    char comando[512];

    while (1) {
        printf("cd %s> ", pastaAtual->pasta.nome);
        fgets(comando, sizeof(comando), stdin);
        comando[strcspn(comando, "\n")] = '\0';

        if (strcmp(comando, "exit") == 0) break;

        else if (strncmp(comando, "mkdir ", 6) == 0)
            inserirEmPasta(pastaAtual, criarPasta(comando + 6, pastaAtual));

        else if (strncmp(comando, "touch ", 6) == 0)
            inserirEmPasta(pastaAtual, criarArquivo(comando + 6, 100, TIPO_CARACTERE, rand() % 1000, 644));

        else if (strncmp(comando, "ls", 2) == 0)
            lsSimples(pastaAtual->pasta.diretorio);

        else if (strncmp(comando, "cd ", 3) == 0) {
            char *nome = comando + 3;
            if (strcmp(nome, "..") == 0) {
                if (pastaAtual->pasta.pai) pastaAtual = pastaAtual->pasta.pai;
                else printf("Você já está na raiz.\n");
            } else {
                No *aux = pastaAtual->pasta.diretorio;
                while (aux && !(aux->tipo == TIPO_PASTA && strcmp(aux->pasta.nome, nome) == 0))
                    aux = aux->proximo;
                if (aux) pastaAtual = aux;
                else printf("Pasta não encontrada.\n");
            }
        }

        else if (strncmp(comando, "cat ", 4) == 0) {
            char *nome = comando + 4;
            No *aux = pastaAtual->pasta.diretorio;
            while (aux && !(aux->tipo == TIPO_ARQUIVO && strcmp(aux->arquivo.nome, nome) == 0))
                aux = aux->proximo;
            if (aux)
                printf("%s\n", aux->arquivo.conteudo ? aux->arquivo.conteudo : "Arquivo vazio.");
            else
                printf("Arquivo não encontrado.\n");
        }

        else if (strncmp(comando, "echo ", 5) == 0) {
            char *textoCompleto = comando + 5;
            char *ultimaEspaco = strrchr(textoCompleto, ' ');
            if (!ultimaEspaco) { printf("Uso: echo <texto> <arquivo>\n"); continue; }
            *ultimaEspaco = '\0';
            char *texto = textoCompleto;
            char *nome = ultimaEspaco + 1;
            No *aux = pastaAtual->pasta.diretorio;
            while (aux && !(aux->tipo == TIPO_ARQUIVO && strcmp(aux->arquivo.nome, nome) == 0))
                aux = aux->proximo;
            if (aux) {
                free(aux->arquivo.conteudo);
                aux->arquivo.conteudo = strdup(texto);
                aux->arquivo.modificado = time(NULL);
            } else {
                printf("Arquivo '%s' não encontrado.\n", nome);
            }
        }

        else if (strncmp(comando, "rm ", 3) == 0)
            removerNo(&(pastaAtual->pasta.diretorio), comando + 3);

        else if (strncmp(comando, "mv ", 3) == 0) {
            char nome1[100], nome2[100];
            if (sscanf(comando + 3, "%s %s", nome1, nome2) != 2) {
                printf("Uso: mv <origem> <destino>\n");
                continue;
            }
            moverOuRenomear(pastaAtual, nome1, nome2);
        }

        else if (strncmp(comando, "cp ", 3) == 0) {
            char origem[100], destino[100];
            if (sscanf(comando + 3, "%s %s", origem, destino) != 2) {
                printf("Uso: cp <origem> <destino>\n");
                continue;
            }
            copiarArquivo(pastaAtual, origem, destino);
        }

        else
            printf("Comando inválido. Use mkdir, touch, ls, cd, cat, echo, rm, mv, exit\n");
    }
}

int main() {
    No *raiz = criarPasta("Raiz", NULL);
    loopComandos(raiz);
    liberarEstrutura(raiz);
    return 0;
}
