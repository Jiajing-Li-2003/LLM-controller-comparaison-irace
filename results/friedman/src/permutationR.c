#include <R.h>
#include <Rdefines.h>
#include <Rinternals.h>
#include <Rmath.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <assert.h>

//#include "random.h"
#define VERYVERBOSE(X) 
#define VERBOSE(X) 
#define LIGHTVERBOSE(X) 

#ifndef LONG_LONG
# ifdef __GNUC__
#  define LONG_LONG long long
//#  ifndef ULLONG_MAX
//#   define ULLONG_MAX   18446744073709551615ULL
//#  endif
# else
#  define LONG_LONG long
//#  ifndef ULLONG_MAX
//#   define ULLONG_MAX   ULONG_MAX
//#  endif
# endif
#endif

#  ifndef ULLONG_MAX
#   define ULLONG_MAX   0
#endif

#define IM1 2147483563 
#define IM2 2147483399 
#define AM (1.0/IM1) 
#define IMM1 (IM1-1) 
#define IA1 40014 
#define IA2 40692 
#define IQ1 53668 
#define IQ2 52774 
#define IR1 12211 
#define IR2 3791 
#define NTAB 32 
#define NDIV (1+IMM1/NTAB) 
#define EPS 1.2e-7 
#define RNMX (1.0-EPS) 

long seed;


float ran2(long *idum) 
     //Long period (> 2 × 1018) random number generator of L Ecuyer with
     //Bays-Durham shu e and added safeguards. Returns a uniform random
     //deviate between 0.0 and 1.0 (exclusive of the endpoint values). Call
     //with idum a negative integer to initialize; thereafter, do not alter
     //idum between successive deviates in a sequence. RNMX should
     //approximate the largest oating value that is less than 1.
{ 
  int j; 
  long k; 
  static long idum2=123456789; 
  static long iy=0; 
  static long iv[NTAB];
  float temp;
  if (*idum <= 0) { 
    //Initialize. 
    if (-(*idum) < 1) *idum=1; //Be sure to prevent idum = 0. 
    else *idum = -(*idum);
    idum2=(*idum);
    for (j=NTAB+7;j>=0;j--) 
      { //Load the shu e table (after 8 warm-ups). 
	k=(*idum)/IQ1;
	*idum=IA1*(*idum-k*IQ1)-k*IR1; 
	if (*idum < 0) *idum += IM1;
	if (j < NTAB) iv[j] = *idum;
      } 
    iy=iv[0];
  } 
  k=(*idum)/IQ1;
  //Start here when not initializing. 
  *idum=IA1*(*idum-k*IQ1)-k*IR1;//Compute idum=(IA1*idum) % IM1 without over ows by Schrage s method.
  if (*idum < 0) *idum += IM1;
  k=idum2/IQ2;
  idum2=IA2*(idum2-k*IQ2)-k*IR2; //Compute idum2=(IA2*idum) % IM2 likewise. 
  if (idum2 < 0) idum2 += IM2;
  j=iy/NDIV; //Will be in the range 0..NTAB-1. 
  iy=iv[j]-idum2; //Here idum is shu ed, idum and idum2 are combined to generate output. 
  iv[j] = *idum;
  if (iy < 1) iy += IMM1;
  if ((temp=AM*iy) > RNMX) return RNMX; //Because users don t expect endpoint values. 
  else return temp;
};



float ran2b(void)
{
  return ran2(&seed);
};

double alpha;
double epsilon;
int nx,ny,nz;
int BB;
//unsigned LONG_LONG *prob;
double *prob;
double ***H;
double **K;
double **Q;
//int count ;


int pairwise_paired(double diff, int i, int j, double ni);
int pairwise_oneway(double diff, int i, int j, double ni);
int pairwise_CSP(double diff, int i, int j, double ni);
int pairwise_CSP_exact(double diff, int i, int j, double ni, int *perm);
int pairwise_USP(double diff, int i, int j, double ni);

inline unsigned LONG_LONG factorial (const unsigned char x)
{
   unsigned char i;
   unsigned LONG_LONG f = 1;

   if (x==0)
     return 1;
   for (i = 2; i <= x; i++) f *= i;
   return f;
};


inline unsigned LONG_LONG binom (const int x, int y)
{
   unsigned LONG_LONG e, d;
   unsigned long i;
   if (y > x) return 0;
   if (y == x || y == 0) return 1;
   if (y > x/2) y = x - y;
   e = x; d = y;
   { for (i = (x-1); i > (x-y); i--) e *= i; };
   { for (i = (y-1); i > 1; i--) d *= i; };
   return (e / d);
}


inline unsigned LONG_LONG power(unsigned LONG_LONG int x, int e)
{
  int i;
  unsigned LONG_LONG y = 1;
  //Rprintf("pow: %u %d \n",x,e);
  for (i = 0; i < e; i++) y = y*x;
  //Rprintf("pow: %u\n",y);
  // assert(y<ULONG_MAX);
  return y;
}

/*
inline unsigned LONG_LONG* assign_probabilities_USP(int b, int r)
{
  unsigned LONG_LONG *p;
  int i;
  int k=0;
  if (r % 2 == 0) //r is even
    {
      k = r/2;
     
    }
  else //r is odd 
    {
      k = (r-1)/2;
     
    }
  p = (unsigned LONG_LONG *) malloc((k+1)*sizeof(unsigned LONG_LONG));
  p[0]=1;
  for (i = 1; i<=k; i++)
    { 
      p[i] = p[i-1] + power(binom(r,i),(2*b));
      Rprintf("%llu %llu %llu\n",binom(r,i),power(binom(r,i),(2*b)),p[i]);
      if (p[i]<=p[i-1])
	{
	  Rprintf("Adjusting for unsigned long (long) overflow. No guarantee of correctness.\n");
	  do {
	    p[i++] = ULLONG_MAX;
	  } while (i<=k);
	}
    }
  return p;
}
*/

inline int choose_mu_USP(int r)
{
  int mu;
  double p;
  
  p = ran2b();
  //Rprintf("%f %f %f\n",prob[r],unif_rand(),p);
  mu=0;
  while (p > prob[mu])
    {
      mu++;
      //Rprintf("%d %f\n",mu,prob[mu]);
    }
  return mu;
}



void shuffle_array(int *v, const int size) {
  int  i, j, help;

  for ( i = 0 ; i < size; i++ ) 
    v[i] = i;

  for ( i = 0 ; i < size-1 ; i++) {
    j = (int) ( ran2b() * (size - i)); 
    //assert( i + j < size );
    help = v[i];
    v[i] = v[i+j];
    v[i+j] = help;
  }
  VERYVERBOSE( Rprintf("Random vector:\n");
	       for (i = 0 ; i < size ; i++ ) 
	       Rprintf(" %ld ",v[i]);
	       Rprintf("\n"); )
    }


void APC_permutation_CMC(double *A, double *Ai, int *k, int *b, int *r,  
		     double *nu_prob,
		     double *ni, 
		     double *alpha_i, int *B,int *csp_usp)
     //SEXP permutation(SEXP A, SEXP Ai, SEXP k, SEXP b, SEXP r,
     // 		SEXP initial_ni, 
     // 		SEXP alpha_i, SEXP B, SEXP epsilon_i) 
{
  int i,x,y,j,l,z;
  int c;
  double *diffs;
  int *indexes;
  int **comparisons;
 
  int (*pairwise)(double diff, int i, int j, double ni);

  // K=NULL;
  //Q=NULL;
  //H=NULL;
  seed = time(NULL);
  nx = *k; 
  ny = *b; 
  nz = *r; 

  BB = *B; 
  alpha = *alpha_i;
  epsilon = 1/BB;
   
  alpha = alpha/2*BB;//ceil(alpha/2*BB);

  if (alpha<1)
    alpha=1;

  LIGHTVERBOSE(Rprintf("Entering APC_permutation_CMC with ni=%f alpha=%f",*ni,alpha,*csp_usp);
    Rprintf("%d, %d, %d, %f, %f, %d, %f\n",nx,ny,nz,*ni,alpha,BB);)
  VERBOSE(
  Rprintf("in C->%f\n",(*ni));
  //Rprintf("Entering permut\n");
  Rprintf("%d, %d, %d, %f, %f, %d, %f\n",nx,ny,nz,*ni,alpha,BB);
  //Rprintf("%f --> accept %f \n",alpha/2,ceil(alpha/2*BB));
  //set_seed(0);
  //PutRNGstate();
  Rprintf("\n%d %d\n",nz,ny,nx);
  )
    
  //------------------------------------------------------------
  c = nx*(nx-1)/2;
  diffs = (double *) malloc(c * sizeof(double));
  indexes = (int *) malloc(c * sizeof(int));
  comparisons = (int **) malloc(2 * sizeof(int *));
  comparisons[0] = (int *) malloc(c * sizeof(int));
  comparisons[1] = (int *) malloc(c * sizeof(int));
  
  z=0;
  for (i=0; i < (nx-1); i++)
    {
      for (j=i+1; j < nx; j++)
	{
	  diffs[z]=fabs(Ai[i]-Ai[j]);
	  comparisons[0][z]=i;
	  comparisons[1][z]=j;
	  indexes[z]=z;
	  z++;
	}
    }
  
  //Rprintf("\n");
  revsort(diffs,indexes,c);
  //------------------------------------------------------------
   
  if (ny==1) //case b=1 k>1 r>1 one-way
    {
      pairwise = pairwise_oneway;
      K = (double **) malloc(nx*sizeof(double **));
       
      for (x = 0; x<nx; x++)
	{ 
	  K[x] = (double *) malloc(nz*sizeof(double));
	  for (z = 0; z<nz; z++)
	    {
	      K[x][z] = (A)[z+x*nz];
	    }
	}
    }
  else if (nz==1) //case b>1 k>1 r==1  paired 
    {
      pairwise = pairwise_paired;
      Q = (double **) malloc(nx*sizeof(double **));
      for (x = 0; x<nx; x++)
	{ 
	  Q[x] = (double *) malloc(ny*sizeof(double));
	  for (y = 0; y<ny; y++)
	    {
	      Q[x][y] = (A)[x+y*nx];
	    }
	}
    }
  else if ((*csp_usp) == 1)//case b>1 k>1 r>1  synchronised CSP
    {
      pairwise = pairwise_CSP;
      H = (double ***) malloc(nx*sizeof(double **));
      for (x = 0; x<nx; x++)
	{ 
	  H[x] = (double **) malloc(ny*sizeof(double *));
	  //Rprintf("\n%f\n",ny);
	  for (y = 0; y<ny; y++)
	    {
	      H[x][y] = (double *) malloc(nz*sizeof(double));
	      //Rprintf("\n%f\n",nz);
	      for (z = 0; z<nz; z++)
		{
		  H[x][y][z] = (A)[z+x*nz+y*nz*nx];
		}
	    }
	}
    }
  else if (*csp_usp == 2) //case b>1 k>1 r>1  synchronised USP
    {
      pairwise = pairwise_USP;
      //prob = assign_probabilities_USP(ny,nz);
      prob = (double *) malloc((nz+1)*sizeof(double));
      for (z = 0; z<=nz; z++)
	{
	  prob[z] = nu_prob[z];
	   VERBOSE(Rprintf("%f ",prob[z]);)
	}
      

      H = (double ***) malloc(nx*sizeof(double **));
      for (x = 0; x<nx; x++)
	{ 
	  H[x] = (double **) malloc(ny*sizeof(double *));
	  //Rprintf("\n%f\n",ny);
	  for (y = 0; y<ny; y++)
	    {
	      H[x][y] = (double *) malloc(nz*sizeof(double));
	      //Rprintf("\n%f\n",nz);
	      for (z = 0; z<nz; z++)
		{
		  H[x][y][z] = (A)[z+x*nz+y*nz*nx];
		}
	    }
	}
    }
  else
    {
      Rprintf("Something wrong in Pairwise: CSP_USP not set?\n");
    }
  /*
   for (i=0; i < (nx-1); i++)
    {
      for (j=i+1; j < nx; j++)
	{
	  Rprintf("%d %d %f \n",i,j,fabs(Ai[i]-Ai[j]));
	}
    }
  */
   /*for (i=0; i < c; i++)
    {
      Rprintf("%f %d %d %d\n",diffs[i],indexes[i],comparisons[0][indexes[i]],comparisons[1][indexes[i]]);
    }
  Rprintf("--------\n");
  return;
  */
   l=0;
  while (l<c)
    {
      i=comparisons[0][indexes[l]];
      j=comparisons[1][indexes[l]];
      if (!pairwise(Ai[i]-Ai[j],i,j,(*ni)))
	{
	  *ni = (*ni) + (double) (*ni)/100;	  
	  l=0;
	  continue;
	}
      l++;
      //Rprintf("%f %d %d %f\n",Ai[i]-Ai[j],i,j,*ni);
    }
  //GetRNGstate();
  LIGHTVERBOSE(Rprintf("\nFinal in C %f\n",(*ni));)
  free(comparisons[1]);
  free(comparisons[0]);
  free(comparisons);
  free(indexes);
  free(diffs);
  // Rprintf("\nFinal in C %p %p %p\n",K,Q,H);

  if (K)
    {
      for (i = 0; i<nx; i++)
	{ 
	  free(K[i]);
	}
      free(K);
    }
 if (Q)
    {
      for (i = 0; i<nx; i++)
	{ 
	  free(Q[i]);
	}
      free(Q);
    }
  if (H)
    {
      for (i = 0; i<nx; i++)
	{ 
	  for (j = 0; j<ny; j++)
	    {
	      free(H[i][j]);
	    }
	  free(H[i]);
	}
      free(H);
    }
  if (prob)
    free(prob);
  //return ni;
}


void Pairwise_permutation_CMC(double *A, double *dif, int *b, int *r, 
			  double *nu_prob,
			  double *ni, 
			  double *alpha_i, int *B, int *csp_usp)
     //SEXP permutation(SEXP A, SEXP Ai, SEXP k, SEXP b, SEXP r,
     // 		SEXP initial_ni, 
     // 		SEXP alpha_i, SEXP B, SEXP epsilon_i) 
{
  int i,x,y,j,z;
  
  int (*pairwise)(double dif, int i, int j, double ni);

  // K=NULL;
  //Q=NULL;
  //H=NULL;
  seed = time(NULL);
  nx = 2; 
  ny = *b; 
  nz = *r; 

  BB = *B; 
  alpha = *alpha_i;
  epsilon = 1/BB;
     
  alpha = alpha/2*BB;//ceil(alpha/2*BB);
  if (alpha<1)
    alpha=1;

  LIGHTVERBOSE(Rprintf("Entering Pairwise_permutation_CMC with ni=%f alpha=%f\n",*ni,alpha);)
  VERBOSE(
  Rprintf("in C->%f\n",(*ni));
  //Rprintf("Entering permut\n");
  Rprintf("k:%d, b:%d, r:%d, dif:%f, ni:%f, alpha:%f B:%d\n",nx,ny,nz,*dif,*ni,alpha,BB);
  //Rprintf("%f --> accept %f \n",alpha/2,ceil(alpha/2*BB));
  //set_seed(0);
  //PutRNGstate();
  Rprintf("\n%d %d\n",nz,ny,nx);
  )
    
 
  //------------------------------------------------------------
  /*for (y = 0; y<ny; y++)
      {
	Rprintf("\nInstance %d\n",y);
	for (z = 0; z<nz; z++)
	  {
	    Rprintf("\nRun %d ",z);
	    for (i = 0; i<nx; i++)
	      { 
		Rprintf("%f ",(A)[z+i*nz+y*nz*nx]);
	      }
	  }
      }
  Rprintf("\n");
  Rprintf("-->%f\n ",*dif);
  */
  if (ny==1) //case b=1 k>1 r>1 one-way
    {
      pairwise = pairwise_oneway;
      K = (double **) malloc(nx*sizeof(double **));
       
      for (x = 0; x<nx; x++)
	{ 
	  K[x] = (double *) malloc(nz*sizeof(double));
	  for (z = 0; z<nz; z++)
	    {
	      K[x][z] = (A)[z+x*nz];
	    }
	}
      
    }
  else if (nz==1) //case b>1 k>1 r==1  paired 
    {
      pairwise = pairwise_paired;
      Q = (double **) malloc(nx*sizeof(double **));
      for (x = 0; x<nx; x++)
	{ 
	  Q[x] = (double *) malloc(ny*sizeof(double));
	  for (y = 0; y<ny; y++)
	    {
	      Q[x][y] = (A)[x+y*nx];
	    }
	}
       /*  for (y = 0; y<ny; y++)
      {
      Rprintf("\nInstance %d\n",y);
      for (z = 0; z<nz; z++)
      {
      Rprintf("\nRun %d ",z);
      for (i = 0; i<nx; i++)
      { 
	       
      Rprintf("%f ",(A)[z+i*nz+y*nz*nx]);
      }
      }
      }*/
    }
  else if (*csp_usp == 1)//case b>1 k>1 r>1  synchronised USP
    {
      pairwise = pairwise_CSP;
      
      H = (double ***) malloc(nx*sizeof(double **));
       
      for (x = 0; x<nx; x++)
	{ 
	  H[x] = (double **) malloc(ny*sizeof(double *));
	  //Rprintf("\n%f\n",ny);
	  for (y = 0; y<ny; y++)
	    {
	      H[x][y] = (double *) malloc(nz*sizeof(double));
	      //Rprintf("\n%f\n",nz);
	      for (z = 0; z<nz; z++)
		{
		  H[x][y][z] = (A)[z+x*nz+y*nz*nx];
		}
	    }
	}
       /*  for (y = 0; y<ny; y++)
      {
      Rprintf("\nInstance %d\n",y);
      for (z = 0; z<nz; z++)
      {
      Rprintf("\nRun %d ",z);
      for (i = 0; i<nx; i++)
      { 
	       
      Rprintf("%f ",(A)[z+i*nz+y*nz*nx]);
      }
      }
      }*/
    }
  else if (*csp_usp == 2)//case b>1 k>1 r>1  synchronised USP
    {
      pairwise = pairwise_USP;
      //prob = assign_probabilities_USP(ny,nz);
      prob = (double *) malloc((nz+1)*sizeof(double));
      for (z = 0; z<=nz; z++)
	{
	  prob[z] = nu_prob[z]; 
	  VERBOSE(Rprintf("%f ",prob[z]);)
	  
	}
      
      H = (double ***) malloc(nx*sizeof(double **));
       
      for (x = 0; x<nx; x++)
	{ 
	  H[x] = (double **) malloc(ny*sizeof(double *));
	  //Rprintf("\n%f\n",ny);
	  for (y = 0; y<ny; y++)
	    {
	      H[x][y] = (double *) malloc(nz*sizeof(double));
	      //Rprintf("\n%f\n",nz);
	      for (z = 0; z<nz; z++)
		{
		  H[x][y][z] = (A)[z+x*nz+y*nz*nx];
		}
	    }
	}
       /*  for (y = 0; y<ny; y++)
      {
      Rprintf("\nInstance %d\n",y);
      for (z = 0; z<nz; z++)
      {
      Rprintf("\nRun %d ",z);
      for (i = 0; i<nx; i++)
      { 
	       
      Rprintf("%f ",(A)[z+i*nz+y*nz*nx]);
      }
      }
      }*/
    }
  else
    {
      Rprintf("Something wrong in Pairwise: CSP_USP not set\n");
    }
  
  
  while (!pairwise(*dif,0,1,(*ni))) 
    {
      *ni = (*ni) + (double) (*ni)/100;
      //Rprintf("-->%f\n",(*ni));
    }
  //GetRNGstate();
  LIGHTVERBOSE(Rprintf("\nFinal in C %f\n",(*ni));)

  if (K)
    {
      for (i = 0; i<nx; i++)
	{ 
	  free(K[i]);
	}
      free(K);
    }
 if (Q)
    {
      for (i = 0; i<nx; i++)
	{ 
	  free(Q[i]);
	}
      free(Q);
    }
  if (H)
    {
      for (i = 0; i<nx; i++)
	{ 
	  for (j = 0; j<ny; j++)
	    {
	      free(H[i][j]);
	    }
	  free(H[i]);
	}
      free(H);
    }
  if (prob)
    free(prob);
  //return ni;
}



void Pairwise_permutation_exact(double *A, double *dif, int *b, int *r, 
				double *nu_prob,
				double *ni, 
				double *alpha_i, 
				int *B, int *permut,
				int *csp_usp)
     //SEXP permutation(SEXP A, SEXP Ai, SEXP k, SEXP b, SEXP r,
     // 		SEXP initial_ni, 
     // 		SEXP alpha_i, SEXP B, SEXP epsilon_i) 
{
  int i,x,y,j,z;
  
  int (*pairwise)(double diff, int i, int j, double ni, int *perm);

  // K=NULL;
  //Q=NULL;
  //H=NULL;
  seed = time(NULL);
  nx = 2; 
  ny = *b; 
  nz = *r; 

  BB = *B; 
  alpha = *alpha_i;
  epsilon = 1/BB;
     
  alpha = alpha/2*BB;//ceil(alpha/2*BB);

  if (alpha<1)
    alpha=1;


  LIGHTVERBOSE(Rprintf("Entering Pairwise_permutation_exact with ni=%f alpha=%f\n",*ni,alpha);)
  VERBOSE(
  Rprintf("in C->%f\n",(*ni));
  //Rprintf("Entering permut\n");
  Rprintf("k:%d, b:%d, r:%d, dif:%f, ni:%f, alpha:%f B:%d\n",nx,ny,nz,*dif,*ni,alpha,BB);
  //Rprintf("%f --> accept %f \n",alpha/2,ceil(alpha/2*BB));
  //set_seed(0);
  //PutRNGstate();
  Rprintf("\n%d %d\n",nz,ny,nx);
  )
    
 
  //------------------------------------------------------------
  /*for (y = 0; y<ny; y++)
      {
	Rprintf("\nInstance %d\n",y);
	for (z = 0; z<nz; z++)
	  {
	    Rprintf("\nRun %d ",z);
	    for (i = 0; i<nx; i++)
	      { 
		Rprintf("%f ",(A)[z+i*nz+y*nz*nx]);
	      }
	  }
      }
  Rprintf("\n");
  Rprintf("-->%f\n ",*dif);
  */
    if (*csp_usp == 1)//case b>1 k>1 r>1  synchronised USP
    {
      pairwise = pairwise_CSP_exact;
      
      H = (double ***) malloc(nx*sizeof(double **));
       
      for (x = 0; x<nx; x++)
	{ 
	  H[x] = (double **) malloc(ny*sizeof(double *));
	  //Rprintf("\n%f\n",ny);
	  for (y = 0; y<ny; y++)
	    {
	      H[x][y] = (double *) malloc(nz*sizeof(double));
	      //Rprintf("\n%f\n",nz);
	      for (z = 0; z<nz; z++)
		{
		  H[x][y][z] = (A)[z+x*nz+y*nz*nx];
		}
	    }
	}
       /*  for (y = 0; y<ny; y++)
      {
      Rprintf("\nInstance %d\n",y);
      for (z = 0; z<nz; z++)
      {
      Rprintf("\nRun %d ",z);
      for (i = 0; i<nx; i++)
      { 
	       
      Rprintf("%f ",(A)[z+i*nz+y*nz*nx]);
      }
      }
      }*/
    }
    else 
    {
      Rprintf("Something wrong in Pairwise_permutation_exact: CSP_USP not set??\n");
    }
  
  
  while (!pairwise(*dif,0,1,(*ni),(permut))) 
    {
      *ni = (*ni) + (double) (*ni)/100;
      //Rprintf("-->%f\n",(*ni));
    }
  //GetRNGstate();
  LIGHTVERBOSE(Rprintf("\nFinal in C %f\n",(*ni));)

  if (K)
    {
      for (i = 0; i<nx; i++)
	{ 
	  free(K[i]);
	}
      free(K);
    }
 if (Q)
    {
      for (i = 0; i<nx; i++)
	{ 
	  free(Q[i]);
	}
      free(Q);
    }
  if (H)
    {
      for (i = 0; i<nx; i++)
	{ 
	  for (j = 0; j<ny; j++)
	    {
	      free(H[i][j]);
	    }
	  free(H[i]);
	}
      free(H);
    }
  if (prob)
    free(prob);
  //return ni;
}



//this function has to be improved. The use of green code investigated.
int pairwise_paired(double diff, int i, int j, double ni)
{
  //double diff = REAL(Ai)[i]-REAL(Ai)[j];
  int greater=0;
  int y,l;
  double D[ny];
  double sum_d;
  double Tni0;
  //int shuffled_index_pool[2*ny];
  
  VERBOSE(
  Rprintf("APC_pairwise_paired %f\n",ni);
  )
 
  /*     for (y = 0; y<ny; y++)
      {
      Rprintf("\nRun %d ",y);
      Rprintf("%f %f",Q[i][y],Q[j][y]);
      
      }
    
      return 1;
  */
  sum_d=0;
  for (y=0;y<ny;y++)
    {
      D[y] =Q[i][y] -Q[j][y];
      sum_d+=D[y];
    }
  //Rprintf("%f %f %f\n",sum_d/ny,diff,ni);
  //return 1;  
//  assert(abs(sum_d/ny-diff)<0.0001);
  Tni0 = -ni; //Xhi.[1]-Xhi.[2]
	   
  for (l=0; l < BB; l++)
    {
      //Rprintf("%d\n",mu);
      //Rprintf("%f %f %f\n",prob[nz],unif_rand(),p);

      /*shuffle_array(shuffled_index_pool, 2*nz);
      sum_d = 0;
      for (y=0; y < ny; y++)
	{
	  if (shuffled_index_pool[y]<ny)
	    sum_d += D[y];
	  else
	    sum_d -= D[y];
	}
      */

      //shuffle_array(shuffled_index_pool, 2*nz);
      sum_d = 0;
      for (y=0; y < ny; y++)
	{
	  if ( ran2b() < 0.5 )
	    sum_d += D[y];
	  else
	    sum_d -= D[y];
	}


      //Rprintf("%f %f\n",Tni0,((double) sum_i/(ny*nz) - (double) sum_j/(ny*nz)));
      //if (fabs(Tni0-((double) sum_i/(ny*nz) - (double) sum_j/(ny*nz)))<0.000001)
      //  {
      //    count++;
      //  }
      if (((double) sum_d/(ny) -diff + ni) <= Tni0)
	greater++;
    }
	
  //p = (double) greater/BB;
  //Rprintf("\ncount %d %f\n",greater,alpha);
  //if (p-alpha/2 < (epsilon/2))
  if ( greater<=alpha )
    {
      //Rprintf("%f %f OK\n", p, ni);
      return 1;
    }
  else
    {
      //Rprintf("%f %f NO\n", p, ni);
      return 0;
    }
}
  




int pairwise_oneway(double diff, int i, int j, double ni)
{
  //double diff = REAL(Ai)[i]-REAL(Ai)[j];
  int greater=0;
  int z,l;
  double sum_i, sum_j;
       
  double Tni0;
  double pool[2*nz];
  int shuffled_index_pool[2*nz];
       
  VERBOSE(
	  Rprintf("pairwise_oneway ");
	  Rprintf("k:%d, b:%d, r:%d, dif:%f, ni:%f, alpha:%f\n",nx,ny,nz,diff,ni,alpha);
	  )
   /*    for (z = 0; z<nz; z++)
      {
      Rprintf("\nRun %d ",z);
      Rprintf("%f %f",K[i][z],K[j][z]);
      
      }
   */
      //return 1;
      
  for (z=0;z<nz;z++)
    {
      pool[z] = K[i][z]-diff-ni;
      pool[z+nz] = K[j][z];
    }
       
  Tni0 = -ni; //Xhi.[1]-Xhi.[2]
       
  for (l=0; l < BB; l++)
    {
      shuffle_array(shuffled_index_pool, 2*nz);
      sum_i=0;
      sum_j=0;
      for (z=0; z < nz; z++)
	{
	  sum_i += pool[shuffled_index_pool[z]];
	  sum_j += pool[shuffled_index_pool[z+nz]];
	}
      if (((double) sum_i/(nz) - (double) sum_j/(nz)) <= Tni0)
	greater++;
    }
  //p = (double) greater/BB;
  //Rprintf("\ncount %d\n",count);
  //if (p-alpha/2 < (epsilon/2))
  if ( greater<=alpha )
    {
      //Rprintf("%f %f OK\n", p, ni);
      return 1;
    }
  else
    {
      //Rprintf("%f %f NO\n", p, ni);
      return 0;
    }
}
  




int pairwise_CSP(double diff, int i, int j, double ni)
{
  //double diff = REAL(Ai)[i]-REAL(Ai)[j];
  int greater=0;
  int y,z,l,t;
  double M[ny][2*nz];
  double sum_i, sum_j;
  int shuffled_index_ij[2*nz];


  double Tni0;
  
  LIGHTVERBOSE(
  Rprintf("pairwise_CSP ");
  Rprintf("k:%d, b:%d, r:%d, dif:%f, ni:%f, alpha:%f B:%d\n",nx,ny,nz,diff,ni,alpha,BB);
  )

  for (y=0;y<ny;y++)
    {
          t=0;
      for (z=0;z<nz;z++)
	M[y][t++] = H[i][y][z]-diff-ni;
      for (z=0;z<nz;z++)
	M[y][t++] = H[j][y][z];   
    }
  //Xhi. <- apply(M,1,mean) 
  Tni0 = -ni; //Xhi.[1]-Xhi.[2]
  for (l=0; l < BB; l++)
    {
      sum_i=0;
      sum_j=0;
      shuffle_array(shuffled_index_ij, 2*nz);
      
      for (y=0; y < ny; y++)
	{
	  for (z=0; z < nz; z++)
	    {
	      sum_i += M[y][shuffled_index_ij[z]];
	      sum_j += M[y][shuffled_index_ij[nz+z]];
	    }
	}
      //if (fabs(Tni0-((double) sum_i/(ny*nz) - (double) sum_j/(ny*nz)))<0.000001)
      //  {
      //    count++;
      //  }
      if (((double) sum_i/(ny*nz) - (double) sum_j/(ny*nz)) <= Tni0)
	greater++;
    }
  //p = (double) greater/BB;
  //if (p-alpha/2 < (epsilon/2))

  if ( greater<=alpha )
    {
      //Rprintf("%f %f OK\n", greater, alpha);
      //Rprintf("%f %f OK\n", p, ni);
      return 1;
    }
  else
    {
      //Rprintf("%f %f NO\n", p, ni);
      return 0;
    }
}



int pairwise_USP(double diff, int i, int j, double ni)
{
  //double diff = REAL(Ai)[i]-REAL(Ai)[j];
  int greater=0;
  int y,z,l,mu;
  double M[2][ny][nz];
  double sum_i, sum_j;
  int shuffled_index_i[nz];
  int shuffled_index_j[nz];
  double Tni0;
  
  LIGHTVERBOSE(
  Rprintf("pairwise_USP ");
  Rprintf("k:%d, b:%d, r:%d, dif:%f, ni:%f, alpha:%f B:%d\n",nx,ny,nz,diff,ni,alpha,BB);
  )
  for (y=0;y<ny;y++)
    for (z=0;z<nz;z++)
      {
	M[0][y][z] = H[i][y][z]-diff-ni;
	M[1][y][z] = H[j][y][z];
      }
	   
  //Xhi. <- apply(M,1,mean) 
  Tni0 = -ni; //Xhi.[1]-Xhi.[2]

 
  
  for (l=0; l < BB; l++)
    {
      //Rprintf("%d\n",mu);
      mu = choose_mu_USP(nz);
      assert(mu>=0 && mu<=nz);
	      
      sum_i=0;
      sum_j=0;
      for (y=0; y < ny; y++)
	{
	  shuffle_array(shuffled_index_i, nz);
	  shuffle_array(shuffled_index_j, nz);
		  
	  for (z=0; z < mu; z++)
	    {
	      sum_i += M[1][y][shuffled_index_j[z]];
	      sum_j += M[0][y][shuffled_index_i[z]];
	    }
	  for (z=mu; z < nz; z++)
	    {
	      sum_i += M[0][y][shuffled_index_i[z]];
	      sum_j += M[1][y][shuffled_index_j[z]];  
	    }
	}
      //Rprintf("%f %f\n",Tni0,((double) sum_i/(ny*nz) - (double) sum_j/(ny*nz)));
      //if (fabs(Tni0-((double) sum_i/(ny*nz) - (double) sum_j/(ny*nz)))<0.000001)
      //  {
      //    count++;
      //  }
      if (((double) sum_i/(ny*nz) - (double) sum_j/(ny*nz)) <= Tni0)
	greater++;
    }

  //p = (double) greater/BB;
  //Rprintf("\ncount %d\n",count);
  //if (p-alpha/2 < (epsilon/2))
  if ( greater<=alpha )
    {
      //Rprintf("%f %f OK\n", p, ni);
      return 1;
    }
  else
    {
      //Rprintf("%f %f NO\n", p, ni);
      return 0;
    }
}
  



void MAIN_EFFECTS_permutation_CMC(double *A, int *k, int *b, int *r, 
			      double *nu_prob, int *B, 
			      double *alpha, int *csp_usp)
{
  unsigned int i,j,y,z,l,t;
  //int *mus;
  double sum_i, sum_j, observed, grand_sum, intermidiate;
  int greater=0;
  double *tmp;
  int *shuffled_index_i;
  int *shuffled_index_j;
  int **shuffled_index_ji;
  int mu;

  seed = time(NULL);
  nx = *k; 
  ny = *b; 
  nz = *r; 
  
  BB = *B; 
  
  FILE *out;
  
  VERBOSE(
	  out = fopen("txt","w");
	  )
  //PutRNGstate();
  //count = 0;
  //c = nx*(nx-1)/2;
  LIGHTVERBOSE(
	  Rprintf("MAIN_EFFECTS_permutation CMC ");
	  Rprintf("k:%d, b:%d, r:%d, alpha:%f B:%d\n",nx,ny,nz,*alpha,BB);
	  )
    
   
    if (ny==1) //case b=1 k>1 r>1 one-way
      {//TO TEST!!!!
	observed = 0;   
	for (i=0; i < nx; i++)
	  {
	    sum_i=0;
	    for (z=0; z < nz; z++)
	      {
		sum_i += (A)[z+i*nz]; 
	      }
	    observed += pow(sum_i/nz,2);
	  }
    
	shuffled_index_j = (int *) malloc(nx*nz * sizeof(int));
	
	for (l=0; l < BB; l++)
	  {
	    shuffle_array(shuffled_index_j, nx*nz);
	    for (i=0; i < nx; i++)
	      {
		for (z=0; z < nz; z++)
		  {
		    sum_i += A[shuffled_index_j[z+i*nz]];//H[j][y][shuffled_index_i[z]];
		  }
		grand_sum += pow(sum_i/nz,2);
	      }
	    if (grand_sum >= observed)
	      greater++;
	  }
	free(shuffled_index_j);
      }
    else if (nz==1) //case b>1 k>1 r==1  paired 
      {

	
	observed = 0;   
	for (i=0; i < nx; i++)
	  {
	    sum_i=0;
	    for (j=0; j < ny; j++)
	      {
		sum_i += (A)[i+j*nx]; 
	      }
	    observed += pow(sum_i/ny,2);
	  }
 
 	shuffled_index_ji = (int **) malloc(ny * sizeof(int *));
	for (j=0; j < ny; j++)
	  {
	    shuffled_index_ji[j] = (int *) malloc(nx * sizeof(int));
	  }

	for (l=0; l < BB; l++)
	  {
	    grand_sum = 0;
	    for (j=0; j < ny; j++)
	      {
		shuffle_array(shuffled_index_ji[j], nx);
	      }
	    for (i=0; i < nx; i++)
	      {
		sum_i=0;
		for (j=0; j < ny; j++)
		  {
		    sum_i += (A)[shuffled_index_ji[j][i]+j*nx];
		  }
		grand_sum += pow(sum_i/ny,2);
	      }
     	
	    if (grand_sum >= observed)
	      greater++;
	  }

	for (j=0; j < ny; j++)
	  {
	    free(shuffled_index_ji[j]);
	  }
	free(shuffled_index_ji);
      }
    else if (*csp_usp == 1)//case b>1 k>1 r>1  synchronised CSP!!!
      {//----------------------------------------------------------------------
	//prob = assign_probabilities_USP(ny,nz);
	shuffled_index_i = (int *) malloc(2*nz * sizeof(int));
	tmp = (double *) malloc(2*nz * sizeof(double));
	
	observed = 0;   
	for (i=0; i < (nx-1); i++)
	  {
	    for (j=i+1; j < nx; j++)
	      {
		intermidiate = 0;
		for (y=0; y < ny; y++)
		  {
		    sum_i=0;
		    sum_j=0;
		    for (z=0; z < nz; z++)
		      {
			sum_i += A[y*nx*nz+i*nz+z]; 
			sum_j += A[y*nx*nz+j*nz+z];
		      }
		    intermidiate += (sum_i-sum_j);
		  }
		observed += pow(intermidiate,2);
	      }
	  }
    
	
	for (l=0; l < BB; l++)
	  {
	    grand_sum = 0;
	    shuffle_array(shuffled_index_i, 2*nz);
	 
	    for (i=0; i < (nx-1); i++)
	      {
		for (j=i+1; j < nx; j++)
		  {
		    intermidiate = 0;
		    
		    for (y =0 ; y < ny; y++)
		      {
			sum_i=0;
			sum_j=0;
			t=0;
			for (z=0; z < nz; z++)
			  tmp[t++] = A[y*nx*nz+i*nz+z];
			for (z=0; z < nz; z++)
			  tmp[t++] = A[y*nx*nz+j*nz+z];
			  
			for (z=0; z < nz; z++)
			  {
			    sum_i += tmp[shuffled_index_i[z]];
			    sum_j += tmp[shuffled_index_i[nz+z]];
			  }
			intermidiate += (sum_i-sum_j);
		      }
		    grand_sum += pow(intermidiate,2);
		  }
	      }
	    VERBOSE(fprintf(out,"%f\n",grand_sum);)
	    if (grand_sum >= observed)
	      greater++;
	  }
	VERBOSE(Rprintf("observed %f\n",observed);)
	free(prob);
	free(shuffled_index_i);
	free(tmp);
      }
    else if (*csp_usp == 2)//case b>1 k>1 r>1  synchronised USP!!!!
      {
	//prob = assign_probabilities_USP(ny,nz);
	prob = (double *) malloc((nz+1)*sizeof(double));
	for (z = 0; z<=nz; z++)
	  {
	    prob[z] = nu_prob[z];
	    VERBOSE(Rprintf("%f ",prob[z]);)
	      }
	shuffled_index_i = (int *) malloc(nz * sizeof(int));
	shuffled_index_j = (int *) malloc(nz * sizeof(int));
	//mus = (int *) malloc(ny * sizeof(int));
	
	observed = 0;   
	for (i=0; i < (nx-1); i++)
	  {
	    
	    for (j=i+1; j < nx; j++)
	      {
		intermidiate = 0;
		for (y=0; y < ny; y++)
		  {
		    sum_i=0;
		    sum_j=0;
		    for (z=0; z < nz; z++)
		      {
			sum_i += A[y*nx*nz+i*nz+z]; 
			sum_j += A[y*nx*nz+j*nz+z];
		      }
		    intermidiate += (sum_i-sum_j);
		  }
		observed += pow(intermidiate,2);
	      }
	  }
    
 
	for (l=0; l < BB; l++)
	  {
	    grand_sum = 0;
    
	    mu = choose_mu_USP(nz);
	    assert(mu>=0 && mu<=nz);
	    for (i=0; i < (nx-1); i++)
	      {
		for (j=i+1; j < nx; j++)
		  {
		    intermidiate = 0;
		    for (y =0 ; y < ny; y++)
		      {
			sum_i=0;
			sum_j=0;
			shuffle_array(shuffled_index_i, nz);
			shuffle_array(shuffled_index_j, nz);
			for (z=0; z < mu; z++)
			  {
			    sum_i += A[y*nx*nz+j*nz+shuffled_index_j[z]];//H[j][y][shuffled_index_i[z]];
			    sum_j += A[y*nx*nz+i*nz+shuffled_index_i[z]];//H[i][y][shuffled_index_j[z]];
			  }
			for (z=mu; z < nz; z++)
			  {
			    sum_i += A[y*nx*nz+i*nz+shuffled_index_i[z]];//H[i][y][shuffled_index_i[z]];
			    sum_j += A[y*nx*nz+j*nz+shuffled_index_j[z]];//H[j][y][shuffled_index_j[z]];
			  }
			intermidiate += (sum_i-sum_j);
		      }
		    grand_sum += pow(intermidiate,2);
		  }
	      }
	    VERBOSE(fprintf(out,"%f\n",grand_sum);)
	    if (grand_sum >= observed)
	      greater++;
	  }
	free(prob);
	free(shuffled_index_i);
	free(shuffled_index_j);
      }
    else 
      {
	Rprintf("Error in Permutation main!\n");
	exit(1);
      }
 
  VERBOSE(fclose(out);)
  //Rprintf("%f ",observed);
  //Rprintf("\ncount %d\n",count);
  //Rprintf("%f \n",observed);
  (*alpha) = (double) greater/BB;
  //Rprintf("%f \n",*alpha);
  //---------------------------------------------------------
  //GetRNGstate();
  //free(mus);
 
}





void MAIN_EFFECTS_permutation_USP_free(double *A, int *k, int *b, int *r, 
				       double *nu_prob, int *B, 
				       double *alpha)
{
  int i,j,y,z,l;
  int mu;
  double sum_i, sum_j, observed, grand_sum, intermidiate;
  int greater=0;
  int *shuffled_index_i;
  int *shuffled_index_j;
  
  seed = time(NULL);
  nx = *k; 
  ny = *b; 
  nz = *r; 
  
  BB = *B; 
  //c = nx*(nx-1)/2;
  LIGHTVERBOSE(
	  Rprintf("MAIN_EFFECTS_permutation USP");
	  Rprintf("k:%d, b:%d, r:%d, alpha:%f B:%d\n",nx,ny,nz,*alpha,BB);
	  )
  //PutRNGstate();
  
  //c = nx*(nx-1)/2;



  //prob = assign_probabilities_USP(ny,nz);
 if (nz > 1 && ny > 1)
      {
	prob = (double *) malloc((nz+1)*sizeof(double));
	for (z = 0; z<=nz; z++)
	  {
	    prob[z] = nu_prob[z];
	    VERBOSE(Rprintf("%f ",prob[z]);)
	  }
      }
  /*
    H = (double ***) malloc(nx*sizeof(double **));
    for (i = 0; i<nx; i++)
    { 
    H[i] = (double **) malloc(ny*sizeof(double *));
    for (j = 0; j<ny; j++)
    {
    H[i][j] = (double *) malloc(nz*sizeof(double));
    for (z = 0; z<nz; z++)
    {
    H[i][j][z] = (A)[z*nx*ny+j*nx+i];
    }
    }
    }
  */
  /*
  Rprintf("\n%d %d\n",nz,ny,nx);
 
   
  for (y = 0; y<ny; y++)
    {
      Rprintf("\nInstance %d\n",y);
      for (z = 0; z<nz; z++)
	{
	  Rprintf("\nRun %d ",z);
	  for (i = 0; i<nx; i++)
	    { 
	       
	      Rprintf("%f ",(A)[y*nx*nz+i*nz+z]);
	    }
	}
    }
  */
  // for (z = 0; z<nz; z++)
  //   {
  //     Rprintf("\n-->%d\n",z);
  //     for (i = 0; i<nx; i++)
  //	{ 
  //	  Rprintf("\n-->%d ",i);
  //	  for (j = 0; j<ny; j++)
  //	    {
  //	      Rprintf("%f ",H[i][j][z]);
  //	    }
  //	}
  //   }
  // 

  //  for (i = 0; i<nx; i++)
  //    {
  //      Rprintf("%f ",Ai[i]);
  //    }
  //
  
  shuffled_index_i = (int *) malloc(nz * sizeof(int));
  shuffled_index_j = (int *) malloc(nz * sizeof(int));
  
  observed = 0;   
  for (i=0; i < (nx-1); i++)
    {
      for (j=i+1; j < nx; j++)
	{
	  intermidiate = 0;
	  for (y=0; y < ny; y++)
	    {
	      sum_i=0;
	      sum_j=0;
	      for (z=0; z < nz; z++)
		{
		  sum_i += A[y*nx*nz+i*nz+z];//H[i][y][z];
		  sum_j += A[y*nx*nz+j*nz+z];//H[j][y][z];
		}
	      intermidiate += (sum_i-sum_j);
	    }
	  observed += pow(intermidiate,2);
	}
    }
  //Rprintf("%f ",observed);
    
  for (l=0; l < BB; l++)
    {
      grand_sum = 0;
      //for (y = 0; y < ny; y++)
      //	{
      //	 
      //	  //Rprintf("%d\n",mu);
      //	}

      for (i=0; i < (nx-1); i++)
	{
	  for (j=i+1; j < nx; j++)
	    {
	      mu = choose_mu_USP(nz);
	      assert(mu>=0 && mu<=nz);

	      /*p = unif_rand()*(long double)(prob[nz]);
	      //Rprintf("%f %f %f\n",prob[nz],unif_rand(),p);
	      mu=0;
	      while (p > (long double) prob[mu])
		{
		  mu++;
		  //Rprintf("%d %f\n",mu,prob[mu]);
		  }*/
	      //Rprintf("%d\n",mu);
	      intermidiate = 0;
	      for (y =0 ; y < ny; y++)
		{
		  sum_i=0;
		  sum_j=0;
		  shuffle_array(shuffled_index_i, nz);
		  shuffle_array(shuffled_index_j, nz);
		  for (z=0; z < mu; z++)
		    {
		      sum_i += A[y*nx*nz+j*nz+shuffled_index_j[z]];//H[j][y][shuffled_index_j[z]];
		      //Rprintf("%f %f\n",A[shuffled_index_j[z]*nx*ny+y*nx+j],A[shuffled_index_i[z]*nx*ny+y*nx+i]);//A[shuffled_index_j[z]*nx*ny+y*nx+j];
		      sum_j += A[y*nx*nz+i*nz+shuffled_index_i[z]];//H[i][y][shuffled_index_i[z]]; //A[shuffled_index_i[z]*nx*ny+y*nx+i];/
		      //Rprintf("%f %f\n",A[shuffled_index_i[z]*nx*ny+y*nx+i],A[shuffled_index_j[z]*nx*ny+y*nx+j]);//,A[shuffled_index_i[z]*nx*ny+y*nx+i]);//A[shuffled_index_j[z]*nx*ny+y*nx+j];
		    }
		  for (z=mu; z < nz; z++)
		    {
		      sum_i += A[y*nx*nz+i*nz+shuffled_index_i[z]];
		      //Rprintf("%f\n",A[shuffled_index_i[z]*nx*ny+y*nx+i]);
		      sum_j += A[y*nx*nz+j*nz+shuffled_index_j[z]];
		      //Rprintf("%f\n",A[shuffled_index_j[z]*nx*ny+y*nx+j]);
		    }
		  intermidiate += (sum_i-sum_j);
		}
	      grand_sum += pow(intermidiate,2);
	    }
	  //if (fabs(grand_sum- observed) < 0.0001)
	  // count++;
	  //Rprintf("%f ",grand_sum);
	}
     
      if (grand_sum >= observed)
	greater++;
    }
  //Rprintf("count %d\n",greater);
  (*alpha) = (double) greater/BB;
  //Rprintf("%f \n",*alpha);
  //---------------------------------------------------------
  //GetRNGstate();
  free(shuffled_index_i);
  free(shuffled_index_j);
  if (prob)
    free(prob);
  //
  //  for (i = 0; i<nx; i++)
  //    { 
  //      for (j = 0; j<ny; j++)
  //	{
  //	  free(H[i][j]);
  //	}
  //      free(H[i]);
  //    }
  //  free(H);
}




void MAIN_EFFECTS_permutation_exact(double *A, int *k, int *b, int *r, 
				    double *nu_prob, int *B, int *permut,
				    double *alpha, int *csp_usp)
{
  unsigned int i,j,y,z,l,t;
  //int *mus;
  double sum_i, sum_j, observed, grand_sum, intermidiate;
  int greater=0;
  double *tmp;
  int *shuffled_index_i;
  int *shuffled_index_j;
  int **shuffled_index_ji;
  int mu;

  seed = time(NULL);
  nx = *k; 
  ny = *b; 
  nz = *r; 
  
  BB = *B; 
  
  FILE *out;
  
  VERBOSE(
	  out = fopen("txt","w");
	  )
  //PutRNGstate();
  //count = 0;
  //c = nx*(nx-1)/2;
  LIGHTVERBOSE(
	  Rprintf("MAIN_EFFECTS_permutation exact");
	  Rprintf("k:%d, b:%d, r:%d, alpha:%f B:%d\n",nx,ny,nz,*alpha,BB);
	  )
    
   
    if (ny==1) //case b=1 k>1 r>1 one-way
      {//TO TEST!!!!
	observed = 0;   
	for (i=0; i < nx; i++)
	  {
	    sum_i=0;
	    for (z=0; z < nz; z++)
	      {
		sum_i += (A)[z+i*nz]; 
	      }
	    observed += pow(sum_i/nz,2);
	  }
    
	shuffled_index_j = (int *) malloc(nx*nz * sizeof(int));
	
	for (l=0; l < BB; l++)
	  {
	    shuffle_array(shuffled_index_j, nx*nz);
	    for (i=0; i < nx; i++)
	      {
		for (z=0; z < nz; z++)
		  {
		    sum_i += A[shuffled_index_j[z+i*nz]];//H[j][y][shuffled_index_i[z]];
		  }
		grand_sum += pow(sum_i/nz,2);
	      }
	    if (grand_sum >= observed)
	      greater++;
	  }
	free(shuffled_index_j);
      }
    else if (nz==1) //case b>1 k>1 r==1  paired 
      {

	
	observed = 0;   
	for (i=0; i < nx; i++)
	  {
	    sum_i=0;
	    for (j=0; j < ny; j++)
	      {
		sum_i += (A)[i+j*nx]; 
	      }
	    observed += pow(sum_i/ny,2);
	  }
 
 	shuffled_index_ji = (int **) malloc(ny * sizeof(int *));
	for (j=0; j < ny; j++)
	  {
	    shuffled_index_ji[j] = (int *) malloc(nx * sizeof(int));
	  }

	for (l=0; l < BB; l++)
	  {
	    grand_sum = 0;
	    for (j=0; j < ny; j++)
	      {
		shuffle_array(shuffled_index_ji[j], nx);
	      }
	    for (i=0; i < nx; i++)
	      {
		sum_i=0;
		for (j=0; j < ny; j++)
		  {
		    sum_i += (A)[shuffled_index_ji[j][i]+j*nx];
		  }
		grand_sum += pow(sum_i/ny,2);
	      }
     	
	    if (grand_sum >= observed)
	      greater++;
	  }

	for (j=0; j < ny; j++)
	  {
	    free(shuffled_index_ji[j]);
	  }
	free(shuffled_index_ji);
      }
    else if (*csp_usp == 1)//case b>1 k>1 r>1  synchronised CSP!!!
      {//----------------------------------------------------------------------
	//prob = assign_probabilities_USP(ny,nz);
	//shuffled_index_i = (int *) malloc(2*nz * sizeof(int));
	tmp = (double *) malloc((2*nz+1) * sizeof(double));
	LIGHTVERBOSE(
		     Rprintf("MAIN_EFFECTS_permutation CSP exact %d\n",BB);
		     )


	//Rprintf("%d ",BB);
	observed = 0;   
	for (i=0; i < (nx-1); i++)
	  {
	    for (j=i+1; j < nx; j++)
	      {
		intermidiate = 0;
		for (y=0; y < ny; y++)
		  {
		    sum_i=0;
		    sum_j=0;
		    for (z=0; z < nz; z++)
		      {
			sum_i += A[y*nx*nz+i*nz+z]; 
			sum_j += A[y*nx*nz+j*nz+z];
		      }
		    intermidiate += (sum_i-sum_j);
		  }
		observed += pow(intermidiate,2);
	      }
	  }
    
	for (l=0; l < BB; l++)
	  {
	    grand_sum = 0;
	    for (i=0; i < (nx-1); i++)
	      {
		for (j=i+1; j < nx; j++)
		  {
		    intermidiate = 0;
		    
		    for (y =0 ; y < ny; y++)
		      {
			sum_i=0;
			sum_j=0;
			t=1;
			for (z=0; z < nz; z++)
			  tmp[t++] = A[y*nx*nz+i*nz+z];
			for (z=0; z < nz; z++)
			  tmp[t++] = A[y*nx*nz+j*nz+z];
			  
			for (z=0; z < nz; z++)
			  {
			    //Rprintf("(%d) [%f] ",permut[(l*2*nz)+z],tmp[permut[(l*2*nz)+z]]);
			    sum_i += tmp[permut[(l*2*nz)+z]];
			    //Rprintf("%d %f\n",permut[(l*2*nz)+nz+z],tmp[permut[(l*2*nz)+nz+z]]);
			    sum_j += tmp[permut[(l*2*nz)+nz+z]];
			  }
			//Rprintf("\n");
			intermidiate += (sum_i-sum_j);
		      }
		    //Rprintf("\n");
		    grand_sum += pow(intermidiate,2);
		  }
	      }
	    VERBOSE(fprintf(out,"%f\n",grand_sum);)
	    if (grand_sum >= observed)
	      greater++;
	  }
	VERBOSE(Rprintf("observed %f\n",observed);)
	free(tmp);
      }
    else if (*csp_usp == 2)//case b>1 k>1 r>1  synchronised USP!!!!
      {
	//prob = assign_probabilities_USP(ny,nz);
	prob = (double *) malloc((nz+1)*sizeof(double));
	for (z = 0; z<=nz; z++)
	  {
	    prob[z] = nu_prob[z];
	    VERBOSE(Rprintf("%f ",prob[z]);)
	      }
	shuffled_index_i = (int *) malloc(nz * sizeof(int));
	shuffled_index_j = (int *) malloc(nz * sizeof(int));
	//mus = (int *) malloc(ny * sizeof(int));
	
	observed = 0;   
	for (i=0; i < (nx-1); i++)
	  {
	    
	    for (j=i+1; j < nx; j++)
	      {
		intermidiate = 0;
		for (y=0; y < ny; y++)
		  {
		    sum_i=0;
		    sum_j=0;
		    for (z=0; z < nz; z++)
		      {
			sum_i += A[y*nx*nz+i*nz+z]; 
			sum_j += A[y*nx*nz+j*nz+z];
		      }
		    intermidiate += (sum_i-sum_j);
		  }
		observed += pow(intermidiate,2);
	      }
	  }
    
 
	for (l=0; l < BB; l++)
	  {
	    grand_sum = 0;
    
	    mu = choose_mu_USP(nz);
	    assert(mu>=0 && mu<=nz);
	    for (i=0; i < (nx-1); i++)
	      {
		for (j=i+1; j < nx; j++)
		  {
		    intermidiate = 0;
		    for (y =0 ; y < ny; y++)
		      {
			sum_i=0;
			sum_j=0;
			shuffle_array(shuffled_index_i, nz);
			shuffle_array(shuffled_index_j, nz);
			for (z=0; z < mu; z++)
			  {
			    sum_i += A[y*nx*nz+j*nz+shuffled_index_j[z]];//H[j][y][shuffled_index_i[z]];
			    sum_j += A[y*nx*nz+i*nz+shuffled_index_i[z]];//H[i][y][shuffled_index_j[z]];
			  }
			for (z=mu; z < nz; z++)
			  {
			    sum_i += A[y*nx*nz+i*nz+shuffled_index_i[z]];//H[i][y][shuffled_index_i[z]];
			    sum_j += A[y*nx*nz+j*nz+shuffled_index_j[z]];//H[j][y][shuffled_index_j[z]];
			  }
			intermidiate += (sum_i-sum_j);
		      }
		    grand_sum += pow(intermidiate,2);
		  }
	      }
	    VERBOSE(fprintf(out,"%f\n",grand_sum);)
	    if (grand_sum >= observed)
	      greater++;
	  }
	free(prob);
	free(shuffled_index_i);
	free(shuffled_index_j);
      }
    else 
      {
	Rprintf("Error in Permutation main!\n");
	exit(1);
      }
 
  VERBOSE(fclose(out);)
  //Rprintf("%f ",observed);
  //Rprintf("\ncount %d\n",count);
  //Rprintf("%d \n",greater);
  (*alpha) = (double) greater/BB;
  //Rprintf("%f \n",*alpha);
  //---------------------------------------------------------
  //GetRNGstate();
  //free(mus);
 
}


/*
//con la statistica di Fisher!!!!
void MAIN_EFFECTS_permutation_exact(double *A, int *k, int *b, int *r, 
				    double *nu_prob, int *B, int *permut,
				    double *alpha, int *csp_usp)
{
  unsigned int i,j,y,z,l,t;
  //int *mus;
  double sum_i, sum_j, observed, grand_sum, intermidiate, cell_mean_i,cell_mean_j,grand_mean,SSA,SSE,mean_i,mean_j;
  int greater=0;
  double *tmp;
  int *shuffled_index_i;
  int *shuffled_index_j;
  int **shuffled_index_ji;
  int mu;


  seed = time(NULL);
  nx = *k; 
  ny = *b; 
  nz = *r; 
  
  BB = *B; 
  
  FILE *out;
  
  VERBOSE(
	  out = fopen("txt","w");
	  )
  //PutRNGstate();
  //count = 0;
  //c = nx*(nx-1)/2;
  LIGHTVERBOSE(
	  Rprintf("MAIN_EFFECTS_permutation exact");
	  Rprintf("k:%d, b:%d, r:%d, alpha:%f B:%d\n",nx,ny,nz,*alpha,BB);
	  )
    
   
    if (ny==1) //case b=1 k>1 r>1 one-way
      {//TO TEST!!!!
	observed = 0;   
	for (i=0; i < nx; i++)
	  {
	    sum_i=0;
	    for (z=0; z < nz; z++)
	      {
		sum_i += (A)[z+i*nz]; 
	      }
	    observed += pow(sum_i/nz,2);
	  }
    
	shuffled_index_j = (int *) malloc(nx*nz * sizeof(int));
	
	for (l=0; l < BB; l++)
	  {
	    shuffle_array(shuffled_index_j, nx*nz);
	    for (i=0; i < nx; i++)
	      {
		for (z=0; z < nz; z++)
		  {
		    sum_i += A[shuffled_index_j[z+i*nz]];//H[j][y][shuffled_index_i[z]];
		  }
		grand_sum += pow(sum_i/nz,2);
	      }
	    if (grand_sum >= observed)
	      greater++;
	  }
	free(shuffled_index_j);
      }
    else if (nz==1) //case b>1 k>1 r==1  paired 
      {

	
	observed = 0;   
	for (i=0; i < nx; i++)
	  {
	    sum_i=0;
	    for (j=0; j < ny; j++)
	      {
		sum_i += (A)[i+j*nx]; 
	      }
	    observed += pow(sum_i/ny,2);
	  }
 
 	shuffled_index_ji = (int **) malloc(ny * sizeof(int *));
	for (j=0; j < ny; j++)
	  {
	    shuffled_index_ji[j] = (int *) malloc(nx * sizeof(int));
	  }

	for (l=0; l < BB; l++)
	  {
	    grand_sum = 0;
	    for (j=0; j < ny; j++)
	      {
		shuffle_array(shuffled_index_ji[j], nx);
	      }
	    for (i=0; i < nx; i++)
	      {
		sum_i=0;
		for (j=0; j < ny; j++)
		  {
		    sum_i += (A)[shuffled_index_ji[j][i]+j*nx];
		  }
		grand_sum += pow(sum_i/ny,2);
	      }
     	
	    if (grand_sum >= observed)
	      greater++;
	  }

	for (j=0; j < ny; j++)
	  {
	    free(shuffled_index_ji[j]);
	  }
	free(shuffled_index_ji);
      }
    else if (*csp_usp == 1)//case b>1 k>1 r>1  synchronised CSP!!!
      {//----------------------------------------------------------------------
	//prob = assign_probabilities_USP(ny,nz);
	//shuffled_index_i = (int *) malloc(2*nz * sizeof(int));
	tmp = (double *) malloc((2*nz+1) * sizeof(double));
	LIGHTVERBOSE(
		     Rprintf("MAIN_EFFECTS_permutation CSP exact %d\n",BB);
		     )


	//Rprintf("%d ",BB);
	  for (i=0; i < nx; i++)
	    {
	      for (y=0; y < ny; y++)
		{
		  sum_j=0;
		  for (z=0; z < nz; z++)
		    {
		      sum_i += A[y*nx*nz+i*nz+z]; 
		    }
		}
	    }
	grand_mean=sum_i/(nx*ny*nz);


	SSA = 0;SSE=0;
	for (i=0; i < (nx-1); i++)
	  {
	    for (j=i+1; j < nx; j++)
	      {
		mean_i = 0;
		mean_j = 0;
		for (y=0; y < ny; y++)
		  {
		    sum_i=0;
		    sum_j=0;
		    for (z=0; z < nz; z++)
		      {
			sum_i += A[y*nx*nz+i*nz+z]; 
			sum_j += A[y*nx*nz+j*nz+z]; 
		      }
		    mean_i += sum_i;
		    mean_j += sum_j;
		    cell_mean_i = sum_i/nz;
		    cell_mean_j = sum_j/nz;
		    
		    for (z=0; z < nz; z++)
		      {
			SSE += pow(A[y*nx*nz+i*nz+z] - cell_mean_i,2);
			SSE += pow(A[y*nx*nz+j*nz+z] - cell_mean_j,2);
		      }
		  }
		SSA += pow(mean_i/(nz*ny)-grand_mean,2);
		SSA += pow(mean_j/(nz*ny)-grand_mean,2);
	      }
	  }
	//MSA = SSA*nz*ny/(nx-1);
	//   MSE = SSE/(nx*ny*(nz-1));

	observed = SSA/SSE;

    
	for (l=0; l < BB; l++)
	  {
	    grand_sum = 0;
	    SSA=0;SSE=0;
	    for (i=0; i < (nx-1); i++)
	      {
		for (j=i+1; j < (nx); j++)
		  {
		    mean_i=0;
		    mean_j=0;
		    for (y =0 ; y < ny; y++)
		  {
		     t=1;
		    for (z=0; z < nz; z++)
		      {
			tmp[t++] = A[y*nx*nz+i*nz+z];

		      }
		    for (z=0; z < nz; z++)
		      {
			tmp[t++] = A[y*nx*nz+j*nz+z];
		      }
		    sum_i=0;
		    sum_j=0;
		    for (z=0; z < nz; z++)
		      {
			//Rprintf("(%d) [%f] ",permut[(l*2*nz)+z],tmp[permut[(l*2*nz)+z]]);
			sum_i += tmp[permut[(l*2*nz)+z]];
			//Rprintf("%d %f\n",permut[(l*2*nz)+nz+z],tmp[permut[(l*2*nz)+nz+z]]);
			sum_j += tmp[permut[(l*2*nz)+nz+z]];
		      }
		    cell_mean_i = sum_i/nz;
		    cell_mean_j = sum_j/nz;
		    
		    mean_i += sum_i;
		    mean_j += sum_j;
		    //Rprintf("\n");
		    //intermidiate += (sum_i-sum_j);
		    for (z=0; z < nz; z++)
		      {
			//Rprintf("(%d) [%f] ",permut[(l*2*nz)+z],tmp[permut[(l*2*nz)+z]]);
			SSE += pow(tmp[permut[(l*2*nz)+z]]-cell_mean_i,2);
			//Rprintf("%d %f\n",permut[(l*2*nz)+nz+z],tmp[permut[(l*2*nz)+nz+z]]);
			SSE += pow(tmp[permut[(l*2*nz)+nz+z]]-cell_mean_j,2);
		      }
		  }
		    //Rprintf("\n");
		    SSA += pow(mean_i/(nz*ny)-grand_mean,2);
		    SSA += pow(mean_j/(nz*ny)-grand_mean,2);
		    
		  }
	      }
	    VERBOSE(fprintf(out,"%f\n",grand_sum);)
	      if ((SSA/SSE) >= observed)
	      greater++;
	  }
	VERBOSE(Rprintf("observed %f\n",observed);)
	free(tmp);
      }
    else if (*csp_usp == 2)//case b>1 k>1 r>1  synchronised USP!!!!
      {
	//prob = assign_probabilities_USP(ny,nz);
	prob = (double *) malloc((nz+1)*sizeof(double));
	for (z = 0; z<=nz; z++)
	  {
	    prob[z] = nu_prob[z];
	    VERBOSE(Rprintf("%f ",prob[z]);)
	      }
	shuffled_index_i = (int *) malloc(nz * sizeof(int));
	shuffled_index_j = (int *) malloc(nz * sizeof(int));
	//mus = (int *) malloc(ny * sizeof(int));
	
	observed = 0;   
	for (i=0; i < (nx-1); i++)
	  {
	    
	    for (j=i+1; j < nx; j++)
	      {
		intermidiate = 0;
		for (y=0; y < ny; y++)
		  {
		    sum_i=0;
		    sum_j=0;
		    for (z=0; z < nz; z++)
		      {
			sum_i += A[y*nx*nz+i*nz+z]; 
			sum_j += A[y*nx*nz+j*nz+z];
		      }
		    intermidiate += (sum_i-sum_j);
		  }
		observed += pow(intermidiate,2);
	      }
	  }
    
 
	for (l=0; l < BB; l++)
	  {
	    grand_sum = 0;
    
	    mu = choose_mu_USP(nz);
	    assert(mu>=0 && mu<=nz);
	    for (i=0; i < (nx-1); i++)
	      {
		for (j=i+1; j < nx; j++)
		  {
		    intermidiate = 0;
		    for (y =0 ; y < ny; y++)
		      {
			sum_i=0;
			sum_j=0;
			shuffle_array(shuffled_index_i, nz);
			shuffle_array(shuffled_index_j, nz);
			for (z=0; z < mu; z++)
			  {
			    sum_i += A[y*nx*nz+j*nz+shuffled_index_j[z]];//H[j][y][shuffled_index_i[z]];
			    sum_j += A[y*nx*nz+i*nz+shuffled_index_i[z]];//H[i][y][shuffled_index_j[z]];
			  }
			for (z=mu; z < nz; z++)
			  {
			    sum_i += A[y*nx*nz+i*nz+shuffled_index_i[z]];//H[i][y][shuffled_index_i[z]];
			    sum_j += A[y*nx*nz+j*nz+shuffled_index_j[z]];//H[j][y][shuffled_index_j[z]];
			  }
			intermidiate += (sum_i-sum_j);
		      }
		    grand_sum += pow(intermidiate,2);
		  }
	      }
	    VERBOSE(fprintf(out,"%f\n",grand_sum);)
	    if (grand_sum >= observed)
	      greater++;
	  }
	free(prob);
	free(shuffled_index_i);
	free(shuffled_index_j);
      }
    else 
      {
	Rprintf("Error in Permutation main!\n");
	exit(1);
      }
 
  VERBOSE(fclose(out);)
  //Rprintf("%f ",observed);
  //Rprintf("\ncount %d\n",count);
  //Rprintf("%d \n",greater);
  (*alpha) = (double) greater/BB;
  //Rprintf("%f \n",*alpha);
  //---------------------------------------------------------
  //GetRNGstate();
  //free(mus);
 
}
*/









void INTERACTIONS_permutation_CMC(double *A, int *k, int *b, int *r, 
			      double *nu_prob, int *B, 
			      double *alpha, int *csp_usp)
{
  int i,j,y1,y2,z,l,t;
  //int *mus;
  double sum_i, sum_j, observed, grand_sum, intermidiate1,intermidiate2;
  int greater=0;
  int *shuffled_index_i;
  int *shuffled_index_j;
  double *tmp;
  int mu;
  
  seed = time(NULL);
  nx = *k; 
  ny = *b; 
  nz = *r; 
  
  BB = *B; 
  
  //c = nx*(nx-1)/2;
  LIGHTVERBOSE(
	  Rprintf("Interaction_permutation CMC");
	  Rprintf("k:%d, b:%d, r:%d, alpha:%f B:%d\n",nx,ny,nz,*alpha,BB);
	  )
  
  if (nz > 1 && ny > 1 && *csp_usp == 1) 
    {
      shuffled_index_i = (int *) malloc(2*nz * sizeof(int));
      tmp = (double *) malloc(2*nz * sizeof(double));
      observed = 0;   
      for (i=0; i < (nx-1); i++)
	{
	  for (j=i+1; j < nx; j++)
	    {
	      for (y1=0; y1 < (ny-1); y1++)
		{
		  sum_i=0;
		  sum_j=0;
		  for (z=0; z < nz; z++)
		    {
		      sum_i += A[y1*nx*nz+i*nz+z];//H[i][y][z];
		      sum_j += A[y1*nx*nz+j*nz+z];//H[j][y][z];
		    }
		  intermidiate1 = (sum_i-sum_j);
		  for (y2=y1+1; y2 < ny; y2++)
		    {
		      sum_i=0;
		      sum_j=0;
		      for (z=0; z < nz; z++)
			{
			  sum_i += A[y2*nx*nz+i*nz+z];//H[i][y][z];
			  sum_j += A[y2*nx*nz+j*nz+z];//H[j][y][z];
			}
		      intermidiate2 = (sum_i-sum_j);
		      observed += pow((intermidiate1-intermidiate2),2);
		    }
		}
	    }
	}
    
 
      for (l=0; l < BB; l++)
	{
	  grand_sum = 0;
	  shuffle_array(shuffled_index_i, 2*nz);
	  for (i=0; i < (nx-1); i++)
	    {
	      for (j=i+1; j < nx; j++)
		{

		  for (y1 =0 ; y1 < (ny-1); y1++)
		    {
		      sum_i=0;
		      sum_j=0;

		      t=0;
		      for (z=0; z < nz; z++)
			tmp[t++]= A[y1*nx*nz+i*nz+z];
		      for (z=0; z < nz; z++)
			tmp[t++]= A[y1*nx*nz+j*nz+z];

		      for (z=0; z < nz; z++)
			{
			  sum_i += tmp[shuffled_index_i[z]];
			  sum_j += tmp[shuffled_index_i[nz+z]];
			}
		      intermidiate1 = (sum_i-sum_j);
		      //shuffle_array(shuffled_index_j, 2*nz);
		      for (y2 = y1+1 ; y2 < ny; y2++)
			{
			  sum_i=0;
			  sum_j=0;

			  
			  t=0;
			  for (z=0; z < nz; z++)
			    tmp[t++]= A[y2*nx*nz+i*nz+z];
			  for (z=0; z < nz; z++)
			    tmp[t++]= A[y2*nx*nz+j*nz+z];
			      
			  for (z=0; z < nz; z++)
			    {
			      sum_i += tmp[shuffled_index_i[z]];
			      sum_j += tmp[shuffled_index_i[nz+z]];
			    }
			  intermidiate2 = (sum_i-sum_j);
			  grand_sum += pow((intermidiate1-intermidiate2),2);
			}
		    }
		}
	    }
	  //Rprintf("%f \n",grand_sum);
	  if (grand_sum >= observed)
	    greater++;
	}
      free(tmp);
      free(shuffled_index_i);
    }
  else if (nz > 1 && ny > 1 && *csp_usp == 2) 
    {
      prob = (double *) malloc((nz+1)*sizeof(double));
      for (z = 0; z<=nz; z++)
	{
	  prob[z] = nu_prob[z];
	  VERBOSE(Rprintf("%f ",prob[z]);)
	    }
      
      shuffled_index_i = (int *) malloc(nz * sizeof(int));
      shuffled_index_j = (int *) malloc(nz * sizeof(int));
      //mus = (int *) malloc(ny * sizeof(int));
  
      observed = 0;   
      for (i=0; i < (nx-1); i++)
	{
	  for (j=i+1; j < nx; j++)
	    {
	      for (y1=0; y1 < (ny-1); y1++)
		{
		  sum_i=0;
		  sum_j=0;
		  for (z=0; z < nz; z++)
		    {
		      sum_i += A[y1*nx*nz+i*nz+z];//H[i][y][z];
		      sum_j += A[y1*nx*nz+j*nz+z];//H[j][y][z];
		    }
		  intermidiate1 = (sum_i-sum_j);
		  for (y2=y1+1; y2 < ny; y2++)
		    {
		      sum_i=0;
		      sum_j=0;
		      for (z=0; z < nz; z++)
			{
			  sum_i += A[y2*nx*nz+i*nz+z];//H[i][y][z];
			  sum_j += A[y2*nx*nz+j*nz+z];//H[j][y][z];
			}
		      intermidiate2 = (sum_i-sum_j);
		      observed += pow((intermidiate1-intermidiate2),2);
		    }
		}
	    }
	}
    
 
      for (l=0; l < BB; l++)
	{
      
	  //  for (y = 0; y < ny; y++)
	  //	{
	  //	  p = unif_rand()*(double)(prob[nz]);
	  //	  //Rprintf("%f %f %f\n",prob[nz],unif_rand(),p);
	  //	  mus[y]=0;
	  //	  while (p > prob[mus[y]])
	  //	    {
	  //	      mus[y]++;
	  //	      //Rprintf("%d %f\n",mu,prob[mu]);
	  //	    }
	  //	  //Rprintf("%d\n",mu);
	  //	}

	  mu = choose_mu_USP(nz);
	  assert(mu>=0 && mu<=nz);

	  grand_sum = 0;
	  /*
	    p = unif_rand()*(long double)(prob[nz]);
	    mu=0;
	    while (p > (long double) prob[mu])
	    {
	    mu++;
	    //Rprintf("%d %f\n",mu,prob[mu]);
	    }
	  */
	  for (i=0; i < (nx-1); i++)
	    {
	      for (j=i+1; j < nx; j++)
		{
		  for (y1 =0 ; y1 < (ny-1); y1++)
		    {
		      sum_i=0;
		      sum_j=0;
		      shuffle_array(shuffled_index_i, nz);
		      shuffle_array(shuffled_index_j, nz);
		      for (z=0; z < mu; z++)
			{
			  sum_i += A[y1*nx*nz+j*nz+shuffled_index_j[z]];//H[j][y][shuffled_index_i[z]];
			  sum_j += A[y1*nx*nz+i*nz+shuffled_index_i[z]];//H[i][y][shuffled_index_j[z]];
			}
		      for (z=mu; z < nz; z++)
			{
			  sum_i += A[y1*nx*nz+i*nz+shuffled_index_i[z]];//H[i][y][shuffled_index_i[z]];
			  sum_j += A[y1*nx*nz+j*nz+shuffled_index_j[z]];//H[j][y][shuffled_index_j[z]];
			}
		      intermidiate1 = (sum_i-sum_j);
		      for (y2 = y1+1 ; y2 < ny; y2++)
			{
			  sum_i=0;
			  sum_j=0;
			  shuffle_array(shuffled_index_i, nz);
			  shuffle_array(shuffled_index_j, nz);
			  for (z=0; z < mu; z++)
			    {
			      sum_i += A[y2*nx*nz+j*nz+shuffled_index_j[z]];//H[j][y][shuffled_index_i[z]];
			      sum_j += A[y2*nx*nz+i*nz+shuffled_index_i[z]];//H[i][y][shuffled_index_j[z]];
			    }
			  for (z=mu; z < nz; z++)
			    {
			      sum_i += A[y2*nx*nz+i*nz+shuffled_index_i[z]];//H[i][y][shuffled_index_i[z]];
			      sum_j += A[y2*nx*nz+j*nz+shuffled_index_j[z]];//H[j][y][shuffled_index_j[z]];
			    }
			  intermidiate2 = (sum_i-sum_j);
		      
			  grand_sum += pow((intermidiate1-intermidiate2),2);
			}
		    }
		}
	    }
	  //Rprintf("%f \n",grand_sum);
	  if (grand_sum >= observed)
	    greater++;
	}
        free(shuffled_index_i);
  free(shuffled_index_j);
  if (prob)
    free(prob);

    }
  else 
    {
      Rprintf("Interactions not possible becasue nz or ny not > 1 or csp_usp not set.\n");
    }



  //Rprintf("%f \n",observed);
  //Rprintf("%d \n",greater);
  (*alpha) = (double) greater/BB;
  //Rprintf("%f \n",*alpha);
  //---------------------------------------------------------
  //GetRNGstate();
  //free(mus);

  //
  //  for (i = 0; i<nx; i++)
  //    { 
  //      for (j = 0; j<ny; j++)
  //	{
  //	  free(H[i][j]);
  //	}
  //      free(H[i]);
  //    }
  //  free(H);
}



void INTERACTIONS_permutation_exact(double *A, int *k, int *b, int *r, 
			      double *nu_prob, int *B, int *permut,
			      double *alpha, int *csp_usp)
{
  int i,j,y1,y2,z,l,t;
  //int *mus;
  double sum_i, sum_j, observed, grand_sum, intermidiate1,intermidiate2;
  int greater=0;
  int *shuffled_index_i;
  int *shuffled_index_j;
  double *tmp;
  int mu;
  
  seed = time(NULL);
  nx = *k; 
  ny = *b; 
  nz = *r; 
  
  BB = *B; 
  
  LIGHTVERBOSE(
	       Rprintf("INTERACTION_permutation CSP exact %d\n",BB);
	       )
  
  if (nz > 1 && ny > 1 && *csp_usp == 1) 
    {
      /*      for (l=0;l<BB;l++)
	{
	  for (z=0;z<nz;z++)
	    Rprintf("%d ",permut[l*2*nz+z]);
	  for (z=0;z<nz;z++)
	    Rprintf("%d ",permut[l*2*nz+nz+z]);
	  Rprintf("\n");
	  }*/
      tmp = (double *) malloc((2*nz+1) * sizeof(double));
      observed = 0;   
      for (i=0; i < (nx-1); i++)
	{
	  for (j=i+1; j < nx; j++)
	    {
	      for (y1=0; y1 < (ny-1); y1++)
		{
		  sum_i=0;
		  sum_j=0;
		  for (z=0; z < nz; z++)
		    {
		      sum_i += A[y1*nx*nz+i*nz+z];//H[i][y][z];
		      sum_j += A[y1*nx*nz+j*nz+z];//H[j][y][z];
		    }
		  intermidiate1 = (sum_i-sum_j);
		  for (y2=y1+1; y2 < ny; y2++)
		    {
		      sum_i=0;
		      sum_j=0;
		      for (z=0; z < nz; z++)
			{
			  sum_i += A[y2*nx*nz+i*nz+z];//H[i][y][z];
			  sum_j += A[y2*nx*nz+j*nz+z];//H[j][y][z];
			}
		      intermidiate2 = (sum_i-sum_j);
		      observed += pow((intermidiate1-intermidiate2),2);
		    }
		}
	    }
	}
    
      //Rprintf("exact %d \n",BB);
      for (l=0; l < BB; l++)
	{
	  grand_sum = 0;
	  for (i=0; i < (nx-1); i++)
	    {
	      for (j=i+1; j < nx; j++)
		{

		  for (y1 =0 ; y1 < (ny-1); y1++)
		    {
		      sum_i=0;
		      sum_j=0;

		      t=0;
		      for (z=0; z < nz; z++)
			tmp[t++]= A[y1*nx*nz+i*nz+z];
		      for (z=0; z < nz; z++)
			tmp[t++]= A[y1*nx*nz+j*nz+z];

		      for (z=0; z < nz; z++)
			{
			  //Rprintf("%f %f\n ",tmp[permut[(l*2*nz)+z]-1],tmp[permut[(l*2*nz)+nz+z]-1]);
			  sum_i += tmp[permut[l*2*nz+z]-1];
			  sum_j += tmp[permut[(l*2*nz)+nz+z]-1];
			}
		      intermidiate1 = (sum_i-sum_j);
		      //shuffle_array(shuffled_index_j, 2*nz);
		      for (y2 = y1+1 ; y2 < ny; y2++)
			{
			  sum_i=0;
			  sum_j=0;
			  t=1;
			  for (z=0; z < nz; z++)
			    tmp[t++]= A[y2*nx*nz+i*nz+z];
			  for (z=0; z < nz; z++)
			    tmp[t++]= A[y2*nx*nz+j*nz+z];
			      
			  for (z=0; z < nz; z++)
			    {
			      //Rprintf("%f %f\n ",tmp[permut[(l*2*nz)+z]-1],tmp[permut[(l*2*nz)+nz+z]-1]);
			      sum_i += tmp[permut[(l*2*nz)+z]];
			      sum_j += tmp[permut[(l*2*nz)+nz+z]];
			    }
			  intermidiate2 = (sum_i-sum_j);
			  grand_sum += pow((intermidiate1-intermidiate2),2);
			}
		    }
		}
	    }
	  //Rprintf("%f \n",grand_sum);
	  if (grand_sum >= observed)
	    greater++;
	}
      free(tmp);
    }
  else if (nz > 1 && ny > 1 && *csp_usp == 2) 
    {
      prob = (double *) malloc((nz+1)*sizeof(double));
      for (z = 0; z<=nz; z++)
	{
	  prob[z] = nu_prob[z];
	  VERBOSE(Rprintf("%f ",prob[z]);)
	    }
      
      shuffled_index_i = (int *) malloc(nz * sizeof(int));
      shuffled_index_j = (int *) malloc(nz * sizeof(int));
      //mus = (int *) malloc(ny * sizeof(int));
  
      observed = 0;   
      for (i=0; i < (nx-1); i++)
	{
	  for (j=i+1; j < nx; j++)
	    {
	      for (y1=0; y1 < (ny-1); y1++)
		{
		  sum_i=0;
		  sum_j=0;
		  for (z=0; z < nz; z++)
		    {
		      sum_i += A[y1*nx*nz+i*nz+z];//H[i][y][z];
		      sum_j += A[y1*nx*nz+j*nz+z];//H[j][y][z];
		    }
		  intermidiate1 = (sum_i-sum_j);
		  for (y2=y1+1; y2 < ny; y2++)
		    {
		      sum_i=0;
		      sum_j=0;
		      for (z=0; z < nz; z++)
			{
			  sum_i += A[y2*nx*nz+i*nz+z];//H[i][y][z];
			  sum_j += A[y2*nx*nz+j*nz+z];//H[j][y][z];
			}
		      intermidiate2 = (sum_i-sum_j);
		      observed += pow((intermidiate1-intermidiate2),2);
		    }
		}
	    }
	}
    
 
      for (l=0; l < BB; l++)
	{
      
	  //  for (y = 0; y < ny; y++)
	  //	{
	  //	  p = unif_rand()*(double)(prob[nz]);
	  //	  //Rprintf("%f %f %f\n",prob[nz],unif_rand(),p);
	  //	  mus[y]=0;
	  //	  while (p > prob[mus[y]])
	  //	    {
	  //	      mus[y]++;
	  //	      //Rprintf("%d %f\n",mu,prob[mu]);
	  //	    }
	  //	  //Rprintf("%d\n",mu);
	  //	}

	  mu = choose_mu_USP(nz);
	  assert(mu>=0 && mu<=nz);

	  grand_sum = 0;
	  /*
	    p = unif_rand()*(long double)(prob[nz]);
	    mu=0;
	    while (p > (long double) prob[mu])
	    {
	    mu++;
	    //Rprintf("%d %f\n",mu,prob[mu]);
	    }
	  */
	  for (i=0; i < (nx-1); i++)
	    {
	      for (j=i+1; j < nx; j++)
		{
		  for (y1 =0 ; y1 < (ny-1); y1++)
		    {
		      sum_i=0;
		      sum_j=0;
		      shuffle_array(shuffled_index_i, nz);
		      shuffle_array(shuffled_index_j, nz);
		      for (z=0; z < mu; z++)
			{
			  sum_i += A[y1*nx*nz+j*nz+shuffled_index_j[z]];//H[j][y][shuffled_index_i[z]];
			  sum_j += A[y1*nx*nz+i*nz+shuffled_index_i[z]];//H[i][y][shuffled_index_j[z]];
			}
		      for (z=mu; z < nz; z++)
			{
			  sum_i += A[y1*nx*nz+i*nz+shuffled_index_i[z]];//H[i][y][shuffled_index_i[z]];
			  sum_j += A[y1*nx*nz+j*nz+shuffled_index_j[z]];//H[j][y][shuffled_index_j[z]];
			}
		      intermidiate1 = (sum_i-sum_j);
		      for (y2 = y1+1 ; y2 < ny; y2++)
			{
			  sum_i=0;
			  sum_j=0;
			  shuffle_array(shuffled_index_i, nz);
			  shuffle_array(shuffled_index_j, nz);
			  for (z=0; z < mu; z++)
			    {
			      sum_i += A[y2*nx*nz+j*nz+shuffled_index_j[z]];//H[j][y][shuffled_index_i[z]];
			      sum_j += A[y2*nx*nz+i*nz+shuffled_index_i[z]];//H[i][y][shuffled_index_j[z]];
			    }
			  for (z=mu; z < nz; z++)
			    {
			      sum_i += A[y2*nx*nz+i*nz+shuffled_index_i[z]];//H[i][y][shuffled_index_i[z]];
			      sum_j += A[y2*nx*nz+j*nz+shuffled_index_j[z]];//H[j][y][shuffled_index_j[z]];
			    }
			  intermidiate2 = (sum_i-sum_j);
		      
			  grand_sum += pow((intermidiate1-intermidiate2),2);
			}
		    }
		}
	    }
	  //Rprintf("%f \n",grand_sum);
	  if (grand_sum >= observed)
	    greater++;
	}
        free(shuffled_index_i);
  free(shuffled_index_j);
  if (prob)
    free(prob);

    }
  else 
    {
      Rprintf("Interactions not possible becasue nz or ny not > 1 or csp_usp not set.\n");
    }



  //Rprintf("%f \n",observed);
  //Rprintf("%d \n",greater);
  (*alpha) = (double) greater/BB;
  //Rprintf("%f \n",*alpha);
  //---------------------------------------------------------
  //GetRNGstate();
  //free(mus);

  //
  //  for (i = 0; i<nx; i++)
  //    { 
  //      for (j = 0; j<ny; j++)
  //	{
  //	  free(H[i][j]);
  //	}
  //      free(H[i]);
  //    }
  //  free(H);
}


void APC_permutation_exact(double *A, double *Ai, int *k, int *b, int *r,  
			   double *nu_prob,
			   double *ni, 
			   double *alpha_i, 
			   int *B, int *permut,
			   int *csp_usp)
     //SEXP permutation(SEXP A, SEXP Ai, SEXP k, SEXP b, SEXP r,
     // 		SEXP initial_ni, 
     // 		SEXP alpha_i, SEXP B, SEXP epsilon_i) 
{
  int i,x,y,j,l,z;
  int c;
  double *diffs;
  int *indexes;
  int **comparisons;
 
  int (*pairwise)(double diff, int i, int j, double ni, int *perm);

  // K=NULL;
  //Q=NULL;
  //H=NULL;
  seed = time(NULL);
  nx = *k; 
  ny = *b; 
  nz = *r; 

  BB = *B; 
  alpha = *alpha_i;
  epsilon = 1/BB;
   
  alpha = alpha/2*BB;//ceil(alpha/2*BB);

  if (alpha<1)
    alpha=1;


  LIGHTVERBOSE(Rprintf("Entering APC_permutation_exact with ni=%f alpha=%f alpha=%f type=%d\n",
		       *ni,*alpha_i,alpha,*csp_usp);)
  
  VERBOSE(
  Rprintf("in C->%f\n",(*ni));
  //Rprintf("Entering permut\n");
  Rprintf("%d, %d, %d, %f, %f, %d, %f %f\n",nx,ny,nz,*ni,*alpha_i,alpha,BB);
  //Rprintf("%f --> accept %f \n",alpha/2,ceil(alpha/2*BB));
  //set_seed(0);
  //PutRNGstate();
  Rprintf("\n%d %d\n",nz,ny,nx);
  )
    
  //------------------------------------------------------------
  c = nx*(nx-1)/2;
  diffs = (double *) malloc(c * sizeof(double));
  indexes = (int *) malloc(c * sizeof(int));
  comparisons = (int **) malloc(2 * sizeof(int *));
  comparisons[0] = (int *) malloc(c * sizeof(int));
  comparisons[1] = (int *) malloc(c * sizeof(int));
  
  z=0;
  for (i=0; i < (nx-1); i++)
    {
      for (j=i+1; j < nx; j++)
	{
	  diffs[z]=fabs(Ai[i]-Ai[j]);
	  comparisons[0][z]=i;
	  comparisons[1][z]=j;
	  indexes[z]=z;
	  z++;
	}
    }
  
  //Rprintf("\n");
  revsort(diffs,indexes,c);
  //------------------------------------------------------------
   
  if ((*csp_usp) == 1)//case b>1 k>1 r>1  synchronised CSP
    {
      pairwise = pairwise_CSP_exact;
      H = (double ***) malloc(nx*sizeof(double **));
      for (x = 0; x<nx; x++)
	{ 
	  H[x] = (double **) malloc(ny*sizeof(double *));
	  //Rprintf("\n%f\n",ny);
	  for (y = 0; y<ny; y++)
	    {
	      H[x][y] = (double *) malloc(nz*sizeof(double));
	      //Rprintf("\n%f\n",nz);
	      for (z = 0; z<nz; z++)
		{
		  H[x][y][z] = (A)[z+x*nz+y*nz*nx];
		}
	    }
	}
    }
  else
    {
      Rprintf("Something wrong in APC_permutation_exact: CSP_USP not set?\n");
      return;
    }
  
   l=0;
  while (l<c)
    {
      i=comparisons[0][indexes[l]];
      j=comparisons[1][indexes[l]];
      if (!pairwise(Ai[i]-Ai[j],i,j,(*ni),(permut)))
	{
	  *ni = (*ni) + (double) (*ni)/100;	  
	  l=0;
	  continue;
	}
      l++;
      //Rprintf("%f %d %d %f\n",Ai[i]-Ai[j],i,j,*ni);
    }
  //GetRNGstate();
  LIGHTVERBOSE(Rprintf("\nFinal in C %f\n",(*ni));)
  free(comparisons[1]);
  free(comparisons[0]);
  free(comparisons);
  free(indexes);
  free(diffs);
  // Rprintf("\nFinal in C %p %p %p\n",K,Q,H);

  if (K)
    {
      for (i = 0; i<nx; i++)
	{ 
	  free(K[i]);
	}
      free(K);
    }
 if (Q)
    {
      for (i = 0; i<nx; i++)
	{ 
	  free(Q[i]);
	}
      free(Q);
    }
  if (H)
    {
      for (i = 0; i<nx; i++)
	{ 
	  for (j = 0; j<ny; j++)
	    {
	      free(H[i][j]);
	    }
	  free(H[i]);
	}
      free(H);
    }
  if (prob)
    free(prob);
  //return ni;
}




int pairwise_CSP_exact(double diff, int i, int j, double ni, int *perm )
{
  //double diff = REAL(Ai)[i]-REAL(Ai)[j];
  int greater=0;
  int y,z,l,t;
  double M[ny][2*nz];
  double sum_i, sum_j,observed;
  //int shuffled_index_ij[2*nz];
  FILE *out;
  
  VERBOSE(
	  out = fopen("txt","w");
	  )

  double Tni0;
  
  LIGHTVERBOSE(
  Rprintf("pairwise_CSP_exact");
  Rprintf("k:%d, b:%d, r:%d, dif:%f, ni:%f, alpha:%f B:%d\n",nx,ny,nz,diff,ni,alpha,BB);
  )

  for (y=0;y<ny;y++)
    {
          t=0;
      for (z=0;z<nz;z++)
	{
	  M[y][t++] = H[i][y][z]-diff-ni;
	}
      for (z=0;z<nz;z++)
	{
	  M[y][t++] = H[j][y][z]; 
	}
    }
  //Xhi. <- apply(M,1,mean) 
  Tni0 = -ni; //Xhi.[1]-Xhi.[2]
  for (l=0; l < BB; l++)
    {
      sum_i=0;
      sum_j=0;
      //shuffle_array(shuffled_index_ij, 2*nz);
      
      for (y=0; y < ny; y++)
	{
	  for (z=0; z < nz; z++)
	    {
	      sum_i += M[y][perm[(l*2*nz)+z]-1];
	      sum_j += M[y][perm[(l*2*nz)+nz+z]-1];
	    }
	}
      //if (fabs(Tni0-((double) sum_i/(ny*nz) - (double) sum_j/(ny*nz)))<0.000001)
      //  {
      //    count++;
      //  }
      
      observed = sum_i/(ny * nz) - sum_j/(ny * nz);
      VERBOSE(fprintf(out,"%f %f %f\n",sum_i/(ny * nz),sum_j/(ny * nz),observed);)
      if ( observed <= Tni0)
	greater++;
    }
  VERBOSE(fclose(out);)
  //p = (double) greater/BB;
  //if (p-alpha/2 < (epsilon/2))
  if ( greater<=alpha )
    {
      //Rprintf("%f %f OK\n", p, ni);
      return 1;
    }
  else
    {
      //Rprintf("%f %f NO\n", p, ni);
      return 0;
    }
}

