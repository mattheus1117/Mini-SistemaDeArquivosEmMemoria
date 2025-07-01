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


void liberarEstrutura(No *inicio) {
    if (inicio == NULL) return;

    if (inicio->tipo == TIPO_PASTA && inicio->pasta.diretorio != NULL)
        liberarEstrutura(inicio->pasta.diretorio);

    liberarEstrutura(inicio->proximo);

    if (inicio->tipo == TIPO_ARQUIVO && inicio->arquivo.conteudo != NULL)
        free(inicio->arquivo.conteudo);

    free(inicio);
}


void removerNo(No **inicio, const char *nome) {
    No *atual = *inicio;

    while (atual != NULL) {
        int igual = 0;
        if (atual->tipo == TIPO_PASTA) {
            igual = (strcmp(atual->pasta.nome, nome) == 0);
        } else {
            igual = (strcmp(atual->arquivo.nome, nome) == 0);
        }

        if (igual) {
            // Atualiza ponteiros na lista
            if (atual->anterior)
                atual->anterior->proximo = atual->proximo;
            else
                *inicio = atual->proximo;

            if (atual->proximo)
                atual->proximo->anterior = atual->anterior;

            // Se for pasta, libera subdiretório recursivamente
            if (atual->tipo == TIPO_PASTA && atual->pasta.diretorio != NULL)
                liberarEstrutura(atual->pasta.diretorio);

            // Libera conteúdo se for arquivo
            if (atual->tipo == TIPO_ARQUIVO && atual->arquivo.conteudo != NULL)
                free(atual->arquivo.conteudo);

            free(atual);

            printf("'%s' removido com sucesso.\n", nome);
            return;
        }
        atual = atual->proximo;
    }

    printf("Arquivo ou pasta '%s' não encontrado.\n", nome);
}


void moverOuRenomear(No *pastaAtual, const char *nome1, const char *nome2) {
    // Procurar o nó origem na pasta atual
    No *atual = pastaAtual->pasta.diretorio;
    No *origem = NULL;
    while (atual != NULL) {
        const char *nome = (atual->tipo == TIPO_PASTA) ? atual->pasta.nome : atual->arquivo.nome;
        if (strcmp(nome, nome1) == 0) {
            origem = atual;
            break;
        }
        atual = atual->proximo;
    }

    if (!origem) {
        printf("Erro: '%s' não encontrado no diretório atual.\n", nome1);
        return;
    }

    if (strcmp(nome1, nome2) == 0) {
        printf("Erro: origem e destino são iguais.\n");
        return;
    }

    // Verificar se nome2 é ".." (pai)
    No *destino = NULL;
    if (strcmp(nome2, "..") == 0 && pastaAtual->pasta.pai != NULL) {
        destino = pastaAtual->pasta.pai;
    }

    // Verificar se nome2 é uma subpasta
    if (!destino) {
        atual = pastaAtual->pasta.diretorio;
        while (atual != NULL) {
            if (atual->tipo == TIPO_PASTA && strcmp(atual->pasta.nome, nome2) == 0) {
                destino = atual;
                break;
            }
            atual = atual->proximo;
        }
    }

    // Se nome2 for pasta → mover
    if (destino) {
        // Verifica se já existe um item com mesmo nome no destino
        No *checar = destino->pasta.diretorio;
        const char *nomeOrigem = (origem->tipo == TIPO_PASTA) ? origem->pasta.nome : origem->arquivo.nome;
        while (checar != NULL) {
            const char *nomeCheck = (checar->tipo == TIPO_PASTA) ? checar->pasta.nome : checar->arquivo.nome;
            if (strcmp(nomeCheck, nomeOrigem) == 0) {
                printf("Erro: já existe '%s' em '%s'.\n", nomeOrigem, nome2);
                return;
            }
            checar = checar->proximo;
        }

        // Remover da lista atual
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

    // Renomear (verifica se nome2 já existe)
    atual = pastaAtual->pasta.diretorio;
    while (atual != NULL) {
        const char *nomeExistente = (atual->tipo == TIPO_PASTA) ? atual->pasta.nome : atual->arquivo.nome;
        if (strcmp(nomeExistente, nome2) == 0) {
            printf("Erro: já existe um item chamado '%s'.\n", nome2);
            return;
        }
        atual = atual->proximo;
    }

    // Aplicar renomeação
    if (origem->tipo == TIPO_PASTA)
        strcpy(origem->pasta.nome, nome2);
    else
        strcpy(origem->arquivo.nome, nome2);

    printf("'%s' renomeado para '%s'.\n", nome1, nome2);
}



// ---------------------------- TERMINAL ----------------------------


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

    while (atual != NULL) {
        if (atual->tipo == TIPO_PASTA) {
            printf("[Pasta] %s\n", atual->pasta.nome);
        } else {
            Arquivo *arq = &atual->arquivo;
            printf("[Arquivo] %s | Tamanho: %d | Tipo: %s | ID: %d | Permissão: %d\n",
                   arq->nome, arq->tamanho, tipoArquivoStr(arq->tipoArquivo), arq->id, arq->permissao);
        }
        atual = atual->proximo;
    }
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
            lsSimples(pastaAtual->pasta.diretorio);
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
            char *textoCompleto = comando + 5;

            // Encontrar a última palavra (nome do arquivo)
            char *ultimaEspaco = strrchr(textoCompleto, ' ');
            if (ultimaEspaco == NULL) {
                printf("Uso: echo <texto> <arquivo>\n");
                continue;
            }

            *ultimaEspaco = '\0'; // termina o texto aqui
            char *nome = ultimaEspaco + 1;
            char *texto = textoCompleto;

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
                printf("Arquivo '%s' não encontrado.\n", nome);
            }
        }

        else if (strncmp(comando, "rm ", 3) == 0) {
            char *nome = comando + 3;
            removerNo(&(pastaAtual->pasta.diretorio), nome);
        }

        else if (strncmp(comando, "mv ", 3) == 0) {
            char nome1[100], nome2[100];
            if (sscanf(comando + 3, "%s %s", nome1, nome2) != 2) {
                printf("Uso: mv <origem> <destino>\n");
                continue;
            }
            moverOuRenomear(pastaAtual, nome1, nome2);
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