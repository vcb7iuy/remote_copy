#include <arpa/inet.h>

#include "network_p.h"

/* サーバに接続 */
int connect_to_server( COMMAND cmd ) {
  int sock;                      // ソケットファイルディスクリプタ
  struct sockaddr_in server;     // 接続先ホスト

  /* ソケットの作成 */
  sock = socket( PF_INET, SOCK_STREAM, 0 );
  if ( sock == -1 ) {     // 作成失敗
    perror("error: sock");
    return ERROR;
  }

  /* 接続先ホストの設定 */
  memset( &server, 0, sizeof(server) );              // ゼロクリア
  server.sin_family = PF_INET;                       // プロトコル: IPv4
  server.sin_port = htons(50000);                    // ポート番号: 50000
  server.sin_addr.s_addr = inet_addr("127.0.0.1");   // IPアドレス: 127.0.0.1

  /* サーバに接続 */
  if ( connect( sock, (const struct sockaddr *)&server, sizeof(server) ) != 0 ) {    // 接続失敗
    perror("error:");
    close(sock);
    return ERROR;
  }

  /* オプション値を送信 */
  ssize_t len = write( sock, cmd.option, strlen(cmd.option) );
  if ( len == -1 ) {    // 送信失敗
    perror("error: option");
    close(sock);
    return ERROR;
  }

  /* 受信確認 */
  if ( receipt_confirmation( sock, SEND ) == ERROR ) {
    close(sock);
    return ERROR;
  }

  /* ファイル名を送信 */
  len = write( sock, cmd.filename, strlen(cmd.filename) );
  if ( len == -1 ) {    // 送信失敗
    perror("error: filename");
    close(sock);
    return ERROR;
  }

  /* 受信確認 */
  if ( receipt_confirmation( sock, SEND ) == ERROR ) {
    close(sock);
    return ERROR;
  }

  /* コマンドを実行 */
  if ( strcmp( cmd.option, "PUT") == 0 ) {    // putを送信したとき
    if ( put_to_remote( sock, cmd.filename ) == ERROR ) {
      close(sock);
      return ERROR;
    }
  }
  else {     // getを送信したとき
    if ( get_from_remote( sock, cmd.filename ) == ERROR ) {
      close(sock);
      return ERROR;
    }
  }
  
  close(sock);    // ソケットを閉じる

  return SUCCESS;
}

/* クライアントからの接続を待ち受ける */
void connect_from_client( ) {
  int sock, fd;                 // ソケットファイルディスクリプタ
  struct sockaddr_in local;     // 自ホスト

  /* ソケットの作成 */
  sock = socket( PF_INET, SOCK_STREAM, 0 );
  if ( sock == -1 ) {     // 作成失敗
    perror("error: sock");
    exit(1);
  }

  /* 自ホストの設定 */
  memset( &local, 0, sizeof(local) );           // ゼロクリア
  local.sin_family = PF_INET;                   // プロトコル: IPv4
  local.sin_port = htons(50000);                // ポート番号: 50000
  local.sin_addr.s_addr = htonl(INADDR_ANY);    // IPアドレス: すべてのIPアドレス

  /* バインド処理  */
  if ( bind( sock, (const struct sockaddr *)&local, sizeof(local) ) != 0  ) {     // バインド失敗
    perror("error:");
    close(sock);
    exit(1);
  }

  /* 接続待ち受けの準備 */
  if ( listen( sock, 5 ) != 0 ) {    // 待ち受け準備失敗
    perror("error:");
    close(sock);
    exit(1);
  }

  struct sockaddr_in client;    // 接続元ホスト
  socklen_t len;                // 構造体のサイズ
  while ( 1 ) {
    len = sizeof(client);
    fd = accept( sock, (struct sockaddr*)&client, &len );    // 接続待ち受け
    
    if ( fd < 0 ) {    // 接続失敗
      perror("error: fd");
    }
    else {             // 接続成功
      COMMAND cmd;
      ssize_t len;
      
      /* オプション値を受信 */
      len = read( fd, cmd.option, OPTIONSIZE - 1 );
      if ( len > 0 ) {    // 受信成功
        cmd.option[len] = '\0';
      }
      else {    // 受信失敗
        perror("error: option");
        close(fd);
        continue;
      }

      /* 送信確認 */
      if ( receipt_confirmation( fd, RECV ) == ERROR ) {
        close(fd);
        continue;
      }

      /* ファイル名を受信 */
      len = read( fd, cmd.filename, BUFFERSIZE - 1 );
      if ( len > 0 ) {    // 受信成功
        cmd.filename[len] = '\0';
      }
      else {    // 受信失敗
        perror("error: filename");
        close(fd);
        continue;
      }

      /* 送信確認 */
      if ( receipt_confirmation( fd, RECV ) == ERROR ) {
        close(fd);
        continue;
      }

      /* コマンドを実行 */
      if ( ( strcmp( cmd.option, "GET" ) == 0 ) ) {    // getを受信したとき
        if ( put_to_remote( fd, cmd.filename ) == SUCCESS ) {
          printf("SUCCESS: %s %s\n", cmd.option, cmd.filename );
        }
        else {
          printf("ERROR: %s %s\n", cmd.option, cmd.filename );
        }
      }
      else {     // putを受信したとき
        if ( get_from_remote( fd, cmd.filename ) == SUCCESS ) {
          printf("SUCCESS: %s %s\n", cmd.option, cmd.filename );
        }
        else {
          printf("ERROR: %s %s\n", cmd.option, cmd.filename );
        }
      }
      
      close(fd);    // ソケットを閉じる
    }
  }
  
  close(sock);    // ソケットを閉じる
}

/* ファイルの読み込み, データを送信 */
int put_to_remote( int sock, const char* const filename ) {
  FILE *fp;
  
  /* バイナリ書き込みでファイルをオープン */
  fp = fopen( filename, "rb" );
  if ( !fp ) {
    perror("error: fp");
    return ERROR;
  }

  /* ファイルの読み込み, 送信 */
  char str[BUFFERSIZE];
  ssize_t len;     // 送信データバイト数
  size_t rlen;     // 読み込みデータ数
  while ( feof(fp) == 0 ) {
    /* ファイルの読み込み */
    rlen = fread( str, 1, sizeof(str) - 1, fp );
    if ( rlen > 0 ) {    // 読み込み成功
      str[rlen] = '\0';

      /* 送信処理 */
      len = write( sock, str, strlen(str) );
      if ( len == -1 ) {    // 送信失敗
        perror("error: len");
        fclose(fp);
        return ERROR;
      }

      /* 送信確認 */
      if ( receipt_confirmation( sock, SEND ) == ERROR ) {
        fclose(fp);
        return ERROR;
      }
    }
    else {    // 読み込み失敗
      perror("error: rlen");
      fclose(fp);
      return ERROR;
    }
  }

  fclose(fp);    // ファイルを閉じる
  
  /* 終了文字列を送信 */
  char quit_str[] = "quit: put_to_remote";  // 終了文字
  len = write( sock, quit_str, strlen(quit_str) );
  if ( len == -1 ) {    // 送信失敗
    perror("error: quit len");
    return 1;
  }

  return SUCCESS;
}

/* データを受信, ファイルに書き込む */
int get_from_remote( int sock, const char* const filename ) {
  FILE *fp;

  /* バイナリ書き込みでファイルをオープン */
  fp = fopen( filename, "wb" );
  if ( !fp ) {
    perror("error: fp");
    return ERROR;
  }

  /* データを受信, ファイルに書き込み */
  char str[BUFFERSIZE];
  ssize_t len;     // 受信データバイト数
  size_t wlen;     // 書き込みデータ数
  while ( 1 ) {
    /* 受信処理 */
    len = read( sock, str, sizeof(str) - 1 );
    if ( len > 0 ) {    // 受信成功
      str[len] = '\0';

      if ( strcmp( str, "quit: put_to_remote" ) == 0 ) {
        break;
      }

      /* ファイルに書き込み */
      wlen = fwrite( str, 1, strlen(str), fp );
      if ( wlen <= 0 ) {    // 書き込み失敗
        perror("error: wlen");
          fclose(fp);
          return ERROR;
      }

      /* 受信確認 */
      if ( receipt_confirmation( sock, RECV ) == ERROR ) {
        fclose(fp);
        return ERROR;
      }
    }
    else {    // 受信失敗
      perror("error: len");
      fclose(fp);
      return ERROR;
    }
  }

  fclose(fp);    // ファイルを閉じる
  
  return SUCCESS;
}

/* 確認文字の送受信 */
int receipt_confirmation( int sock, int check ) {
  ssize_t len;
  
  if ( check == SEND ) {
    char str[3];
    len = read( sock, str, sizeof(str) - 1 );
    if ( len > 0 ) {    // 受信成功
      str[len] = '\0';
      if ( strcmp( str, "OK" ) ) {
        return ERROR;
      }
    }
    else {    // 受信失敗
      perror("error: OK");
      return ERROR;
    }
  }
  else {
    char str[] = "OK";
    /* 送信処理 */
    len = write( sock, str, strlen(str) );
    if ( len == -1 ) {    // 送信失敗
      perror("error: OK");
      return ERROR;
    }
  }
    
  return SUCCESS;
}
