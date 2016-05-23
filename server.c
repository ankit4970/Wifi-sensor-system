/* 
*	A simple server using TCP.
*	Ankit Gandhi
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <mysql/mysql.h>
#include <signal.h>

static char *host = "localhost";				/// Mysql server
static char *user = "root";						/// Mysql database user name
static char *pass = "ankit";					/// Mysql database password
static char *dbname = "test";					/// Mysql database name
int sockfd, newsockfd, portno = 0, clilen;		/// Socket Identifiers
unsigned int port = 3306;						/// Mysql database port
static char *unix_socket = NULL;				/// unix socket for mysql
unsigned int flag = 0; //ODBC connnection
int loop_handle = 1;							/// Loop handle
MYSQL *conn;									/// Mysql database pointer

// Sensor Structure to store data
typedef struct sensorData{
	uint16_t s_year;
	uint8_t s_month;
	uint8_t s_day;
	uint8_t s_hour;
	uint8_t s_min;
	uint8_t s_sec;
	float temperature;
	float lightValue;
	float latitude;
	float longitude;
}senseordata; 

senseordata sensor;

/******************************************************************************
* @brief sig_handler
*			Signal handler
******************************************************************************/
void sig_handler(int signo)
{
	printf("Caught signal\n");
	loop_handle = 0;
	mysql_close(conn);
	close( newsockfd );
	close(sockfd);
}

/******************************************************************************
* @brief mysql_insert
*		Inserts formatted string to data base
******************************************************************************/
void mysql_insert()
{
    char hold[512];
    
	sprintf(hold,"insert into sensorData (lat,lon,temperature,datetime,lightPercentValue) values(\"%f\",\"%f\",\"%.2f\",\"%d-%d-%d %d:%d:%d\",\"%.2f\")"   \
	,sensor.latitude,sensor.longitude, sensor.temperature,sensor.s_year,sensor.s_month,sensor.s_day,sensor.s_hour,sensor.s_min,sensor.s_sec,sensor.lightValue); 
	
	if (mysql_query(conn, hold)) 
	{
		fprintf(stderr,"\nError : %s [%d]\n",mysql_error(conn),mysql_errno(conn));
	}
}

/******************************************************************************
* @brief main
*		Main entry
*		It creates a server socket and waits for client.
*		After successful connection it reads data from client and puts it into 
*		the database
******************************************************************************/
int main(int argc, char *argv[]) 
{
	
	char buffer[256]={0};
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	int data;
	float temperature ;
	uint8_t light =0;
	char *pch = NULL;
	int option = 1;
	int i =0;
	struct sigaction sa;
	
	if(argc != 2)
	{
		printf("Usage %s <port>\n",argv[0]);
		return -1;
	}

	do
	{
		//Setup the sighub handler
		sa.sa_handler = &sig_handler;

		// Intercept SIGINT
		if (sigaction(SIGINT, &sa, NULL) == -1) 
		{
			printf("Error: cannot handle SIGHUP");
			break;
		}
		
		conn = mysql_init(NULL);
		if (!(mysql_real_connect(conn,host,user,pass,dbname,port,unix_socket,flag)))
		{
			fprintf(stderr,"\nError : %s [%d]\n",mysql_error(conn),mysql_errno(conn));
			break;
		}
		
		printf("Database connection sucessful\n");
		
		portno = atoi(argv[1]);
		printf( "using port #%d\n", portno );

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		{
			printf( "ERROR opening socket") ;
			break;
		}
		bzero((char *) &serv_addr, sizeof(serv_addr));

		if(setsockopt(sockfd,SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0)
		{
			printf("setsockopt failed\n");
			break;
		}

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons( portno );
		if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		{
			printf("ERROR on binding" ) ;
			break;
		}
		
		listen(sockfd,5);
		clilen = sizeof(cli_addr);
		
		while(1)
		{
			printf( "waiting for new client...\n" );
			if ( ( newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen) ) < 0 )
			{
				printf("ERROR on accept") ;
				break;
			}

			printf( "opened new communication with client\n" );
			while(1)
			{
				n = read(newsockfd,buffer,256);
				if(n < 0)
				{
					printf("Error readnig data\n");
					break;
				}
				
				pch = strtok (buffer,",");
				sensor.s_year = atoi(pch);
				printf("Year is %d\n",sensor.s_year);
				
				pch = strtok (NULL,",");
				sensor.s_month = atoi(pch);
				printf("Month is %d\n",sensor.s_month);
				
				pch = strtok(NULL, ",");
				sensor.s_day = atoi(pch);
				printf("Day is %d\n",sensor.s_day);
				
				pch = strtok(NULL, ",");
				sensor.s_hour = atoi(pch);
				printf("Hour value is %d\n",sensor.s_hour);
				
				pch = strtok(NULL, ",");
				sensor.s_min = atoi(pch);
				printf("Minute value is %d\n",sensor.s_min);
				
				pch = strtok(NULL, ",");
				sensor.s_sec = atoi(pch);
				printf("Second value is %d\n",sensor.s_sec);
				
				pch = strtok(NULL, ",");
				sensor.temperature = atof(pch);
				printf("Temperature value is %f\n",sensor.temperature);
				
				pch = strtok(NULL, ",");
				sensor.lightValue = atof(pch);
				printf("Light PercentValue value is %f\n",sensor.lightValue);
				
				pch = strtok(NULL, ",");
				sensor.latitude = atof(pch);
				printf("Latitude value is %f\n",sensor.latitude);
				
				pch = strtok(NULL, ",");
				sensor.longitude = atof(pch);
				printf("Longitude value is %f\n",sensor.longitude);
				
				mysql_insert();
				
				memset(buffer,0,sizeof(buffer));
			}	
		}
		
	}while(0);
	
	if(NULL != conn )
	{
		mysql_close(conn);
	}	
	
	if(newsockfd > 0)
	{
		close(newsockfd);
	}
	
	if(sockfd > 0)
	{
		close(sockfd);
	}
	
	return 0;
}


