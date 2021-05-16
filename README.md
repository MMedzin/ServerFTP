# ServerFTP

Project developed by:
- Maria Mikołajczak (https://github.com/marmiko)
- Michał Mędzin

### Description
Simple ftp server based on BSD sockets. Server is compliant with ```RFC 959``` standard.

### Project compilation
```gcc -pthread -Wall commands.c command_parser.c hashmap_threads.c utils.c main.c -o server.out```

### How to run
```./server.out <IPv4 ADDRESS> <PORT NUMBER>```

#### Supported ftp commands
```USER```, ```PASS```, ```SYST```, ```PWD```, ```TYPE``` (ASCII and IMAGE), ```PORT```, ```LIST```, ```QUIT```, ```RMD```, ```CWD```, ```CDUP```, ```MKD```, ```STOR```, ```DELE```, ```RETR```
