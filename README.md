# Mini-Sistema De Arquivos Em Memoria

Este projeto implementa um sistema de arquivos básico simulado em C, utilizando uma estrutura hierárquica de diretórios e arquivos com listas duplamente encadeadas.

---

## Funcionalidades

- Estrutura hierárquica de pastas e arquivos  
- Criação de pastas (`mkdir`) e arquivos (`touch`)  
- Navegação entre diretórios (`cd`, `cd ..` para pasta pai)  
- Listagem de conteúdo do diretório atual (`ls`)  
- Visualização do conteúdo dos arquivos (`cat`)  
- Escrita de texto em arquivos (`echo <texto> <arquivo>`)  
- Remoção de arquivos e pastas (`rm`)  
- Renomeação ou movimentação de arquivos e pastas (`mv <origem> <destino>`)  
- Cópia de arquivos (`cp <origem> <destino>`)  
- Simulação básica de um terminal com prompt e comandos  

---

## Comandos suportados

| Comando                  | Descrição                                      |
| ------------------------ | ----------------------------------------------|
| `mkdir <nome>`           | Cria uma nova pasta no diretório atual        |
| `touch <nome>`           | Cria um novo arquivo no diretório atual       |
| `ls`                     | Lista o conteúdo do diretório atual            |
| `cd <nome>`              | Entra na pasta especificada                     |
| `cd ..`                  | Volta para a pasta pai                           |
| `cat <arquivo>`          | Exibe o conteúdo do arquivo                      |
| `echo <texto> <arquivo>` | Grava (sobrescreve) texto no arquivo            |
| `rm <nome>`              | Remove arquivo ou pasta                          |
| `mv <origem> <destino>`  | Move ou renomeia arquivo ou pasta                |
| `cp <origem> <destino>`  | Copia arquivo para novo nome no mesmo diretório  |
| `exit`                   | Sai do programa                                 |

---

## Estrutura interna

- Nós duplamente encadeados representam arquivos e pastas.  
- Pastas armazenam ponteiros para seus filhos (arquivos e subpastas).  
- Arquivos armazenam metadados e conteúdo textual.  
- Cada nó mantém informações como nome, tipo, permissões, timestamps, e identificador único.

---

## Como usar

Compile e execute o programa `main.exe`. No terminal, utilize os comandos para manipular o sistema de arquivos simulado.

---

## Exemplo rápido

```bash
cd Raiz> mkdir documentos
cd Raiz> touch notas.txt
cd Raiz> echo Minha nota importante notas.txt
cd Raiz> cat notas.txt
Minha nota importante
cd Raiz> cp notas.txt copia_notas.txt
Arquivo 'notas.txt' copiado para 'copia_notas.txt'.
cd Raiz> mv notas.txt documentos
cd Raiz> ls
[Pasta] documentos
[Arquivo] copia_notas.txt
cd Raiz> cd documentos
cd documentos> ls
[Arquivo] notas.txt
