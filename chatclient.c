#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUFF_SIZE 256
#define USERNAME_MAX_SIZE 20

static unsigned short port = 55555;
static char username[USERNAME_MAX_SIZE];

void changeName();
void MenuOption();
void Menu();

pthread_cond_t console_cv;
pthread_mutex_t console_cv_lock;

void MenuOption()
{
        int select;
        printf("\n\t === Menu === \n");
        printf("\t1. change name \n");
        printf("\t========================");
        printf("\n -> ");
        scanf("%d", &select);
        getchar();

        switch(select)
        {
                case 1:
                changeName();
                break;

                default:
                printf("\ncancel");
                break;
        }
}
void changeName()
{
        char nameTemp[20];
        printf("\n Enter a new username (max 20 characters, no spaces) : ");
        scanf("%s", nameTemp);
        printf(username, "[%s]", nameTemp);
        printf("hello [%s]!", nameTemp);
}

void Menu()
{
    printf("\t========== MENU ==========\n");
    printf("\t if you want to select menu -> !menu\n");
    printf("\t 1. change name\n");
    printf("\t===============================\n");
}

void error(void)
{
        fprintf(stderr, "%s\n", "\t Wrong command...\n"
                "syntax : [command] [optional recipient] [optional msg]");
}

void console(int sockfd)
{
        char buffer[BUFF_SIZE];
        char *recipient, *msg, *tmp;

        memset(buffer, 0, sizeof buffer);
        printf( "\tHello! Now you can send messages!\n");
        printf("syntax : [command] [optional recipient] [optional msg]\n");

        while(1) {
                printf("[%s]$ ", username);
                fgets(buffer, sizeof buffer, stdin);
                buffer[strlen(buffer) - 1] = '\0';

                if(strcmp(buffer, "") == 0)
                        continue;

if(strncmp(buffer, "exit", 4) == 0) {
                        write(sockfd, "exit", 6);
                        pthread_mutex_destroy(&console_cv_lock);
                        pthread_cond_destroy(&console_cv);
                        _exit(EXIT_SUCCESS);
                }

                if(strncmp(buffer, "ls", 2) == 0) {
                        pthread_mutex_lock(&console_cv_lock);
                        write(sockfd, "ls", 2);
                        pthread_cond_wait(&console_cv, &console_cv_lock);
                        pthread_mutex_unlock(&console_cv_lock);
                        continue;
                }

                if(strncmp(buffer, "send ", 5) == 0) {
                        tmp = strchr(buffer, ' ');
                        if(tmp == NULL) {
                                error();
                                continue;
                        }
                        recipient = tmp + 1;

                        tmp = strchr(recipient, ' ');
                        if(tmp == NULL) {
                                error();
                                continue;
                        }
                        msg = tmp + 1;

                        write(sockfd, buffer, 5 + strlen(recipient) + 1 + strlen(msg) + 1);
                        continue;
                }

                error();
        }
}

void register_username(int sockfd)
{
        char *regstring = malloc(USERNAME_MAX_SIZE + 18);
  sprintf(regstring, "register username %s", username);
        write(sockfd, regstring, strlen(regstring));
        free(regstring);
}

void *receiver(void *sfd)
{
        char buffer[BUFF_SIZE] = {0};
        int sockfd = *(int*)sfd;
        int readlen;
        while(1) {
                memset(buffer, 0, sizeof buffer);
                readlen = read(sockfd, buffer, sizeof buffer);
                if(readlen < 1)
                        continue;
                pthread_mutex_lock(&console_cv_lock);
                printf("%s\n", buffer);
                pthread_cond_signal(&console_cv);
                pthread_mutex_unlock(&console_cv_lock);
        }
}

int main(void)
{
        int sockfd;
        struct sockaddr_in serv_addr;

        pthread_t receiver_thread;

        pthread_cond_init(&console_cv, NULL);
        pthread_mutex_init(&console_cv_lock, NULL);

        sockfd = socket(AF_INET, SOCK_STREAM, 0);


        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);



        connect(sockfd, (struct sockaddr*) &serv_addr, sizeof serv_addr);

        printf("%s\n", "Enter a username (max 20 characters, no spaces):");
        fgets(username, sizeof username, stdin);
        username[strlen(username) - 1] = '\0';

        register_username(sockfd);
        Menu();

        pthread_create(&receiver_thread, NULL, receiver, (void*)&sockfd);
        console(sockfd);

        return 0;
}
