#include "../Headers/user.h"
#include <iostream>
#include <string.h>
#include <cstring>
#include <fstream>


#define MAX_SIZE 100

using std::cout;
using std::endl;
using std::cin;

user::user() {
  strcpy(username, "NULL");
  strcpy(password, "NULL");
  numPreferedAuthors = 0;
  numPreferedGenres = 0;
  numReadBooks = 0;
  for(int i = 0; i < MAX_SIZE; i++) {
    strcpy(genreArray[i].genre, "NULL");
    genreArray[i].level = 0;
    strcpy(authorArray[i].author, "NULL");
    authorArray[i].level = 0;
    strcpy(readBooksArray[i].ISBN, "NULL");
  }
}

user::user(char u[], char p[], int nA, int nG, struct genrePreference g[], struct authorPreference a[]) {
  strlcpy(username, u, MAX_SIZE);
  strlcpy(password, p, MAX_SIZE);
  numPreferedAuthors = nA;
  numPreferedGenres = nG;
  numReadBooks = 0;
  for(int i = 0; i < MAX_SIZE; i++) {
    strlcpy(genreArray[i].genre, g[i].genre, MAX_SIZE);
    genreArray[i].level = g[i].level;
    strlcpy(authorArray[i].author, a[i].author, MAX_SIZE);
    authorArray[i].level = a[i].level;
    strlcpy(readBooksArray[i].ISBN, "NULL", MAX_SIZE);
  }
}

void user::loadArrayfromXML(FILE *fp, user users[], int &num_users) {
  cout<<"LOADING USERS FROM XML\n";
  char *line = new char[10001];
  size_t len = 0;
  ssize_t read;
  char *token = new char[10001];
  int i = 0;
  rewind(fp);
  fflush(fp);
  read = getline(&line, &len, fp);
  while (strstr(line, "</document>") == NULL) {
    read = getline(&line, &len, fp);
    if(read == -1) {
      cout<<"Error reading from file"<<endl;
      break;
    }
    token = NULL;

    // get the username
    if (strstr(line, "<username>") != NULL) {
      token = strtok(line, ">");
      token = strtok(NULL, "<");
      strlcpy(users[i].username, token, MAX_SIZE);
    }
    // get the password
    if (strstr(line, "<password>") != NULL) {
      token = strtok(line, ">");
      token = strtok(NULL, "<");
      strlcpy(users[i].password, token, MAX_SIZE);
    }
    // get the number of genres
    if (strstr(line, "<num_genres>") != NULL) {
      token = strtok(line, ">");
      token = strtok(NULL, "<");
      users[i].numPreferedGenres = atoi(token);
    }
    // get the number of authors
    if (strstr(line, "<num_authors>") != NULL) {
      token = strtok(line, ">");
      token = strtok(NULL, "<");
      users[i].numPreferedAuthors = atoi(token);
    }

    // get the number of read books
    if (strstr(line, "<num_read_books>") != NULL) {
      token = strtok(line, ">");
      token = strtok(NULL, "<");
      users[i].numReadBooks = atoi(token);
    }

    // get the genres
    if (strstr(line, "<prefered_genres>") != NULL) {
      for (int j = 0; j < users[i].numPreferedGenres; j++) {
        // get the genre
        read = getline(&line, &len, fp);
        if (strstr(line, "<genre>") != NULL) {
          token = strtok(line, ">");
          token = strtok(NULL, "<");
          strlcpy(users[i].genreArray[j].genre, token, MAX_SIZE);
        }
        read = getline(&line, &len, fp);
        // get the level
        if (strstr(line, "<level>") != NULL) {
          token = strtok(line, ">");
          token = strtok(NULL, "<");
          users[i].genreArray[j].level = atoi(token);
        }
      } // for
    } // if

    // get the authors
    if(strstr(line, "<prefered_authors>") != NULL) {
      for (int j = 0; j < users[i].numPreferedAuthors; j++) {
        // get the author
        read = getline(&line, &len, fp);
        if (strstr(line, "<author>") != NULL) {
          token = strtok(line, ">");
          token = strtok(NULL, "<");
          strlcpy(users[i].authorArray[j].author, token, MAX_SIZE);
        }
        read = getline(&line, &len, fp);
        // get the level
        if (strstr(line, "<level>") != NULL) {
          token = strtok(line, ">");
          token = strtok(NULL, "<");
          users[i].authorArray[j].level = atoi(token);
        }
      } // for
    } // if

    // get the read books
    if(strstr(line, "<read_books>") != NULL) {
      for (int j = 0; j < users[i].numReadBooks; j++) {
        // get the ISBN of the read book
        read = getline(&line, &len, fp);
        if (strstr(line, "<ISBN>") != NULL) {
          token = strtok(line, ">");
          token = strtok(NULL, "<");
          strlcpy(users[i].readBooksArray[j].ISBN, token, MAX_SIZE);
        }
      } // for
    } // if

    if (strstr(line, "</user>") != NULL) {
      i++;
    } // if
  } // while
  delete[] token;
  num_users = i;
  cout<<num_users<<" users loaded from XML"<<endl;
}


void user::writeArraytoXML(FILE *fp, user users[], int num_users) {
  cout<<"Writing to XML"<<endl;
  rewind(fp);
  fflush(fp);
  fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n");
  fprintf(fp, "<document>\n");
  for (int i = 0; i < num_users; i++) {
    fprintf(fp, "\t<user>\n");
    fprintf(fp, "\t\t<username>%s</username>\n", users[i].username);
    fprintf(fp, "\t\t<password>%s</password>\n", users[i].password);
    fprintf(fp, "\t\t<num_genres>%d</num_genres>\n", users[i].numPreferedGenres);
    fprintf(fp, "\t\t<num_authors>%d</num_authors>\n", users[i].numPreferedAuthors);
    fprintf(fp, "\t\t<num_read_books>%d</num_read_books>\n", users[i].numReadBooks);
    fprintf(fp, "\t\t<prefered_genres>\n");
    for (int j = 0; j < users[i].numPreferedGenres; j++) {
      fprintf(fp, "\t\t\t<genre>%s</genre>\n", users[i].genreArray[j].genre);
      fprintf(fp, "\t\t\t<level>%d</level>\n", users[i].genreArray[j].level);
    }
    fprintf(fp, "\t\t</prefered_genres>\n");
    fprintf(fp, "\t\t<prefered_authors>\n");
    for(int j = 0; j < users[i].numPreferedAuthors; j++) {
      fprintf(fp, "\t\t\t<author>%s</author>\n", users[i].authorArray[j].author);
      fprintf(fp, "\t\t\t<level>%d</level>\n", users[i].authorArray[j].level);
    }
    fprintf(fp, "\t\t</prefered_authors>\n");
    fprintf(fp, "\t\t<read_books>\n");
    for (int j = 0; j < users[i].numReadBooks; j++) {
      fprintf(fp, "\t\t\t<ISBN>%s</ISBN>\n", users[i].readBooksArray[j].ISBN);
    }
    fprintf(fp, "\t\t</read_books>\n");
    fprintf(fp, "\t</user>\n\n");
  }
  fprintf(fp, "</document>\n");
  fclose(fp);
  cout<<"Done writing to XML"<<endl;
}

void user::printUser() {
  cout<<"---------------\n";
  cout << "Username: " << username << endl;
  cout << "Password: " << password << endl;
  cout << "Number of genres: " << numPreferedGenres << endl;
  for (int i = 0; i < numPreferedGenres; i++) {
    cout << "Genre " << i+1 << ": " << genreArray[i].genre << " " << genreArray[i].level << endl;
  }
  cout << "Number of authors: " << numPreferedAuthors << endl;
  for (int i = 0; i < numPreferedAuthors; i++) {
    cout << "Author " << i+1 << ": " << authorArray[i].author << " " << authorArray[i].level << endl;
  }
  cout << "Number of read books: " << numReadBooks << endl;
  for (int i = 0; i < numReadBooks; i++) {
    cout << "Read book " << i+1 << ": " << readBooksArray[i].ISBN << endl;
  }
  cout<<"---------------\n";
}

void user::addPreference(char g[], char a[]) {
  bool genreAlreadyExists = false;
  bool authorAlreadyExists = false;
  for(int i = 0; i < numPreferedGenres; i++) {
    if (!strcmp(genreArray[i].genre, g)) {
      genreAlreadyExists = true;
      genreArray[i].level++;
    }
  }
  if (genreAlreadyExists == false) {
    strlcpy(genreArray[numPreferedGenres].genre, g, MAX_SIZE);
    genreArray[numPreferedGenres].level = 1;
    numPreferedGenres++;
  }
  for(int i = 0; i < numPreferedAuthors; i++) {
    if (!strcmp(authorArray[i].author, a)) {
      authorAlreadyExists = true;
      authorArray[i].level++;
    }
  }
  if(authorAlreadyExists == false) {
    strlcpy(authorArray[numPreferedAuthors].author, a, MAX_SIZE);
    authorArray[numPreferedAuthors].level = 1;
    numPreferedAuthors++;
  }
}

bool user::bookAlreadyRead(char ISBN[]) {
  bool bookAlreadyExists = false;
  for(int i = 0; i < numReadBooks; i++) {
    if(!strcmp(ISBN, readBooksArray[i].ISBN)) {
      cout<<"Book already exists"<<endl;
      bookAlreadyExists = true;
    }
  }
  return bookAlreadyExists;
}

void user::addReadBook(char ISBN[]) {
  if(bookAlreadyRead(ISBN) == false) {
    strlcpy(readBooksArray[numReadBooks].ISBN, ISBN, MAX_SIZE);
    numReadBooks++;
  }
}


int user::getPreferedGenre() {
  int maxLevel = -1;
  int maxIndex = -1;
  for(int i = 0; i < numPreferedGenres; i++) {
    if (genreArray[i].level > maxLevel) {
      maxLevel = genreArray[i].level;
      maxIndex = i;
    }
  }
  return maxIndex;
}

int user::getPreferedAuthor() {
  int maxLevel = -1;
  int maxIndex = -1;
  for(int i = 0; i < numPreferedAuthors; i++) {
    if (authorArray[i].level > maxLevel) {
      maxLevel = authorArray[i].level;
      maxIndex = i;
    }
  }
  return maxIndex;
}

int user::createAccount(char username[], char password[], int &num_users, user users[]){
  if(num_users < MAX_SIZE) {
    for(int i = 0; i < num_users; i++) {
      if(!strcasecmp(username, users[i].getUsername()))
        return -1; /// username already exists
    }
    num_users++;
    cout<<"user: "<<username<<endl;
    cout<<"password: "<<password<<endl;
    users[num_users-1].setUsername(username);
    users[num_users-1].setPassword(password);
    cout<<"user_user: "<<users[num_users-1].getUsername()<<endl;
    cout<<"user_password: "<<users[num_users-1].getPassword()<<endl;
    users[num_users-1].setNumAuthors(0);
    users[num_users-1].setNumGenres(0);
    users[num_users-1].setNumReadBooks(0);
    return 1; /// account created
  }
  else {
    return 0; /// max limit of users
  }
}