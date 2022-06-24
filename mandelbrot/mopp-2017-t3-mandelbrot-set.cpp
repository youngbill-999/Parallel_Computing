#include <complex>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;
static sem_t g_semaphore;

void count(int row_max,int row_start,int row,int column,int part_n,char **mat){

				
      for(int r = row_start; r < row_start+row; ++r){
		for(int c = 0; c < column; ++c){
			complex<float> z;
			int n = 0;
			while(abs(z) < 2 && ++n < part_n)
				z = pow(z, 2) + decltype(z)(
					(float)c * 2 / column - 1.5,
					(float)r * 2 / row_max - 1
				);
			mat[r-row_start][c]=(n == part_n ? '#' : '.');
		}
	}
}

struct draw_args {
	int part_row;
	int part_column;
	char **part_mart;
};
struct max_args {
	int max_row;
	int max_column;
	int max_n;
	int i;
	char **max_mart;
	int threads;
	struct draw_args args;
	
	

};

void devide(int max_row, int max_column,int max_n,char **mat,int threads,struct draw_args *args,struct max_args *max_args){
		 
		 
         char **part_mart[threads];
		 int part_r = max_row/threads;
		 
		 for(int i=0;i<threads;i++)
		 {
			 int part_row;
			 
			 if(i==threads-1)
			 {
                part_row = max_row-(part_r*i);
                
			 }
			 else{
				 part_row = part_r;
                
			 }
			 
             part_mart[i] = (char**)malloc(sizeof(char*)*part_row);
			 
             for (int j=0; j<part_row;j++)
			 {
		       part_mart[i][j]=(char*)malloc(sizeof(char)*max_column);
			      
             }
              
		   args[i].part_row = part_row;
		   args[i].part_column = max_column;
		   args[i].part_mart = part_mart[i];


		   max_args[i].max_row = max_row;
	       max_args[i].max_column = max_column;
	       max_args[i].max_n = max_n;
	       max_args[i].i = i;
	       max_args[i].max_mart = mat;
	       max_args[i].threads = threads;
	       max_args[i].args = args[i];
		 

          }

}

void *draw(void* part_args)
{
	struct max_args data = *(struct max_args *)part_args;
	int max_row = data.max_row;
	int max_column = data.max_column;
	int max_n = data.max_n;
	char **max_mart = data.max_mart;
    int threads = data.threads;
	int i = data.i;

	struct draw_args args = data.args;
	int part_row = args.part_row;
	int part_column  = args.part_column;
	char **part_mart = args.part_mart;

	
    int start = i*(max_row/threads);
    count(max_row, start, part_row, part_column, max_n, part_mart);
	   for (int j=0; j<part_row;j++)
			 {
		      
			      for(int m=0;m<part_column;m++)
				  {
					
					 max_mart[start+j][m]= part_mart[j][m];
					 
				  }

        }


	
}

int main(){
	int max_row, max_column, max_n;
	cin >> max_row;
	cin >> max_column;
	cin >> max_n;
    char **mat = (char**)malloc(sizeof(char*)*max_row);
    for (int i=0; i<max_row;i++)
		mat[i]=(char*)malloc(sizeof(char)*max_column);
	int cpus = get_nprocs();
   if (getenv("MAX_CPUS")) {
        cpus = atoi(getenv("MAX_CPUS"));
    }
    
    assert(cpus > 0 && cpus <= 64);
    fprintf(stderr, "Running on %d CPUs\n", cpus);
	
   int threads ;
   if(cpus==1)
   {
      for (int i=0; i<max_row;i++)
		mat[i]=(char*)malloc(sizeof(char)*max_column);

	  for(int r = 0; r < max_row; ++r){
		for(int c = 0; c < max_column; ++c){
			complex<float> z;
			int n = 0;
			while(abs(z) < 2 && ++n < max_n)
				z = pow(z, 2) + decltype(z)(
					(float)c * 2 / max_column - 1.5,
					(float)r * 2 / max_row - 1
				);
			mat[r][c]=(n == max_n ? '#' : '.');
		}
	}
   }
   else
   {
	   threads = 2*cpus+1;
   
  
	struct draw_args *allargs=(draw_args*)malloc(threads*sizeof(draw_args));
	struct max_args *data=(max_args*)malloc(threads*sizeof(max_args));
	devide(max_row, max_column, max_n, mat, threads,allargs,data);
    

	pthread_t Pthreads[threads];
	for (int i = 0; i < threads; ++i)
	{
	            
		pthread_create(&Pthreads[i], NULL, &draw, (void*)&data[i]);
		
	}

	for (int i = 0; i < threads; ++i)
		pthread_join(Pthreads[i], NULL);
    }



	for(int r = 0; r < max_row; ++r){
		for(int c = 0; c < max_column; ++c)
			std::cout << mat[r][c];
		cout << '\n';
	}	
}


