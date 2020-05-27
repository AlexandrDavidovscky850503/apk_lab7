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
            printf("Message from client 1 ::: ");
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

int Client()
{
    HANDLE hFile = CreateFile(L"COM2", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == NULL)
    {
        printf("Error %d\n", GetLastError());
        exit(GetLastError());
    }
    SetCommMask(hFile, EV_RXCHAR); // Устанавливаем маску на события порта.
    SetupComm(hFile, 1500, 1500);  // Инициализирует коммуникационные параметры для заданного устройства (Дескриптор, буфер ввода-вывода)

    COMMTIMEOUTS CommTimeOuts;                     // Структура, характеризующая временные параметры последовательного порта.
    CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF; // Mаксимальное время для интервала между поступлением двух символов по линии связи.
    CommTimeOuts.ReadTotalTimeoutMultiplier = 0;   // Множитель, используемый, чтобы вычислить полный период времени простоя для операций чтения.
    CommTimeOuts.ReadTotalTimeoutConstant = 1200;  // Константа, используемая, чтобы вычислить полный (максимальный) период времени простоя для операций чтения.
    CommTimeOuts.WriteTotalTimeoutMultiplier = 0;  // Множитель, используемый, чтобы вычислить полный период времени простоя для операций записи.
    CommTimeOuts.WriteTotalTimeoutConstant = 1200; // Константа, используемая, чтобы вычислить полный период времени простоя для операций записи.

    if (!SetCommTimeouts(hFile, &CommTimeOuts))    // Устанавливает параметры времени ожидания для всех операций чтения и записи на указанном устройстве связи
    {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        return 1;
    }

    DCB ComDCM;                               // Структура, характеризующая основные параметры последовательного порта.
    memset(&ComDCM, 0, sizeof(ComDCM));       
    ComDCM.DCBlength = sizeof(DCB);           // Задает длину, в байтах, структуры.
    GetCommState(hFile, &ComDCM);             // Извлекает данные о текущих настройках управляющих сигналов для указанного устройства.
    ComDCM.BaudRate = DWORD(9600);            // Скорость передачи данных.
    ComDCM.ByteSize = 8;                      // Определяет число информационных бит в передаваемых и принимаемых байтах.
    ComDCM.Parity = NOPARITY;                 // Определяет выбор схемы контроля четности (Бит честности отсутствует).
    ComDCM.StopBits = ONESTOPBIT;             // Задает количество стоповых бит (Один бит).
    ComDCM.fAbortOnError = TRUE;              // Задает игнорирование всех операций чтения/записи при возникновении ошибки.
    ComDCM.fDtrControl = DTR_CONTROL_DISABLE; // Задает режим управления обменом для сигнала DTR.DTR_CONTROL_DISABLE - Отключает линию DTR, когда устройство открыто, и оставляет его отключенным.
    ComDCM.fRtsControl = RTS_CONTROL_DISABLE; // Задает режим управления потоком для сигнала RTS.RTS_CONTROL_DISABLE - Отключает линию RTS, когда устройство открыто, и оставляет его отключенным.
    ComDCM.fBinary = TRUE;                    // Включает двоичный режим обмена.
    ComDCM.fParity = FALSE;                   // Включает режим контроля четности.
    ComDCM.fInX = FALSE;                      // Задает использование XON/XOFF управления потоком при приеме.
    ComDCM.fOutX = FALSE;                     // Задает использование XON/XOFF управления потоком при передаче.
    ComDCM.XonChar = 0;                       // Задает символ XON используемый как для приема, так и для передачи.
    ComDCM.XoffChar = (unsigned char)0xFF;    // Задает символ XOFF используемый как для приема, так и для передачи.
    ComDCM.fErrorChar = FALSE;                // Задает символ, использующийся для замены символов с ошибочной четностью.
    ComDCM.fNull = FALSE;                     // Указывает на необходимость замены символов с ошибкой четности на символ задаваемый полем ErrorChar.
    ComDCM.fOutxCtsFlow = FALSE;              // Включает режим слежения за сигналом CTS.
    ComDCM.fOutxDsrFlow = FALSE;              // Включает режим слежения за сигналом DSR.
    ComDCM.XonLim = 128;                      // Задает минимальное число символов в приемном буфере перед посылкой символа XON.
    ComDCM.XoffLim = 128;                     // Определяет максимальное количество байт в приемном буфере перед посылкой символа XOFF.

    if (!SetCommState(hFile, &ComDCM))        // Настраивает устройство связи в соответствии со спецификациями в блоке управления устройством
    {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        return 2;
    }

    HANDLE readEnd = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"readEnd");    // Семафор чтения
    HANDLE writeEnd = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"writeEnd");  // Семафор записи
    HANDLE hExit = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"exit");         // Семафор выхода
    HANDLE hWork = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"work");         // Семафор выполнения

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
    puts("Client 2 ::: Print 'exit' to quit");
    int response = Client();
    if (!response)
        puts("bay bay");
    else
        puts("COMn can not be opened");
    return 0;
}