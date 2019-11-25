#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#define MAX 80 
#define PORT 3550 
#define SA struct sockaddr 

//Prototipos de funciones
int recorrer();
void impresion_tabla();
void completar_matriz(char datos[]);
void completar_matriz_ajena(char datos[]);
void setear_matriz();

//variable global
char matriz[3][3];

void func(int sockfd) 
{ 

/*    char buff[MAX]; 
    int n; 
    for (;;) { 
        bzero(buff, sizeof(buff)); 
        printf("Enter the string : "); 
        n = 0; 
        while ((buff[n++] = getchar()) != '\n') 
            ; 
        write(sockfd, buff, sizeof(buff)); 
        bzero(buff, sizeof(buff)); 
        read(sockfd, buff, sizeof(buff)); 
        printf("From Server : %s", buff); 
        if ((strncmp(buff, "exit", 4)) == 0) { 
            printf("Client Exit...\n"); 
            break; 
        } 
    }
*/
    setear_matriz();
    //Sacado de servidor
    char buff[MAX]; 
    int n, opcion, respuesta; 
    // infinite loop for chat 
    for (;;) { 
        bzero(buff, MAX); 
  
        // read the message from client and copy it in buffer 
        read(sockfd, buff, sizeof(buff));

        completar_matriz_ajena(buff);
        //printf("....%s....", buff); 
        // print buffer which contains the client contents 
        printf("From client: %s\t To client : ", buff);

        impresion_tabla();

        respuesta = recorrer();

        if(respuesta == 1){
   
            printf("\n----Cliente ha ganado la partida...!!!----\n");

        }else if(respuesta == 0){
                
            printf("\n---Empate---\n");
    
        }else if (respuesta == 2){

            printf("\n----Servidor ha ganado la partida...!!!----\n");

        }
        if (respuesta == 0 || respuesta == 1 || respuesta == 2){
            printf("Escriba exit en el siguiente turno para finalizar, caso contrario se reinicia el juego\n");
            setear_matriz();
        }

        //impresion_tabla();

        printf("Se debe ingresar los datos en el siguiente formato: N1 + - + N2, esto sin contar los signos +. \nEjemplo: 1-2\n");

        printf("\nIngresar fila, columna > ");
        bzero(buff, MAX); 
        n = 0; 
        // copy server message in the buffer 
        while ((buff[n++] = getchar()) != '\n') 
            ; 
  
        // and send that buffer to client 
        write(sockfd, buff, sizeof(buff)); 
        

        completar_matriz(buff);

        respuesta = recorrer();

        if(respuesta == 1){
   
            printf("\n----Cliente ha ganado la partida...!!!----\n");

        }else if(respuesta == 0){
                
            printf("\n---Empate---\n");
    
        }else if (respuesta == 2){

            printf("\n----Servidor ha ganado la partida...!!!----\n");

        }
        if (respuesta == 0 || respuesta == 1 || respuesta == 2){
            printf("Escriba exit en el siguiente turno para finalizar, caso contrario se reinicia el juego\n");
            setear_matriz();
        }
        impresion_tabla();

        // if msg contains "Exit" then server exit and chat ended. 
        if (strncmp("exit", buff, 4) == 0) { 
            printf("Server Exit...\n"); 
            break; 
        } 
    }     
} 
  
int main() 
{ 
    int sockfd, connfd; 
    struct sockaddr_in servaddr, cli; 
  
    // socket create and varification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    servaddr.sin_port = htons(PORT); 
  
    // connect the client socket to server socket 
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 
    else
        printf("connected to the server..\n"); 
  
    // function for chat 
    func(sockfd); 
  
    // close the socket 
    close(sockfd); 
} 

void setear_matriz(){
    int i, j;

    for (i = 0; i < 3; i++){
        for (j = 0; j < 3; j++){
            matriz[i][j] = '-';
        }
    }
}

//funciones de tateti
void completar_matriz(char dat[]){

    int fila, columna, i, j;
    fila = dat[0] - '0';
    columna = dat[2] - '0';
    //printf("%i %i \n", fila, columna);
    matriz[fila][columna] = '/';

}

int recorrer(){

    int i, j, contador = 0, valor_retorno = -1;
/*    
    for (i = 0; i < 3; i++){
        for (j = 0; j < 3; j++){
            printf("%c", matriz[i][j]);
        }
        printf("\n");
    }
*/
    if (((matriz[0][0] == matriz[0][1]) && ( matriz[0][1] == matriz[0][2]))){//control primera fila
        if (matriz[0][0] != '-'){
            if (matriz[0][0] == '/'){
                valor_retorno = 1;
            }else if (matriz[0][0] == 'x'){
                valor_retorno = 2;
            }
        }    
    }else if((matriz[1][0] == matriz[1][1]) && ( matriz[1][1] == matriz[1][2])){//control segunda fila
        if (matriz[1][0] != '-'){
            if (matriz[1][0] == '/'){
                valor_retorno = 1;
            }else if (matriz[1][0] == 'x'){
                valor_retorno = 2;
            }
        }      
    }else if (((matriz[2][0] == matriz[2][1]) && ( matriz[2][1] == matriz[2][2]))){//control tercera fila
        if (matriz[2][0] != '-'){
            if (matriz[2][0] == '/'){
                valor_retorno = 1;
            }else if (matriz[2][0] == 'x'){
                valor_retorno = 2;
            }
        }       
    }else if (((matriz[0][0] == matriz[1][0]) && ( matriz[1][0] == matriz[2][0]))){//control primera columna
        if (matriz[0][0] != '-'){
            if (matriz[0][0] == '/'){
                valor_retorno = 1;
            }else if (matriz[0][0] == 'x'){
                valor_retorno = 2;
            }
        }    
    }else if (((matriz[0][1] == matriz[1][1]) && ( matriz[1][1] == matriz[2][1]))){//control segunda columna
        if (matriz[0][1] != '-'){
            if (matriz[0][1] == '/'){
                valor_retorno = 1;
            }else if (matriz[0][1] == 'x'){
                valor_retorno = 2;
            }
        }    
    }else if(((matriz[0][2] == matriz[1][2]) && ( matriz[1][2] == matriz[2][2]))){//control tercera columna
        if (matriz[0][2] != '-'){
            if (matriz[0][2] == '/'){
                valor_retorno = 1;
            }else if (matriz[0][2] == 'x'){
                valor_retorno = 2;
            }
        }       
    }else if(((matriz[0][0] == matriz[1][1]) && ( matriz[1][1] == matriz[2][2]))){ //control diagonal principal
        if (matriz[0][0] != '-'){   
            if (matriz[0][0] == '/'){
                valor_retorno = 1;
            }else if (matriz[0][0] == 'x'){
                valor_retorno = 2;
            }
        }       
    }else if(((matriz[0][2] == matriz[1][1]) && ( matriz[1][1] == matriz[2][0]))){//control diagonal secundaria
        if (matriz[0][2] != '-'){
            if (matriz[0][2] == '/'){
                valor_retorno = 1;
            }else if (matriz[0][2] == 'x'){
                valor_retorno = 2;
            }
        }    
    }else{
        for (i = 0; i < 3; i++){
            for (j = 0; j < 3; j++){
                if (matriz[i][j] != '-'){
                    contador++;
                }
            }
        }
        if (contador == 9){
            valor_retorno = 0;//empate
        }
    }
    return valor_retorno;   
}

void impresion_tabla(){

    int i, j, k, l, bandera = 0;
    
    for (i = 0; i < 3; i++){

        for (j = 0; j < 3; j++){
            if (i == 0){
                printf("\tc%d", j);//imprime indicaciones de columnas c0, c1, c2
            }
        }
        if (i == 0){
            printf("\n\t▬▬▬▬▬▬▬  ▬▬▬▬▬▬  ▬▬▬▬▬▬▬");
        }
        printf("\n");
        for (k = 0; k < 4; k++){
            //printf("\t▌  %c", matriz[i][k]);
            printf("\t▌");
            if (k < 3){
                printf("  %c", matriz[i][k]);   
            }
        }

        printf("\nf%d\t▌\t▌\t▌\t▌", i);//imprime indicaciones de filas f0, f1, f2
        printf("\n\t▬▬▬▬▬▬▬  ▬▬▬▬▬▬  ▬▬▬▬▬▬▬");
    }
    printf("\n");
}

//Para el servidor, las casillas se completan con x y para el cliente con /
void completar_matriz_ajena(char datos[]){
    int fila, columna, i, j;
    fila = datos[0] - '0';
    columna = datos[2] - '0';
    //printf("%i %i \n", fila, columna);
    matriz[fila][columna] = 'x'; 
}
