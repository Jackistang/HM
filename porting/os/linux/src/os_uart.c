#include "os_port.h"
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

static int fd;

enum {
    UART_TYPE_BAUDRATE,
    UART_TYPE_DATABIT,
    UART_TYPE_STOPBIT,
    UART_TYPE_PARITY,
    UART_TYPE_FLOWCONTROL,
    UART_TYPE_INIT,
};

static int set_baudrate(struct termios * toptions, void *args)
{
    uint32_t baudrate = *(uint32_t *)args;
    // printf("h4_set_baudrate %u\n", baudrate);

    speed_t brate = baudrate; // let you override switch below if needed
    switch(baudrate) {
        case    9600: brate=B9600;    break;
        case   19200: brate=B19200;   break;
        case   38400: brate=B38400;   break;
        case 57600:  brate=B57600;  break;
        case 115200: brate=B115200; break;
#ifdef B230400
        case 230400: brate=B230400; break;
#endif
#ifdef B460800
        case 460800: brate=B460800; break;
#endif
#ifdef B500000
        case  500000: brate=B500000;  break;
#endif
#ifdef B576000
        case  576000: brate=B576000;  break;
#endif
#ifdef B921600
        case 921600: brate=B921600; break;
#endif
#ifdef B1000000
        case 1000000: brate=B1000000; break;
#endif
#ifdef B1152000
        case 1152000: brate=B1152000; break;
#endif
#ifdef B1500000
        case 1500000: brate=B1500000; break;
#endif
#ifdef B2000000
        case 2000000: brate=B2000000; break;
#endif
#ifdef B2500000
        case 2500000: brate=B2500000; break;
#endif
#ifdef B3000000
        case 3000000: brate=B3000000; break;
#endif
#ifdef B3500000
        case 3500000: brate=B3500000; break;
#endif
#ifdef B400000
        case 4000000: brate=B4000000; break;
#endif
        default:
            printf("can't set baudrate %dn\n", baudrate );
            return -1;
    }
    cfsetospeed(toptions, brate);
    cfsetispeed(toptions, brate);

    return 0;
}

static int set_parity(struct termios * toptions, void *args)
{
    int parity = *(int *)args;

    switch (parity){
        case OS_UART_PARITY_NONE:
            toptions->c_cflag &= ~PARENB;
            toptions->c_cflag &= ~PARODD;
            break;
        case OS_UART_PARITY_EVEN:
            toptions->c_cflag |=  PARENB;
            toptions->c_cflag &= ~PARODD;
            break;
        case OS_UART_PARITY_ODD:
            toptions->c_cflag |= PARENB;
            toptions->c_cflag |= PARODD;
        default:
            break;
    }
    return 0;
}

static int set_stopbit(struct termios * toptions, void *args)
{
    int stopbit = *(int *)args;

    switch (stopbit) {
    case OS_UART_STOPBIT_1_BIT:
        toptions->c_cflag &= ~CSTOPB;
        break;
    case OS_UART_STOPBIT_2_BIT:
        toptions->c_cflag |= CSTOPB;
        break;
    default:
        break;
    }

    return 0;
}

static int set_databit(struct termios * toptions, void *args)
{
    int databit = *(int *)args;

    toptions->c_cflag &= ~CSIZE;
    switch (databit) {
    case OS_UART_DATABIT_5_BIT:
        toptions->c_cflag |= CS5;
        break;
    case OS_UART_DATABIT_6_BIT:
        toptions->c_cflag |= CS6;
        break;
    case OS_UART_DATABIT_7_BIT:
        toptions->c_cflag |= CS7;
        break;
    case OS_UART_DATABIT_8_BIT:
        toptions->c_cflag |= CS8;
        break;
    default:
        toptions->c_cflag |= CS8;
        break;
    }
}

static int set_flowcontrol(struct termios * toptions, void *args)
{
    bool flowcontrol = *(bool *)args;

    if (flowcontrol) {
        // with flow control
        toptions->c_cflag |= CRTSCTS;
    } else {
        // no flow control
        toptions->c_cflag &= ~CRTSCTS;
    }

    return 0;
}

static int hci_transport_uart_set_params(int type, void *params)
{
    int err;
    struct termios toptions;
    if (tcgetattr(fd, &toptions) < 0) {
        printf("Couldn't get term attributes\n");
        return -1;
    }

    switch (type) {
    case UART_TYPE_INIT:
        toptions.c_cflag |= CREAD;
    case UART_TYPE_BAUDRATE:
        set_baudrate(&toptions, params);
        break;
    case UART_TYPE_DATABIT:
        set_databit(&toptions, params);
        break;
    case UART_TYPE_STOPBIT:
        set_stopbit(&toptions, params);
        break;
    case UART_TYPE_PARITY:
        set_parity(&toptions, params);
        break;
    case UART_TYPE_FLOWCONTROL:
        set_flowcontrol(&toptions, params);
        break;
    default:
        break;
    }

    if (type == UART_TYPE_BAUDRATE)
        err = tcsetattr(fd, TCSADRAIN, &toptions);
    else
        err = tcsetattr(fd, TCSANOW, &toptions);

    if (err < 0) {
        printf("Couldn't set term attributes\n");
        return -1;
    }
    return 0;
}

int os_uart_init(struct os_uart_config *config)
{
    fd = open(config->device_name, O_RDWR | O_NOCTTY);
    if (fd == -1) {
        perror("Open hci transport uart error");
        return -1;
    }

    int err;
    if ((err = hci_transport_uart_set_params(UART_TYPE_BAUDRATE, &config->baudrate)))
        return -1;
    if ((err = hci_transport_uart_set_params(UART_TYPE_DATABIT, &config->databit)))
        return -1;
    if ((err = hci_transport_uart_set_params(UART_TYPE_STOPBIT, &config->stopbit)))
        return -1;
    if ((err = hci_transport_uart_set_params(UART_TYPE_PARITY, &config->parity)))
        return -1;
    if ((err = hci_transport_uart_set_params(UART_TYPE_FLOWCONTROL, &config->flowcontrol)))
        return -1;
    
    return 0;
}

int os_uart_deinit(void)
{
    return close(fd);
}

int os_uart_send(uint8_t *buffer, uint16_t length)
{
    int len = length;
    while (len > 0) {
        int size = write(fd, buffer, len);
        if (size <= 0)
            return -1;
        len -= size; 
    }

    return length;
}

int os_uart_recv(uint8_t *buffer, uint16_t length)
{
    return read(fd, buffer, length);
}

int os_uart_set_baudrate(uint32_t baudrate)
{
    return hci_transport_uart_set_params(UART_TYPE_BAUDRATE, &baudrate);
}