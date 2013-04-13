remote_copy
===========
Install: make
Usage:<remote_copy> <オプション値> <ファイル名>

オプション値: 
-g: file gets from remote
-p: file puts to remote
-i: The file to which the command list was written

-iの場合のファイルの書き方:
PUT aa
GET makefile

Install: make server
Usage:<remote_copy_server>
