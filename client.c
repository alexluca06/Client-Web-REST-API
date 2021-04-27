#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

#define IP_SERVER "3.8.116.10"
#define PORT_SERVER 8080

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    char *cookies = malloc(100 * sizeof(char));
    char *token = malloc(400 * sizeof(char));
    int sockfd;
    while (1) {
        // citire comanda introdusa de client
        char* client_command = malloc(10*sizeof(char));
        printf("Introduceti comanda: ");
        scanf("%s", client_command);

        // implementare comanda register
        if (!strcmp(client_command, "register")) {
            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            char username[20];  // stocare username introdusa de utilizator
            char password[20];  //stocare parola introdusa de utilizator
            char *json = calloc(100, sizeof(char));  // stocare json
            printf("username = ");
            scanf("%s", username);
            printf("password = ");
            scanf("%s", password);
            // formare json cu username-ul si parola introduse de utilizator
            sprintf(json,"{ \"username\": \"%s\", \"password\": \"%s\" }", username, password);
            // creare si trimitere cerere POST catre server
            message = compute_post_request(IP_SERVER, "/api/v1/tema/auth/register", "application/json",json, NULL, 0, NULL);
            send_to_server(sockfd, message);
            // primire si afisare raspuns de la server
            response = receive_from_server(sockfd);
            char *error = strstr(response, "error");
            if (error != NULL) {
                char *error_end;
                error_end = strchr(error, '!');
                
                int end = error_end - error+1;
                error[end] = '\0';
                printf("%s\n", error);
            }
            else {
                printf("Contul a fost creat cu succes!\n");
            }
            
        // implementare comanda login
        } else if (!strcmp(client_command, "login")) {
            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            char username[20];  // stocare username introdusa de utilizator
            char password[20];  //stocare parola introdusa de utilizator
            char *json = calloc(100, sizeof(char));  // stocare json
            printf("username = ");
            scanf("%s", username);
            printf("password = ");
            scanf("%s", password);
            // formare json cu username-ul si parola introduse de utilizator
            sprintf(json,"{ \"username\": \"%s\", \"password\": \"%s\" }", username, password);
            // creare si trimitere cerere POST catre server
            message = compute_post_request(IP_SERVER, "/api/v1/tema/auth/login", "application/json", json, NULL, 0, NULL);
            send_to_server(sockfd, message);
            // primire raspuns de la server
            response = receive_from_server(sockfd);
            // extragere cookie din raspunsul primit de la server
            cookies = strstr(response, "Set-Cookie: ");
            if (cookies != NULL) {
                char *cookie_end;
                cookie_end = strchr(cookies, ';');
                cookies = cookies + 12;
                int end = cookie_end - cookies;
                cookies[end] = '\0';
            }
             char *error = strstr(response, "error");
            if (error != NULL) {
                char *error_end;
                error_end = strchr(error, '!');
                
                int end = error_end - error+1;
                error[end] = '\0';
                printf("%s\n", error);
            }
            else {
                printf("Buna ziua! Autentificare reusita!\n");
            }

        // implementare comanda enter_library
        } else if (!strcmp(client_command, "enter_library")) {

            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            // creare si trimitere cerere GET catre server
            message = compute_get_request(IP_SERVER, "/api/v1/tema/library/access", NULL, cookies, 1, NULL);
            send_to_server(sockfd, message);
            // primire raspuns de la server
            response = receive_from_server(sockfd);
            // extragere token din raspunsul primit de la server
            token = strstr(response, "token");
            if (token != NULL && cookies != NULL) {
                char *token_end;
                token = token + 8;
                token_end = strchr(token, '}');
                int end = token_end - token;
                token[end-1] = '\0';
            }
             char *error = strstr(response, "error");
            if (error != NULL) {
                char *error_end;
                error_end = strchr(error, '!');
                
                int end = error_end - error+1;
                error[end] = '\0';
                printf("%s\n", error);
            }
            else {
                printf("Bine ati venit in librarie!\n");
            }

        // implementare comanda get_books
        } else if (!strcmp(client_command, "get_books")) {

            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            // creare si trimitere cerere GET catre server
            message = compute_get_request(IP_SERVER, "/api/v1/tema/library/books", NULL, cookies, 1, token);
            send_to_server(sockfd, message);
            // primire raspuns de la server
            response = receive_from_server(sockfd);
             char *error = strstr(response, "error");
            if (error != NULL) {
                char *error_end;
                error_end = strchr(error, '!');
                
                int end = error_end - error+1;
                error[end] = '\0';
                printf("%s\n", error);
            }
            else {
                char *carti = strstr(response, "[{\"");
                if (carti != NULL) {
                    printf("Cartile au fost extrase cu succes!\n");
                    printf("%s\n", carti);
                }
                else {
                    printf("Momentan nu exista carti!\n");
                }
    
            }

        // implementare comanda get_book
        } else if (!strcmp(client_command, "get_book")) {

            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            // citire id carte
            int id;
            printf("id = ");
            scanf("%d", &id);
            if (id < 0) {
                printf("Id trebuie sa fie o valoare pozitiva. Introduceti din nou.\n");
                printf("id = ");
                scanf("%d", &id);
            }
            // formare cale catre cartea dorita
            char *cale = calloc(100,sizeof(char));
            sprintf(cale,"/api/v1/tema/library/books/%d", id);
            // creare si trimitere cerere GET catre server
            message = compute_get_request(IP_SERVER, cale, NULL, cookies, 1, token);
            send_to_server(sockfd, message);
            // primire raspuns de la server
            response = receive_from_server(sockfd);
            char *error = strstr(response, "error");
            if (error != NULL) {
                char *error_end;
                error_end = strchr(error, '!');
                
                int end = error_end - error+1;
                error[end] = '\0';
                printf("%s\n", error);
            }
            else {
                printf("Cartea a fost extrasa cu succes!\n");
                char *carti = strstr(response, "[{\"");
                if (carti != NULL) {
                printf("%s\n", carti);
                }
            }

        // implementare comanda add_book
        } else if (!strcmp(client_command, "add_book")) {

            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            char title[20];  
            char author[20];
            char genre[20]; 
            char publisher[20]; 
            int page_count;
            char *json_add_books = calloc(300, sizeof(char));  // stocare json
            // citire input de la utilizator
            printf("title = ");
            scanf("%s", title);
            printf("author = ");
            scanf("%s", author);
            printf("genre = ");
            scanf("%s", genre);
            printf("publisher = ");
            scanf("%s", publisher);
            printf("page_count = ");
            scanf("%d", &page_count);
            // verificare validitate page_count
            if (page_count < 0) {
                printf("Page_count trebuie sa fie o valoare pozitiva. Introduceti din nou.\n");
                printf("page_count = ");
                scanf("%d", &page_count);
            }
            // formare json cu datele despre carte introduse de client
            sprintf(json_add_books,"{ \"title\": \"%s\", \"author\": \"%s\", \"genre\": \"%s\", \"page_count\": \"%d\", \"publisher\": \"%s\" }",
                title, author, genre, page_count, publisher);
            // creare si trimitere cerere POST catre server
            message = compute_post_request(IP_SERVER, "/api/v1/tema/library/books", "application/json", json_add_books, cookies, 0, token);
            send_to_server(sockfd, message);
            // primire si afisare raspuns de la server
            response = receive_from_server(sockfd);
             char *error = strstr(response, "error");
            if (error != NULL) {
                char *error_end;
                error_end = strchr(error, '!');
                
                int end = error_end - error+1;
                error[end] = '\0';
                printf("%s\n", error);
            }
            else {
                printf("Cartea a fost adaugata cu succes!\n");
            }

        // implementare comanda delete_book
        } else if (!strcmp(client_command, "delete_book")) {

            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            // creare si trimitere cerere GET catre server
            int id;
            printf("id = ");
            scanf("%d", &id);
            char *cale = calloc(100,sizeof(char));
            sprintf(cale,"/api/v1/tema/library/books/%d", id);
            message = compute_delete_request(IP_SERVER, cale,"apllication/json", NULL, cookies, 1, token);
            send_to_server(sockfd, message);
            // primire si afisare raspuns de la server
            response = receive_from_server(sockfd);
             char *error = strstr(response, "error");
            if (error != NULL) {
                char *error_end;
                error_end = strchr(error, '!');
                
                int end = error_end - error+1;
                error[end] = '\0';
                printf("%s\n", error);
            }
            else {
                printf("Cartea a fost eliminata cu succes!\n");
            }

        // implementare comanda logout
        } else if (!strcmp(client_command, "logout")) {

            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            // creare si trimitere cerere GET catre server
            message = compute_get_request(IP_SERVER, "/api/v1/tema/auth/logout", NULL, cookies, 1, token);
            send_to_server(sockfd, message);
            // primire raspuns de la server
            response = receive_from_server(sockfd);
            char *error = strstr(response, "error");
            if (error != NULL) {
                char *error_end;
                error_end = strchr(error, '!');
                
                int end = error_end - error+1;
                error[end] = '\0';
                printf("%s\n", error);
            }
            else {
                printf("La revedere!Va asteptam mai tarziu!\n");
                cookies = NULL;
                token = NULL;
            }
            
        
        } else if (!strcmp(client_command, "exit")) {
            break;
        }
        // afisare pentru comenzi gresite
        else {
            printf("Comanda introdusa este gresita!\n");
        }

    }
    close_connection(sockfd);
  

    return 0;
}
