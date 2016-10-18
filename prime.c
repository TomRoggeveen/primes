/********************************************************************************/
/* Create 'binary' file with long primes					*/
/*                                                                              */
/* - create first table in memory with primes. Use this table as source of      */
/*   dividers to determine prime						*/ 
/********************************************************************************/
#define _FILE_OFFSET_BITS 64

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <signal.h>

#define	LIM	1000000000LLU
#define	PTLIM	100000LLU		/* Number of primes (elements) in prime table	*/

#define TRUE	1
#define FALSE	0


void			findlastprimeinfile(FILE *f);
unsigned long long	fillprimetable(unsigned long long pt[]);
void			handlesignalint(int s);
void			handlesignalusr1(int s);

unsigned long long	a;
int			l;			// Loop

int main()
{
	time_t sec;
	struct tm *localnow;
	char   datestr[30];

	unsigned long long b,c,i,sp,m;
	unsigned long long pt[PTLIM];		/* Prime table 				*/

	struct {
		unsigned long long p;
	} s;

	FILE *f;

	clock_t start, end;
	double ti;

	if(signal(SIGINT,handlesignalint)==SIG_ERR)
		printf("ERROR: SIGINT handler not installed");

	if(signal(SIGUSR1,handlesignalusr1)==SIG_ERR)
		printf("ERROR: SIGUSR1 handler not installed");

	printf("\n");

	f=fopen("/home/tom/c/primes/primes.dat","a+");
	if(f==NULL){
		fprintf(stderr,"Error opening file: %s(%d)\n",strerror(errno),errno);
	}

	findlastprimeinfile(f);
	m=fillprimetable(pt);

	sp=a;
	a=a+2;
	time(&sec); 
	localnow=localtime(&sec);
	strftime(datestr,78,"%Y-%m-%d",localnow);
	printf("%s %.24s. Start calculating next %llu primes ...\n",datestr,ctime(&sec),LIM);

/*	printf("\nStorage size and value ranges of unsigned long long are %d, %llud\n",sizeof(unsigned long long),ULLONG_MAX);
	printf("\nSizeof array is %d\n",sizeof(s));
*/	

	start=clock();

	for(i=0,l=TRUE;i<LIM;a=a+2){
                for(b=1;b<a;b++){
                        c=a%pt[b];
                        if(c==0||pt[b]*pt[b]>a)
                                break;
                }
                if(pt[b]*pt[b]>a){
			i++;
			s.p=a;
			fwrite(&s,sizeof(s),1,f);
			if(f==NULL)
				fprintf(stderr,"Error writing file: %s(%d)\n",strerror(errno),errno);
		if(l==FALSE)
			break;
		}
        }
	time(&sec);
	localnow=localtime(&sec);
        strftime(datestr,78,"%Y-%m-%d",localnow);
	end=clock();
	printf("%s %.24s. End is %.f, Start is %.f, clockspersec is %.f\n",datestr,ctime(&sec),(double)end,(double)start,(double)CLOCKS_PER_SEC);
	ti= ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("%s %.24s. Last calculated prime was %lld\n",datestr,ctime(&sec),s.p);
	printf("%s %.24s. %llu primes calculated in %f seconds (%f seconds per prime from %llu for %llu)\n"
		,datestr,ctime(&sec),i,ti,ti/i,sp,i);
	if(fclose(f)!=0)
		printf("Error closing file\n");

	return(0);
}

/*****************************************************************************/
/* Position record pointer to eof.                                           */
/*****************************************************************************/
void findlastprimeinfile(FILE *f){

	unsigned long long b, tell;
	time_t sec;
        struct tm *localnow;
        char   datestr[30];

	time(&sec);
	localnow=localtime(&sec);
        strftime(datestr,78,"%Y-%m-%d",localnow);
        printf("%s %.24s. Find last primenumber in file\n",datestr,ctime(&sec));

	if(fseek(f,-sizeof(a),SEEK_END)==0)
		;
	else
		fprintf(stderr,"Error file seek_end: %s(%d)\n",strerror(errno),errno);
	
	tell=ftello(f);	 			// Determine absolute position.
	if(tell==-1)
                printf("%.24s. Error in ftello",ctime(&sec));

	b=tell/sizeof(a);			// Device by size to get number of primes.
        a=0;
        while(fread(&a,sizeof(a),1,f)==1)	// Read the last number in the file.
                ;

	time(&sec);
	localnow=localtime(&sec);
        strftime(datestr,78,"%Y-%m-%d",localnow);
        if (a==0){
                printf("%s %.24s. No primenumbers found\n",datestr,ctime(&sec));
                a=1;
        }
        else{
                time(&sec);
		localnow=localtime(&sec);
	        strftime(datestr,78,"%Y-%m-%d",localnow);
                printf("%s %.24s. Primenumber read is %llu\n",datestr,ctime(&sec),a);
                printf("%s %.24s. Number of primes read is %llu\n",datestr,ctime(&sec),b);
        }
}

/*****************************************************************************/
/* Fille prime table with primes. These primes are used to speedup prime     */
/* calculation.                                                              */
/*****************************************************************************/
unsigned long long fillprimetable(unsigned long long pt[])
{
	clock_t starttime, endtime;

	unsigned long long a,b,c,i;
	double ti;
	time_t sec;
        struct tm *localnow;
        char   datestr[30];

	time(&sec);
	localnow=localtime(&sec);
	strftime(datestr,78,"%Y-%m-%d",localnow);

	printf("%s %.24s. Fill prime table start.\n",datestr,ctime(&sec));

	starttime=clock();

	for(i=0,a=3;i<PTLIM;a=a+2){
                for(b=3;b<a;b=b+2){
                        c=a%b;
                        if(c==0||b*b>a)
                                break;
                }
                if(b*b>a){
                        i++;
                        pt[i]=a;
                }
	}

	endtime=clock();
	ti= ((double) (endtime - starttime)) / CLOCKS_PER_SEC;

	time(&sec);
	localnow=localtime(&sec);
	strftime(datestr,78,"%Y-%m-%d",localnow);

	printf("%s %.24s. Fill prime table end. %llu primes added in %f cpu seconds. Prime is %llu, Max possible prime is %llu\n"\
               ,datestr,ctime(&sec),PTLIM,ti,a,a*a);

	return(a*a);	// Biggest possible prime you can investigate with this table.
}

/*****************************************************************************/
/* Handle SIGINT sginal                                                      */
/* Stop. Set global variable to stop calculating primes.                     */
/*****************************************************************************/
void handlesignalint(int s)
{
	time_t sec;
	time(&sec);
        struct tm *localnow;
        char   datestr[30];

	localnow=localtime(&sec);
	strftime(datestr,78,"%Y-%m-%d",localnow);

	l=FALSE;
	printf("%s %24.s Signal %d (SIGINT) received. Current prime is %lld\n",datestr,ctime(&sec),s,a);
}

/*****************************************************************************/
/* Handle SIGUSR1 signal.                                                    */
/* Do not stop. Just show the number of primes calculated and the last prime */
/*****************************************************************************/
void handlesignalusr1(int s)
{
	time_t sec;
	time(&sec);
	printf("%24.s Signal %d (SIGUSR1) received. Current prime is %lld\n",ctime(&sec),s,a);
}
/* eof ***********************************************************************/
