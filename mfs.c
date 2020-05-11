//Shubham Arya
//1001650536
// The MIT License (MIT)
//
// Copyright (c) 2016, 2017 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <ctype.h>
#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     //Mavs shell supports 5 args

int16_t BPB_BytsPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATs;
int16_t BPB_RootEntCnt;
int32_t BPB_FATSz32;
int32_t RootDirSector =0;
int32_t FirstDataSector =0;
int32_t FirstSectorOfCluster=0;
FILE *fp;

struct __attribute__((__packed__)) DirectoryEntry
{
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t Unused1[8];
    uint16_t DIR_FirstClusterHigh;
    uint8_t Unused2[4];
    uint16_t DIR_FirstClusterLow;
    uint32_t DIR_FileSize;
};
struct DirectoryEntry dir[16];

int compare(char IMG_Name[],char input[])
{
    char expanded_name[12];
    memset( expanded_name, ' ', 12 );

    char *token = (char*)malloc(sizeof(char) * 12);
    strncpy( token, input, 12 );
    strtok( token, "." );

    strncpy( expanded_name, token, strlen( token ) );

    token = strtok( NULL, "." );

    if( token )
    {
      strncpy( (char*)(expanded_name+8), token, strlen(token ) );
    }

    expanded_name[11] = '\0';

    int i;
    for( i = 0; i < 11; i++ )
    {
      expanded_name[i] = toupper( expanded_name[i] );
    }

    if( strncmp( expanded_name, IMG_Name, 11 ) == 0 )
    {
        return 1;
    }

    return 0;
}
//Function: LBAToOffset
//Parameters: The current sector number that points to a block of data
//Returns: The value of the address for that block of data
//Description: Finds the starting address of a block given the sector number
//corresponding to that block
int LBAToOffset(int32_t sector)
{
    return (( sector - 2 ) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt) +
                                    (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}
//Name: NextLB
// Purpose: Given a logical block address, look up into the first FAT and return
//the logical block address of the block in the file. If there is no further
//blocks then return -1.
int16_t NextLB(uint32_t sector)
{
    uint32_t FATAddress = (BPB_BytsPerSec * BPB_RsvdSecCnt) + (sector * 4);
    int16_t val;
    fseek(fp, FATAddress, SEEK_SET);
    fread(&val, 2, 1, fp);
    return val;
}

int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  int i;
  int fileOpen=0;
  char filename[12];
  char temp[100];
    
  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;
                                                           
    char *working_str  = strdup( cmd_str );

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }
      if(token[0]!=NULL)
      {
          //************* OPEN ****************************************************
          /*
           Opens a file with the command open <filename>. Once it is opened, it is
           seeks and reads the data and sets it to variables depending on where it is present
           in the file system. Once that is done, it is used to calculate the root cluster which
           is used to read all the directories.
           */
          if(strcmp(token[0],"open")==0)
          {
              if(fileOpen == 1)
              {
                  printf("Error: your file system image already open.\n");
              }
              else
              {
                  int flag=0;
                  fp = fopen(token[1],"r");
                  if(fp==NULL)
                  {
                      printf("Error: File system image not found.\n");
                      //exit(1);
                      flag =1;
                  }
                  if(flag == 0)
                  {
                      fileOpen=1;
                      fseek(fp,11,SEEK_SET);
                      fread(&BPB_BytsPerSec,2,1,fp);
                      fseek(fp,13,SEEK_SET);
                      fread(&BPB_SecPerClus,1,1,fp);
                      fseek(fp,14,SEEK_SET);
                      fread(&BPB_RsvdSecCnt,2,1,fp);
                      fseek(fp,16,SEEK_SET);
                      fread(&BPB_NumFATs,1,1,fp);
                      fseek(fp,17,SEEK_SET);
                      fread(&BPB_RootEntCnt,2,1,fp);
                      fseek(fp,36,SEEK_SET);
                      fread(&BPB_FATSz32,4,1,fp);
                        
                      int root_cluster = (BPB_NumFATs * BPB_FATSz32*BPB_BytsPerSec)+(BPB_RsvdSecCnt* BPB_BytsPerSec);
                      //printf("\nRoot cluster is %d\n",root_cluster);
                      fseek(fp,root_cluster,SEEK_SET);
                      fread(&dir[0],sizeof(struct DirectoryEntry),16,fp);
                  }
              }
          }
          //************ CLOSE *********************************************************
          /*
           This basically closes the opened file. It displays an error message if there is no file open.
           */
          if(strcmp(token[0],"close")==0)
          {
              if(fileOpen==1)
              {
                  fclose(fp);
                  memset(dir, 0, sizeof(dir));
                  fileOpen=0;
              }
              else
              {
                  printf("Error: File system not open.\n");
              }
          }
          //******************* INFO **************************************************
          /*
           This simply displays the decimal and hexadecimal values of the variable set
           during the opening of the file
           */
          if (strcmp(token[0], "info")==0)
          {
              if(fileOpen==0)
              {
                  printf("Error: A file system image must be opened first.\n");
              }
              else
              {
                  printf("               DEC   HEX\n");
                  printf("BPB_BytsPerSec- %d   %x\n", BPB_BytsPerSec, BPB_BytsPerSec);
                  printf("BPB_SecPerClus- %d   %x\n", BPB_SecPerClus, BPB_SecPerClus);
                  printf("BPB_RsvdSecCnt- %d   %x\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
                  printf("BPB_NumFATs-    %d   %x\n", BPB_NumFATs, BPB_NumFATs);
                  printf("BPB_FATSz32-    %d   %x\n\n", BPB_FATSz32, BPB_FATSz32);
              }
          }
          //******************* STAT **************************************************
          /*
           Displays the stat of the file entered with the command stat <filename>.
           It iterates through all the 16 directories and checks if the name matches with any
           of the directories name. If it matches, it displays the stats of the directory with that index.
           */
          if(strcmp(token[0],"stat")==0)
          {
              int found =0;
              if(fileOpen==0)
              {
                  printf("Error: A file system image must be opened first.\n");
              }
              else
              {
                  if(token[1]!=NULL)
                  {
                      for(i=0; i<16;i++)
                      {
                          if ((dir[i].DIR_Attr == 0x01)||dir[i].DIR_Attr == 0x10||
                              dir[i].DIR_Attr == 0x20)
                          {
                              memset(&filename, 0, 12);
                              strncpy(filename, dir[i].DIR_Name, 11);
                              
                              if (compare(dir[i].DIR_Name,token[1])==1)
                              {
                                  printf("Directory Name- %s\n",filename);
                                  printf("Directory Attribute- %d\n",dir[i].DIR_Attr);
                                  printf("Start Cluster No.- %d\n",dir[i].DIR_FirstClusterLow);
                                  printf("File size- %d\n",dir[i].DIR_FileSize);
                                  found = 1;
                              }
                              else if (strcmp(dir[i].DIR_Name,token[1]) ==0)
                              {
                                  if (dir[i].DIR_FirstClusterLow == 0)
                                  {
                                      dir[i].DIR_FirstClusterLow = 2;
                                  }
                                  printf("Directory Name- %s\n",filename);
                                  printf("Directory Attribute- %d\n",dir[i].DIR_Attr);
                                  printf("Start Cluster No.- %d\n",dir[i].DIR_FirstClusterLow);
                                  printf("File size- %d\n",dir[i].DIR_FileSize);
                                  found = 1;
                              }
                          }
                      }
                  }
                  if(found!= 1)
                  {
                      printf("Error: File not found\n");
                  }
              }
              
          }
          //******************* GET **************************************************
          /*
           This is used by calling the command get <filename>. It gets the file you called into the
           directory in which you called. So it places that file in the current directory. It iterates through
           the directory names to see if the names match and if they do, then it open a new file to write.
           It calculates the file pposition by cluster number and filesize and if the size is greater than 512,
           then it is dealt accordingly.
           */
          if(strcmp(token[0],"get")==0)
          {
              if(fileOpen==0)
              {
                  printf("Error: A file system image must be opened first.\n");
              }
              else
              {
                  int index = -1;
                  for (i = 0; i < 16; i++)
                  {
                      if ((dir[i].DIR_Attr == 0x01)||dir[i].DIR_Attr == 0x10||
                            dir[i].DIR_Attr == 0x20)
                      {
                          if(compare(dir[i].DIR_Name,token[1])==1)
                          {
                              fseek(fp, LBAToOffset(dir[i].DIR_FirstClusterLow), SEEK_SET);
                              FILE *getFile = fopen(token[1], "w");
                              char write[512];
                              
                              if(dir[i].DIR_FileSize<512)
                              {
                                  fread(&write[0], dir[i].DIR_FileSize, 1, fp);
                                  fwrite(&write[0],dir[i].DIR_FileSize, 1, getFile);
                              }
                              else
                              {
                                  fread(&write[0], 512, 1, fp);
                                  fwrite(&write[0], 512, 1, getFile);
                                  int size = dir[i].DIR_FileSize;
                                  size = size - 512;
                                  
                                  while (size > 0)
                                  {
                                      int cluster = NextLB(dir[i].DIR_FirstClusterLow);
                                      fseek(fp, LBAToOffset(cluster), SEEK_SET);
                                      fread(&write[0], 512, 1, fp);
                                      fread(&write[0], 512, 1, getFile);
                                      size -=512;
                                  }
                              }
                              fclose( getFile );
                              index = 1;
                              break;
                          }
                          else
                          {
                              index =-1;
                          }
                      }
                  }
                  if(index == -1)
                  {
                      printf("Error: file not found\n");
                  }
                }
            }
          //************* cd ***********************************************************
          /*
           This commands places you into the directory you mention or takes you back to the previous
           directory. This iterates over the 16 directories and checks if the commands match. If they do,
           it uses seek and read to place the user into that directory.
           */
          if(strcmp(token[0],"cd")==0)
          {
              if(fileOpen==0)
              {
                  printf("Error: A file system image must be opened first.\n");
              }
              else
              {
                  for(i = 0; i < 16; i++)
                  {
                      if (strcmp(token[1],".")==0 ||strcmp(token[1],"..")==0)
                      {
                          if(strstr(dir[i].DIR_Name,token[1]) != NULL)
                          {
                              if (dir[i].DIR_FirstClusterLow == 0)
                              {
                                  dir[i].DIR_FirstClusterLow = 2;
                              }
                              fseek(fp, LBAToOffset(dir[i].DIR_FirstClusterLow), SEEK_SET);
                              fread(&dir[0], sizeof(struct DirectoryEntry), 16, fp);
                          }
                      }
                      else
                      {
                          strcpy(temp, token[1]);
                          if (compare(dir[i].DIR_Name,temp))
                          {
                              fseek(fp, LBAToOffset(dir[i].DIR_FirstClusterLow), SEEK_SET);
                              fread(&dir[0], sizeof(struct DirectoryEntry), 16, fp);
                          }
                      }
                  }
              }
              
          }
          //************* ls ***********************************************************
          /*
           It simply lists the filenames present in a directory
           */
          if (strcmp(token[0], "ls")==0)
          {
              if(fileOpen==0)
              {
                  printf("Error: A file system image must be opened first.\n");
              }
              else
              {
                  for(i=0; i<16;i++)
                  {
                       if ((dir[i].DIR_Attr == 0x01 ||dir[i].DIR_Attr == 0x10 ||
                            dir[i].DIR_Attr == 0x20 )&& dir[i].DIR_Name[0]!= 0xffffffe5)
                       {
                           memset(&filename, 0, 12);
                           strncpy(filename, dir[i].DIR_Name,11);
                           printf("%s\n",filename);
                       }
                  }
              }
          }
          //******************* READ **************************************************
          /*
           It reads the contents of a file. It is called by the command read <filename> position byte.
           fseek is used to reach that position and the data of the file is read until that posiiton.
           */
          if(strcmp(token[0],"read")==0)
          {
              if(fileOpen==0)
              {
                  printf("Error: A file system image must be opened first.\n");
              }
              else
              {
                  for (i = 0; i < 16; i++)
                  {
                      strcpy(filename,token[1]);
                      int position = atoi(token[2]);
                      int noOfBytes = atoi(token[3]);
                      if (compare(dir[i].DIR_Name,filename)==1)
                      {
                          if ((dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 ||
                               dir[i].DIR_Attr == 0x20))
                          {
                              char buff[noOfBytes];
                              fseek(fp, LBAToOffset(dir[i].DIR_FirstClusterLow), SEEK_SET);
                              fseek(fp, position, SEEK_CUR);
                              fread(&buff[0], noOfBytes, 1, fp);
                              printf("%s\n", buff);
                          }
                      }
                  }
              }
          }
      }
    free( working_root );
  }
  return 0;
}
