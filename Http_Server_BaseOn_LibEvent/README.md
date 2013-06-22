The http server base on libevent.

git clone the code, then goto source code dir to compile

$ make

in the src/ dir, you can see the exec bin "http_server"

run the server like this
$./http_server --port 8080 --dir /home --host 127.0.0.1

you can put "127.0.0.1:8080" in the web browser to test the server