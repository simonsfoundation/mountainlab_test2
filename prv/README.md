# prv

## Downloading

> git clone https://github.com/magland/prv.git

> cd prv

## Building and running web server using docker

Install docker. On ubuntu this would be

> apt-get install docker.io

Then build the image from the Dockerfile

> ./build_prvfileserver.sh

(The first time you run this, it will take some time to build because it downloads ubuntu and qt5)

Create a directory where the data will be stored. Or use an existing directory.

Then you can run the server in the container using:

> ./start_prvfileserver.sh /data/directory (use the absolute path)

## Testing the installation

First create a file inside your data directory called, say, hello_world.txt

> echo "Hello, world! - from prv." > /data/directory/hello_world.txt

Open a web browser and point it to

> http://localhost:8080/prv/hello_world.txt

> http://localhost:8080/prv/hello_world.txt?a=stat

> http://localhost:8080/prv/?a=locate&checksum=[fill in]&size=[fill in]

Replace localhost/8080 by the ip/port of your server if you are on a different machine.

## Configuring the listen port, data directory, etc

> cp prv.json.default prv.json

Then edit this file as needed. You should delete any fields for which you want to use the default value.

Now start the container.

Note that prv.json is copied at run time (start_prvfileserver.sh), whereas prv.json.default is copied during build (build_prvfileserver.sh). So you should re-build if that file gets modified.

To stop the web server just use

> ./stop_prvfileserver.sh

## Compiling the prv command-line tool

First install Qt5. If you are on Ubuntu, you can look at the Dockerfile to see how this is done. Just run all the apt-get commands in the section labeled "Install qt5". Then compilation is straightforward.

> cd src

> qmake

> make

This will create the executable file "bin/prv". Add the bin directory to your PATH environment variable. For example at the end of ~/.bashrc.

> export PATH=~/prv/bin:$PATH

Replacing ~/prv by wherever you cloned the repository. Restart the terminal.

Now the command-line prv should be installed

> prv

## Using the prv command-line tool

Try this:

> prv-upload [file_name] localhost

> prv-locate [file_name]

> prv-create [file_name] test.prv

> prv-recover test.prv test2.file

Now, test2.file should have the same content as [file_name]

> diff test2.file [file_name]

The significance is that if you send somebody the .prv file, then they can restore it on their end. Of course they don't have access to your local machine. So you would need to replace localhost by, say, datalaboratory, which is the name of a server that is publicly accessible.

In general, prv may be configured (via prv.json) to point to other servers (for example your own that you have set up with the docker procedure above).





