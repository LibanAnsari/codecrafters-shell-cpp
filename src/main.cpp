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

    if(firstWord == "type"){
      string to_check = input.substr(5);
      if(to_check == "echo" || to_check == "exit" || to_check == "type"){
        cout << to_check << " is a shell builtin" << endl;
      }else{
        cout << to_check << ": not found" << endl;
      }
    }else if(firstWord == "echo"){
      cout << input.substr(input.find(' ')+1) << endl;
    }else{
      cout << input << ": command not found" << endl;
    }
    
  }

  return 0;
}
