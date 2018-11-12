/* This is the main file for recupero.
It cantain main and the interface for recupero.
@author: samuel Launt
@author: James Whitcroft

*/




#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
using namespace std;
#include "ext2_fs.h"
#include "main.h"
#include "Extended2.h"

//globals
string path;


//DEBug
void debug(vector<string> a){
  cout << "legnth is: " << a.size() << "\n";
  for(int i=0;i < a.size();i++){
    cout << a[i] << "\n";
  }
}





/* This is the main method for recupero. It will take in command line args
and either enter interactive mode or just run the command.
@parm: int - number of command arguemnts
@parm: char* - actual command arguments
@return: int - return code

*/
int main(int argc, char* argv[]){
  setbuf(stdout,NULL);
  string args = string(*argv, argc);
  path = string(argv[argc - 1]);
  if(argc < 2){
    help();
    return 0;
  }//end if 
  if(argc > 2){      
    vector<string> tmp;
    for(int i = 0; i < argc; i++){
      if(i == 0 || i == argc -1){
        //do nothing
      }//end if
      else{
        tmp.push_back(string(argv[i]));
      }//end else
    }//end for
    //debug(tmp);
    return parse(tmp);
  }//end if
  else{
    return interface(args);
  }//end else
}//end main


/* This is recupero's interactive interface.
@parm: string - arguments
@return: int - return code

*/
int interface(string args){
  int exit = 1;
  int re = 0;
  string input;
  while(exit){
    cout << "[recupero]: ";
    getline(cin,input);
    vector<string> vect = tok(input);
    re = parse(vect);
    if(re != 0){
      exit = 0;
    }//end if
  }//end while
  return 0;
}//end interface


/*this method tokenizes the string based on spaces

*/
vector<string> tok(string s){
  string delim = " ";
  vector<string> ans;
  for(int i = 0; i < s.length(); i++){
    int current = i;
    i  = s.find(delim,i);
    if(i == -1){
      ans.push_back(s.substr(current,s.length()));
      break;
    }//end
    else{
      if(i == current){
        //do nothing
      }//end if
      else{
        ans.push_back(s.substr(current,i - current));
      }//end else
    }//end else
  }//end for
  return ans;
}//end tok


/* Parse will take in input and run the corresponding command. If the command is 
incorrect then parse will return -1 and 0 on success.
@parm: string - arguments
@return: int - return code
*/
int  parse(vector<string> args){
  //debug(args);
  if(args.size() == 0){
    help();
    return 0;
  }
  if(args[0].compare("print") == 0){
    if(args.size() < 2){
      help();
    }//end if 
    else if(args[1].compare("inode") == 0){
      if(args.size() < 3){
        help();
      }//end if
      else{
        int num = conv(args[2]);
        if(num == -1){
          help();
        } else{
          Extended2 obj = Extended2(path.c_str());
          obj.printInode(num);
        }
      }//end else
    }//end if

    else if(args[1].compare("superblock") == 0){
      Extended2 obj = Extended2(path.c_str());
      obj.printSuper();
    }//end elif
    else if(args[1].compare("group") == 0){
      Extended2 obj = Extended2(path.c_str());
      obj.printGroup();
    }//end else if
    else if(args[1].compare("meta") == 0){
     Extended2 obj = Extended2(path.c_str());
     obj.printMeta();
    }
    else if(args[1].compare("bitmap") == 0){
        if(args.size() < 3){
          help();
        }
      else if(args[2].compare("inode") == 0){
        if(args.size() < 4){
          help();
        }//end if
        else{
          int num = conv(args[3]);
          if(num == -1){
            help();
          }
          else{
            Extended2 obj = Extended2(path.c_str());
            obj.printBitmapInode(num);
          }//end else
        }//end else
      }//end else

      else if(args[2].compare("data") == 0){
        if(args.size() < 3){
          help();
        }//end if
        else{
          int num = conv(args[3]);
          if(num == -1){
            help();
          }
          else{
            Extended2 obj = Extended2(path.c_str());
            obj.printBitmapData(num);
          }
        }//end else
      }//end else if
    }//end else if

    else if(args[1].compare("recover") == 0){
      Extended2 obj = Extended2(path.c_str());
      obj.recoverData(0);
      cout << "after" << endl;
    }

    else{
      help();
    }
  }//end if

  else if(args[0].compare("recover") == 0){
    if(args.size() < 2){
      help();
    }//end if
    else if(args[1].compare("superblock") == 0){
      if(args.size() < 3){
        help();
      }//end if
      else{
        int num1 = conv(args[2]);
        if(num1 == -1){
          help();
        }
        else{
          Extended2 obj = Extended2(path.c_str(), num1);
        }//end else
      }//end else if
    }//end else if 
    else if(args[1].compare("data") == 0){
      Extended2 obj = Extended2(path.c_str());
      obj.recoverData(1);
    }//end else
    else{
      int num = conv(args[1]);
      if(num == -1){
        help();
      }
      else{
        Extended2 obj = Extended2(path.c_str());
        obj.recoverInode(num);    
      }//end else
    }//end else
  }//end else if 

  else if(args[0].compare("exit") == 0){
    return 1;
  }
  else{
    help();
  }//end else
  return 0;
}//end parse

/* This method prints the help menu for the interface


*/
void help(){
  cout << "Usage: ./recupero <command> <options> [path to device]\n";
  cout << "\tprint inode [number]- prints inode information of inode [number]\n";
  cout << "\tprint superblock [number] - prints superblock number [number]\n";
  cout << "\tprint meta - prints meta data about file system\n";
  cout << "\tprint group - prints the groups descriptors\n";
  cout << "\tprint bitmap [inode/data] [number]\n";
  cout << "\tprint recover - prints files that can be recovered\n";
  cout << "\trecover [inode number/data] - this recovers from a single inode. Data recovers all possible inodes.\n";
  cout << "\trecover superblock [backup superblock offset in bytes] - over write the primary superblock with a backup superblock\n";
  cout << "\texit - stops the program and exits\n";
  cout << "\thelp - prints this menu\n";
}//end help


/* This method does the error checking on the string to int conversion
@parm string - the string to be converted
@post int - the return value, if value is negative string isnt a number

*/
int conv(string x){
  char* endpnt;
  long value = strtol(x.c_str(),&endpnt,10);
  if(endpnt == x){
    return -1;
  }
  else if(*endpnt != '\0'){
    return -1;
  }
  else{
    return (int)value;
  }
}//end conv



