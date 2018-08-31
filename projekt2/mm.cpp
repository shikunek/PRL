 // Program: Násobení matic
 // Autor: Petr Polanský
 // Login: xpolan07
 /* Popis: Program vypočítá součin dvou matic, které jsou v souborech mat1 a mat2.
            Algoritmus je znám pod názvem Mesh multiplication, tedy násobení za 
            pomoci mřížky procesorů.
 */

 #include <mpi.h>
 #include <iostream>
 #include <stdio.h>
 #include <string.h>
 #include <fstream>
 #include <sstream>
 #include <vector>	
 using namespace std;

 #define TAG_A 0
 #define TAG_B 1
 #define TAG_C 2
 #define TAG_poc 3
 #define TAG_rad 4
 #define BUFSIZE 128

int main(int argc, char *argv[])
{

	int numprocs;               //pocet procesoru
    int myid;                   //muj rank
 
    char idstr[32];     
    char buff[BUFSIZE];
	  //MPI INIT
    MPI_Init(&argc,&argv);                          // inicializace MPI 
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);       // zjistíme, kolik procesů běží 
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);           // zjistíme id svého procesu 
  	MPI_Status stat;            //struct- obsahuje kod- source, tag, error
               

	if(myid == 0)
	{
        /* PRIJEM ZE SOUBORU MAT1 */
		ifstream infile("mat1");
		string line;
   
        int l = 0;
    
        int pocet_radku = 0;
        int k = 0;
    
        vector<vector<int>> matice1;
    
	    while (std::getline(infile, line))
	    {
	  	    istringstream iss(line);
	  	    int n;	  
            k = 0;	
            vector<int> mat1;
        
            if (l == 0)
            {
                iss >> pocet_radku;
                l = 1;
            }

  		    while (iss >> n)
  		    {
  		  	    mat1.push_back(n);
                k++;    
  		    }  
             
            matice1.push_back(mat1); 
  		
	    }    

        /* PRIJEM ZE SOUBORU MAT2 */
        ifstream file("mat2");
        string line1;
        vector<int> mat2;

        int l1 = 0;
        // int k1 = 0;
        int pocet_sloupcu = 0;

    
        vector<vector<int>> matice2;
    
        while (std::getline(file, line1))
        {
            istringstream iss(line1);
            int n;        
            vector<int> mat2;
            if (l1 == 0)
            {
                iss >> pocet_sloupcu;
                l1= 1;
            }

            while (iss >> n)
            {
                mat2.push_back(n);
            
            }       
            matice2.push_back(mat2); 
      
        }   

        int mat1_rad = k; 

        double start, finish;
        start = MPI_Wtime();

        // KOLIK SLOUPCU MA MATICE
        for (int p = 1; p < numprocs; ++p)
        {
            MPI_Send(&pocet_sloupcu, 1, MPI_INT, p, TAG_rad, MPI_COMM_WORLD);   
        }



        // KOLIK CISEL MA PROCESOR PRIJMOUT
        for (int p = 1; p < numprocs; ++p)
        {
            MPI_Send(&mat1_rad, 1, MPI_INT, p, TAG_poc, MPI_COMM_WORLD);   
        }

        int index_A = 1;
        
        /* POSILANI "A" NA PRVNI PROCESORY NA RADCICH */
        for (int q = 1; q < matice1.size(); ++q)
        {
            for (int h = 0; h < matice1[1].size(); ++h)
            {            
                MPI_Send(&matice1[q][h], 1, MPI_INT, index_A, TAG_A, MPI_COMM_WORLD); 
            }
               
            index_A += pocet_sloupcu;
                
        }
		
        /* POSILANI "B" */
        for (int i = 1; i < matice2.size(); ++i)
        {
            for (int l = 0; l < matice2[i].size(); ++l)
            {                                        
                MPI_Send(&matice2[i][l], 1, MPI_INT, l+1, TAG_B, MPI_COMM_WORLD);                          
            }                
        }          

        printf("%d:%d\n",pocet_radku,pocet_sloupcu);

        for (int i = 1; i <= numprocs-1; ++i)
        {
            MPI_Recv(buff, BUFSIZE, MPI_CHAR, i, TAG_C, MPI_COMM_WORLD, &stat);

            printf("%s",buff);
            
            if (i % pocet_sloupcu == 0)
            {
                printf("\n");
            }
            
            else
            {
                printf(" ");
            }
        }
        
        // printf("\n");
        finish = MPI_Wtime();
        // printf("%f\n", finish - start);      
    }

    else
    {
      int myA = 0;       
      int myB = 0;
      int c = 0;
      int pocet_A = 0;
      int poc_slo = 0;
      
      /* PRIJEM POCTU SLOUPCU, KTERE SE MAJI PRIJMOUT */
      MPI_Recv(&poc_slo, BUFSIZE, MPI_INT, 0, TAG_rad, MPI_COMM_WORLD, &stat);      

      /* PRIJEM POCTU PRVKU, KTERE SE MAJI PRIJMOUT */
      MPI_Recv(&pocet_A, BUFSIZE, MPI_INT, 0, TAG_poc, MPI_COMM_WORLD, &stat);

      /* KAZDY PROCESOR PRIJIMA STEJNY POCET PRVKU */
      for (int i = 0; i < pocet_A; ++i)
      {
            if (myid == 1)
            {
                MPI_Recv(&myB, BUFSIZE, MPI_INT, 0, TAG_B, MPI_COMM_WORLD, &stat);
                MPI_Recv(&myA, BUFSIZE, MPI_INT, 0, TAG_A, MPI_COMM_WORLD, &stat); 
                c += myA * myB;
                
                if (myid + 1 <= numprocs-1 && (poc_slo != 1))
                {
                    MPI_Send(&myA, 1, MPI_INT, myid + 1, TAG_A, MPI_COMM_WORLD);    
                }
                if (myid + poc_slo <= numprocs-1)
                {
                    MPI_Send(&myB, 1, MPI_INT, myid + poc_slo, TAG_B, MPI_COMM_WORLD);    
                }
                
                
            }

            // LEVE KRAJNI PROCESORY PRIJIMAJI OD HLAVNIHO PROCESORU "A" A POSILAJI TO DAL
            else if ((myid % poc_slo == 1) && myid != 1)
            {
                
                MPI_Recv(&myA, BUFSIZE, MPI_INT, 0, TAG_A, MPI_COMM_WORLD, &stat); 
                MPI_Recv(&myB, BUFSIZE, MPI_INT, myid - poc_slo, TAG_B, MPI_COMM_WORLD, &stat); 
                
                c += myA * myB;
                if (myid + 1 <= numprocs-1 && (poc_slo != 1))
                {
                    MPI_Send(&myA, 1, MPI_INT, myid+1, TAG_A, MPI_COMM_WORLD);
                }
                
                if (myid + poc_slo < numprocs)
                {
                    MPI_Send(&myB, 1, MPI_INT, myid + poc_slo, TAG_B, MPI_COMM_WORLD);                          
                }
                
            }

            // HORNI KRAJNI PROCESORY PRIJIMAJI OD HLAVNIHO PROCESORU "B" A POSILAJI TO DAL
            else if(myid > 1 && myid <= poc_slo)
            {               
                
                MPI_Recv(&myB, BUFSIZE, MPI_INT, 0, TAG_B, MPI_COMM_WORLD, &stat);
                MPI_Recv(&myA, BUFSIZE, MPI_INT, myid-1, TAG_A, MPI_COMM_WORLD, &stat); 
                c += myA * myB;
                
                if (myid + 1 <= numprocs-1)
                {
                    MPI_Send(&myA, 1, MPI_INT, myid+1, TAG_A, MPI_COMM_WORLD);
                }

                if (myid + poc_slo <= numprocs-1)
                {
                    MPI_Send(&myB, 1, MPI_INT, myid + poc_slo, TAG_B, MPI_COMM_WORLD);
                }
                
                
            }

            // POSLEDNI RADEK PROCESORU UZ NEPOSILA DAL "B" A UPLNE POSLEDNI NEPOSILA ANI "A"
            else if (myid + poc_slo >= numprocs)
            {
                // PRO PRIPAD KDY DRUHA MATICE MA JEN JEDEN SLOUPEC
                if (poc_slo == 1)
                {
                    MPI_Recv(&myA, BUFSIZE, MPI_INT, 0, TAG_A, MPI_COMM_WORLD, &stat); 
                }

                else
                {
                    MPI_Recv(&myA, BUFSIZE, MPI_INT, myid - 1, TAG_A, MPI_COMM_WORLD, &stat); 
                }
                
                MPI_Recv(&myB, BUFSIZE, MPI_INT, myid - poc_slo, TAG_B, MPI_COMM_WORLD, &stat); 
                c += myA * myB;
                
                if (myid < numprocs-1)
                {
                    MPI_Send(&myA, 1, MPI_INT, myid + 1, TAG_A, MPI_COMM_WORLD);   
                }
                
                
            }

            // KRAJNI PROCESORY KROME POSLEDNIHO JIZ NIKAM NEPOSILAJI "A"
            else if((myid % poc_slo == 0) && (myid + poc_slo <= numprocs))
            {
                
                if (poc_slo == 1)
                {
                    MPI_Recv(&myA, BUFSIZE, MPI_INT, 0, TAG_A, MPI_COMM_WORLD, &stat);
                }

                else
                {
                    MPI_Recv(&myA, BUFSIZE, MPI_INT, myid-1, TAG_A, MPI_COMM_WORLD, &stat);     
                }
                
                MPI_Recv(&myB, BUFSIZE, MPI_INT, myid - poc_slo, TAG_B, MPI_COMM_WORLD, &stat);     
                c += myA * myB;
                MPI_Send(&myB, 1, MPI_INT, myid + poc_slo, TAG_B, MPI_COMM_WORLD); 
                
                
            }

            // OSTATNI POSILAJI DAL
            else
            {
                MPI_Recv(&myA, BUFSIZE, MPI_INT, myid-1, TAG_A, MPI_COMM_WORLD, &stat); 
                MPI_Recv(&myB, BUFSIZE, MPI_INT, myid - poc_slo, TAG_B, MPI_COMM_WORLD, &stat); 
                
                c += myA * myB;
                MPI_Send(&myA, 1, MPI_INT, myid + 1, TAG_A, MPI_COMM_WORLD); 
                MPI_Send(&myB, 1, MPI_INT, myid + poc_slo, TAG_B, MPI_COMM_WORLD); 
            }

           
      }
      // POSLANI VYPOCTU HLAVNIMU PROCESORU
      sprintf(idstr, "%d",c);
      MPI_Send(idstr, BUFSIZE, MPI_CHAR, 0, TAG_C, MPI_COMM_WORLD);            	
    
    }

    MPI_Finalize(); 
	return 0;
}