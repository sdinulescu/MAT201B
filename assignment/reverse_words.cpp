// Stejara Dinulescu
// MAT 201B Assignment 0

#include <iostream>
#include <string>
using namespace std;
int main() {
  while (true) {
    printf("Type a sentence (then hit return): ");
    string line;
    getline(cin, line);
    if (!cin.good()) {
      printf("Done\n");
      return 0;
    }

    string temp = "";
    string newline = "";
    for (int i = 0; i < line.length(); i++) {
      if (line.at(i) != ' ') {
        temp = temp + line.at(i);
        //cout << temp << endl;
      }
      if (i == line.length()-1 || line.at(i) == ' '){
        for (int j = temp.length()-1; j >= 0; j--) {
          newline = newline + temp.at(j);
            //cout << newline << endl;
        }
        newline = newline + " ";
        temp = "";
      }
    }
    cout << newline << endl;
  }
  
}
