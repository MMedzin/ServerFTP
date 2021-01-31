
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "command_parser.h"


//Funkcja odpowiedzialna za zwrócenie kodu komendy
int command_code(char *cmd) {
    str_upper(cmd);
    printf("CommandCode generation...\n");
    if (strcmp(cmd, "USER") == 0) {
        printf("USER cmd recognized\n");
        return USER_CMD;
    } else if (strcmp(cmd, "PASS") == 0) {
        printf("PASS cmd recognized\n");
        return PASS_CMD;
    } else if (strcmp(cmd, "SYST") == 0) {
        printf("SYST cmd recognized\n");
        return SYST_CMD;
    } else if (strcmp(cmd, "PWD") == 0) {
        printf("PWD cmd recognized\n");
        return PWD_CMD;
    } else if (strcmp(cmd, "TYPE") == 0) {
        printf("TYPE cmd recognized\n");
        return TYPE_CMD;
    } else if (strcmp(cmd, "FEAT") == 0) {
        printf("FEAT cmd recognized\n");
        return FEAT_CMD;
    } else if (strcmp(cmd, "PORT") == 0) {
        printf("PORT cmd recognized\n");
        return PORT_CMD;
    } else if (strcmp(cmd, "LIST") == 0) {
        printf("LIST cmd recognized\n");
        return LIST_CMD;
    } else if (strcmp(cmd, "QUIT") == 0) {
        printf("QUIT cmd recognized\n");
        return QUIT_CMD;
    } else if (strcmp(cmd, "RMD") == 0) {
        printf("RMD cmd recognized\n");
        return RMD_CMD;
    } else if (strcmp(cmd, "CWD") == 0) {
        printf("CWD cmd recognized\n");
        return CWD_CMD;
    } else if (strcmp(cmd, "CDUP") == 0) {
        printf("CDUP cmd recognized\n");
        return CDUP_CMD;
    } else if (strcmp(cmd, "MKD") == 0) {
        printf("MKD cmd recognized\n");
        return MKD_CMD;
    } else if (strcmp(cmd, "STOR") == 0) {
        printf("STOR cmd recognized\n");
        return STOR_CMD;
    } else if (strcmp(cmd, "DELE") == 0) {
        printf("DELE cmd recognized\n");
        return DELE_CMD;
    } else if (strcmp(cmd, "RETR") == 0) {
        printf("RETR cmd recognized\n");
        return RETR_CMD;
    } else {
        printf("Unknown cmd\n");
        return UNKNOWN_CMD;
    }
}

