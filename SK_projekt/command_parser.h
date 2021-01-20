//
// Created by marysia on 20.01.2021.
//

#ifndef SK_PROJEKT_COMMAND_PARSER_H
#define SK_PROJEKT_COMMAND_PARSER_H

#define USER_CMD 1
#define PASS_CMD 2
#define SYST_CMD 3
#define FEAT_CMD 4
#define PWD_CMD 5
#define TYPE_CMD 6
#define PORT_CMD 7
#define LIST_CMD 8
#define QUIT_CMD 9
#define RMD_CMD (10)
#define CWD_CMD 11
#define CDUP_CMD 12
#define MKD_CMD 13
#define STOR_CMD 14
#define DELE_CMD 15
#define RETR_CMD 16
#define UNKNOWN_CMD (-1)

int commandCode(char* cmd);

#endif //SK_PROJEKT_COMMAND_PARSER_H
