#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#define NONE 1 		// 00000001
#define INST 65 	// 01000001
#define IF 2   		// 00000010
#define ELSEIF 6	// 00000110
#define ELSE 70		// 01000110
#define WHILE 8		// 00001000
#define DOWHILE 24	// 00011000
#define FOR 40		// 00101000

int stkpos=0;
char* neststk [256];

int doflag = 0;

void print_usage(char* name){
	fprintf(stderr, "Usage\n%s -f inputfilename -o outputfilename\n\n",name);
}
char *ltrim(char *s)
{
	while(isspace(*s)) s++;
	return s;
}

char *rtrim(char *s)
{
	char* back = s + strlen(s);
	while(isspace(*--back));
	*(back+1) = '\0';
	return s;
}

char *trim(char *s)
{
	return rtrim(ltrim(s)); 
}

char* tr_pran(char* str){
	int cnt = 0;
	do{
		if(*str == '\0')
			return NULL;
		if(*str == '(')
			cnt ++;
		else if(*str == ')')
			cnt --;
		str ++;
	}while(cnt !=0);
	return str-1;
}

void uml_app_if(FILE* fout, char* txt, int len){
	int nlen = len + 17;
	char* ifstr = malloc(nlen*sizeof(char));
	char* ptr = ifstr;
	memcpy(ptr, "if ", 3);
	ptr += 3;
	memcpy(ptr, txt, len);
	ptr += len;
	memcpy(ptr, " then (yes)\n\0", 13);
	fputs(ifstr, fout);
//	printf("in uml_app_if(%s)\n", txt);
	free(ifstr);
}

void uml_app_while(FILE* fout, char* txt, int len){
	int nlen = len + 7;
	char* ifstr = malloc(nlen*sizeof(char));
	char* ptr = ifstr;
	memcpy(ptr, "while ", 6);
	ptr += 6;
	memcpy(ptr, txt, len);
	ifstr[nlen-1] = '\n';
	ifstr[nlen] = '\0';
	fputs(ifstr, fout);
//	printf("in uml_app_while(%s)\n", txt);
	free(ifstr);
}

void uml_app_elseif(FILE* fout, char* txt, int len){
	int nlen = len + 20;
	char* ifstr = malloc(nlen*sizeof(char));
	char* ptr = ifstr;
	memcpy(ptr, "elseif ", 7);
	ptr += 7;
	memcpy(ptr, txt, len);
	ptr += len;
	memcpy(ptr, " then (yes)\n\0", 13);
	fputs(ifstr, fout);
//	printf("in uml_app_elseif(%s)\n", txt);
	free(ifstr);
}

void uml_app_rect(FILE* fout, char* txt, int len){
	fprintf(fout, ":%s;\n", txt);
}

void uml_endif(FILE* fout){
	fputs("endif", fout);
}

void handle_close(FILE* fout, char* buff){
	char* blockname = neststk[--stkpos];
	if( blockname[0] == '{' ){
		return;
	}
//	printf("%d\n", stkpos);
	if( strcmp(blockname, "for") == 0){
		fputs("repeat while (next)\n", fout);
	}else if(! strcmp(blockname, "do") ){
		fprintf(fout, "repeat %s\n", buff);
		doflag=1;
	}else{
		fprintf(fout, "end%s\n", blockname);
	}
}

void handle_if(FILE* fout, char* buff){
	buff = ltrim(buff+2);
	if(buff[0] == '('){
		char* stpos = buff;
		char* endpos = tr_pran(buff);
		int len = endpos - stpos+1;
		char iftxt[len];
		memcpy(iftxt, stpos, len);
		uml_app_if(fout, iftxt, len);
//		printf("in handle_if(%s)\n", buff);
	}else{
		fprintf(stderr, "Error in parsing if");
		exit(3);
	}
}

void handle_while(FILE* fout, char* buff){
	buff = ltrim(buff+5);
	if(buff[0] == '('){
		char* stpos = buff;
		char* endpos = tr_pran(buff);
		int len = endpos - stpos+1;
		char iftxt[len];
		memcpy(iftxt, stpos, len);
		uml_app_while(fout, iftxt, len);
//		printf("in handle_while(%s)\n", buff);
	}else{
		fprintf(stderr, "Error in parsing while");
		exit(3);
	}
}

void handle_elseif(FILE* fout, char* buff){
	buff = ltrim(buff+7);
	if(buff[0] == '('){
		char* stpos = buff;
		char* endpos = tr_pran(buff);
		int len = endpos - stpos+1;
		char iftxt[len];
		memcpy(iftxt, stpos, len);
		uml_app_elseif(fout, iftxt, len);
//		printf("in handle_elseif(%s)\n", buff);
	}else{
		fprintf(stderr, "Error in parsing elseif");
		exit(3);
	}
}

void handle_dowhile(FILE* fout, char* buff){
	fputs("repeat\n", fout);	
}


void handle_for(FILE* fout, char* buff){
	fprintf(fout, "repeat :%s ;\n",buff);	
}

int has_join(char* str){
	char* point = strstr(str, "//@&\\");
	if(point == NULL) return 0;
	*point = '\0';
	str = trim(str);
	int len = strlen(str);
	if(str[len-1] == ';') len--;
	str[len] = '\n';
	str[len+1] = '\0';
	return 1;
}

int main(int argc, char* argv[]){
	char* appname = argv[0];
	char* filename = "none";
	char* outfilename="none";
	int c;
	
	while((c = getopt(argc, argv, "f:o:")) != -1){
		switch(c){
			case 'f':
				filename = optarg;
				break;
			case 'o':
				outfilename = optarg;
				break;
			case '?':
				print_usage(appname);
				exit(1);
		}
	}

	if(!(strcmp(filename, "none") | strcmp(outfilename, "none"))){
		fprintf(stderr, "option values cant be \"none\"");
		exit(2);
	}
	puts("NOTE: we only support pseudo code for now");
	FILE* fin = fopen(filename, "r");
	if(fin == NULL){
		fprintf(stderr, "error opening input file");
		exit(errno);
	}
	FILE* fout = fopen(outfilename, "w");
	if(fout == NULL){
		fprintf(stderr, "error opening output file");
		exit(errno);
	}
	const int buffsz = 256;
	char* buffst= malloc(buffsz*sizeof(char));
	char* buff = buffst;

	char* chunkbuff = malloc(4096*sizeof(char));
	chunkbuff[0] = '\0';

	fputs("@startuml\n", fout);
	fputs("start\n", fout);
	int ifindex =0;
	int pendingclose = 0;

	int skiplines = 0;

	int previous = NONE;
	int joinflag = 0;
	while( fgets(buff, buffsz, fin) != NULL){
		buff = trim(buff);
		int inlen = strlen(buff);
		if(buff[0] == '\0'){
			if(pendingclose){
				handle_close(fout, buff);
			}
			continue;
		}
		if(inlen >=6 && buff[0] == '/' && buff[1] == '/' && buff[2] == '@' && buff[3] == '&' && buff[4] == 's' && buff[5] == '}'){ // skip directive
			skiplines = 0;
			continue;
		}
		if(skiplines) continue;
		if(buff[0] == '{'){
			if(previous & 1){
				neststk[stkpos++] = "{";
			}
		}
		else if(buff[0] == '}'){
			if(pendingclose){
				handle_close(fout, buff);
			}
			pendingclose=1;
		}
		
		if(inlen < 2) continue;
		if(inlen >=6 && buff[0] == '/' && buff[1] == '/' && buff[2] == '@' && buff[3] == '&' && buff[4] == 's' && buff[5] == '{'){ // skip directive
			skiplines = 1;
		}
		// comments
		if(buff[0] == '/' && buff[1] == '*'){ // skip long comment
			buff = buffst;
			while(buff[0] != '*' || buff[1] != '/'){
				fgets(buff, buffsz, fin);
			}
			continue;
		}
		if(buff[0] == '/' && buff[1] == '/') {
			if(buff[2] == '@' && buff[3] == '&'){ // parser directive
				if(buff[4] == '{'){
					buff = buffst;
					chunkbuff[0] = '\0';
					while(1){
						fgets(buff,buffsz,fin);
					        buff= trim(buff);
							
						if(! strcmp(buff, "//@&}") )
							break;
						inlen = strlen(buff);
						if(buff[inlen-1] == ';') inlen--;
						buff[inlen] = '\"';
						buff[inlen+1] = '\n';
						buff[inlen+2] = '\0';
						strcat(chunkbuff,"\"");
						strcat(chunkbuff, buff);
					}
					int chunklen = strlen(chunkbuff)-1;
					chunkbuff[chunklen] = '\0';
					uml_app_rect(fout, chunkbuff, chunklen);
					continue;
				}
			}else{ // regular comment
				continue;
			}
		}
		
		if(pendingclose){
			if(inlen >= 6 && buff[0] == 'e' && buff[1] == 'l' && buff[2] == 's' && buff[3] == 'e'
					&& buff[4] == ' ' && buff[5] == 'i' && buff[6] == 'f') // elseif
			{
				handle_elseif(fout, buff);
				pendingclose = 0;
				previous = ELSEIF;
				continue;
			}else if(inlen >=4 && buff[0] == 'e' && buff[1] == 'l' && buff[2] == 's' && buff[3] == 'e'){ //else
				fputs("else\n", fout);
				pendingclose = 0;
				previous = ELSE;
				continue;
			}else{
				handle_close(fout, buff);
				pendingclose=0;
			}	
		}

		// capture if
		if(buff[0] == 'i' && buff[1] == 'f'){
			handle_if(fout, buff);
			previous = IF;
			neststk[stkpos++] = "if";
		}else if(inlen>=5 && buff[0] == 'w' && buff[1] == 'h' && buff[2] == 'i' && buff[3] == 'l' && buff[4] == 'e'){
			if(doflag){
				doflag = 0;
			}else{
				handle_while(fout,buff);
				previous = WHILE;
				neststk[stkpos++] = "while";
			}
		}else if(buff[0] == 'd' && buff[1] == 'o'){
			previous = DOWHILE;
			neststk[stkpos++] = "do";
			handle_dowhile(fout, buff);
		}else if(buff[0] == 'f' && buff[1] == 'o' && buff[2] == 'r'){
			previous = FOR;
			handle_for(fout,buff);
			neststk[stkpos++] = "for";
		}else{ // instruction block
			// check for join directive
			if( has_join(buff) ){
				if(joinflag)
					strcat(chunkbuff, buff);
				else{
					strcpy(chunkbuff, buff);
					joinflag = 1;
				}
				continue;
			}
			int nnlen = strlen(buff);
			if(buff[nnlen-1] == ';') buff[nnlen-1] = '\0';
			if(joinflag){
				strcat(chunkbuff, buff);
				uml_app_rect(fout, chunkbuff, strlen(chunkbuff));
				joinflag = 0;
			}else{
				uml_app_rect(fout, buff, strlen(buff));
			}
			previous = INST;
		}

		buff = buffst;

	}
	fprintf(fout, "stop\n\n@enduml\n");
	fclose(fin);
	fclose(fout);
	free(buffst);
	return 0;
}
