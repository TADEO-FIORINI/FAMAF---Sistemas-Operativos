#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


void intToString(int num, char str[], int len) {
    int i = 0;
    int isNegative = 0;

    // Manejar el caso del número negativo
    if (num < 0) {
        isNegative = 1;
        num = -num;
    }

    // Convertir dígitos en cadena de caracteres de derecha a izquierda
    do {
        str[i++] = num % 10 + '0';
        num /= 10;
    } while (num > 0);

    // Agregar el signo negativo si es necesario
    if (isNegative) {
        str[i++] = '-';
    }

    // Agregar el carácter nulo para finalizar la cadena
    str[i] = '\0';

    // Invertir la cadena para que esté en el orden correcto
    int j = 0;
    int temp;
    for (j = 0; j < i / 2; j++) {
        temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

int intLength(int num) {
    int length = 0;

    // Manejar el caso de número negativo
    if (num < 0) {
        length++; // Suma un dígito para el signo negativo
        num = -num; // Convierte el número a positivo para contar los dígitos
    }

    // Contar los dígitos del número
    do {
        length++;
        num /= 10;
    } while (num > 0);

    return length;
}

//int
//main(int argc, char *argv[])
//{
//  char num1[10], num2[10];
//  intToString(7,num1,intLength(7));
//  printf("%d: %s",7,num1);
//  intToString(10,num2,intLength(10));
//  printf("\n%d: %s",10,num2);
//  exit(0);
//}
