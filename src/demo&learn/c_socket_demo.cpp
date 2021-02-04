/*
c++ socket demo
*/
#include <stdio.h>  
#include <Winsock2.h>  
#pragma comment(lib,"ws2_32.lib")
int main()
{
    WORD wVersionRequested;  // WORD的意思为字，是2byte的无符号整数，表示范围0~65535.
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(1, 1);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        return -1;
    }

    if (LOBYTE(wsaData.wVersion) != 1 ||
        HIBYTE(wsaData.wVersion) != 1) {
        WSACleanup();
        return -1;
    }

    /*SOCKET WSAAPI socket(
    * int af, //地址族规范。地址族的可能值在Winsock2.h头文件中定义。
    * int type,
    * int protocol
    * );
    */
    SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);

    /* sockaddr和sockaddr_in 结构体socket地址
    * typedef struct sockaddr_in {
    * short          sin_family;
    * u_short        sin_port;
    * struct in_addr sin_addr;
    * char           sin_zero[8];
    * } SOCKADDR_IN, *PSOCKADDR_IN, *LPSOCKADDR_IN;
    */
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(8888);
    /*
    * int WSAAPI connect(
    * SOCKET         s,  //A descriptor identifying an unconnected socket.
    * const sockaddr *name,  //A pointer to the sockaddr structure to which the connection should be established.
    * int            namelen //The length, in bytes, of the sockaddr structure pointed to by the name parameter.
    * );
    */
    connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    /*
    * int WSAAPI send(
    * __in SOCKET s,
    * __in_bcount(len) const char FAR * buf, 指明存放要发送的数据的缓冲区。
    * __in int len, 指明实际要发送的数据的字节数。
    * __in int flags
    * );
    */
    printf("%d",strlen("hello"));
    send(sockClient, "hello", strlen("hello") , 0);
    char recvBuf[50];
    recv(sockClient, recvBuf, 50, 0);
    printf("%s\n", recvBuf);

    closesocket(sockClient); // 关闭socket
    WSACleanup(); // 关闭Ws2_32.lib库

    getchar();
    return 0;
}
