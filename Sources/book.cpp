#include "../Headers/book.h"

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

using std::cout;
using std::endl;
using std::cin;
using std::max;

book::book() {
  strlcpy(title, "NULL", MAX_SIZE);
  strlcpy(author, "NULL", MAX_SIZE);
  strlcpy(ISBN, "NULL", MAX_SIZE);
};

book::book(char t[], char a[], char i[]) {
  strlcpy(title, t, MAX_SIZE);
  strlcpy(author, a, MAX_SIZE);
  strlcpy(ISBN, i, MAX_SIZE);
}

bool book::loadBookFromXML(FILE *fp) {
  bool found = false;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  char *token;
  rewind(fp);

  while ((read = getline(&line, &len, fp)) != -1) {
    if (strstr(line, this->title) != NULL) {
      // found the book
      found = true;
      // read the author
      getline(&line, &len, fp);
      token = strtok(line, ">");
      token = strtok(NULL, "<");
      this->setAuthor(token);

      // read the genre
      getline(&line, &len, fp);
      token = strtok(line, ">");
      token = strtok(NULL, "<");
      this->setGenre(token);

      // read the ISBN
      getline(&line, &len, fp);
      token = strtok(line, ">");
      token = strtok(NULL, "<");
      this->setISBN(token);

      break;
    } // if

    getline(&line, &len, fp);
    token = strtok(line, ">");
  } // while
  
  if (found) {
    return true;
  }
  else {
    return false;
  }
}

void book::loadBooksfromXML(FILE *fp, book bookArray[], int &num_books) {
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  char *token;
  int i = 0;
  rewind(fp);
  while ((read = getline(&line, &len, fp)) != -1) {
    // get the title
    if (strstr(line, "<title>") != NULL) {
      token = strtok(line, ">");
      token = strtok(NULL, "<");
      strlcpy(bookArray[i].title, token, MAX_SIZE);
    }
    // get the author
    if (strstr(line, "<author>") != NULL) {
      token = strtok(line, ">");
      token = strtok(NULL, "<");
      strlcpy(bookArray[i].author, token, MAX_SIZE);
    }
    // get the genre
    if (strstr(line, "<genre>") != NULL) {
      token = strtok(line, ">");
      token = strtok(NULL, "<");
      strlcpy(bookArray[i].genre, token, MAX_SIZE);
    }
    // get the ISBN
    if (strstr(line, "<ISBN>") != NULL) {
      token = strtok(line, ">");
      token = strtok(NULL, "<");
      strlcpy(bookArray[i].ISBN, token, MAX_SIZE);
    }
    if (strstr(line, "</book>") != NULL) {
      i++;
    }
  } // while

  num_books = i;
}

int longestCommonSequence(char X[], char Y[]) {
    // https://en.wikipedia.org/wiki/Longest_common_subsequence_problem
    // https://www.geeksforgeeks.org/longest-common-subsequence-dp-4/
    int m = strlen(X);
    int n = strlen(Y);
    int L[m + 1][n + 1];

    /* Following steps build L[m+1][n+1] in bottom up fashion. Note
        that L[i][j] contains length of LCS of X[0..i-1] and Y[0..j-1] */
    for (int i = 0; i <= m; i++) {
        for (int j = 0; j <= n; j++) {
            if (i == 0 || j == 0)
                L[i][j] = 0;
            else if (X[i - 1] == Y[j - 1])
                L[i][j] = L[i - 1][j - 1] + 1;
            else
                L[i][j] = max(L[i - 1][j], L[i][j - 1]);
        }
    }
    // L[m][n] contains length of LCS for X[0..n-1] and Y[0..m-1]
    return L[m][n];
}

bool book::searchForBook(book &b, book books[], int num_books) {
  // treat case for when books[i].title = b.title and books[i].author != b.author
  int titleCount = 0;
  int longestCommon = 0;
  int longestCommonIndex = 0;
  for(int i = 0; i < num_books; i++) {
    if(!strcasecmp(books[i].getISBN(), b.getISBN())) { // perfect ISBN match
      cout<<"PERFECT ISBN MATCH"<<endl;
      b.setTitle(books[i].getTitle());
      b.setAuthor(books[i].getAuthor());
      b.setGenre(books[i].getGenre());
      b.setISBN(books[i].getISBN());
      return true;
    }
  }
  for(int i = 0; i < num_books; i++) {
    if(!strcasecmp(books[i].getTitle(), b.getTitle())) {
      titleCount++;
      cout<<"title "<<books[i].getTitle()<<" matches "<<b.getTitle()<<endl;
      if(!strcasecmp(books[i].getAuthor(), b.getAuthor())) { // perfect title & author match
        cout<<"PERFECT TITLE & AUTHOR MATCH"<<endl;
        b.setTitle(books[i].getTitle());
        b.setAuthor(books[i].getAuthor());
        b.setGenre(books[i].getGenre());
        b.setISBN(books[i].getISBN());
        return true;
      }
    }
  }
  if(titleCount > 1) { // if there are more books with same title, different author
    longestCommon = 0;
    longestCommonIndex = 0;
    for(int i = 0; i < num_books; i++) {
      if(!strcasecmp(books[i].getTitle(), b.getTitle())) {
        int localLongest = longestCommonSequence(books[i].getAuthor(), b.getAuthor());
        if(localLongest > longestCommon) {
          longestCommon = localLongest;
          longestCommonIndex = i;
        }
      }
    }
    if(longestCommon > 5) { // best match of authors (maybe client spelling error)
      cout<<"PERFECT TITLE & BEST AUTHOR MATCH"<<endl;
      b.setTitle(books[longestCommonIndex].getTitle());
      b.setAuthor(books[longestCommonIndex].getAuthor());
      b.setGenre(books[longestCommonIndex].getGenre());
      b.setISBN(books[longestCommonIndex].getISBN());
      return true;
    }
    else // no match of authors
      return false;
  }
  else if(titleCount < 1) { // for no perfect title match, try author and LCS(title)
    longestCommon = 0;
    longestCommonIndex = 0;
    for(int i = 0; i < num_books; i++) {
      if(!strcasecmp(books[i].getAuthor(), b.getAuthor())) {
        int localLongest = longestCommonSequence(books[i].getTitle(), b.getTitle());
        if(localLongest > longestCommon) {
          longestCommon = localLongest;
          longestCommonIndex = i;
        }
      }
    }
    if(longestCommon > 5) { // best match of titles (maybe client spelling error)
      cout<<"PERFECT AUTHOR & BEST TITLE MATCH"<<endl;
      b.setTitle(books[longestCommonIndex].getTitle());
      b.setAuthor(books[longestCommonIndex].getAuthor());
      b.setGenre(books[longestCommonIndex].getGenre());
      b.setISBN(books[longestCommonIndex].getISBN());
      return true;
    }
    else // no match of titles 
    {
      cout<<"NO TITLE MATCH -> [longesCommon] "<<longestCommon<<endl;
      return false;
    }
  }
  else if(titleCount == 1) { // for one perfect title match, try author and LCS(title)
    longestCommon = 0;
    longestCommonIndex = 0;
    for(int i = 0; i < num_books; i++) {
      if(!strcasecmp(books[i].getTitle(), b.getTitle())) {
        int localLongest = longestCommonSequence(books[i].getAuthor(), b.getAuthor());
        if(localLongest > longestCommon) {
          longestCommon = localLongest;
          longestCommonIndex = i;
        }
      }
    }
    if(longestCommon > 5) { // best match of authors (maybe client spelling error)
      cout<<"PERFECT [only] TITLE & BEST AUTHOR MATCH"<<endl;
      b.setTitle(books[longestCommonIndex].getTitle());
      b.setAuthor(books[longestCommonIndex].getAuthor());
      b.setGenre(books[longestCommonIndex].getGenre());
      b.setISBN(books[longestCommonIndex].getISBN());
      return true;
    }
    else // no match of authors
      return false;
  }
  cout<<"NO MATCH"<<endl;
  return false;
}



