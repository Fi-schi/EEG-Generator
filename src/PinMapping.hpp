#ifndef PINMAPPING_HPP
#define PINMAPPING_HPP

// Steuerleitungen
#define RST         32
#define Load_Data   41

// Datenleitungen
#define DB0         33
#define DB1         34
#define DB2         35
#define DB3         36
#define DB4         29
#define DB5         27
#define DB6         26
#define DB7         25

// Adressleitungen (Beispiel: falls im Schaltplan anders benannt bitte anpassen)
#define ADD0        16
#define ADD1        17
#define ADD2        18
#define ADD3        19

// Steuerleitungen (weitere)
#define P_AV        20
#define P_RD        21

void initPinModes();
#endif // PINMAPPING_HPP

