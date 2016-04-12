#!/usr/bin/env python
import SimpleHTTPServer
import SocketServer
import urlparse
import subprocess
import os
import sys
import ConfigParser
import shutil
from json import JSONDecoder, JSONEncoder
from exceptions import ValueError

class ConfigReader(object):
    def __init__(self, **kwargs):
        self.defaults = kwargs

    def read(self, path):
        reader = ConfigParser.ConfigParser(allow_no_value=True,
                                           defaults=self.defaults)
        reader.read(path)
        return reader

class MyRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def do_GET(self):  #handle a GET request
        request_path = urlparse.urlparse(self.path).path  # sanity check needed
        #Witold, by sanity check do you mean check whether this is a valid path?
        #  if so, could you help with this part?

        methods = {
            'readJson': self.handle_READJSON,
            'getConfig': self.handle_GETCONFIG
        }
        method = self.query('a')

        if method and methods.has_key(method):
            print("method: "+method)
            response = {"request": {"method": method, "path": request_path}}
            try:
                result = methods.get(method)(request_path)
                self.send_plain_text(result)
            except ValueError as e:
                self.send_plain_text("Error: "+str(e))


            #if self.query('output') == "text":
                #self.send_plain_text(response)
                #if response.has_key("result"):
                #    self.send_plain_text(response["result"])
                #elif response.has_key("error"):
                #    self.send_plain_text("ERROR: " + response["error"])
            #the encoded json wasn't working well. somehow there were "\t" and "\n" characters that were not parsed correctly by the javascript receiver
            #else:
            #    encoded = JSONEncoder(indent=4).encode(response)
            #    self.send_plain_text(encoded)
        else:
            SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

    #Witold, please help handle error cases in the following code
    def handle_READJSON(self,path):
        base_path = self.cfg("mountainbrowser_basepath")
        json_fname=base_path+"/"+path;
        with open(json_fname, 'r') as myfile:
            str = myfile.read()
        return str
    def handle_GETCONFIG(self,path):
        obj=dict()
        obj["mdaserver_url"]=self.cfg('mdaserver_url')
        obj["mscmdserver_url"]=self.cfg('mscmdserver_url')
        return JSONEncoder().encode(obj)

    def cfg(self, key, section='General'):
        return self.config.get(section, key)

    def query(self, field, defaultval=""):  #for convenience
        parts = urlparse.urlparse(self.path)
        query = urlparse.parse_qs(parts.query)
        tmp = query.get(field, [defaultval])
        return tmp[0] if tmp else ""

    def send_plain_text(self, txt):  #send a plain text response
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.send_header("Content-length", len(txt))
        self.end_headers()
        self.wfile.write(txt)


class MyTCPServer(SocketServer.TCPServer):
    allow_reuse_address = True

#Witold, curious. In nodejs, I found the server would stop running if there was an error. For python it seems like it recovers gracefully
#and keeps running. Will the server ever crash? If it does, what's the probable cause -- what do we need to watch for?

def main():
    #Witold, since this behavior is common to all three servers, what's the best way to make it non-redundant?
    config_fname = 'mountainbrowserserver.cfg'
    example_config_fname = 'mountainbrowserserver.example.cfg'
    if not os.path.isfile(config_fname):
        shutil.copyfile(example_config_fname, config_fname)
        print("Please edit the configuration file " + config_fname +
              " and then re-run this program")
        return
    Handler = MyRequestHandler

    config = ConfigParser.ConfigParser(allow_no_value=True)
    config.read(config_fname)
    Handler.config = config
    port=int(config.get('General', 'mountainbrowser_port'))
    server = MyTCPServer(('0.0.0.0', port), Handler)
    print('Serving on port: '+str(port));
    server.serve_forever()

if __name__ == "__main__":
    main()
