#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>

int read(HANDLE hFile, char* buffer)
{
    int size;
    DWORD numberOfBytesRead;
    ReadFile(hFile, &size, 1 * sizeof(int), &numberOfBytesRead, NULL);      //read size
    ReadFile(hFile, buffer, size * sizeof(char), &numberOfBytesRead, NULL); //read info
    return size;
}

bool write(HANDLE hFile, char* buffer)
{
    char symb;
    int i = 0;
    DWORD numberOfBytesWrite;
    while (true)
    {
        scanf_s("%c", &symb);
        if (symb == '\n')
        {
            buffer[i] = '\0';
            WriteFile(hFile, &i, 1 * sizeof(int), &numberOfBytesWrite, NULL);      //write i(size)
            WriteFile(hFile, buffer, i * sizeof(char), &numberOfBytesWrite, NULL); //write info

            if (!strcmp(buffer, "exit\0"))
                return false;
            else
                return true;
        }
        else
        {
            buffer[i] = symb;
        }
        i++;
    }
}


void PORTWORK(HANDLE readEnd, HANDLE writeEnd, HANDLE hFile, HANDLE hExit, HANDLE hWork)
{
    char buffer[100];
    int size;
    while (WaitForSingleObject(hExit, 1) == WAIT_TIMEOUT)
    {
        if (WaitForSingleObject(hWork, 1) != WAIT_TIMEOUT)
        {
            puts("waiting for a message from a friend");
            WaitForSingleObject(writeEnd, INFINITE);
            if (WaitForSingleObject(hExit, 1) != WAIT_TIMEOUT)
                return;
            size = read(hFile, buffer);
            printf("Message from client 2 ::: ");
            for (int i = 0; i < size; i++)
                printf("%c", buffer[i]);
            printf("\n");
            SetEvent(readEnd);
        }
        SetEvent(hWork);
        printf("write message -> ");
        if (!write(hFile, buffer))
        {
            SetEvent(hExit);
            SetEvent(writeEnd);
            return;
        }
        else
        {
            SetEvent(writeEnd);
            WaitForSingleObject(readEnd, INFINITE);
            puts("message sent");
        }
    }
}

int Server()
{
    HANDLE readEnd = CreateEvent(NULL, FALSE, FALSE, L"readEnd");   // ������� ������
    HANDLE writeEnd = CreateEvent(NULL, FALSE, FALSE, L"writeEnd"); // ������� ������
    HANDLE hExit = CreateEvent(NULL, FALSE, FALSE, L"exit");        // ������� ������
    HANDLE hWork = CreateEvent(NULL, FALSE, FALSE, L"work");        // ������� ����������

    HANDLE hFile = CreateFile(L"COM1", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == NULL)
    {
        printf("Error\n");
        exit(GetLastError());
    }
    SetCommMask(hFile, EV_RXCHAR);                      // ������������� ����� �� ������� �����.
    SetupComm(hFile, 1500, 1500);                       // �������������� ���������������� ��������� ��� ��������� ���������� (����������, ����� �����-������)

    COMMTIMEOUTS CommTimeOuts;                          // ���������, ��������������� ��������� ��������� ����������������� �����.
    CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;      // M����������� ����� ��� ��������� ����� ������������ ���� �������� �� ����� �����.
    CommTimeOuts.ReadTotalTimeoutMultiplier = 0;        // ���������, ������������, ����� ��������� ������ ������ ������� ������� ��� �������� ������.
    CommTimeOuts.ReadTotalTimeoutConstant = 1200;       // ���������, ������������, ����� ��������� ������ (������������) ������ ������� ������� ��� �������� ������.
    CommTimeOuts.WriteTotalTimeoutMultiplier = 0;       // ���������, ������������, ����� ��������� ������ ������ ������� ������� ��� �������� ������.
    CommTimeOuts.WriteTotalTimeoutConstant = 1200;      // ���������, ������������, ����� ��������� ������ ������ ������� ������� ��� �������� ������.

    if (!SetCommTimeouts(hFile, &CommTimeOuts))         // ������������� ��������� ������� �������� ��� ���� �������� ������ � ������ �� ��������� ���������� �����
    {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        return 1;
    }

    DCB ComDCM;                               // ���������, ��������������� �������� ��������� ����������������� �����.
    memset(&ComDCM, 0, sizeof(ComDCM));      
    ComDCM.DCBlength = sizeof(DCB);           // ������ �����, � ������, ���������.
    GetCommState(hFile, &ComDCM);             // ��������� ������ � ������� ���������� ����������� �������� ��� ���������� ����������.
    ComDCM.BaudRate = DWORD(9600);            // �������� �������� ������.
    ComDCM.ByteSize = 8;                      // ���������� ����� �������������� ��� � ������������ � ����������� ������.
    ComDCM.Parity = NOPARITY;                 // ���������� ����� ����� �������� �������� (��� ��������� �����������).
    ComDCM.StopBits = ONESTOPBIT;             // ������ ���������� �������� ��� (���� ���).
    ComDCM.fAbortOnError = TRUE;              // ������ ������������� ���� �������� ������/������ ��� ������������� ������.
    ComDCM.fDtrControl = DTR_CONTROL_DISABLE; // ������ ����� ���������� ������� ��� ������� DTR.DTR_CONTROL_DISABLE - ��������� ����� DTR, ����� ���������� �������, � ��������� ��� �����������.
    ComDCM.fRtsControl = RTS_CONTROL_DISABLE; // ������ ����� ���������� ������� ��� ������� RTS.RTS_CONTROL_DISABLE - ��������� ����� RTS, ����� ���������� �������, � ��������� ��� �����������.
    ComDCM.fBinary = TRUE;                    // �������� �������� ����� ������.
    ComDCM.fParity = FALSE;                   // �������� ����� �������� ��������.
    ComDCM.fInX = FALSE;                      // ������ ������������� XON/XOFF ���������� ������� ��� ������.
    ComDCM.fOutX = FALSE;                     // ������ ������������� XON/XOFF ���������� ������� ��� ��������.
    ComDCM.XonChar = 0;                       // ������ ������ XON ������������ ��� ��� ������, ��� � ��� ��������.
    ComDCM.XoffChar = (unsigned char)0xFF;    // ������ ������ XOFF ������������ ��� ��� ������, ��� � ��� ��������.
    ComDCM.fErrorChar = FALSE;                // ������ ������, �������������� ��� ������ �������� � ��������� ���������.
    ComDCM.fNull = FALSE;                     // ��������� �� ������������� ������ �������� � ������� �������� �� ������ ���������� ����� ErrorChar.
    ComDCM.fOutxCtsFlow = FALSE;              // �������� ����� �������� �� �������� CTS.
    ComDCM.fOutxDsrFlow = FALSE;              // �������� ����� �������� �� �������� DSR.
    ComDCM.XonLim = 128;                      // ������ ����������� ����� �������� � �������� ������ ����� �������� ������� XON.
    ComDCM.XoffLim = 128;                     // ���������� ������������ ���������� ���� � �������� ������ ����� �������� ������� XOFF.

    if (!SetCommState(hFile, &ComDCM))        // ����������� ���������� ����� � ������������ �� �������������� � ����� ���������� �����������
    {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        return 2;
    }

    PORTWORK(readEnd, writeEnd, hFile, hExit, hWork);

    CloseHandle(hFile);
    CloseHandle(readEnd);
    CloseHandle(writeEnd);
    CloseHandle(hExit);
    CloseHandle(hWork);
    return 0;
}

int main()
{
    puts("Client 1 ::: Print 'exit' to quit");
    int response = Server();
    if (!response)
        puts("bay bay");
    else
        puts("COMn can not be opened");
    return 0;
}
