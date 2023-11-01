//#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <cstring>

FILE *file;
//char charTemp; 
char nextChar; //Salva o caracter lido do arquivo
int charClass; //Armazena o tipo do caracter nextChar (LETRA, NUMERO ou DESCONHECIDO)
char lexemas[100]; //Funciona como um buffer para armazenar os lexemas
int indexLexemas=0;//eh o index de lexemas
int token; //Armazena o tipo de um dado lexema armazenado em lexemas

//OBSERVACAO: Operador de negacao trocado para '!'.

/*Erros: 
		1) Não sao aceitos identificadores numéricos, entao caso haja algum número isolado dentro do codigo, o programa será terminado com retorno 1
			OBS: uma letra seguida de um numero eh valido para nomes de identificadores (Exemplo: B1)
	
		2) Caso haja operadores ("->", "<->", "True" ou "False) escritos forma incorreta, o código vai apontar que existem lexemas DESCONHECIDO (valor 13)
		
		3) Caso haja caracteres desconhecidos, o programa ira printar token: 13 (DESCONHECIDO) e o caracter em questão
		
		4) Caso haja apenas uma "/" ao invés de "//" para comentário, o programa vai avisar o erro e sair com retorno 1 
*/

#define FALSE 0 //"False"
#define TRUE 1 //"True"

#define NUMERO 10
#define LETRA 11
#define IDENTIFICADOR 12
#define DESCONHECIDO 13

#define AND_OP 20  //*
#define OR_OP 21  //+
#define NOT_OP 22  //!
#define IMPLICA_OP 23  //->
#define BIIMPLICA_OP 24  //<->
#define COMENT_OP 25 // //

#define PARENT_ESQ 30  //(
#define PARENT_DIR 31  //)

void lerNovoChar();
void identChar();
void identLexemas();
void identOperadores();
int identOpImplicacao();
int identTrueOUFalse(int tipoParaIdent);
void addChar(char ch);
int ignorarComentarios();

/*
Teste que usei: 

//(A+B1)*False<->D //comentario
(A+B1)*(True+False)<->D //TRue sendo identificado como identificador por conta do R maiusculo
( A  +% B1  ) * C < - D 
( A + 1B )*False<->D //identificador nao pode ser numero ou começar com um numero

*/

int main(int argc, char** argv) {
	if((file = fopen("entrada.in", "r")) != NULL){
		
		do{
			identLexemas();
		}while(charClass != EOF);
		fclose(file);
		
	} else {
		printf("ERRO - Leitura de arquivo mal sucecida");
		exit(1);
	}
	
	return 0;
}

void lerNovoChar(){ //Lê um novo caracter do arquivo e identifica se eh fim do arquivo
	if((nextChar = getc(file)) == EOF)
		charClass = EOF;
}


void identChar(){ //Lê um novo caracter do arquivo e identifica esse caracter
	
	lerNovoChar();
	
	ignorarComentarios(); //busca por comentarios para ignora-los

	if(charClass != EOF){
		if(isspace(nextChar)) {
			identChar(); //chama recursivamente a funcao para eliminar espaços em branco e quebras de linha
		} else {
			//Identifica o tipo dos caracteres armazenados em nextChar
			if(isalpha(nextChar)){
				charClass = LETRA;
			} else if(isdigit(nextChar)){
				charClass = NUMERO;
			} else {
				charClass = DESCONHECIDO;
			}	
		
		}
	}
}


void identLexemas(){ 
//Identifica lexemas (nextChar) baseado no seu tipo (charClass)
	
	identChar(); //chama uma nova leitura e idenficiacao de caracteres
	
	switch (charClass) {
		
		case LETRA:
			
			if(nextChar == 'T'){
				if(identTrueOUFalse(TRUE)){
					token = TRUE; //se o retorno for verdadeiro, o token eh palavra reservada TRUE
				} else {
					token = IDENTIFICADOR; //se for falso, o token é identificador
				}
				break; //sai do switch case
			}
			
			if(nextChar == 'F'){ //Testa se pode ser uma 
				if(identTrueOUFalse(FALSE)){
					token = FALSE; //se o retorno for verdadeiro, o token eh palavra reservada FALSE
				} else {
					token = IDENTIFICADOR; //se for falso, o token é identificador
				}
				break; //sai do switch case
			}
			
			//Caso a palavra nao começar com T ou F, 
			//	o codigo abaixo sera executado a fim de detectar esse identificador
			addChar(nextChar);
			identChar();
			while(charClass == LETRA ||  charClass == NUMERO){
				addChar(nextChar);
				identChar();
			}
			ungetc(nextChar, file); //para "voltar" o ponteiro de leitura do arquivo e 
									//evitar o "erro" de leitura do arquivo causado pelo laço acima
									//OBS: reverte a leitura adicional (chamada de "identChar();") da ultima interação do laço while acima
			token = IDENTIFICADOR;
			break;
		
		
		case NUMERO:
			printf("ERRO: nao existem comandos que comecam com numeros OU identificadores nao devem comecar com numeros.\n");
			exit(1);
			break;


		case DESCONHECIDO:
			identOperadores();
			break;
			
			
		case EOF:
			token = EOF;
			addChar('E');
			addChar('O');
			addChar('F');
			exit(0); //Termina o programa pois será o fim do arquivo
			break;
	}
	
	printf("TOKEN: %d\t LEXEMA: %s\n", token, lexemas);
	indexLexemas = 0;
}


void identOperadores(){
//Identifica e Agrupa os caracteres especiais dos lexemas
	switch (nextChar){
		
		case '>':
			addChar(nextChar);
			token = DESCONHECIDO;
			printf("ERRO! Um caractere desconhecido foi inserido: ");
			break;
		
		case '-':
			if(identOpImplicacao()){
				token = IMPLICA_OP;
			} else {
				token = DESCONHECIDO;
				printf("ERRO! Um caractere desconhecido foi inserido: ");
			}
			break;
			
		case '<':
			addChar(nextChar);
			identChar();
			if(identOpImplicacao()){
				token = BIIMPLICA_OP;	
			} else {
				token = DESCONHECIDO;
				printf("ERRO! Um caractere desconhecido foi inserido: ");
			}
			break;
			
		case '*':
			token = AND_OP;
			addChar(nextChar);
			break;	
		
		case '+':
			token = OR_OP;
			addChar(nextChar);
			break;
		
		case '!':
			token = NOT_OP;
			addChar(nextChar);
			break;
		
		case ')':
			token = PARENT_DIR;
			addChar(nextChar);
			break;
			
		case '(':
			token = PARENT_ESQ;
			addChar(nextChar);
			break;
			
		default:
			token = DESCONHECIDO;
			printf("ERRO! Um caractere desconhecido foi inserido: ");
			addChar(nextChar);
			break;		
	}
}


int identOpImplicacao() {
//checa a ocorrencia consecultiva dos caracteres '-' e '>'(implicacao) e adiciona-os em lexemas (atraves do addChar())
	if(nextChar == '-') {
		addChar(nextChar);
		identChar();
		if(nextChar ==  '>') {
			addChar(nextChar);
			return 1;
		} else {
			ungetc(nextChar, file); //para "voltar" o ponteiro de leitura do arquivo e 
										//evitar o "erro" de leitura do arquivo causado pelo laço acima 
										//OBS: reverte o efeito da leitura a mais pela chamada "identificarChar();" quando o prox caractere NAO eh '>'
		}
	}
	
	return 0;
}


int identTrueOUFalse(int tipoParaIdent){ 
//Retorna 1 se os caracteres formam as palavras "True" ou "False" e carrega elas na String lexemas (utilizando addChar()) para poder ser tratada como um valor true ou false
//Retorna 0 se não formarem e além disso carrega toda a palavra na String lexemas (utilizando addChar()) para poder ser tratada como um identificador
//Parametro "tipoParaIdent": 0 para identificar se a palavra a ser testada sera "False" ou 1 para "True"
	int i=0;
	char palavraReservada[6] = "";
	int tamanhoPalavra = 0;
	
	if(tipoParaIdent == 1){
		strcpy(palavraReservada,"True");
		tamanhoPalavra = 4;
	} else {
		strcpy(palavraReservada,"False");
		tamanhoPalavra = 5;
	}
	
	if(tipoParaIdent==0 || tipoParaIdent==1){
		for(; i<tamanhoPalavra; i++){ //laço para checar todos os caracteres da possivel palavra reservada "True"
		if(palavraReservada[i] != nextChar){
			//Bloco de comando executado quando umcaracter eh diferente dos caracteres das palavras reservadas "True" ou "False"
			
			while(charClass == LETRA){ //Lê caracteres ate o momento em que eles nao sejam letras
				addChar(nextChar);		// para assegurar que toda a palavra (que agora sera tratada como identificador), seja lida corretamente
				identChar();
			}
			ungetc(nextChar, file); //para evitar o erro da leitura adicional pelo laço acima
	
				return 0; //sendo um caractere diferente, sai da funcao com um valor de retono falso
			} else {
				addChar(nextChar); //enquanto for igual, adiciona os caracteres no lexema e
				identChar(); //	passa para o proximo caracter
			}
		}
		ungetc(nextChar, file); //para evitar o erro da leitura de um caractere a mais
		return 1;
	}
	return 0;
}


void addChar(char ch){ //Adiciona os lexemas no array de caracteres "lexemas"
	if(indexLexemas < 99){
		lexemas[indexLexemas++] = ch;
		lexemas[indexLexemas] = '\0';
	} else {
		printf("ERRO: lexema muito longo - deve ter no max 100 caracteres");
		exit(1);
	}
}


int ignorarComentarios(){
//Checa se há a ocorrencia de "//" e lê caracteres até chegar em uma quebra de linha (\n)ou no fim de arquivo (EOF)
//Retorna 0 se nao houveram comentarios e Retorna 1 se houve algum comentario	
	if(nextChar == '/') {
		lerNovoChar();
		if(nextChar == '/'){
			do{
				lerNovoChar();
			}while(nextChar != 10 && charClass != EOF);
			return 1;
		} else {
			printf("ERRO: A sintaxe correta para comentarios eh: //{msg} "); //caso haja ocorrencia de apenas uma '/' haverá erro.
			exit(1);
		}
	}
	return 0;	
}
