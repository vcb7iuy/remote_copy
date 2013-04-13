#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "network.h"

#define ON 1
#define OFF 0
#define OPTION 3

enum {
  GET,
  PUT,
  COM
};

void usage();

int main( int argc, char** argv ) {
  int flag[OPTION];

  /* オプション値のフラグの初期化 */
  flag[GET] = OFF;
  flag[PUT] = OFF;
  flag[COM] = OFF;

  /* 引数の確認 */
  if ( argc != 3 ) {
    usage();
  }

  /* オプション値のチェック */
  if ( strcmp( argv[1], "-g") == 0 ) {
    flag[GET] = ON;
  }
  else if ( strcmp( argv[1], "-p" ) == 0 ) {
    flag[PUT] = ON;
  }
  else if ( strcmp( argv[1], "-i" ) == 0 ) {
    flag[COM] = ON;
  }
  else {
    usage();
  }

  if ( strlen(argv[2]) + 1 >= BUFFERSIZE ) {
    fprintf( stderr, "error: filename size\n");
  }
  
  /* サーバに接続, コマンドを実行 */
  COMMAND cmd;
  if ( flag[GET] ) {
    strcpy( cmd.option, "GET" );
    strcpy( cmd.filename, argv[2] );

    if ( connect_to_server( cmd ) == SUCCESS ) {
      printf("SUCCESS: %s %s\n", cmd.option, cmd.filename );
    }
    else {
      printf("ERROR: %s %s\n", cmd.option, cmd.filename );
    }
  }
  else if ( flag[PUT] ) {
    strcpy( cmd.option, "PUT" );
    strcpy( cmd.filename, argv[2] );

    if ( connect_to_server( cmd ) == SUCCESS ) {
      printf("SUCCESS: %s %s\n", cmd.option, cmd.filename );
    }
    else {
      printf("ERROR: %s %s\n", cmd.option, cmd.filename );
    }
  }
  else if ( flag[COM] ) {
    FILE *fp;

    fp = fopen( argv[2], "r" );    // ファイルを読み込みで開く    
    if ( !fp ) {    // エラー処理
      perror("error: fp");
      exit(1);
    }

    const char *tp;
    char buf[BUFFERSIZE];
    while ( fgets( buf, BUFFERSIZE, fp ) ) {      // ファイル末尾まで読み込み
      /* オプション値を取得 */
      tp = strtok( buf, " " );
      if( strcmp( tp, "GET") == 0 ) {
        strcpy( cmd.option, "GET" );
      }
      else if ( strcmp( tp, "PUT" ) == 0 ) {
        strcpy( cmd.option, "PUT" );
      }
      else {
        fprintf( stderr, "filename: %s, error: %s", argv[2], tp );
        continue;
      }

      /* ファイル名を取得 */
      tp = strtok( NULL, "\n" );
      strcpy( cmd.filename, tp );
      
      if ( connect_to_server( cmd ) == SUCCESS ) {
        printf("SUCCESS: %s %s\n", cmd.option, cmd.filename );
      }
      else {
        printf("ERROR: %s %s\n", cmd.option, cmd.filename );
      }
    }
  }

  return 0;
}

void usage() {
  fprintf( stderr, "Usege: <プログラム名> <オプション値> <ファイル名>\n");
  fprintf( stderr, "オプション値:\n");
  fprintf( stderr, "-g: file gets from remote\n");
  fprintf( stderr, "-p: file puts to remote\n");
  fprintf( stderr, "-i: The file to which the command list was written\n");
  exit(1);
}


