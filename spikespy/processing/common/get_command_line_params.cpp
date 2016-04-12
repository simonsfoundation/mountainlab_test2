#include "get_command_line_params.h"

CLParams get_command_line_params(int argc,char *argv[],const QStringList &required_params,QStringList &optional_params) {
	CLParams ret;
	ret.success=true; //let's be optimistic!

	//find the named and unnamed parameters checking for errors along the way
	for (int i=1; i<argc; i++) {
		QString str=QString(argv[i]);
		if (str.startsWith("--")) {
			int ind2=str.indexOf("=");
			QString name=str.mid(2,ind2-2);
			QString val="";
			if (ind2>=0) val=str.mid(ind2+1);
			if (name.isEmpty()) {
				ret.success=false;
				ret.error_message="Problem with parameter: "+str;
				return ret;
			}
			if ((required_params.indexOf(name)<0)&&(optional_params.indexOf(name)<0)) {
				ret.success=false;
				ret.error_message="Unexpected parameter: "+name;
				return ret;
			}
			ret.named_parameters[name]=val;
		}
		else {
			ret.unnamed_parameters << str;
		}
	}

	//make sure we have all of the required parameters
	for (int i=0; i<required_params.count(); i++) {
		if (!ret.named_parameters.contains(required_params[i])) {
			ret.success=false;
			ret.error_message="Missing required parameter: "+required_params[i];
			return ret;
		}
	}

	//we did it!
	return ret;
}
