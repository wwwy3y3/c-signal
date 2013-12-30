#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FILEPATH "/tmp/mmapped.bin"
#define NUMINTS  (1000)
#define FILESIZE (NUMINTS * sizeof(int))

using namespace std;
using namespace boost::numeric::ublas;

void initMatrix(matrix<int> &m){
	for (unsigned i = 0; i < m.size1 (); ++ i)
        for (unsigned j = 0; j < m.size2 (); ++ j)
        	cin>> m(i,j);
}

void writeMatrix(matrix<int> &m, int* &map, int idx){
	for (unsigned i = 0; i < m.size1 (); ++ i)
        for (unsigned j = 0; j < m.size2 (); ++ j){
        	map[idx]= m(i,j);
        	idx++;
        }
}

void handler1(int signum){
	int idx=2;
    int fd;
    int *map;  /* mmapped array of int's */

    fd = open(FILEPATH, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
		perror("Error opening file for reading");
		exit(EXIT_FAILURE);
    }

    map = (int*)mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
		close(fd);
		perror("Error mmapping the file");
		exit(EXIT_FAILURE);
    }

    //read A1,B1
    int m= map[0];
    int n= map[1];
    matrix<int> A(m,n);
	matrix<int> B(n,m);
 
	for (unsigned i = 0; i < A.size1 (); ++ i)
        for (unsigned j = 0; j < A.size2 (); ++ j)
        	A(i,j)= map[idx++];

    for (unsigned i = 0; i < B.size1 (); ++ i)
        for (unsigned j = 0; j < B.size2 (); ++ j)
        	B(i,j)= map[idx++];

    //write result back
    matrix<int> M(m,m);
    M= prod(A,B);
    cout<<M;
    idx= 26;
    for (unsigned i = 0; i < M.size1 (); ++ i)
        for (unsigned j = 0; j < M.size2 (); ++ j)
        	map[idx++]= M(i,j);

    if (munmap(map, FILESIZE) == -1) {
	perror("Error un-mmapping the file");
	/* Decide here whether to close(fd) and exit() or not. Depends... */
    }
	exit(1);
}

void handler2(int signum){
	int idx=14;
    int fd;
    int *map;  /* mmapped array of int's */

    fd = open(FILEPATH, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
		perror("Error opening file for reading");
		exit(EXIT_FAILURE);
    }

    map = (int*)mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
		close(fd);
		perror("Error mmapping the file");
		exit(EXIT_FAILURE);
    }
    
    //read A1,B1
    int m= map[0];
    int n= map[1];

    matrix<int> A(m,n);
	matrix<int> B(n,m);

	for (unsigned i = 0; i < A.size1 (); ++ i)
        for (unsigned j = 0; j < A.size2 (); ++ j)
        	A(i,j)= map[idx++];

    for (unsigned i = 0; i < B.size1 (); ++ i)
        for (unsigned j = 0; j < B.size2 (); ++ j)
        	B(i,j)= map[idx++];

    //write result back
    matrix<int> M(m,m);
    M= prod(A,B);
    cout<<M;
    
    idx= 35;
    for (unsigned i = 0; i < M.size1 (); ++ i)
        for (unsigned j = 0; j < M.size2 (); ++ j)
        	map[idx++]= M(i,j);

    if (munmap(map, FILESIZE) == -1) {
	perror("Error un-mmapping the file");
	/* Decide here whether to close(fd) and exit() or not. Depends... */
    }
	exit(1);
}

int main(int argc, char const *argv[])
{
	int m,n;
	int status = 0;
	pid_t fp1,fp2, wpid;
	cin>>m>>n;
	matrix<int> A1(m,n);
	matrix<int> B1(n,m);
	matrix<int> A2(m,n);
	matrix<int> B2(n,m);
	matrix<int> M(m,m);
	matrix<int> N(m,m);

	initMatrix(A1);
	initMatrix(B1);
	initMatrix(A2);
	initMatrix(B2);

	//mmap
	int i;
    int fd;
    int result;
    int *map;  /* mmapped array of int's */

	/*
	 * Create file we are using for mmap. The file must be
	 * size of memory we wish to map.
	 */
	fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (fd == -1) {
		perror("Error opening file for writing");
		exit(EXIT_FAILURE);
    }

    /* Stretch the file size to the size of the (mmapped) array of ints
     */
    result = lseek(fd, FILESIZE-1, SEEK_SET);
    if (result == -1) {
		close(fd);
		perror("Error calling lseek() to 'stretch' the file");
		exit(EXIT_FAILURE);
    }

    result = write(fd, "", 1);
	if (result != 1) {
		close(fd);
		perror("Error writing last byte of the file");
		exit(EXIT_FAILURE);
    }

    /* Now the file is ready to be mmapped.
     */
    map = (int*)mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
		close(fd);
		perror("Error mmapping the file");
		exit(EXIT_FAILURE);
    }

	 /* Now write int's to the file as if it were memory (an array of ints).
	 */
    map[0]= m;
    map[1]= n;
    writeMatrix(A1,map,2);
    writeMatrix(B1,map,8);
    writeMatrix(A2,map,14);
    writeMatrix(B2,map,20);

	//signal
	struct sigaction sa,sa2;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler= &handler1;
	sigaction(SIGUSR1, &sa, NULL);

	memset(&sa2, 0, sizeof(sa2));
	sa2.sa_handler= &handler2;
	sigaction(SIGUSR2, &sa2, NULL);

	//fork two process
	fp1= fork();
	if(fp1<0)
		printf("error in fork!");
	else if(fp1 == 0){ //child
		//set signal handler
		
	}else{
		//dad
		//fork another process
		fp2= fork();
		if(fp2<0)
			printf("error in fork!");
		else if(fp2 == 0){ //child
			//set signal handler
			
		}
	}

	//kill
	if (fp1>0 && fp2>0) //father
	{
		cout<<"father"<<endl;
		kill(fp1, SIGUSR1);
		kill(fp2, SIGUSR2);
	
	//wait child
	while ((wpid = wait(&status)) > 0)
	    {
	        printf("Exit status of %d was %d (%s)\n", (int)wpid, status,
	               (status > 0) ? "accept" : "reject");
	    }

	//get the eventual result
	int idx= 26;
	for (unsigned i = 0; i < M.size1 (); ++ i)
        for (unsigned j = 0; j < M.size2 (); ++ j)
        	M(i,j)= map[idx++];

    for (unsigned i = 0; i < N.size1 (); ++ i)
        for (unsigned j = 0; j < N.size2 (); ++ j)
        	N(i,j)= map[idx++];

    matrix<int> sum(m,m);
    sum= M+N;
    cout<<sum;
	if (munmap(map, FILESIZE) == -1) {
		perror("Error un-mmapping the file");
	/* Decide here whether to close(fd) and exit() or not. Depends... */
    }

	}
	return 0;
}