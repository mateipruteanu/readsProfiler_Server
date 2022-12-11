#include <iostream>
#include <fstream>
#include <cstring>

#define MAX_SIZE 100

class user {
  private:
    char username[MAX_SIZE];
    char password[MAX_SIZE];
    int numPreferedGenres;
    int numPreferedAuthors;
    struct genrePreference {
      char genre[MAX_SIZE];
      int level;
    } genreArray[MAX_SIZE];
    struct authorPreference {
      char author[MAX_SIZE];
      int level;
    } authorArray[MAX_SIZE];
  public:
    user();
    user(char u[], char p[], int nG, int nA, struct genrePreference g[], struct authorPreference a[]);

    static bool createAccount(char username[], char password[], int &num_users, user users[]);

    char *getUsername() {return username;}
    char *getPassword() {return password;}
    char *getGenre(int i) {return genreArray[i].genre;}
    char *getAuthor(int i) {return authorArray[i].author;}
    int getGenreLevel(int i) {return genreArray[i].level;}
    int getNumGenres() {return numPreferedGenres;}
    int getNumAuthors() {return numPreferedAuthors;}
    int getPreferedGenre();
    int getPreferedAuthor();
    void setUsername(char u[]) {strlcpy(username, u, MAX_SIZE);}
    void setPassword(char p[]) {strlcpy(password, p, MAX_SIZE);}
    void setNumGenres(int n) {numPreferedGenres = n;}
    void setNumAuthors(int n) {numPreferedAuthors = n;}
    void setGenrePreference(int i, char g[], int l) {
      strlcpy(genreArray[i].genre, g, MAX_SIZE);
      genreArray[i].level = l;
    }
    void setGenre(int i, char g[]) {strlcpy(genreArray[i].genre, g, MAX_SIZE);}
    void setGenreLevel(int i, int l) {genreArray[i].level = l;}
    void setAuthorPreference(int i, char a[], int l) {
      strlcpy(authorArray[i].author, a, MAX_SIZE);
      authorArray[i].level = l;
    }
    void setAuthor(int i, char a[]) {strlcpy(authorArray[i].author, a, MAX_SIZE);}
    void setAuthorLevel(int i, int l) {authorArray[i].level = l;}
    void addPreference(char g[], char a[]);
    // void loadUserFromXML(FILE *fp);

    static void loadArrayfromXML(FILE *fp, user users[], int &num_users);

    static void writeArraytoXML(FILE *fp, user users[], int num_users);

    void printUser();
};
