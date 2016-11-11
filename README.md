nginx_graphdat
--------------

graphdat nginx module

## DEPRECATED - This is now longer in use or supported.

This project uses a submodule for common module functionality after cloning this repo you will need to:

```
git submodule init
git submodule update
```

Configure nginx by adding the module to your standard configure command.  If you are unsure, `nginx -V` will show you the configure options that were used to compile your current nginx the executable

```
--add-module=../src/nginx/nginx_graphdat/
```

compile and install nginx

```
make
sudo make install
```
