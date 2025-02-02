#include <bits/stdc++.h>
using namespace std;

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // Uncomment this block to pass the first stage

  while(true){
    cout << "$ ";

    string input;
    getline(std::cin, input);

    if(input == "exit 0") return 0;

    cout << input << ": command not found" << std::endl;
  }


  return 0;
}
