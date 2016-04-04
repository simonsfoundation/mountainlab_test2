#!/usr/bin/env python
import SimpleHTTPServer
import SocketServer
import urlparse
import subprocess
import os
import sys
import ConfigParser
import shutil

class ConfigReader(object):
    def __init__(self, **kwargs):
        self.defaults = kwargs

    def read(self, path):
        reader = ConfigParser.ConfigParser(allow_no_value=True, defaults=self.defaults)
        reader.read(path)
        return reader

#example call:
#http://localhost:8000/firings.mda?a=readChunk&size=5,100

class MyRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):

    def translate_path(self, path):
        if ".." in path: #to be safe?
            return ""
        path=self.cfg("mdachunk_data_path")+path
	return path

    def do_GET(self): #handle a GET request
        mdachunk_exe        = self.cfg("mdachunk_exe")
        mda_path            = self.cfg("mda_path")
        mdachunk_data_path  = self.cfg("mdachunk_data_path")
        mdachunk_data_url   = self.cfg("mdachunk_data_url")
    	mda_fname=mda_path+"/"+urlparse.urlparse(self.path).path
        if self.query("a")=="size": #need to return the dimensions of the .mda
            (str,exit_code)=self.call_and_read_output(" ".join([mdachunk_exe, "size", mda_fname]))
            self.send_plain_text(str)
            return
        elif self.query("a")=="info": #need to return the dimensions and checksum of the .mda
            (str,exit_code)=self.call_and_read_output(" ".join([mdachunk_exe, "info", mda_fname]))
            self.send_plain_text(str)
            return
        elif self.query("a")=="readChunk": #read a chunk and return url to retrieve the .mda binary data
    		print(mda_fname)
    		datatype=self.query("datatype","float32")
    		index=self.query("index","0")
    		size=self.query("size","")
    		self.mkdir_if_needed(mdachunk_data_path)
    		outpath=self.query("outpath",mdachunk_data_path)
                cmd = " ".join([
                    mdachunk_exe,
                    "readChunk",
                    mda_fname,
                    "--index="+index,
                    "--size="+size,
                    "--datatype="+datatype,
                    "--outpath="+outpath
                ])
    		(str,exit_code)=self.call_and_read_output(cmd)
                if not exit_code:
    			url0=mdachunk_data_url+"/"+str
    			self.send_plain_text(url0)
    		else:
    			self.send_plain_text("ERROR: "+str)
    		return
	else:
            SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

    def cfg(self, key, section = 'General'):
        return self.config.get(section, key)

    def query(self,field,defaultval=""): #for convenience
    	parts=urlparse.urlparse(self.path)
    	query=urlparse.parse_qs(parts.query)
    	tmp=query.get(field,[defaultval])
        return tmp[0] if tmp else ""

    def call_and_read_output(self,cmd): #make a system call and return the output string and exit code
    	print(cmd)
    	process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    	exit_code = process.wait()
    	(out,err)=process.communicate()
    	process.stderr.close()
    	process.stdout.close()
        ret = "ERROR: {}".format(out) if exit_code else out

    	print(out)
    	return (out,exit_code)

    def mkdir_if_needed(self,path): #for convenience
		if not os.path.exists(path):
			os.makedirs(path)

    def send_plain_text(self,txt): #send a plain text response
		self.send_response(200)
		self.send_header("Content-type", "text/html")
		self.send_header("Content-length", len(txt))
		self.end_headers()
		self.wfile.write(txt)
	

class MyTCPServer(SocketServer.TCPServer):
    allow_reuse_address = True

def main():
    config_fname='mdaserver.cfg'
    example_config_fname='mdaserver.example.cfg'
    if not os.path.isfile(config_fname):
        shutil.copyfile(example_config_fname,config_fname)
        print("Please edit the configuration file "+config_fname+" and then re-run this program")
        return
    Handler = MyRequestHandler
    port = sys.argv[1] if sys.argv[1:] else "8000"
    config = ConfigReader(port=port).read(config_fname)
    Handler.config = config
    server = MyTCPServer(('0.0.0.0', int(config.get('General', 'port'))), Handler)
    server.serve_forever()

if __name__ == "__main__":
    main()
