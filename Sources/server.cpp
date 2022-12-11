#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "../Headers/user.h"
#include "../Headers/book.h"

//using namespace std;

using std::cout;
using std::endl;
using std::cin;

/* portul folosit */
#define PORT 2728
#define MAX_SIZE 100
#define MAX_FILE_SIZE 1024
#define BOOK_FILE_PATH "./Data/books.xml"
#define USER_FILE_PATH "./Data/users.xml"
#define TEMP_USERS_FILE_PATH "./Data/tempUsers.xml"

extern int errno;

typedef struct thData{
	int idThread; // thread ID
	int cl; // client descriptor
} thData;

int num_users = 0;
user users[MAX_SIZE];
FILE *usersFP;

int num_books = 0;
book books[MAX_SIZE];

int serverInit(int &sd, struct sockaddr_in &clientInfo);
void serveClients(int sd, struct sockaddr_in &clientInfo);
bool authenticate(char *username, char *password);
bool login(void *, int &clientId);
void processThread(void *, bool &LOGGED_IN, int &clientId);
static void *treatClient(void *); /* function executed by every thread */
book getBookInfo(void *);
void recommendBook(void *, int clientId);
bool sendData(int sock, void *buf, int buflen);
bool sendBook(int sock, book b);
char *readThread(void *);
void respondThread(void *, const char s[]);

int main () {
  int sd;		                      // socket descriptor
  struct sockaddr_in clientInfo;	// client information
  serverInit(sd, clientInfo);

  serveClients(sd, clientInfo);
}

void serveClients(int sd, struct sockaddr_in &clientInfo) {
  /* serving clients concurrently using threads */
  int i = 0;
  pthread_t th[MAX_SIZE];              // thread ID

  while (1) {
    int client;                       // client descriptor
    thData * threadInfo = new thData; // parameters for thread
    unsigned int length = sizeof (clientInfo);

    printf ("[serveClients] Waiting for clients at port %d... \n",PORT);
    fflush (stdout);

    if ( (client = ::accept (sd, (struct sockaddr *) &clientInfo, &length)) < 0) {
      perror ("[serveClients] Accept() Error.\n");
      continue;
    }

    threadInfo->idThread = i++;
    threadInfo->cl = client;
    
    pthread_create(&th[i], NULL, &treatClient, threadInfo); // creating the thread
  } //while    
}


int serverInit(int &sd, struct sockaddr_in &clientInfo) {
  if(access(TEMP_USERS_FILE_PATH, F_OK) != -1) {
    // file exists -> get the users with their udpdated preferences from the temporary file
    FILE *tempUsersFP = fopen(TEMP_USERS_FILE_PATH, "r");
    FILE *usersFP = fopen(USER_FILE_PATH, "w+");
    char buf[MAX_FILE_SIZE];
    while (fgets(buf, MAX_FILE_SIZE, tempUsersFP) != NULL) {
      fputs(buf, usersFP);
    }
    user::loadArrayfromXML(usersFP, users, num_users);
    fclose(tempUsersFP);
    fclose(usersFP);
    remove(TEMP_USERS_FILE_PATH);
  } else {
    // file doesn't exist -> get the users from the original file
    usersFP = fopen(USER_FILE_PATH, "r");
    rewind(usersFP);
    fflush(usersFP);
    user::loadArrayfromXML(usersFP, users, num_users);
    fclose(usersFP);
  }


  FILE *booksFP = fopen(BOOK_FILE_PATH, "r");
  book::loadBooksfromXML(booksFP, books, num_books);

  struct sockaddr_in serverInfo;	// server structure
  
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
    perror ("[serverInit] Socket() Error.\n");
    return errno;
  }

  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  // preparing the data structures
  bzero (&serverInfo, sizeof (serverInfo));
  bzero (&clientInfo, sizeof (clientInfo));
  
  serverInfo.sin_family = AF_INET;	
  serverInfo.sin_addr.s_addr = htonl (INADDR_ANY);
  serverInfo.sin_port = htons (PORT);
  
  // attaching the socket
  if (bind (sd, (struct sockaddr *) &serverInfo, sizeof (struct sockaddr)) == -1) {
    perror ("[serverInit] Bind() Error.\n");
    return errno;
  }

  if (listen (sd, 2) == -1) {
    perror ("[serverInit] Listen() Error.\n");
    return errno;
  }
  return 0;
}


static void *treatClient(void * arg) {
    bool LOGGED_IN = false;
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		pthread_detach(pthread_self());		
    int clientId = -1;

    processThread((struct thData*)arg, LOGGED_IN, clientId);
    
		/* finished with this client */
		close ((intptr_t)arg);
		return(NULL);
};

void processThread(void *arg, bool &LOGGED_IN, int &clientId) {
  char receivedMessage[MAX_SIZE] = "";
  struct thData tdL; 
  tdL= *((struct thData*)arg);	
  clientId = -1;
  book b;


  do {
    receivedMessage[0] = '\0';
    printf ("[thread]- %d - Waiting for message\n", tdL.idThread);
    strlcpy(receivedMessage, readThread((struct thData*)arg), MAX_SIZE);
    if(LOGGED_IN) {
      if(!strcmp(receivedMessage, "book_search")) {
        b = getBookInfo((struct thData*)arg);
        if(book::searchForBook(b, books, num_books)) {
          respondThread((struct thData*)arg, "book_found");
          respondThread((struct thData*)arg, b.getTitle());
          respondThread((struct thData*)arg, b.getAuthor());
          respondThread((struct thData*)arg, b.getISBN());
          cout<<"\nBOOK_FOUND\n\n";
        }
        else {
          respondThread((struct thData*)arg, "book_not_found");
          cout<<"\nBOOK_NOT_FOUND\n\n";
        }
      }
      else if(!strcmp(receivedMessage, "book_download")) {
        char fileName[MAX_SIZE] = "";
        strcpy(fileName, "../../../");
        strcat(fileName, b.getTitle());
        strcat(fileName, " - ");
        strcat(fileName, b.getAuthor());
        strcat(fileName, ".mobi");
        cout<<"FILE NAME :\""<<fileName<<"\""<<endl;
        respondThread((struct thData*)arg, fileName); // filename
        sendBook(tdL.cl, b);
        cout<<"BOOK SENT\n";
        users[clientId].addPreference(b.getGenre(), b.getAuthor());
        users[clientId].printUser();

        FILE *tempUsersFP = fopen(TEMP_USERS_FILE_PATH, "w");
        user::writeArraytoXML(tempUsersFP, users, num_users);
        fclose(tempUsersFP);
        cout<<"num_users= "<<num_users<<endl;
      }
      else if(!strcmp(receivedMessage, "book_recommend")) {
        respondThread((struct thData*)arg, "received");
        recommendBook((struct thData*)arg, clientId);

      }
      else if(!strcmp(receivedMessage, "logout")) {
        LOGGED_IN = false;
        cout<<"Logged_out\n";
      }
      else {
        respondThread((struct thData*)arg, "LOGGED_IN");
      }
    }
    else {
      if(!strcasecmp(receivedMessage, "login")) {
        LOGGED_IN = login((struct thData*)arg, clientId);
      }
      else if(!strcasecmp(receivedMessage, "create_account")) {
        char username[MAX_SIZE] = "";
        char password[MAX_SIZE] = "";
        strlcpy(username, readThread((struct thData*)arg), MAX_SIZE);
        strlcpy(password, readThread((struct thData*)arg), MAX_SIZE);
        if(user::createAccount(username, password, num_users, users)) {
          FILE *tempUsersFP = fopen(TEMP_USERS_FILE_PATH, "w+");
          users[num_users-1].printUser();
          user::writeArraytoXML(tempUsersFP, users, num_users);
          fclose(tempUsersFP);
          respondThread((struct thData*)arg, "created_account");
        }
        else {
          respondThread((struct thData*)arg, "account_not_created");
        }
      }
      else {
        respondThread((struct thData*)arg, "NOT_LOGGED_IN");
      }
    }
  } while(strcmp(receivedMessage, "close"));

  respondThread((struct thData*)arg, "closed.");
}


bool authenticate(char *username, char *password, int &clientId) {
  clientId = -1;
  cout<<"[authenticate]"<<username<<" "<<password<<num_users<<endl;
  if(!strcasecmp(username, "") || !strcasecmp(password, ""))
    return false;

  for(int i = 0; i < num_users; i++) {
    if(!strcasecmp(users[i].getUsername(), username) && !strcasecmp(users[i].getPassword(), password)) {
      clientId = i;
      return true;
    }
    cout<<"\n\nCOMPARA: ------ "<<users[i].getPassword()<<" "<<password<<endl;
  }
  
  return false;
}

bool login(void *arg, int &clientId) {
  clientId = -1;
  char username[MAX_SIZE] = "";
  char password[MAX_SIZE] = "";
  struct thData tdL; 
  tdL= *((struct thData*)arg);	
  // respondThread((struct thData*)arg, "try");

  strcpy(username, readThread((struct thData*)arg)); /// getUsername
  respondThread((struct thData*)arg, username);      /// sendUsername
  cout<<"READ USERNAME "<<username<<endl;

  strcpy(password, readThread((struct thData*)arg)); /// getPassword
  respondThread((struct thData*)arg, password);      /// sendPassword
  cout<<"READ PASSWORD "<<password<<endl;
  
  // yes or no
  if(authenticate(username, password, clientId)) {
    respondThread((struct thData*)arg, "TRUE");
    return true;
  }
  else {
    respondThread((struct thData*)arg, "FALSE");
    return false;
  }
}


book getBookInfo(void *arg) {
  char bookTitle[MAX_SIZE] = "";
  char bookAuthor[MAX_SIZE] = "";
  char bookISBN[MAX_SIZE] = "";

  strlcpy(bookTitle, readThread((struct thData*)arg), MAX_SIZE);
  respondThread((struct thData*)arg, bookTitle);
  strlcpy(bookAuthor, readThread((struct thData*)arg), MAX_SIZE);
  respondThread((struct thData*)arg, bookAuthor);
  strlcpy(bookISBN, readThread((struct thData*)arg), MAX_SIZE);
  respondThread((struct thData*)arg, bookISBN);
  book b(bookTitle, bookAuthor, bookISBN);

  return b;
}

bool sendData(int sock, void *buf, int buflen) {
  unsigned char *pbuf = (unsigned char *)buf;

  while (buflen > 0)
  {
    int num = write(sock, pbuf, buflen);
    if(num < 0)
      perror("[sendData] num");
    pbuf += num;
    buflen -= num;
  }

  return true;
}


void recommendBook(void *arg, int clientId) {
  int nMatchingBooks = 0;
  bool bestMatch = false;
  book possibleRecommendations[10];
  int preferedGenreID = users[clientId].getPreferedGenre();
  int preferedAuthorID = users[clientId].getPreferedAuthor();
  char preferedGenre[MAX_SIZE] = "";
  char preferedAuthor[MAX_SIZE] = "";

  strcpy(preferedGenre, users[clientId].getGenre(preferedGenreID));
  strcpy(preferedAuthor, users[clientId].getAuthor(preferedAuthorID));
  cout<<"PREFERED GENRE: "<<preferedGenre<<endl;
  cout<<"PREFERED AUTHOR: "<<preferedAuthor<<endl;

  /// preferably getting a book from the prefered genre and author, but if there are no such matches, getting a random book from the prefered genre

  for(int i = 0; i < num_books; i++) {
    if(!strcmp(books[i].getGenre(), preferedGenre) && 
      !strcmp(books[i].getAuthor(),preferedAuthor)) {
        possibleRecommendations[nMatchingBooks++] = books[i];
        bestMatch = true;
        cout<<"Best match\n";
    }
  }

  if(bestMatch == false) {
    for(int i = 0; i < num_books; i++) {
      if(!strcmp(books[i].getGenre(), preferedGenre)) {
          possibleRecommendations[nMatchingBooks++] = books[i];
      }
    }
  }

  cout<<"Number of matching books = "<<nMatchingBooks<<endl;
  int randomBook = rand() % nMatchingBooks;
  cout<<"Random book ID:"<<randomBook<<endl;

  respondThread((struct thData*)arg, possibleRecommendations[randomBook].getTitle());
  respondThread((struct thData*)arg, possibleRecommendations[randomBook].getAuthor());
  respondThread((struct thData*)arg, possibleRecommendations[randomBook].getISBN());
  cout<<"\nBOOK_RECOMMENDED\n\n";
}

bool sendBook(int sock, book b) {
  cout<<"OK\n";
  // struct thData tdL; 
	// tdL= *((struct thData*)arg);

  char directoryName[MAX_SIZE] = "";
  char fileName[MAX_SIZE] = "";
  strcpy(directoryName, "./Books/");
  strcat(directoryName, b.getAuthor());
  strcat(directoryName, "/");
  strcat(directoryName, b.getTitle());
  // strcpy(directoryName, "./Books/Frank_Herbert/Dune"); // testing
  cout<<"DIRECTORY NAME :\""<<directoryName<<"\""<<endl;

  strcpy(fileName, b.getTitle());
  strcat(fileName, " - ");
  strcat(fileName, b.getAuthor());
  strcat(fileName, ".mobi");
  // strcpy(fileName, "cover.jpg"); // testing
  cout<<"FILE NAME :\""<<fileName<<"\""<<endl;

  char filePath[MAX_SIZE] = "";
  strcpy(filePath, directoryName);
  strcat(filePath, "/");
  strcat(filePath, fileName);
  cout<<"FILE PATH :\""<<filePath<<"\""<<endl;

  
  
  FILE *bookFile = fopen(filePath, "rb");

  // fseek(bookFile, 0L, SEEK_END);
  // long FILESIZE = ftell(bookFile);
  // rewind(bookFile);

  // get the filesize of bookFile
  long long FILESIZE = -1;
  fseek(bookFile, 0, SEEK_END);
  FILESIZE = ftell(bookFile);
  rewind(bookFile);
  cout<<"FILESIZE : "<<FILESIZE<<endl;


  if (FILESIZE == EOF)
    return false;

  // Books/Frank Herbert/Dune/Dune - Frank Herbert.mobi
  //../Books/Frank Herbert/Dune/Dune - Frank Herbert.mobi

  long long nValue = htonl(FILESIZE);
  if (!sendData(sock, &nValue, sizeof(nValue)))
    return false;


  printf("[server] Sending file of size %lld\n", FILESIZE);
  if (FILESIZE > 0) {
    char buffer[MAX_FILE_SIZE];
    do {
      long long MAX_SENT_SIZE;
      if (FILESIZE <= sizeof(buffer))
        MAX_SENT_SIZE = FILESIZE;
      else
        MAX_SENT_SIZE = sizeof(buffer);
      /// reading the bytes from source file up until MAX_SIZE
      long long bytes_read = fread(buffer, 1, MAX_SENT_SIZE, bookFile);
      if (bytes_read < 1)
        return false;
      if (!sendData(sock, buffer, bytes_read))
        return false;
      FILESIZE -= bytes_read;
    } while (FILESIZE > 0);
  }
  
  printf("[server] File sent!\n");
  return true;
}


char *readThread(void *arg) {
  char *message = new char[MAX_SIZE];
  message[0] = '\0';
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	if (read (tdL.cl, message, MAX_SIZE) <= 0)
  {
    printf("[Thread %d]\n",tdL.idThread);
    perror ("Eroare la read() de la client.\n");
  }
	printf ("[Thread %d]Mesajul a fost receptionat...%s\n",tdL.idThread, message);
  return message;
}

void respondThread(void *arg, const char message[]) {
	struct thData tdL; 
	tdL= *((struct thData*)arg);

	printf("[Thread %d]Trimitem mesajul inapoi...%s\n",tdL.idThread, message);
  /* returnam mesajul clientului */
  if (write (tdL.cl, message, MAX_SIZE) <= 0) {
    printf("[Thread %d] ",tdL.idThread);
    perror ("[Thread]Eroare la write() catre client.\n");
  }
	else
		printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	
}
