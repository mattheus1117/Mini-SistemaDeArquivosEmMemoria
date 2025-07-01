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

// -------------------- ESTRUTURAS ESPECÍFICAS --------------------

typedef struct {
    char nome[100];
    int tamanho;
    TipoArquivo tipoArquivo;
    time_t criado;
    time_t modificado;
    time_t acessado;
    int id;
    int permissao;
    char *conteudo;
} Arquivo;

typedef struct No {
    TipoNo tipo;
    struct No *anterior;
    struct No *proximo;

    union {
        struct {
            char nome[100];
            struct No *diretorio;
            struct No *pai;
        } pasta;

        Arquivo arquivo;
    };
} No;

// ---------------------------- FUNÇÕES ----------------------------

// Criar um novo arquivo
No* criarArquivo(const char *nome, int tamanho, TipoArquivo tipo, int id, int permissao) {
    No *novo = (No*) malloc(sizeof(No));
    novo->tipo = TIPO_ARQUIVO;
    novo->anterior = novo->proximo = NULL;

    strcpy(novo->arquivo.nome, nome);
    novo->arquivo.tamanho = tamanho;
    novo->arquivo.tipoArquivo = tipo;
    novo->arquivo.criado = time(NULL);
    novo->arquivo.modificado = time(NULL);
    novo->arquivo.acessado = time(NULL);
    novo->arquivo.id = id;
    novo->arquivo.permissao = permissao;
    novo->arquivo.conteudo = NULL;  // <- novo

    return novo;
}


// Criar uma nova pasta
No* criarPasta(const char *nome, No *pai) {
    No *nova = (No*) malloc(sizeof(No));
    nova->tipo = TIPO_PASTA;
    nova->anterior = nova->proximo = NULL;
    strcpy(nova->pasta.nome, nome);
    nova->pasta.diretorio = NULL;
    nova->pasta.pai = pai; // <- Aqui
    return nova;
}


void inserirNoFinal(No **inicio, No *novo) {
    if (*inicio == NULL) {
        *inicio = novo;
        return;
    }

    No *atual = *inicio;
    while (atual->proximo != NULL)
        atual = atual->proximo;

    atual->proximo = novo;
    novo->anterior = atual;
}


void inserirEmPasta(No *pasta, No *novoConteudo) {
    if (pasta == NULL || pasta->tipo != TIPO_PASTA) {
        printf("Erro: destino não é uma pasta.\n");
        return;
    }

    inserirNoFinal(&(pasta->pasta.diretorio), novoConteudo);
}


// Função para imprimir tipo de arquivo
const char* tipoArquivoStr(TipoArquivo tipo) {
    switch (tipo) {
        case TIPO_NUMERICO: return "Numérico";
        case TIPO_CARACTERE: return "Caractere";
        case TIPO_BINARIO: return "Binário";
        case TIPO_PROGRAMA: return "Programa";
        default: return "Desconhecido";
    }
}


void imprimirEstrutura(No *inicio, int nivel) {
    No *atual = inicio;

    while (atual != NULL) {
        for (int i = 0; i < nivel; i++) printf("  ");

        if (atual->tipo == TIPO_PASTA) {
            printf("[Pasta] %s\n", atual->pasta.nome);
            imprimirEstrutura(atual->pasta.diretorio, nivel + 1);
        } else {
            Arquivo *arq = &atual->arquivo;
            printf("[Arquivo] %s | Tamanho: %d | Tipo: %s | ID: %d | Permissão: %d\n",
                   arq->nome, arq->tamanho, tipoArquivoStr(arq->tipoArquivo), arq->id, arq->permissao);
        }

        atual = atual->proximo;
    }
}


void liberarEstrutura(No *inicio) {
    if (inicio == NULL) return;

    if (inicio->tipo == TIPO_PASTA && inicio->pasta.diretorio != NULL)
        liberarEstrutura(inicio->pasta.diretorio);

    liberarEstrutura(inicio->proximo);

    if (inicio->tipo == TIPO_ARQUIVO && inicio->arquivo.conteudo != NULL)
        free(inicio->arquivo.conteudo);

    free(inicio);
}


void loopComandos(No *raiz) {
    No *pastaAtual = raiz;
    char comando[512];

    while (1) {
        printf("cd %s> ", pastaAtual->pasta.nome);
        fgets(comando, sizeof(comando), stdin);
        comando[strcspn(comando, "\n")] = '\0';

        if (strcmp(comando, "exit") == 0) break;

        else if (strncmp(comando, "mkdir ", 6) == 0) {
            char *nome = comando + 6;
            No *nova = criarPasta(nome, pastaAtual);
            inserirEmPasta(pastaAtual, nova);
        }

        else if (strncmp(comando, "touch ", 6) == 0) {
            char *nome = comando + 6;
            No *novo = criarArquivo(nome, 100, TIPO_CARACTERE, rand() % 1000, 644);
            inserirEmPasta(pastaAtual, novo);
        }

        else if (strncmp(comando, "ls", 2) == 0) {
            imprimirEstrutura(pastaAtual->pasta.diretorio, 1);
        }

        else if (strncmp(comando, "cd ", 3) == 0) {
            char *nome = comando + 3;
            if (strcmp(nome, "..") == 0) {
                if (pastaAtual->pasta.pai != NULL)
                    pastaAtual = pastaAtual->pasta.pai;
                else
                    printf("Você já está na raiz.\n");
            } else {
                No *aux = pastaAtual->pasta.diretorio;
                int encontrado = 0;
                while (aux != NULL) {
                    if (aux->tipo == TIPO_PASTA && strcmp(aux->pasta.nome, nome) == 0) {
                        pastaAtual = aux;
                        encontrado = 1;
                        break;
                    }
                    aux = aux->proximo;
                }
                if (!encontrado)
                    printf("Pasta não encontrada.\n");
            }
        }

        else if (strncmp(comando, "cat ", 4) == 0) {
            char *nome = comando + 4;
            No *aux = pastaAtual->pasta.diretorio;
            int encontrado = 0;
            while (aux != NULL) {
                if (aux->tipo == TIPO_ARQUIVO && strcmp(aux->arquivo.nome, nome) == 0) {
                    if (aux->arquivo.conteudo != NULL)
                        printf("%s\n", aux->arquivo.conteudo);
                    else
                        printf("Arquivo vazio.\n");
                    encontrado = 1;
                    break;
                }
                aux = aux->proximo;
            }
            if (!encontrado)
                printf("Arquivo não encontrado.\n");
        }

        else if (strncmp(comando, "echo ", 5) == 0) {
            char *texto = comando + 5;

            // Procura o ' > ' para decidir se é gravação ou só imprimir
            char *arrow = strstr(texto, " > ");

            if (arrow == NULL) {
                // Só imprimir o texto
                printf("%s\n", texto);
            } else {
                // Separar texto e nome do arquivo
                *arrow = '\0'; // termina o texto aqui
                char *nome = arrow + 3;

                // Procurar o arquivo na pasta atual
                No *aux = pastaAtual->pasta.diretorio;
                int encontrado = 0;
                while (aux != NULL) {
                    if (aux->tipo == TIPO_ARQUIVO && strcmp(aux->arquivo.nome, nome) == 0) {
                        free(aux->arquivo.conteudo);
                        aux->arquivo.conteudo = strdup(texto);
                        aux->arquivo.modificado = time(NULL);
                        encontrado = 1;
                        break;
                    }
                    aux = aux->proximo;
                }
                if (!encontrado) {
                    printf("Arquivo não encontrado.\n");
                }
            }
        }

        else {
            printf("Comando inválido. Use mkdir, touch, ls, cd, cat, echo, exit\n");
        }
    }
}


// ----------------------------- MAIN -----------------------------

int main() {
    No *raiz = criarPasta("Raiz", NULL);
    loopComandos(raiz);
    liberarEstrutura(raiz);
    return 0;
}