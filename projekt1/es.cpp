 #include <mpi.h>
 #include <stdio.h>
 #include <string.h>
 #include <fstream>
 #include <vector>	
 using namespace std;

 #define TAG_X 0
 #define TAG 3
 #define TAG_Y 1
 #define TAG_Z 2
 #define BUFSIZE 128

 struct Reg
 {
 	int C;
 	int X;
 	int Y;
 	int Z;
 };


 int main(int argc, char *argv[])
 {

  	int numprocs;
   	int myid;
   	char idstr[32];   	
   	char buff[BUFSIZE];
  	int my_number;
   	int i;
   	MPI_Status stat; 
 	    

 	MPI_Init(&argc,&argv); /* inicializace MPI */
   	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);  /*zjistíme, kolik procesů běží */
   	MPI_Comm_rank(MPI_COMM_WORLD,&myid); /* zjistíme id svého procesu */	

  
   	Reg* proc= new Reg [numprocs];  // seznam procesoru       
   	
    if(myid == 0){
        
        char input[]= "numbers";                          //jmeno souboru    
        int number;                                     //hodnota pri nacitani souboru
        int invar= 1;                                   //invariant- urcuje cislo proc, kteremu se bude posilat
        fstream fin;                                    //cteni ze souboru
        fin.open(input, ios::in);                   
        double start, finish;
        start = MPI_Wtime();
        while(fin.good())
        {
            number= fin.get();
            if(!fin.good()) 
            {
                break;
            }                 
            cout<<number<<" ";         
            // nacitani cisel a posilani jich procesorum                
            MPI_Send(&number, 1, MPI_INT, invar, TAG_X, MPI_COMM_WORLD); 
            MPI_Send(&number, 1, MPI_INT, 1, TAG_Y, MPI_COMM_WORLD);   
            
            invar++;
        }
        
        fin.close(); 
        cout << endl;

        // prijimani serazenych cisel
     	for(i = 1; i < numprocs; i++)
     	{
     	    MPI_Recv(buff, BUFSIZE, MPI_CHAR, i, TAG, MPI_COMM_WORLD, &stat);
     	    printf("%s\n", buff);
     	} 
      
        finish = MPI_Wtime();
        // printf("%f\n", finish - start);                         
    } 

    else
    {
      
    	MPI_Recv(&my_number, BUFSIZE, MPI_INT, 0, TAG_X, MPI_COMM_WORLD, &stat);
    	
        // pridavani prijatych cisel do registru X
    	proc[myid].X = my_number;  	    	
		proc[myid].C = 1;
	

		int h = 0;
		
        // cyklus razeni
        for (int i = 1; i <= numprocs*2; ++i)
		{
			if (i < numprocs)
     		{	
        		MPI_Recv(&proc[myid].Y, BUFSIZE, MPI_INT, myid - 1 , TAG_Y, MPI_COMM_WORLD, &stat);       				
     		}			

            // algoritmus pro inkrementovani registru C 
            // slouzi pro osetreni stejnych cisel ze vstupu
            if(i <= (myid - 1))
            {
                if ((proc[myid].X >= proc[myid].Y))
                {
                    proc[myid].C = proc[myid].C + 1;                
                }
            }

            if(i >= myid && i < numprocs)
            {
                if ((proc[myid].X > proc[myid].Y))
                {
                    proc[myid].C = proc[myid].C + 1;                
                }
            }


  			if (myid < (numprocs-1))
	   		{		  								   
				MPI_Send(&proc[myid].Y, 1, MPI_INT, myid + 1, TAG_Y, MPI_COMM_WORLD);
			}


            // posilani registru X procesoru s cislem podle registru C
            // nasledny prijem cisla procesoru odkohokoliv a ulozeni do registru Z
			if (i> numprocs)
			{            
                MPI_Send(&proc[myid].X, 1, MPI_INT, proc[myid].C , TAG_Z, MPI_COMM_WORLD);
                MPI_Recv(&proc[myid].Z, BUFSIZE, MPI_INT, MPI_ANY_SOURCE , TAG_Z, MPI_COMM_WORLD, &stat);
			}
         
		}

        // posilani serazenych cisel hlavnimu procesoru
		for (int k = 1; k < numprocs; ++k)
		{
			sprintf(idstr, "%d",proc[myid].Z);
     		MPI_Send(idstr, BUFSIZE, MPI_CHAR, 0, TAG, MPI_COMM_WORLD);
		}
    }
  
    MPI_Finalize(); 
 	return 0;
 }