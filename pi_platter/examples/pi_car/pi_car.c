// Program to control the pi_car
//
//   pi_car [-p <serial device>]
//
// by default the program attempts to open /dev/ttyACM0
//
//   -p <serial device> : Specify the serial port to use (e.g. /dev/ACM1)
//
// Build: gcc -o pi_car pi_car.c -lwiringPi
//
#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>

#define MAX_SER_NAME_CHARS  64
#define MAX_CMD_CHARS       128
#define MAX_RSP_CHARS       64

// MAX_WAIT_TIME is uSec to wait for a response after sending a command
// (seems to need to be pretty high a pi)
#define MAX_WAIT_TIME       500000

#define TCP_PORT 8082


// Command word masks
// 32-bit command value
//   bit 24: request sensor (A1)
//   bit 23: shutdown
//   bit 22: request light enable (U1)
//   bit 21: request sensor (A2)
//   bit 20: request status (S)
//   bit 19: request battery voltage (B)
//   bit 18: light enable
//   bit 17: dir2
//   bit 16: dir1
//   bit 15:8: pwm2
//   bit 7:0: pwm1
#define CMD_PWM1_MASK 0x000000FF
#define CMD_PWM2_MASK 0x0000FF00
#define CMD_DIR1_MASK 0x00010000
#define CMD_DIR2_MASK 0x00020000
#define CMD_ENL_MASK  0x00040000
#define CMD_REQB_MASK 0x00080000
#define CMD_REQS_MASK 0x00100000
#define CMD_REQA_MASK 0x00200000
#define CMD_REQL_MASK 0x00400000
#define CMD_SHDN_MASK 0x00800000
#define CMD_REQC_MASK 0x01000000
#define CMD_ACCUM_MASK (CMD_REQB_MASK | CMD_REQS_MASK | CMD_REQA_MASK | CMD_REQL_MASK | CMD_SHDN_MASK | CMD_REQC_MASK)
#define CMD_MOTOR_MASK (CMD_PWM1_MASK | CMD_PWM2_MASK | CMD_DIR1_MASK | CMD_DIR2_MASK)


// Globals
char socketRxData[MAX_CMD_CHARS];
int socketRxPushI = 0;
int socketRxPopI = 0;
char commandData[32];
int serialFd;



static int OpenSerialPort(const char *deviceFilePath)
{
    int fd = -1;
    struct termios options;

    // Attempt to open
    fd = open(deviceFilePath, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1)
    {
        perror("Error opening serial port");
        close(fd);
        return(-1);
    }

    // Configure options
    options.c_cflag = CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNCR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcsetattr(fd,TCSANOW,&options);

    // success
    return fd;
}


void CloseSerialPort(int fd)
{
    // Block until all written output has been sent from the device.
    if (tcdrain(fd) == -1)
    {
        perror("Error waiting for drain");
    }   

    close(fd);
}


static int OpenSocket(int port)
{
    int socket_desc;
    struct sockaddr_in server;

    // Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        perror("Could not create socket");
        return -1;
    }
     
    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
     
    // Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("Bind failed");
        return -1;
    }
     
    // Listen (??? used to be 3)
    listen(socket_desc, 1);

    return socket_desc;
}

    
void SendCmd(int fd, char* cmdP) {
    write(fd, cmdP, strlen(cmdP));
    write(fd, "\r", 1);
    if (tcdrain(fd) == -1)
    {
        perror("Error waiting for drain after sending command");
    }   
}


// Process an incoming message dealing with the fact that we may receive multiple
// command words in the same recv or we may only receive a partial word.  
int ProcessClientMsg(char* msg, uint32_t* retVal) {
    char curCmdS[16]; // Large enough for our command word string + null
    int curCmdI;
    int i = 0;
    int gotCommand = 0;
    uint32_t lastVal;
    uint32_t accumVal = 0;

    // Copy the current data into our receive buffer
    while (*(msg + i) != 0) {
//printf("copy %d %c\n", i, *(msg + i));
        socketRxData[socketRxPushI] = *(msg + i);
        if (++socketRxPushI >= MAX_CMD_CHARS) socketRxPushI = 0;
        i++;
    }

    // Scan through our receive buffer looking for complete command words (bracketed
    // by 'S' and 'E'
    i = socketRxPopI;
    while (i != socketRxPushI) {
//printf("Process %d %c\n", i, socketRxData[i]);
        if (socketRxData[i] == 'S') {
            // Reset the curCmdI index 
            curCmdI = 0;
            memset(curCmdS, '\0', sizeof(curCmdS));
        } else if (socketRxData[i] == 'E') {
            // Process a complete command word
            if (curCmdI != 0) {
                lastVal = atoi(curCmdS);
//printf("  0x%8x\n", lastVal);
                // Modify accumVal with latest light enable
                if (lastVal & CMD_ENL_MASK) {
                    accumVal |= CMD_ENL_MASK;
                } else {
                    accumVal &= ~CMD_ENL_MASK;
                }

                // Accumulate requests for info
                accumVal |= (lastVal & CMD_ACCUM_MASK);

                // Note success
                gotCommand = 1;
            }

            // Update pop index to last command processed
            socketRxPopI = i;
        } else {
            // Store command data
            if (curCmdI < sizeof(curCmdS)-1) {
                curCmdS[curCmdI++] = socketRxData[i];
            }
        }
        if (++i >= MAX_CMD_CHARS) i = 0;
    }

    // Update command word with all accumulated info requests and final motor command
    if (gotCommand) {
        *retVal = accumVal | (lastVal & CMD_MOTOR_MASK);
    }

    return gotCommand;
}


void ProcessMotor(uint32_t cmd) {
    static uint8_t pwm1 = 0;
    static uint8_t pwm2 = 0;
    static int dir1 = 0;
    static int dir2 = 0;
    uint8_t newPwm1, newPwm2;
    int newDir1, newDir2;

    newPwm1 = cmd & 0xFF;
    newPwm2 = (cmd >> 8) & 0xFF;
    newDir1 = (cmd & CMD_DIR1_MASK) ? 1 : 0;
    newDir2 = (cmd & CMD_DIR2_MASK) ? 1 : 0;

    if ((newPwm1 != pwm1) || (newDir1 != dir1) || (newPwm2 != pwm2) || (newDir2 != dir2)) {
        pwm1 = newPwm1;
        pwm2 = newPwm2;
        dir1 = newDir1;
        dir2 = newDir2;

        sprintf(commandData, "P1=%0d\r", pwm1);
        SendCmd(serialFd, commandData);
        sprintf(commandData, "P2=%0d\r", pwm2);
        SendCmd(serialFd, commandData);
        if (dir1 == 0) {
            digitalWrite(0, LOW);
        } else {
            digitalWrite(0, HIGH);
        }
        if (dir2 == 0) {
            digitalWrite(1, LOW);
        } else {
            digitalWrite(1, HIGH);
        }
    }
}


void ProcessLight(uint32_t cmd) {
    static int lightEn = 0;
    int newEn = (cmd & CMD_ENL_MASK) ? 1 : 0;

    if (newEn != lightEn) {
        lightEn = newEn;
        sprintf(commandData, "U1=%0d\r", lightEn);
        SendCmd(serialFd, commandData);
    }
}


void ProcessQuery(uint32_t cmd) {
    if (cmd & CMD_REQB_MASK) {
        SendCmd(serialFd, "B");
    }
    if (cmd & CMD_REQS_MASK) {
        SendCmd(serialFd, "S");
    }
    if (cmd & CMD_REQA_MASK) {
        SendCmd(serialFd, "A1");
    }
    if (cmd & CMD_REQC_MASK) {
        SendCmd(serialFd, "A2");
    }
    if (cmd & CMD_REQL_MASK) {
        SendCmd(serialFd, "U1");
    }
}


void ProcessShutdown(uint32_t cmd) {
    if (cmd & CMD_SHDN_MASK) {
        puts("shutdown now");
        SendCmd(serialFd, "O=30");
        system("shutdown now&");
    }
}



int main(int argc, char** argv) {
    char *optValue = NULL;
    char serName[MAX_SER_NAME_CHARS];
    char clientMsg[MAX_CMD_CHARS/4 + 1];
    char deviceMsg[MAX_RSP_CHARS + 1];
    int c;
    int socketFd, clientFd;
    int read_size;
    int clientConnected;
    struct sockaddr_in client;
    uint32_t inVal;

    // setup
    memset(serName, '\0', sizeof(serName));
    strcpy(serName, "/dev/ttyACM0");

    // process command line arguments
    while ((c = getopt(argc, argv, "p:")) != -1) {
        switch(c) {
            case 'p':
                memset(serName, '\0', sizeof(serName));
                strncpy(serName, optarg, MAX_SER_NAME_CHARS-1);
                break;
            default:
              abort();
        }
    }

    // Try to open the serial port
    if ((serialFd = OpenSerialPort(serName)) == -1) {
        return(-1);
    }

    // Try to open the socket
    if ((socketFd = OpenSocket(TCP_PORT)) == -1) {
        return(-1);
    }

    // Setup wiring
    wiringPiSetup () ;
    pinMode (0, OUTPUT) ;
    pinMode (1, OUTPUT) ;

    // Configure Analog input 1 to use a 4.096 volt Vref (15 cm min)
    SendCmd(serialFd, "C4=3");
    // Configure Analog input 1 to use a 2.048 volt Vref (30 cm min)
    //SendCmd(serialFd, "C4=2");
    // Configure Analog input 2 to use 2.048 volt Vref (2 A max)
    SendCmd(serialFd, "C5=2");

    // Configure the PWM to fast mode for best motor control
    SendCmd(serialFd, "C6=0");
     
    // Accept incoming connections - first time through this loop, we block until connected
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    clientConnected = 0;
    while (1) {
        if ((clientFd = accept(socketFd, (struct sockaddr *)&client, (socklen_t*)&c)) < 0) {
            perror("Accept failed");
            return(-1);
        }
        clientConnected = 1;
        puts("Connection accepted");

        // Convert to non-blocking
        fcntl(clientFd, F_SETFL, O_NONBLOCK);

        // Process messages from client until it disconnects
        while (clientConnected) {
            // Look for incoming commands to process
            memset(clientMsg, '\0', sizeof(clientMsg));
            read_size = recv(clientFd, clientMsg, sizeof(clientMsg)-1, 0);
            if (read_size <= 0) {
                if (read_size == 0) {
                    clientConnected = 0;
                    puts("Client disconnect");
                }
            } else {
                if (ProcessClientMsg(clientMsg, &inVal)) {
                    ProcessMotor(inVal);
                    ProcessLight(inVal);
                    ProcessQuery(inVal);
                    ProcessShutdown(inVal);
                }
            }

            // Look for device serial responses to send back
            memset(deviceMsg, '\0', sizeof(deviceMsg));
            read_size = read(serialFd, deviceMsg, MAX_RSP_CHARS);
            if (read_size > 0) {
                (void) send(clientFd, deviceMsg, read_size, 0);
            }

            // Be a little nice...
            (void) usleep((unsigned int) 20000);
        }

        // Be a little nice...
        (void) usleep((unsigned int) 20000);
    } 

    // Successfully done
    CloseSerialPort(serialFd);
    return(0);
}


