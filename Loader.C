/**
 * Names: Joe Sawin Jerome Glenn
 * Team:24
*/
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <ctype.h>
#include <string>
#include "Loader.h"
#include "Memory.h"

//first column in file is assumed to be 0
#define ADDRBEGIN 2   //starting column of 3 digit hex address 
#define ADDREND 4     //ending column of 3 digit hext address
#define DATABEGIN 7   //starting column of data bytes
#define COMMENT 28    //location of the '|' character 



/**
 * Loader constructor
 * Opens the .yo file named in the command line arguments, reads the contents of the file
 * line by line and loads the program into memory.  If no file is given or the file doesn't
 * exist or the file doesn't end with a .yo extension or the .yo file contains errors then
 * loaded is set to false.  Otherwise loaded is set to true.
 *
 * @param argc is the number of command line arguments passed to the main; should
 *        be 2
 * @param argv[0] is the name of the executable
 *        argv[1] is the name of the .yo file
 */
Loader::Loader(int argc, char * argv[])
{
   loaded = false;
   char arr[250];
   int lineNumber = 1;
      
   if (!Loader::canLoad(argv[1]))
   {
       return;
   }

   while (inf.peek() != EOF)
   {
        inf.getline(arr, 250);
        


        if (hasErrors(arr)) 
            {
                std::cout << "Error on line " << std::dec << 
                             lineNumber << ": " << arr << std::endl;
                loaded = false;
                return;
            } 
   
         
        if(arr[DATABEGIN] != ' ' && arr[ADDRBEGIN] != ' ')
        Loader::loadline(arr);
        loaded = true;
        lineNumber++;
   }
}



/**
 * isLoaded
 * returns the value of the loaded data member; loaded is set by the constructor
 *
 * @return value of loaded (true or false)
 */
bool Loader::isLoaded()
{
   return loaded;
}


//You'll need to add more helper methods to this file.  Don't put all of your code in the
//Loader constructor.  When you add a method here, add the prototype to Loader.h in the private
//section.

//takes as input the name of the input file, returns true if the name of the file ends
//with a .yo extension and the file can be opened (it opens the file using the 
//c++ ifstream open method).  If the file can't be opened (doesn't exist or the filename 
//is incorrect), the function returns false. Note that the file handle you'll use is 
//declared as a private data member (an ifstream) in the Loader class
bool Loader::canLoad(char inFile[])
{
    int size = strlen(inFile);

    if (!(inFile[size - 3] == '.' 
        && inFile[size - 2] == 'y'
        && inFile[size - 1] == 'o' ))
    {
        return false;
    }        

    inf.open(inFile, std::ifstream::in);
    if (!(inf.good()))
    {
        return false;
    }
    
    return true;
}   
   
   
//Parameter = char line[].  This should be the current line that the constructor
//is loading into memory.  It can be assumed here that line has an address and
//has data.  Loadline should put the data into memory at the given address.
//It should not return any data.
    
void Loader::loadline(char line[])
{   
    Memory * mem = Memory::getInstance();              
    int32_t addr = convert(line, ADDRBEGIN, ADDREND);     
    bool error = false;                                
   
     
    for (int i = DATABEGIN; line[i] != ' '; i += 2)              
    {                                                            
        int8_t data = Loader::convert(line, i, (i + 1));
        mem->putByte(data, addr, error);                         
        addr++;                          
    }
}    
    
//takes as input the full line, the start of the address to store in,
//and the end of the address to store in, and returns the numeric
//value of the address in mem where it should be stored in
//if you give it 0x00a return 10.
    
int32_t Loader::convert(char line[], int32_t beg, int32_t end)
{        
    int32_t j = 0;
    char chrArray[end - beg + 2];
    
    for (int i = beg; i <= end; i++, j++)
    {
        chrArray[j] = line[i];      
    }
    
    chrArray[j] = '\0';
    std::string test = chrArray;
    return std::stoul(test, NULL, 16);
}

//takes as input an individual line.  Method goes through several checks
//on the given line, returning true at any point where an error is 
//detected.  If every check passes, the method returns false. Each check
//is self contained in a helper method.

bool Loader::hasErrors(char line[])
{
    if(line[0] == '0' && line[1] == 'x' && line[7] == ' ')
    {
        return positionlinecheck(line);
    }
    else if(line[0] == ' ')
    {
        return commentlinecheck(line);
    }
    else if(line[0] == '0' && line[7] != ' ')
    {
        return datalinecheck(line);
    }

    return 1;
}
 
 
bool Loader::positionlinecheck(char line[])
{
    for(int i = DATABEGIN; i < 28; i++)
    {
        if(line[i] != ' ')
            return 1;
    }
    return 0;
}

bool Loader::commentlinecheck(char line[])
{
    for (int i = 0; i <= 27; i++)
    {
        if(line[i] != ' ')
            return 1;
    }

    if(line[28] != '|')
        return 1;
    else
        return 0;
}

bool Loader::datalinecheck(char line[])
{
    static int lastAddress;

    if(line[0] != '0')
        return 1;
    if(line[1] != 'x')
        return 1;
    for (int i = 2; i <= 4; i++)
        {
            if (!isHex(line[i]))
                return 1;
        }
    if(line[5] != ':')
        return 1;
    if(line[6] != ' ')
        return 1;
    
    ///////////////////////////

    int iter = 7;
    

    while(line[iter] != ' ')
    {
        if (!isHex(line[iter]))
        {
            return 1;
        }
        iter++;    
    }

    if(iter % 2 == 0)
        return 1;

    for(;iter < 28; iter++)
    {
        if(line[iter] != ' ')
            return 1;
    }

    if(line[28] != '|')
        return 1;

    int lineAddr = convert(line, 2, 4);

    
    if(lineAddr < lastAddress) return 1;
    lastAddress = lineAddr + countBytes(line) - 1;
    
    if(lastAddress < 0 || lastAddress >= MEMSIZE) return 1;

    return 0;

}

int Loader::countBytes(char line[])
{
    int count = 0;
    for (int i = DATABEGIN; line[i] != ' '; i++)
    {
        count++;
    }
    return count / 2;
}

bool Loader::isHex(char ch)
{
     if((ch >= '0' && ch <= '9') || 
          (ch >= 'A' && ch <= 'F') ||
          (ch >= 'a' && ch <= 'f'))
     {
        return true;
     }
     else
     {
        return false;    
     }
}
