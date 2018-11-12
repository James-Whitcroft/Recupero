/* This is recupero's interactive interface.
@parm: string - args
@return: int - return code

*/
int interface(std::string args);


/* this method tokenizes the string based on spaces

*/
vector<string> tok(string s);



/* Parse will take in input and run the corresponding command. If the command is 
incorrect then parse will return -1 and 0 on success.
@parm: string - args
@return: int - return code
*/
int  parse(vector<string> args);


/* this method prints the help menu for the interface

*/
void help();


/* This method does the error checking on the string to int conversion
@parm string - the string to be converted
@post int - the return value, if value is negative string isnt a number

*/
int conv(string x);

