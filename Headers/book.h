#include <iostream>
#include <cstring>
#include <fstream>

#define MAX_SIZE 100

class book {
  private:
    char title[MAX_SIZE];
    char author[MAX_SIZE];
    char ISBN[MAX_SIZE];
    char genre[MAX_SIZE];
  public: 
    book();
    book(char t[], char a[], char i[]);
    char *getTitle() {return title;}
    char *getAuthor() {return author;}
    char *getISBN() {return ISBN;}
    char *getGenre() {return genre;}
    void setTitle(char t[]) {strlcpy(title, t, MAX_SIZE);}
    void setAuthor(char a[]) {strlcpy(author, a, MAX_SIZE);}
    void setISBN(char i[]) {strlcpy(ISBN, i, MAX_SIZE);}
    void setGenre(char g[]) {strlcpy(genre, g, MAX_SIZE);}
    bool loadBookFromXML(FILE *fp);
    static void loadBooksfromXML(FILE *fp, book bookArray[], int &num_books);
    static bool searchForBook(book &b, book books[], int num_books);
};