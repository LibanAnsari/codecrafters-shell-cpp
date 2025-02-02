#include <bits/stdc++.h>
using namespace std;

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while(true){
    cout << "$ ";

    string input;
    getline(cin, input);

    if(input == "exit 0") return 0;

    string firstWord = input.substr(0, input.find(' ')); 

    if(firstWord == "echo"){
      cout << input.substr(input.find(' ')+1) << endl;
    }else{
      cout << input << ": command not found" << endl;
    }

  }

  return 0;
}
