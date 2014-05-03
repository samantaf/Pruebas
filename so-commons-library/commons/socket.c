#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

#include "socket.h"
//#include "list.h"

#include "errno.h"

/*
 *						Socket Buffer Functions
 */

void sockets_bufferDestroy(t_socket_buffer *tbuffer){
	free(tbuffer);
}

void sockets_sbufferDestroy(t_socket_sbuffer *tbuffer){
	free(tbuffer->serializated_data);
	free(tbuffer);
}


/*
 *						Private Low Level Socket Functions
 */

void 		*sockets_makeaddr (char* ip, int port);
void *sockets_makeaddrUnix(char *path);
t_socket 	*sockets_create(char* ip, int port);
int 		 sockets_bind(t_socket* sckt, char* ip, int port);
void 		 sockets_destroy(t_socket* sckt);

void *sockets_makeaddr (char* ip, int port){
	struct sockaddr_in *addr = malloc(sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	if(ip == NULL){
		addr->sin_addr.s_addr=INADDR_ANY;
		//addr->sin_addr.s_addr=htonl(INADDR_LOOPBACK);
	}else{
		addr->sin_addr.s_addr=inet_addr(ip);
	}
	addr->sin_port = htons(port);
	memset(&addr->sin_zero,'\0',8);
    return addr;
}

void *sockets_makeaddrUnix(char *path){
	struct sockaddr_un *addr = malloc(sizeof(struct sockaddr_un));

	addr->sun_family = AF_UNIX;
	strcpy(addr->sun_path, path);

	return addr;
}

t_socket *sockets_create(char* ip, int port){
	t_socket* sckt = malloc( sizeof(t_socket) );
	sckt->desc = socket(AF_INET, SOCK_STREAM, 0);
	if( !sockets_bind(sckt, ip, port) ){
		free(sckt);
		return NULL;
	}
	sockets_setMode(sckt, SOCKETMODE_BLOCK);
	return sckt;
}

t_socket *sockets_createUnix(char *path){
	t_socket *sckt = malloc( sizeof(t_socket) );
	sckt->desc = socket(PF_UNIX, SOCK_STREAM, 0);
	sckt->my_addr = sockets_makeaddrUnix(path);

	sockets_setMode(sckt, SOCKETMODE_BLOCK);
	return sckt;
}

int sockets_bind(t_socket* sckt, char* ip, int port){
	int yes=1;
	sckt->my_addr = sockets_makeaddr(ip,port);

	if (setsockopt(sckt->desc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		sckt->my_addr = NULL;
		free(sckt->my_addr);
		return 0;
	}

	if (bind(sckt->desc, (struct sockaddr *)sckt->my_addr, sizeof(struct sockaddr_in)) == -1) {
		sckt->my_addr = NULL;
		free(sckt->my_addr);
		return 0;
	}
	return 1;
}

int sockets_bindUnix(t_socket *sckt, char *path){
	struct sockaddr_un *addr = sockets_makeaddrUnix(path);
	sckt->my_addr = (struct sockaddr_in *)addr;
	unlink(path);
	if (bind(sckt->desc, (struct sockaddr *)sckt->my_addr, SUN_LEN(addr)) == -1){
		printf("%s\n",strerror(errno));
		printf("%i\n",errno);
		sckt->my_addr = NULL;
		free(addr);
		return 0;
	}
	return 1;
}

void sockets_destroy(t_socket* sckt){
	if( sckt->desc > 0 ){
		close(sckt->desc);
	}
	free(sckt->my_addr);
	free(sckt);
}


/*
 *						Public Low Level Socket Functions
 */

char* sockets_getIp(t_socket* sckt){
	return inet_ntoa(((struct sockaddr_in *)sckt->my_addr)->sin_addr);
}

void sockets_getIpAsBytes(t_socket* sckt, unsigned char ip[]){
	sockets_getStringIpAsBytes(inet_ntoa(((struct sockaddr_in *)sckt->my_addr)->sin_addr),ip);
}

void sockets_getStringIpAsBytes(char* str_ip, unsigned char ip[]){
	int ip_lenght = strlen(str_ip);
	int cont,index_ip=0;

	ip[index_ip] = 0;
	for(cont=0; cont<ip_lenght; cont++){
		if(str_ip[cont]=='.'){
			index_ip++;
			ip[index_ip] = 0;
		}else{
			ip[index_ip] = (ip[index_ip] * 10) + (str_ip[cont]-'0');
		}
	}
}

int sockets_getPort(t_socket* sckt){
	return  htons( ((struct sockaddr_in *)sckt->my_addr)->sin_port );
}

int sockets_getPort1(t_socket* sckt){
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	getsockname(sckt->desc, (struct sockaddr *)&sin, &len);

	return ( ntohs(sin.sin_port) ) ;

}

void sockets_setMode(t_socket *sckt, e_socket_mode mode){
	sckt->mode = mode;
}

e_socket_mode sockets_getMode(t_socket *sckt){
	return sckt->mode;
}

int sockets_isBlocked(t_socket *sckt){
	return sckt->mode == SOCKETMODE_BLOCK;
}


/*
 *						Client Socket Functions
 */

t_socket_client *sockets_createClient(char *ip, int port){
	t_socket_client* client = malloc( sizeof(t_socket_client) );
	if( (client->socket = sockets_create(ip, port)) == NULL ){
		free(client);
		return NULL;
	}
	if (client->socket == NULL){
		return NULL;
	}
	sockets_setState(client, SOCKETSTATE_DISCONNECTED);
	return client;
}

t_socket_client *sockets_createClientUnix(char *path){
	t_socket_client* client = malloc( sizeof(t_socket_client) );
	if( (client->socket = sockets_createUnix(path)) == NULL ){
		free(client);
		return NULL;
	}
	sockets_setState(client, SOCKETSTATE_DISCONNECTED);
	return client;
}

t_socket *sockets_getClientSocket(t_socket_client *client){
	return client->socket;
}

int sockets_equalsClients(t_socket_client *client1, t_socket_client *client2){
	return client1->socket->desc ==  client2->socket->desc;
}

int	sockets_isConnected(t_socket_client *client){
	return client->state == SOCKETSTATE_CONNECTED;
}

e_socket_state sockets_getState(t_socket_client *client){
	return client->state;
}

void sockets_setState(t_socket_client *client, e_socket_state state){
	client->state = state;
}

int sockets_connect(t_socket_client *client, char *server_ip, int server_port){
	t_socket *serv_socket = malloc( sizeof(t_socket) );
	serv_socket->my_addr = sockets_makeaddr(server_ip, server_port);

	if ( connect(client->socket->desc, (struct sockaddr *)serv_socket->my_addr , sizeof(struct sockaddr_in) ) == -1) {
		client->serv_socket = NULL;
		free(serv_socket->my_addr);
		free(serv_socket);
		return 0;
	}
	client->serv_socket = serv_socket;
	sockets_setState(client, SOCKETSTATE_CONNECTED);
	return 1;
}

int sockets_connectUnix(t_socket_client *client, char *path){
	t_socket *serv_socket = malloc( sizeof(t_socket) );
	serv_socket->my_addr = sockets_makeaddrUnix(path);
	if ( connect(client->socket->desc, (struct sockaddr *)serv_socket->my_addr , SUN_LEN((struct sockaddr_un *)serv_socket->my_addr)) == -1) {
		printf("%s\n", strerror(errno));
		client->serv_socket = NULL;
		free(serv_socket->my_addr);
		free(serv_socket);
		return 0;
	}
	client->serv_socket = serv_socket;
	sockets_setState(client, SOCKETSTATE_CONNECTED);
	return 1;
}

int sockets_send(t_socket_client *client, void *data, int datalen){
	int aux = send(client->socket->desc, data, datalen, 0);
	if( aux == -1 )
		sockets_setState(client, SOCKETSTATE_DISCONNECTED);

	return aux;
}

int sockets_write(t_socket_client *client, void *data, int datalen){
	return write(client->socket->desc, data, datalen);
}

int sockets_sendBuffer(t_socket_client *client, t_socket_buffer *buffer){
	return sockets_send(client, buffer->data, buffer->size);
}

int sockets_sendSBuffer(t_socket_client *client, t_socket_sbuffer *buffer){
	return sockets_send(client, buffer->serializated_data, buffer->size);
}

int sockets_sendString(t_socket_client *client, char *str){
	return sockets_send(client, (void*)str, strlen(str)+1);
}

int sockets_sendSerialized(t_socket_client *client, void *data, t_socket_sbuffer *(*serializer)(void*)){
	t_socket_sbuffer *sbuffer = serializer(data);
	if( sbuffer != NULL){
		int ret = sockets_send(client, (void*)sbuffer->serializated_data, sbuffer->size);
		free( sbuffer->serializated_data );
		free( sbuffer );
		return ret;
	}
	return -1;
}


void *sockets_recvSerialized(t_socket_client *client, void *(*deserializer)(t_socket_sbuffer*)){
	t_socket_buffer *buffer = sockets_recv(client);
	void *data = NULL;

	if( buffer != NULL ){
		t_socket_sbuffer sbuffer;
		sbuffer.size = buffer->size;
		sbuffer.serializated_data = malloc(buffer->size);
		memcpy(sbuffer.serializated_data, buffer->data, buffer->size);
		data = deserializer(&sbuffer);
		free(sbuffer.serializated_data);
		free(buffer);
	}
	return data;
}

t_socket_buffer *sockets_recv(t_socket_client *client){
	t_socket_buffer *tbuffer = malloc( sizeof(t_socket_buffer) );
	int datasize = sockets_recvInBuffer(client, tbuffer);

	if( datasize <= 0 ){
		free(tbuffer);
		return NULL;
	}

	return tbuffer;
}


int sockets_recvInBuffer(t_socket_client *client, t_socket_buffer *buffer){
	memset(buffer->data, 0, DEFAULT_BUFFER_SIZE);
	if( !sockets_isBlocked(client->socket) ){
		fcntl(client->socket->desc, F_SETFL, O_NONBLOCK);
	}
	buffer->size = recv(client->socket->desc, buffer->data, DEFAULT_BUFFER_SIZE, 0);
	if( !sockets_isBlocked(client->socket) ){
		fcntl(client->socket->desc, F_SETFL, O_NONBLOCK);
	}

	if( buffer->size == -1 )
		sockets_setState(client, SOCKETSTATE_DISCONNECTED);

	return buffer->size;
}

void sockets_destroyClient(t_socket_client *client){
	if( !sockets_isConnected(client) ) client->socket->desc = -1;
	sockets_destroy(client->socket);
	free(client);
}

/*
 *						Server Socket Functions
 */

t_socket_server *sockets_createServer(char *ip, int port){
	t_socket_server *sckt = malloc( sizeof(t_socket_server) );
	if( (sckt->socket = sockets_create(ip, port)) == NULL ){
		free(sckt);
		return NULL;
	}
	sckt->maxconexions = DEFAULT_MAX_CONEXIONS;
	return sckt;
}

t_socket_server *sockets_createServerUnix(char *path){
	t_socket_server *sckt = malloc( sizeof(t_socket_server) );
	if( (sckt->socket = sockets_createUnix(path)) == NULL ){
		free(sckt);
		return NULL;
	}

	sockets_bindUnix(sckt->socket, path);

	sckt->maxconexions = DEFAULT_MAX_CONEXIONS;
	return sckt;
}

t_socket *sockets_getServerSocket(t_socket_server* server){
	return server->socket;
}

void sockets_setMaxConexions(t_socket_server* server, int conexions){
	server->maxconexions = conexions;
}

int sockets_getMaxConexions(t_socket_server* server){
	return server->maxconexions;
}

int sockets_listen(t_socket_server* server){
	if (listen(server->socket->desc, server->maxconexions) == -1) {
		return 0;
	}
	return 1;
}

t_socket_client *sockets_accept(t_socket_server* server){
	t_socket_client* client = malloc( sizeof(t_socket_client) );
	int addrlen = sizeof(struct sockaddr_in);
	client->socket = malloc( sizeof(t_socket) );
	client->socket->my_addr = malloc( sizeof(struct sockaddr) );

	if( !sockets_isBlocked(server->socket) ){
		fcntl(server->socket->desc, F_SETFL, O_NONBLOCK);
	}

	if ((client->socket->desc = accept(server->socket->desc, (struct sockaddr *)client->socket->my_addr, (void *)&addrlen)) == -1) {
		free(client->socket->my_addr);
		free(client->socket);
		free(client);
		return NULL;
	}

	if( !sockets_isBlocked(server->socket) ){
		fcntl(server->socket->desc, F_SETFL, O_NONBLOCK);
	}

	sockets_setState(client, SOCKETSTATE_CONNECTED);
	sockets_setMode(client->socket, SOCKETMODE_BLOCK);
	return client;
}

t_socket_client *sockets_acceptUnix(t_socket_server* server){
	t_socket_client* client = malloc( sizeof(t_socket_client) );
	socklen_t len;
	client->socket = malloc( sizeof(t_socket) );
	client->socket->my_addr = malloc(sizeof(struct sockaddr_un));

	if( !sockets_isBlocked(server->socket) ){
		fcntl(server->socket->desc, F_SETFL, O_NONBLOCK);
	}

	if ((client->socket->desc = accept(server->socket->desc, (struct sockaddr *) client->socket->my_addr, &len)) == -1) {
		printf("%s", strerror(errno));
		free(client->socket->my_addr);
		free(client->socket);
		free(client);
		return NULL;
	}

	if( !sockets_isBlocked(server->socket) ){
		fcntl(server->socket->desc, F_SETFL, O_NONBLOCK);
	}

	sockets_setState(client, SOCKETSTATE_CONNECTED);
	sockets_setMode(client->socket, SOCKETMODE_BLOCK);
	return client;
}

/*
 * TODO En closure_clientsBuildSet hay que eliminar los sockets sockets_isConnected == 0
 */
//void sockets_select(t_list* servers, t_list* clients, int usec_timeout,  t_socket_client *(*onAcceptClosure)(t_socket_server*),  int (*onRecvClosure)(t_socket_client*) ){
//	fd_set conexions_set;
//	int max_desc = 3;
//	int select_return;
//    struct timeval tv;
//    t_list *close_clients = NULL;

//    tv.tv_sec = 0;
//    tv.tv_usec = usec_timeout;

//    void closure_clientsBuildSet(t_socket_client *client){
//    	int curr_desc = client->socket->desc;
//    	FD_SET(curr_desc, &conexions_set);
//    	if( max_desc < curr_desc ) max_desc = curr_desc;
//    }
//
//    /* if( clients != NULL && collection_list_size(clients) > 0 ){
//    	collection_list_iterator(clients, (void*)&closure_clientsBuildSet);
//    }*/
//
//    void closure_serversBuildSet(t_socket_server *server){
//    	int curr_desc = server->socket->desc;
//    	FD_SET(curr_desc, &conexions_set);
//    	if( max_desc < curr_desc ) max_desc = curr_desc;
//    }

    /*if( servers != NULL && collection_list_size(servers) > 0 ){
    	collection_list_iterator(servers, (void*)&closure_serversBuildSet);
    }*/

//    if( usec_timeout == 0 ){
//    	select_return = select( max_desc + 1, &conexions_set, NULL, NULL, NULL );
//    }else{
//    	select_return = select( max_desc + 1, &conexions_set, NULL, NULL, &tv );
//    }
//
//    if( select_return == -1 ) return;

   /* void closure_acceptFromSocket(t_socket_server *server){
    	if( FD_ISSET(server->socket->desc, &conexions_set) ){
    	   	t_socket_client *auxClient = NULL;
    	   	if( onAcceptClosure != NULL){
    	   		auxClient = onAcceptClosure(server);
    	   	}else if( clients != NULL ){
    	   		auxClient = sockets_accept(server);
    	   	}

    	   	if( clients != NULL && auxClient != NULL){
    	   		collection_list_add(clients, auxClient);
    	   	}
    	}
    }*/
//    if( servers != NULL && collection_list_size(servers) > 0 ){
//    	collection_list_iterator(servers, (void*)&closure_acceptFromSocket);
//    }

    /*void closure_recvFromSocket(t_socket_client *client){
    	if( FD_ISSET(client->socket->desc, &conexions_set) && (&onRecvClosure != NULL) ){
    		if( onRecvClosure(client) == 0 ){
    			if( close_clients == NULL ) close_clients = collection_list_create();
    			collection_list_add(close_clients, client);
    		}
		}
    }
    if( clients != NULL && collection_list_size(clients) > 0 ){
    	collection_list_iterator(clients, (void*)&closure_recvFromSocket);
    	if( close_clients != NULL ){
    		int index;
    		for(index=0; index<collection_list_size(close_clients); index++ ){
    			t_socket_client *client = collection_list_get(close_clients, index);
    			int closure_removeSocket(t_socket_client *aux){ return client->socket->desc == aux->socket->desc; }
    			collection_list_removeByClosure(clients, (void*)&closure_removeSocket, NULL);
    		}
    		collection_list_destroy(close_clients, NULL);
    	}
    }
}*/

void sockets_destroyServer(t_socket_server* server){
	sockets_destroy(server->socket);
	free(server);
}

/*Agregado 01/05/2013 - Envío y Recepción por Socket*/
int EnviarRecibirSocket(t_socket_client* Sock,t_socket_buffer* Buffer,char* Mensaje) {
	memset(Buffer->data, 0, DEFAULT_BUFFER_SIZE);
	strcpy( Buffer->data, Mensaje );
	Buffer->size = strlen(Buffer->data);
	if (sockets_sendBuffer(Sock, Buffer) <= 0) {
		perror ("No puedo enviar datos");
		return -1;
	}
	if (sockets_recvInBuffer(Sock,Buffer) <= 0) {
		perror ("No puedo recibir datos");
		return -1;
	}
	return Buffer->size;
}

int EnviarSocket(t_socket_client* Sock,t_socket_buffer* Buffer,char* Mensaje) {
	memset(Buffer->data, 0, DEFAULT_BUFFER_SIZE);
	strcpy( Buffer->data, Mensaje );
	Buffer->size = strlen(Buffer->data);
	if (sockets_sendBuffer(Sock, Buffer) <= 0) {
		perror ("No puedo enviar datos");
		return -1;
	}
	return Buffer->size;
}

int RecibirSocket(t_socket_client* Sock,t_socket_buffer* Buffer) {
	if (sockets_recvInBuffer(Sock,Buffer) <= 0) {
		/*lo comento para que no rompa la gui de nivel*/
		//perror ("No puedo recibir datos");
		return -1;
	}
	return Buffer->size;
}

int EnviarDescriptor(int Sock,t_socket_buffer* Buffer,char* Mensaje) {
	memset(Buffer->data, 0, DEFAULT_BUFFER_SIZE);
	strcpy( Buffer->data, Mensaje );
	Buffer->size = strlen(Buffer->data);
	if (send(Sock, Buffer->data, Buffer->size, 0) <= 0 ) {
		perror ("No puedo enviar datos");
		return -1;
	}
	return Buffer->size;
}

int RecibirDescriptor(int Sock,t_socket_buffer* Buffer) {
	memset(Buffer->data, 0, DEFAULT_BUFFER_SIZE);
	if (recv(Sock, Buffer->data, sizeof(Buffer->data)/*DEFAULT_BUFFER_SIZE*/, 0) <= 0 ) {
		perror ("No puedo enviar datos");
		return -1;
	}
	return Buffer->size;
}

/*Agregado 05/05/2013 - Tratamiento de Socket en Select en Listas*/
int guardarSocketSelectEnLista(t_list *ListaSocketSelect,t_socket_client *ClientSock) {
	dato_socket_select *socket_select = malloc(sizeof(dato_socket_select));
	/*meto el socket select en la lista*/
	socket_select->socket = *ClientSock;
	socket_select->Descriptor = ClientSock->socket->desc;
	list_add(ListaSocketSelect,(void*)socket_select);
	return 1;
}

int guardarSocketSelectEnListaConHilos(t_list *ListaSocketSelect,t_socket_client *ClientSock,pthread_t *Hilo) {
	dato_socket_select_hilos *socket_select = malloc(sizeof(dato_socket_select_hilos));
	/*meto el socket select en la lista*/
	socket_select->socket = *ClientSock;
	socket_select->Hilo = *Hilo;
	socket_select->Descriptor = ClientSock->socket->desc;
	list_add(ListaSocketSelect,(void*)socket_select);
	return 1;
}

t_socket_client* buscarSocketEnListaSocketSelect(t_list* ListaSocketSelect, int Descriptor){

	int _is_Socket(t_socket_client *ClientSock) {
		return ClientSock->socket->desc == Descriptor;
	}
	t_socket_client *aux = list_find(ListaSocketSelect, (void*) _is_Socket);
	return aux;
}

int eliminarSocketEnListaSocketSelect(t_list* ListaSocketSelect, int Descriptor){

	int _is_Socket(t_socket_client *ClientSock) {
		return ClientSock->socket->desc == Descriptor;
	}
	void _destroy_Socket (dato_socket_select *socket_select) {
		sockets_destroyClient(&socket_select->socket);
	}
	//list_remove_and_destroy_by_condition(ListaSocketSelect,(void*)_is_Socket,_destroy_Socket);
	return 1;
}

int eliminarSocketEnListaSocketSelectConHilos(t_list* ListaSocketSelect, int Descriptor){

	int _is_Socket(t_socket_client *ClientSock) {
		return ClientSock->socket->desc == Descriptor;
	}
	void _destroy_Socket (dato_socket_select_hilos *socket_select_hilos) {
		int s;
		//printf("hilo planificador del nivel: %lu\n", socket_select_hilos->Hilo);
        s = pthread_cancel(socket_select_hilos->Hilo);
        if (s != 0)
    		perror("error en pthread_cancel");
        /*if (s == 0)
        	printf("cancele bien el hilo \n");*/
		sockets_destroyClient(&socket_select_hilos->socket);
	}
	list_remove_and_destroy_by_condition(ListaSocketSelect,(void*)_is_Socket,_destroy_Socket);
	return 1;
}
