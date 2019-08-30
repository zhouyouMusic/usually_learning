#include "../third/mongoose/mongoose.h"

static int s_done = 0;
static int s_is_connected = 0;
sock_t sock[2];

static void ws_ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  struct websocket_message *wm = (struct websocket_message *) ev_data;
  (void) nc;
  
  switch (ev) {
    case MG_EV_CONNECT: {
      printf("111111ev_handler--MG_EV_CONNECT-\n");
      int status = *((int *) ev_data);
      if (status != 0) {
        printf("-- Connection error: %d\n", status);
      }
      break;
    }
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
      printf("111111ev_handler--MG_EV_WEBSOCKET_HANDSHAKE_DONE-\n");
      printf("-- Connected\n");
      s_is_connected = 1;
      char connect[] = "{\"cmd\":\"connect\",\"param\":{\"sdk\":{\"version\":16781568,\"source\":7,\"arch\":\"8664\",\"protocol\":1,\"os\":\"6.2(9200)3\",\"os_version\":\"6.2(9200)\",\"product\":\"\"},\"app\":{\"applicationId\":\"17KouyuTestAppKey\",\"timestamp\":\"1534832410\",\"sig\":\"93bab2045d372f3012f9a8b0f91fe8afb04c41bd\"}}}";
      int n = strlen(connect);
      mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, connect, n);
      char start[] = "{\"cmd\":\"start\",\"param\":{\"app\":{\"applicationId\":\"17KouyuTestAppKey\",\"timestamp\":\"1534832410\",\"sig\":\"93bab2045d372f3012f9a8b0f91fe8afb04c41bd\",\"userId\":\"w@test\",\"clientId\":\"4ccc6a6e37ea\"},\"audio\":{\"sampleBytes\":2,\"sampleRate\":16000,\"channel\":1,\"quality1\":8,\"complexity1\":2,\"compress\":\"speex\",\"audioType\":\"ogg\"},\"request\":{\"phoneme_output\":1,\"dict_type\":\"KK\",\"agegroup\":3,\"paragraph_need_word_score1\":1,\"attachAudioUrl\":1,\"slack\":0,\"coreType\":\"word.eval\",\"refText\":\"exporter\",\"tokenId\":\"5b7baf1a7e60492aac000001\"}}}";
      n = strlen(start);
      mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, start, n);
      mg_send_websocket_frame(nc, WEBSOCKET_OP_BINARY, "", 0);
      break;
    }
    case MG_EV_POLL: {
      printf("111111ev_handler--MG_EV_POLL-\n");

      break;
    }
    case MG_EV_WEBSOCKET_FRAME: {
      printf("111111ev_handler--MG_EV_WEBSOCKET_FRAME-\n");
      printf("%.*s\n", (int) wm->size, wm->data);
      // mg_close_conn(nc);
      mg_send_websocket_frame(nc,WEBSOCKET_OP_CLOSE, "", 0);
      break;
    }
    case MG_EV_CLOSE: {
      printf("111111ev_handler--MG_EV_CLOSE-\n");
      // nc->sock = INVALID_SOCKET;
      // nc->ev_timer_time = 0;
      // nc->flags = MG_F_CLOSE_IMMEDIATELY;
      if (s_is_connected) printf("-- Disconnected\n");
      // mg_close_conn(nc);
      break;
    }
  }
  printf("111111ev_handler--------------\n");
  fflush(stdout);
}

static void ws_ev_handler2(struct mg_connection *nc, int ev, void *ev_data) {
  struct websocket_message *wm = (struct websocket_message *) ev_data;
  (void) nc;
  
  switch (ev) {
    case MG_EV_CONNECT: {
      printf("2222222ev_handler--MG_EV_CONNECT-\n");
      int status = *((int *) ev_data);
      if (status != 0) {
        printf("-- Connection error: %d\n", status);
      }
      break;
    }
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
      printf("2222222ev_handler--MG_EV_WEBSOCKET_HANDSHAKE_DONE-\n");
      printf("-- Connected\n");
      s_is_connected = 1;
      char connect[] = "{\"cmd\":\"connect\",\"param\":{\"sdk\":{\"version\":16781568,\"source\":7,\"arch\":\"8664\",\"protocol\":1,\"os\":\"6.2(9200)3\",\"os_version\":\"6.2(9200)\",\"product\":\"\"},\"app\":{\"applicationId\":\"17KouyuTestAppKey\",\"timestamp\":\"1534832410\",\"sig\":\"93bab2045d372f3012f9a8b0f91fe8afb04c41bd\"}}}";
      int n = strlen(connect);
      mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, connect, n);
      char start[] = "{\"cmd\":\"start\",\"param\":{\"app\":{\"applicationId\":\"17KouyuTestAppKey\",\"timestamp\":\"1534832410\",\"sig\":\"93bab2045d372f3012f9a8b0f91fe8afb04c41bd\",\"userId\":\"w@test\",\"clientId\":\"4ccc6a6e37ea\"},\"audio\":{\"sampleBytes\":2,\"sampleRate\":16000,\"channel\":1,\"quality1\":8,\"complexity1\":2,\"compress\":\"speex\",\"audioType\":\"ogg\"},\"request\":{\"phoneme_output\":1,\"dict_type\":\"KK\",\"agegroup\":3,\"paragraph_need_word_score1\":1,\"attachAudioUrl\":1,\"slack\":0,\"coreType\":\"word.eval\",\"refText\":\"exporter\",\"tokenId\":\"5b7baf1a7e60492aac000001\"}}}";
      n = strlen(start);
      mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, start, n);
      mg_send_websocket_frame(nc, WEBSOCKET_OP_BINARY, "", 0);
      break;
    }
    case MG_EV_POLL: {
      printf("2222222ev_handler--MG_EV_POLL-\n");

      break;
    }
    case MG_EV_WEBSOCKET_FRAME: {
      printf("2222222ev_handler--MG_EV_WEBSOCKET_FRAME-\n");
      printf("%.*s\n", (int) wm->size, wm->data);
      // mg_close_conn(nc);
      // mg_send_websocket_frame(nc,WEBSOCKET_OP_CLOSE, "", 0);
      break;
    }
    case MG_EV_CLOSE: {
      printf("2222222ev_handler--MG_EV_CLOSE-\n");
      // nc->sock = INVALID_SOCKET;
      // nc->ev_timer_time = 0;
      // nc->flags = MG_F_CLOSE_IMMEDIATELY;
      printf("--------------------------------------------------:%d,bbbb:%d\n", nc->sock, INVALID_SOCKET);
      if (s_is_connected) printf("-- Disconnected\n");
      // mg_close_conn(nc);
      break;
    }
  }
  printf("2222222ev_handler--------------\n");
  fflush(stdout);
}


static void http_ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  struct http_message *hm = (struct http_message *) ev_data;
   printf("http ev_handler--------------\n");
  switch (ev) {
    case MG_EV_CONNECT:
    printf("http_ev_handler--MG_EV_CONNECT-\n");
      if (*(int *) ev_data != 0) {
        fprintf(stderr, "connect() failed: %s\n", strerror(*(int *) ev_data));
      }
      break;
    case MG_EV_HTTP_REPLY:
      nc->flags |= MG_F_CLOSE_IMMEDIATELY;
      putchar('\n');
      break;
    case MG_EV_CLOSE:
      break;
    default:
      break;
  }
  fflush(stdout);
}

static void socket_ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  struct mbuf *io = &nc->recv_mbuf;
  printf("socket ev_handler--------------\n");
  switch (ev) {
    case MG_EV_CONNECT:
    printf("http_ev_handler--MG_EV_CONNECT-\n");
      if (*(int *) ev_data != 0) {
        fprintf(stderr, "connect() failed: %s\n", strerror(*(int *) ev_data));
      }
      break;
    case MG_EV_RECV:
      printf("recv data is:%.*s", io->len, io->buf);
      break;
    case MG_EV_CLOSE:
      break;
    default:
      break;
  }
  fflush(stdout);
}

static void *stdin_thread(void *param) {
  int ch, sock = *(int *) param;
  // Forward all types characters to the socketpair
  while ((ch = getchar()) != EOF) {
    unsigned char c = (unsigned char) ch;
    if (send(sock, (const char *) &c, 1, 0) < 0) {
      fprintf(stderr, "Failed to send byte to the socket");
    }
    // closesocket(sock);
  }
  return NULL;
}

int main() {
  struct mg_mgr mgr;
  struct mg_connection *nc;

  const char *ws_url = "ws://192.168.0.12:8090/sent.eval?e=0&t=0";
  const char *ws_gray_url = "ws://gray.17kouyu.com:8090/sent.eval?e=0&t=0";
  const char *http_url = "http://192.168.0.12:8090/sent.eval?e=0&t=0";
  mg_mgr_init(&mgr, NULL);

  if (mg_socketpair(sock, SOCK_STREAM) == 0){
    perror("Opening socket pair");
    exit(1);
  }
  mg_start_thread(stdin_thread, &sock[1]);
  nc = mg_add_sock(&mgr, sock[0], socket_ev_handler);

      
  nc = mg_connect_ws(&mgr, ws_ev_handler, ws_url, "ws_chat", NULL);
  nc = mg_connect_ws(&mgr, ws_ev_handler2, ws_gray_url, "ws_chat", NULL);
  if (nc == NULL) {
    fprintf(stderr, "Invalid address\n");
    return 1;
  }
  nc = mg_connect_http(&mgr, http_ev_handler, http_url, NULL, NULL);
  
  while (1) {
    mg_mgr_poll(&mgr, 100);
  }
  mg_mgr_free(&mgr);

  return 0;
}

//todo: 以mg_mgr_poll作为事件处理驱动核心，同时处理线程间通信数据、websockt数据、http数据
