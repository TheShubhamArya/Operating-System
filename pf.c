/*
    Name: Shubham Arya
    ID: 1001650536
    Description: The program implements the FIFO, Optimal, MFU, and LRU page-replacement
    algorithms. Given a page-reference string, where page numbers range from 0 to 9,
    apply the page-reference string to each algorithm, and output the number of page faults
    incurred by each algorithm
 */
// The MIT License (MIT)
//
// Copyright (c) 2020 Trevor Bakker
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_LINE 1024

/*
 This function is used for optimal page reolacement. It is used to predict the page
 that appears at the farthest position. It loops over the page table and checks
 which element in the table from the current position to the end of the table appears
 at the farthest position. This index is then returned to optimal page replacement.
 */
int predict(int pg[], int data[], int count, int index,int working_set_size)
{
    int result = -1, farthest = index;
    int i;
    for ( i = 0; i < working_set_size; i++)
    {
        int j;
        for (j = index; j < count; j++)
        {
            if (data[i] == pg[j])
            {
                if (j > farthest)
                {
                    farthest = j;
                    result = i;
                }
                break;
            }
        }
  
        if (j == count)
            return i;
    }
    return (result == -1) ? 0 : result;
}
/*
 This function is used in LRU index. it returns the number with the highest index
 in the array. The index of the number with the highest index is  returned which is used to
 determine which element was last recently used.
 */
int LRUindex(int index[])
{
    int i;
    int highest = index[1];
    for(i=1;i<10;i++)
    {
        if (highest < index[i])
            highest = index[i];
    }
    for(i=1;i<10;i++)
    {
        if(index[i]==highest)
            return i;
    }
    return 1;
}

int main( int argc, char * argv[] )
{
  char * line = NULL;
  size_t line_length = MAX_LINE;
  char * filename;

    int FIFOpagefault=0;
    int LRUpagefault=0;
    int MRUpagefault=0;
    int Optimalpagefault=0;
    
    int count=0;
    int *data;
    const int datacount = 100;
    int d=0,flag2=0;
    int copy;
    int i=0,j=0,k=0,l=0,m=0,flag=0,found=0;
    int LRUflag=0;
    int flag3=0;
    int index[10];
    data = malloc(sizeof(int) * datacount);
  FILE * file;

  if( argc < 2 )
  {
    printf("Error: You must provide a datafile as an argument.\n");
    printf("Example: ./fp datafile.txt\n");
    exit( EXIT_FAILURE );
  }

  filename = ( char * ) malloc( strlen( argv[1] ) + 1 );
  line     = ( char * ) malloc( MAX_LINE );

  memset( filename, 0, strlen( argv[1] + 1 ) );
  strncpy( filename, argv[1], strlen( argv[1] ) );

  printf("Opening file %s\n", filename );
  file = fopen( filename , "r");

  if ( file )
  {
    while ( fgets( line, line_length, file ) )
    {
      char * token;

      token = strtok( line, " ");
      int working_set_size = atoi( token );

      printf("\nWorking set size: %d\n", working_set_size );
      int pageTable[working_set_size];
        
        //Initializing pageTable and index
        for(i =0; i< working_set_size;i++)
            pageTable[i]=0;
        for(i=1;i<10;i++)
               index[i]=0;
        
      while( token != NULL )
      {
        token = strtok( NULL, " " );
        
        if( token != NULL )
        {
           printf("Request: %d\n", atoi( token ) );
            // values in reference string are put in the data
            data[count]=atoi(token);
            count++;
        
        }
      }
        
//********* FIFO *****************  FIFO  ***************************
    i=0;
    
   // printf("\n\n FIFO \n\n");
    /*
    This part of program enters the elements into the pagetable one by one.
     If that element is already present, then a flag of 0 is set otherwise a flag
     of 1 is set. Whenever the flag is 1, the element is added into the pagetable and
     whenever the flag is 0, nothing is added but the value of j is decremented. Therefore,
     the element that is added first is the one that is out first
    */
    for(j=0;j<working_set_size;j++)
    {//This for loop is to input data into the pagetable
        for(k=0;k<working_set_size;k++)
        {   //This for loop is to traverse through the filled up pageTable
            if(data[i]==pageTable[k])
            {
                //printf("\n***NOpagefault in %d***\n",i);
                //pageTable[k]=data[i];
                found=1;
                if(flag2==0) //no pagefault first time
                {
                    copy=j;
                    flag2=1;
                    if(k==(working_set_size-1))
                    {
                        flag=0;
                    }
                }
                else
                {
                    j=copy;
                    j=j-1;
                    flag=0;
                }
            }
            else
            {
                if(found==1)
                    flag=0;
                else
                    flag=1;
            }
        }
        if(flag==1)
        {
            if(flag2==1)
            {
                j=copy;
                pageTable[j]=data[i];
                copy=0;
            }
            else
            {
                pageTable[j]=data[i];
            }
            FIFOpagefault++;
            //printf("\npagefault %d in else\n",FIFOpagefault);
            flag2=0;
        }
        if(j==(working_set_size-1))
        {   //when max frame size is reached, it is set to -1 so that the
            //for loop increments it to 0th position
            j=-1;
        }
        if(i==(count-1))
        {
            //when all the elements are iterated, j is set such that the program
            //doesnt loop again
            j=working_set_size;
        }
        i++;
        //printf("pagetable");
        //for(k=0;k<working_set_size;k++)
         //   printf(" %d ",pageTable[k]);
       // printf("\n");
        
        flag=0;
        found=0;
    }
        
//********* FIFO *****************  FIFO  ***************************
//********* LRU *****************  LRU  ***************************
         //printf("\n\n LRU \n\n");
        for(i =0; i< working_set_size;i++)  //to reset the page table
        {
            pageTable[i]=0;
        }
            
        i=0;
        flag=found=flag2=0;
        /*
         In this part of the program, we keep track of all the the numbers that are being entered
         and we store it in index array. Initially index elements are all iniotialized to 0.
         When a pagefault occurs or when the element is not replaced in the pagetable, 1 is added to
         the index elements which corresponds to the element in pagetable. If there is a hit, then
         that element value in the index array is set to 0. Once all the frames are used and there
         is another entry, we look through the index and the index element with the greatest value
         is removed because it was the least recently element used.
         */
        for(j=0;j<working_set_size;j++)
        {
            for(k=0;k<working_set_size;k++)
            {
                if(data[i]==pageTable[k])
                {
                    //printf("\n***NOpagefault in i = %d and j=%d***\n",i,j);
                    found=1;
                    if(flag2==0) //no pagefault first time
                    {
                        copy=j;
                        flag2=1;
                        if(k==(working_set_size-1))
                        {
                            flag=0;
                        }
                    }
                    index[data[i]]=0;
                }
                else
                {
                    if(found==1)
                        flag=0;
                    else
                        flag=1;
                }
            
                if(pageTable[k]>0)
                    flag3++;
            }
            if(flag==1)
            {
                if(flag2==1 || flag3==working_set_size)
                {
                    m=LRUindex(index);
                    for(k=0;k<working_set_size;k++)
                    {
                        if((pageTable[k]==m) && LRUflag==0)
                        {
                            j=k;
                            index[pageTable[k]]=0;
                        }
                    }
                    pageTable[j]=data[i];
                    copy=0;
                }
                else
                {
                    pageTable[j]=data[i];
                }
                LRUpagefault++;
                flag2=0;
            }
            else //no page fault
            {
                for(l=1;l<10;l++)
                {
                    if(l==data[i])
                    {
                        index[l]=1;
                        flag2=l;
                        j=j-1;
                    }
                }
            }
            for(k=0;k<working_set_size;k++)
            {
                for(l=1;l<10;l++)
                {
                    if(pageTable[k]==l)
                    {
                        if(flag2==l)
                        {
                            index[l]=1;
                            flag2=0;
                        }
                        else
                        {
                            index[l]++;
                        }
                    }
                }
                if(pageTable[k]==0)
                {
                    LRUflag=1;
                }
                else
                    LRUflag=0;
            }
            
            if(j==(working_set_size-1))
            {
                j=-1;
            }
            if(i==(count-1))
            {
                j=working_set_size;
            }
            i++;
            //printf("\n");
            //printf("pagetable");
            //for(k=0;k<working_set_size;k++)
             //   printf(" %d ",pageTable[k]);
            //printf("\n");
            flag2=flag3=0;
            flag=0;
            found=0;
        }
        
//********* LRU *****************  LRU  ***************************
//********* MRU *****************  MRU  ***************************
        //printf("\n\n MRU\n\n");
        for(i =0; i< working_set_size;i++)  //to reset the page table
            pageTable[i]=0;
        
        i=0;
        int MRUelement=-1;
        int MRUindex;
        /*
         In this, every new element that is added is stored ad the Most
         recently used element. This most recently used element is then
         compared with the entering element and if they are not equal, then
         the MRUelement is update. Otherwise, the value of j is decremented and
         and MRUelement is updated.
         */
        for(j=0;j< working_set_size;j++)
        {
            for(k=0;k<working_set_size;k++)
            {
                if(data[i]==pageTable[k])
                {
                    //printf("\n No page fault in %d\n",i);
                    flag =1;
                    j=j-1;
                    MRUelement=pageTable[k];
                }
            }
            if(flag==0)
            {
                if(MRUpagefault<working_set_size)
                {
                    pageTable[j]=data[i];
                    MRUelement=pageTable[j];
                }
                else
                {
                    for(k=0;k<working_set_size;k++)
                    {
                        if(pageTable[k]==MRUelement)
                        {
                            MRUindex=k;
                        }
                    }
                    pageTable[MRUindex]=data[i];
                    MRUelement=pageTable[MRUindex];
                }
                MRUpagefault++;
                //printf("\n page fault in %d",i);
            }
            
            if(j==(working_set_size-1))
            {
                j=-1;
            }
            if(i==(count-1))
            {
                j=working_set_size;
            }
            i++;
            //printf("\n");
            //printf("pagetable");
            //for(k=0;k<working_set_size;k++)
             //   printf(" %d ",pageTable[k]);
            //printf("\n");
            flag=0;
        }
        
//********* MRU *****************  MRU  ***************************
//********* optimal ***************  optimal  ***********************
        //printf("\n\n OPTIMAL\n\n");
      for(i =0; i< working_set_size;i++)  //to reset the page table
          pageTable[i]=0;
        
        j=0;
        copy=0;
        int PTfull=0;
        /*
         This part of the code to look ahead in the future in the upcoming
         pages. It uses the function predict to predict the number that wouldn't
         be required in the future use, and the index of that number is returned so
         a new element can be entered at that index.
         */
        for (i = 0; i < count; i++)
        {
            for ( d = 0; d < working_set_size; d++)
            {
                if (pageTable[d] == data[i])
                {
                    //printf("\n***NOpagefault in %d***\n",i);
                    flag=0;
                    j=j-1;
                    break;
                }
                else
                {
                    flag=1;
                }
            }
            if (flag==1 && PTfull<working_set_size)
            {
                pageTable[j]=data[i];
                Optimalpagefault++;
                //printf("\npagefault %d in %d\n",Optimalpagefault,i);
                PTfull++;
            }
            else
            {
                if(flag==1)
                {
                    int l = predict(data, pageTable, count, i+1 ,working_set_size);
                    for ( d = 0; d < working_set_size; d++)
                    {
                        if(pageTable[d]==pageTable[l])
                        {
                            copy=d;
                            break;
                        }
                        else
                        {
                            copy=j;
                        }
                    }
                    pageTable[copy]=data[i];
                    Optimalpagefault++;
                    PTfull++;
                }
            }
            j++;
            if(j==working_set_size)
            {
                j=0;
            }
            //printf("pagetable");
            //for(k=0;k<working_set_size;k++)
              //  printf(" %d ",pageTable[k]);
            //printf("\n");
            flag=0;
        }
    
//********* optimal ***************  optimal  ***********************
        
        
      printf("\n");
      printf("\nPage faults of FIFO:\t%d\n",FIFOpagefault);
      printf("\nPage faults of LRU:\t%d\n",LRUpagefault);
      printf("\nPage faults of MRU:\t%d\n",MRUpagefault);
      printf("\nPage faults of Optimal:\t%d\n",Optimalpagefault);
      memset(data, 0, sizeof(int)*datacount);
      count=0;
      FIFOpagefault=0;
      LRUpagefault=0;
      Optimalpagefault=0;
      MRUpagefault=0;
    }

    free( line );
    fclose(file);
  }

  return 0;
}
