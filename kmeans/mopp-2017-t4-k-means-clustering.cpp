#include <iostream>
#include <sstream>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <semaphore.h>
#include <unistd.h>

#include <sys/sysinfo.h>
#define DIM 3//数据维度

using namespace std;
struct assign_data{
   int i;
   int start;
   int end;
   int threads;
};
struct thread_data{
   //int threads;
   int n;
   int k;
  
   double *x ;
   
   int *flips_index;
   int *cluster;
   double *mean;
   int *count;
   double *sum;
   struct assign_data ad;
  
};




int main(void) {
  void *clusterFun(void *threadarg);
  void getCenter(int k,int * count,double *sum,double *mean);
 
  

  int i,j,flips; 
  double dmin,dx;
  int k,n;
  cin>>k>>n;
  
  double *x, *mean, *sum;
  int *cluster, *count, color;
  x = (double *)malloc(sizeof(double)*DIM*n);
  mean = (double *)malloc(sizeof(double)*DIM*k);
  cluster = (int *)malloc(sizeof(int)*n);

  count = (int *)malloc(sizeof(int)*k);
  sum= (double *)malloc(sizeof(double)*DIM*k);

  
  for (int i = 0; i<n; i++) 
    cluster[i] = 0;
  for (int i = 0; i<k; i++){ 
     cin>>mean[i*DIM]>>mean[i*DIM+1]>>mean[i*DIM+2];
  }
  for (int i = 0; i<n; i++){
     cin>>x[i*DIM]>>x[i*DIM+1]>>x[i*DIM+2];
  }
    
	int cpus = get_nprocs();
   if (getenv("MAX_CPUS")) {
        cpus = atoi(getenv("MAX_CPUS"));
    }
    assert(cpus > 0 && cpus <= 64);
    fprintf(stderr, "Running on %d CPUs\n", cpus);
    int threads=4*cpus+1;

    flips = n;
   
   if(cpus==1)
   {
   while(flips>0)
   {
	   flips=0;
	   
       for (int j = 0; j < k; j++) {
	        count[j] = 0; 
	   for (int i = 0; i < DIM; i++) 
	      sum[j*DIM+i] = 0.0;
	   }
	       
          for (int i = 0; i < n; i++) {
			dmin = -1; color = cluster[i];
			for (int c = 0; c < k; c++) {
				dx = 0.0;
				for (int j = 0; j < DIM; j++) 
				{
				   dx +=  (x[i*DIM+j] - mean[c*DIM+j]) * (x[i*DIM+j] - mean[c*DIM+j]);
				}
				if (dx < dmin || dmin == -1) {
					color = c;
					dmin = dx;
				}
			}
			if (cluster[i] != color) {
				flips++;
				cluster[i] = color;
	      	}
			  count[cluster[i]]++;
			for (j = 0; j < DIM; j++) 
				sum[cluster[i]*DIM+j] += x[i*DIM+j];
			  
		}
        for (int i = 0; i < n; i++) {
		          count[cluster[i]]++;
			for (j = 0; j < DIM; j++) 
				sum[cluster[i]*DIM+j] += x[i*DIM+j];
       
		}
		getCenter(k,count,sum,mean);
		
    }
   }
   else
   {
    int part = n/threads;
    pthread_t threads_index[threads];
	
	struct thread_data tw[threads];
    struct assign_data ad[threads];
	
	int *sub_count[threads];
    double *sub_sum[threads];	
    int flipss[threads];

	
     for(int i=0;i<threads;i++)
	   {
              ad[i].i = i;
		      ad[i].start = i * part;
		      if(i==threads-1)
		      {
                ad[i].end=n-1;
		      }
		      else
			  {
	            ad[i].end=(i+1)*part-1;
		      }
		      ad[i].threads = threads;
	   
			  tw[i].k=k;
			  tw[i].n=n;
			  tw[i].x=x;
			  tw[i].cluster=cluster;
	          tw[i].flips_index= flipss;
			  tw[i].mean=mean;
              sub_count[i]=(int *)malloc(sizeof(int)*k);
              sub_sum[i] = (double *)malloc(sizeof(double)*DIM*k); 
			  tw[i].sum=sub_sum[i];
			  tw[i].count=sub_count[i];
			  tw[i].ad = ad[i];
	    }

	 while(flips>0)
	{
		flips = 0;
		for (int j = 0; j < k; j++) {
	        count[j] = 0; 
	      for (int i = 0; i < DIM; i++) 
	        sum[j*DIM+i] = 0.0;
	    }  
		
        for(int i=0;i<threads;i++)
	   {

	         int ret = pthread_create(&threads_index[i], NULL, clusterFun,(void *)&tw[i]);
			 if (ret != 0)
			 {
				fprintf(stderr,"失败\n");
			 }
       
	   }
       for(int i=0;i<threads;i++)
	    {
	      pthread_join(threads_index[i], NULL);
		  flips += flipss[i];
		   for (int j = 0; j < k; j++)
		    {
			  count[j]+= sub_count[i][j];
			  for (int m = 0; m < DIM; m++) 
			  {
		        sum[j*DIM+m] += sub_sum[i][j*DIM+m];
			  }
			
		    }
	    }
	    getCenter(k,count,sum,mean);
	
    
	}

    }
	for (i = 0; i < k; i++) {
		      for (j = 0; j < DIM; j++)
			      printf("%5.2f ", mean[i*DIM+j]);
		    printf("\n");
	}
	
    return 0;
}

 
 void *clusterFun(void *threadarg)
 {     
	 
	  struct thread_data *my_data = (struct thread_data*) threadarg;
	  int *flips_index =my_data->flips_index;//
	  int n=my_data->n;
	  int k=my_data->k;
	  double *x=my_data->x;
	  int *cluster=my_data->cluster;//
	  double *mean=my_data->mean;
	  int *count=my_data->count;//
	  double *sum=my_data->sum;
      struct assign_data ad = my_data->ad;
	  int turn = ad.i;
	  int start = ad.start;
	  int end = ad.end;
	  int threads = ad.threads;

       flips_index[turn]=0;
		
		for (int j = 0; j < k; j++) {
	        count[j] = 0; 
	    for (int i = 0; i < DIM; i++) 
	        sum[j*DIM+i] = 0.0;
	    }
	    
	  
	   for (int i = start; i < end+1; i++) 
	   {
			double dmin = -1; 
			int color = cluster[i];
			for (int c = 0; c < k; c++) {
				double dx = 0.0;
				for (int j = 0; j < DIM; j++) 
				{
				   
				   dx += (x[i*DIM+j] - mean[c*DIM+j]) * (x[i*DIM+j] - mean[c*DIM+j]);
				}   
				if (dx < dmin || dmin == -1) {
					color = c;
					dmin = dx;
				}
			}
			
			if (cluster[i] != color) {
			
				flips_index[turn]++;
				cluster[i] = color;
				
	      	}
			 
	    } 
		
	   
	         for (int i = start; i < end+1; i++) 
	        {
	           count[cluster[i]]++;
			   for (int j = 0; j < DIM; j++) 
			   {
				  sum[cluster[i]*DIM+j] += x[i*DIM+j];
			   }
			 
          
	        }
			
		 
}

void getCenter(int k,int * count,double *sum,double *mean)
{
	 
    for (int i = 0; i < k; i++) {
			for (int j = 0; j < DIM; j++) {
				mean[i*DIM+j] = sum[i*DIM+j]/count[i];
  			}
		}
}

