# Client-Server App
Client must ask the user what command they want to execute through a console, and send this information through a TCP socket.
The server receives the command and reconstruct it so that it can be executed using execvp.
Here i

## Here is an example

Assuming that the server is running on a machone with IP 192.168.100 on the port 8080 :
```
$ ./client 192.168.100 8080
Conectado exitosamente a 192.168.100 en el puerto 8080.
> ls -lh
OK
> sadfsad
ERROR
```
The output of the command is shown on the server console

## Compiling
To compile client and server:
```
$ make
```
