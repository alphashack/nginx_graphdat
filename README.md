nginx_graphdat
--------------

graphdat nginx module

this project uses a submodule for common module functionality
after cloning this repo you will need to:

- git submodule init
- git submodule update

configure nginx adding the module to your standard configure command
--add-module=../src/nginx/nginx_graphdat/
(nginx -V will show you the configure command options that were used to compile the executable if you are unsure)

compile and install nginx

- make
-sudo make install

