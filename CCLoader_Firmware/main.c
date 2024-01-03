#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>


#define SBEGIN  0x01
#define SDATA   0x02
#define SRSP    0x03
#define SEND    0x04
#define ERRO   0x05

FILE *pfile = NULL;
long fsize = 0;
int BlkTot = 0;
int Remain = 0;
int BlkNum = 0;
int DownloadProgress = 0;
int com = -1;
int end = 0;


int sockfd;

void ProcessProgram(void);

int RS232_OpenComport(int comport_number, int baudrate)
{
    struct sockaddr_in servaddr, cli;
 
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed: %s\n", strerror(errno));
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
 
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(3434);
 
    // connect the client socket to server socket
    if (connect(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
       printf("connect failed...\n");
      

    return(0);
}



int RS232_PollComport(int comport_number, unsigned char *buf, int size)
{
  int n;

#ifndef __STRICT_ANSI__                       /* __STRICT_ANSI__ is defined when the -ansi option is used for gcc */
  if(size>SSIZE_MAX)  size = (int)SSIZE_MAX;  /* SSIZE_MAX is defined in limits.h */
#else
  if(size>4096)  size = 4096;
#endif

  n = read(sockfd, buf, size);
  return(n);
}


int RS232_SendByte(int comport_number, unsigned char byte)
{
  int n;

  n = write(sockfd, &byte, 1);
  if(n<0)  return(1);

  return(0);
}


int RS232_SendBuf(int comport_number, unsigned char *buf, int size)
{
  return(write(sockfd, buf, size));
}


void RS232_CloseComport(int comport_number)
{
  close(sockfd);
}

/*
Constant  Description
TIOCM_LE  DSR (data set ready/line enable)
TIOCM_DTR DTR (data terminal ready)
TIOCM_RTS RTS (request to send)
TIOCM_ST  Secondary TXD (transmit)
TIOCM_SR  Secondary RXD (receive)
TIOCM_CTS CTS (clear to send)
TIOCM_CAR DCD (data carrier detect)
TIOCM_CD  Synonym for TIOCM_CAR
TIOCM_RNG RNG (ring)
TIOCM_RI  Synonym for TIOCM_RNG
TIOCM_DSR DSR (data set ready)
*/

int RS232_IsCTSEnabled(int comport_number)
{
  return(0);
}

int RS232_IsDSREnabled(int comport_number)
{
  return(0);
}


/*
* argv[0]----.exe file name
* argv[1]----ComPort number
* argv[2]----file path
*/
int main(int arg, char *argv[])
{	
	int fLen = 0;
	int device = 0;

	if(arg < 2)
	{
		printf("Invalid parameters.\n");
		printf("Usage: %s <bin file>\n", argv[0]);
		printf("Example: %s abc.bin\n", argv[0]);
		return 0;
	}
	
  if(1 == RS232_OpenComport(com, 460800))
	{
		return 0;	// Open comprt error
	}
	printf("Comport open:\n");

	char form[5] = ".bin";
	char format[5] = "    ";
	fLen = strlen(argv[1]);
	if(fLen < 5) 
	{
		printf("File path is invalid!\n");
		return 0;  // file path is not valid
	}
	format[3] = argv[1][fLen-1];
	format[2] = argv[1][fLen-2];
	format[1] = argv[1][fLen-3];
	format[0] = argv[1][fLen-4];	
	if(0 != strcmp(form, format))
	{
		printf("File format must be .bin");
		return 0;
	}
	pfile = fopen(argv[1], "rb");      // read only
	if(NULL == pfile)
    {
		printf("file doesn't exist or is occupied!\n");
        return 0;
    }
	printf("File open success!\n");
	fseek(pfile,0,SEEK_SET);
    fseek(pfile,0,SEEK_END);
    fsize = ftell(pfile);
    fseek(pfile,0,SEEK_SET);
    //BlkTot = fsize / 512;
	Remain = fsize % 512;
	if(Remain != 0)
	{
		BlkTot = fsize / 512 + 1;
		printf("Warning: file size isn't the integer multiples of 512, last bytes will be set to 0xFF\n");
	}
	else
	{
		BlkTot = fsize / 512;
	}
	
	printf("Block total: %d\n", BlkTot);
    BlkNum = 0;

	printf("Enable transmission...\n");
	unsigned char buf[2] = {SBEGIN, 0};      // Enable transmission,  do not verify
	if(RS232_SendBuf(com, buf, 2) != 2)
	{
		printf("Enable failed!\n");
		fclose(pfile);
		printf("File closed!\n");
		RS232_CloseComport(com);
		printf("Comport closed!\n");
		return 0;
	}
	else
	{
		printf("Request sent already! Waiting for respond...\n");
	}
	
	while(!end)
	{
		ProcessProgram();
	}
	printf("Program successfully!\n");
	BlkNum = 0;
	DownloadProgress = 0;
    fclose(pfile);
	printf("File closed!\n");
	RS232_CloseComport(com);
	printf("Comport closed!\n");

	return 0;
}

void ProcessProgram()
{
    int len;
	unsigned char rx;
	len = RS232_PollComport(com, &rx, 1);
    if(len > 0)
    {
        switch(rx)
        {
            case SRSP:
            {
                if(BlkNum == BlkTot)
                {
                    unsigned char temp = SEND;
                   	RS232_SendByte(com, temp);
					end = 1;
                }
                else
                {
					if(BlkNum == 0)
					{	
						printf("Begin programming...\n");
					}
					DownloadProgress = 1;
                    unsigned char buf[515];
                    buf[0] = SDATA;

					if((BlkNum == (BlkTot-1)) && (Remain != 0))
					{
						fread(buf+1, Remain, 1, pfile);
						int filled = 512 - Remain;
						//int i = 0;
						for(int i = 0; i<filled; i++)
						{
							buf[Remain+1+i] = 0xFF;
						}
					}
					else
					{
						fread(buf+1, 512, 1, pfile);
					}
                    

                    unsigned short CheckSum = 0x0000;
					//unsigned int i;
                    for(unsigned int i=0; i<512; i++)
                    {
                        CheckSum += (unsigned char)buf[i+1];
                    }
                    buf[513] = (CheckSum >> 8) & 0x00FF;
                    buf[514] = CheckSum & 0x00FF;
                    
					RS232_SendBuf(com, buf, 515);
                    BlkNum++;
					printf("%d  \r\n", BlkNum);
                }
                break;
            }

            case ERRO:
            {
                if(DownloadProgress == 1)
                {
                    end = 1;
                    printf("Verify failed!\n");
                }
                else
                {
                    end = 1;
                    printf("No chip detected!\n");
                }
                break;
            }

            default:
                break;
        }
		len = 0;
    }
}



#ifdef __cplusplus
} /* extern "C" */
#endif

