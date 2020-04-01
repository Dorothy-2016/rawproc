
#include "gimage_parse.h"
#include "gimage/strutil.h"
#include "myConfig.h"
#include "elapsedtime.h"
#include "cJSON.h"

#include <algorithm>

bool paramexists (std::map<std::string,std::string> m, std::string k)
{
	return (m.find(k) != m.end());
}

std::map<std::string,std::string> parse_JSONparams(std::string JSONstring)
{
	std::map<std::string,std::string> pmap;
	cJSON *json = cJSON_Parse(JSONstring.c_str());
	cJSON *iterator;
	cJSON_ArrayForEach(iterator, json) {
		pmap[std::string(iterator->string)] = std::string(iterator->valuestring);
	}
	return pmap;
}

void paramprint(std::map<std::string,std::string> params)
{
	printf("parmstring: %ld elements:\n",params.size()); fflush(stdout);
	if (params.empty()) return;
	
	for (std::map<std::string,std::string>::iterator it=params.begin(); it!=params.end(); ++it) {
		printf("\t%s: %s\n",it->first.c_str(), it->second.c_str()); 
		fflush(stdout);
	}
}


//blackwhitepoint
//auto:		rgb|red|green|blue - do auto on channel
//values:	rgb|red|green|blue,<nbr>[,<nbr>] - specific b/w on channel, if only one number, b=<nbr>, w=255
//data:		rgb|red|green|blue,data - b/w on data limits, b=smallest r|g|b, w=largest r|g|b
//data:		rgb|red|green|blue,data,minwhite - b/w on data limits w=smallest r|g|b
//camera:	camera - b/w on rgb using camera exif limits
//values:	<nbr>,[<nbr>] - specific b/w on rgb, if only one number, b=<nbr>, w=255
//auto:		NULL - do auto on rgb

std::map<std::string,std::string> parse_blackwhitepoint(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();

		if (paramstring == std::string()) {  //NULL string, default processing
			pmap["channel"] = "rgb";
			pmap["mode"] = "auto";
		}
		else if (p[0] == "rgb" | p[0] == "red" | p[0] == "green" | p[0] == "blue") {    // | p[0] == "min") {   ??
			pmap["channel"] = p[0];
			if (psize >= 2) {
				if (p[1] == "data") { //bound on min/max of image data
					pmap["mode"] = "data";
					if (psize >= 3 && p[2] == "minwhite") pmap["minwhite"] = "true";
				}
				else if (p[1] == "auto") {
					pmap["mode"] = "auto";
					pmap["channel"] = p[0];
				}
				else { 
					if (!isFloat(p[1])) {
						pmap["error"] = string_format("blackwhitepoint:ParseError - invalid float: %s",p[1].c_str());
						return pmap;
					}
					pmap["mode"] = "values";
					pmap["black"] = p[1];
					if (psize >= 3) {
						if (!isFloat(p[2])) {
							pmap["error"] = string_format("blackwhitepoint:ParseError - invalid float: %s",p[2].c_str());
							return pmap;
						}
						pmap["white"] = p[2];
					}
					else {
						if (atof(p[1].c_str()) < 1.0)
							pmap["white"] = "1.0";
						else
							pmap["white"] = "255";
					}
				}
			}
			else pmap["mode"] = "auto";
		}
		else if (p[0] == "camera") {
			pmap["mode"] = "camera";
			pmap["channel"] = "rgb";
		}
		else if (p[0] == "norm") {
			pmap["mode"] = "norm";
			pmap["channel"] = "rgb";
			pmap["black"] = "0.0";
			pmap["white"] = "1.0";
			if (psize == 3) {
				pmap["black"] = p[1];
				pmap["white"] = p[2];
			}
			else {
				pmap["error"] = "blackwhitepoint:ParseError - norm requires two values.";
				return pmap;
			}
		}
		else if (psize == 2) {  //just two values
			if (!isFloat(p[0])) {
				pmap["error"] = string_format("blackwhitepoint:ParseError - invalid float: %s",p[0].c_str());
				return pmap;
			}
			if (!isFloat(p[1])) {
				pmap["error"] = string_format("blackwhitepoint:ParseError - invalid float: %s",p[1].c_str());
				return pmap;
			}
			pmap["mode"] = "values";
			pmap["channel"] = "rgb";
			pmap["black"] = p[0];
			pmap["white"] = p[1];
		}
		else {
			pmap["error"] = string_format("blackwhitepoint:ParseError - unrecognized positional parameter string: %s",p[0].c_str());
			return pmap;
		}
	}
	return pmap;
}



//colorspace
//file		:<profile_filename>[,assign|convert][,absolute_colorimetric|relative_colorimetric|perceptual|saturation][,bpc] - open profile file and use to assign|convert
//camera	:camera[,assign|convert][,absolute_colorimetric|relative_colorimetric|perceptual|saturation][,bpc] - find camera primaries in dcraw.c|camconst.json|libraw, make a d65 profile and use to assign|convert
//primaries	:<int>,<int>,<int>,<int>,<int>,<int>,<int>,<int>,<int>[,assign|convert][,absolute_colorimetric|relative_colorimetric|perceptual|saturation][,bpc] - use 9 ints to make a d65 profile and use to assign|convert 
//built-in	:srgb|wide|adobe|prophoto|identity[,assign|convert][,absolute_colorimetric|relative_colorimetric|perceptual|saturation][,bpc]

std::map<std::string,std::string> parse_colorspace(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();
		unsigned token = 0;

		pmap["bpc"] = "false";

		if (p[0] == "camera") {
			pmap["mode"] = "camera";
			token = 1;
		}

		else if (p[0] == "srgb" | p[0] == "wide" | p[0] == "adobe" | p[0] == "prophoto" | p[0] == "identity") {
			pmap["mode"] = "built-in";
			pmap["icc"] = p[0];
			token = 1;
		}

		else if (std::count(p[0].begin(), p[0].end(), ',') == 8) {
			pmap["mode"] = "primaries";
			pmap["icc"] = 
				string_format("%s,%s,%s,%s,%s,%s,%s,%s,%s",p[1].c_str(),p[2].c_str(),p[3].c_str(),p[4].c_str(),p[5].c_str(),p[6].c_str(),p[7].c_str(),p[8].c_str(),p[9].c_str());
			token = 10;
		}
		else {  //treat whatever is in p[0] as a file name
			pmap["mode"] = "file";
			pmap["icc"] = p[0];
			token = 1;
		}

		for (unsigned i=token; i<psize; i++) {  //op, rendering intent, and/or bpc in any order
			if (p[i] == "convert" | p[i] == "assign") {
				pmap["op"] = p[i];
			}
			else if (p[i] == "absolute_colorimetric" | 
				 p[i] == "relative_colorimetric" | 
				 p[i] == "perceptual" | 
				 p[i] == "saturation") {
					pmap["rendering_intent"] = p[i];
			}
			else if (p[i] == "bpc") {
				pmap["bpc"] = true;
			}				
			else {
				pmap["error"] = string_format("colorspace:ParseError - unrecognized positional parameter: %s",p[i].c_str());
				return pmap;
			}
		}

	}
	return pmap;
}

//crop
//default	:<x>,<y>,<w>,<h> - extract subimage at x,y,w,h bounds and make the new image.  can be either int coords or 0.0-1.0 proportions to w|h
std::map<std::string,std::string> parse_crop(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();
		
		if (psize < 4) {
			pmap["error"] = string_format("crop:ParseError - not enough crop parameters.");
			return pmap;
		}
		pmap["mode"] = "default";
		pmap["x"] = p[0];
		pmap["y"] = p[1];
		pmap["w"] = p[2];
		pmap["h"] = p[3];
		
	}
	return pmap;
}

//curve
//default	:rgb|red|green|blue,<x1>,<y1>,...,<xn>,<yn> - apply curve to the designated channel defined by the x,y coordinates, 0-255
std::map<std::string,std::string> parse_curve(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();
		
		if (p[0] == "rgb" | p[0] == "red" | p[0] == "green" | p[0] == "blue") {  
			pmap["channel"] =  p[0];
			pmap["mode"] = "default";
			if (isFloat(p[1])) 
				pmap["curvecoords"] = p[1];
			else {
				pmap["error"] = string_format("curve:ParseError - not a float: %s.",p[1].c_str());
				return pmap;
			}
			for (unsigned i=2; i<psize; i++) {
				if (isFloat(p[i])) {
					pmap["curvecoords"] += std::string(",") + p[i];
				}
				else {
					pmap["error"] = string_format("curve:ParseError - not a float: %s.",p[i].c_str());
					return pmap;
				}
			}
		}
		else if (isFloat(p[0])) {
			pmap["channel"] =  "rgb";
			pmap["mode"] = "default";
			for (unsigned i=0; i<psize; i++) {
				if (isFloat(p[i])) {
					pmap["curvecoords"] += std::string(",") + p[i];
				}
				else {
					pmap["error"] = string_format("curve:ParseError - not a float: %s.",p[i].c_str());
					return pmap;
				}
			}
		}
		else {
			
		}
		
	}
	return pmap;
}


//demosaic (each algo is a mode...)
//:color - color the unmosaiced image with the pattern colors
//:half|half_resize|vng|rcd|igv|ahd|xtran_fast - demosaic the image
//:dcb[,<iterations>][,dcb_enhance] - demosaic the image with the supplied parameters
//:amaze[,<initgain>][,<border>] - demosaic the image with the supplied parameters
//:lmmse[,<iterations] - demosaic the image with the supplied parameters
//:xtran_markesteijn[,<passes>][,usecielab] - demosaic the image with the supplied parameters
//:proof - xtran_fast or half, depending on image type (xtran|bayer)
std::map<std::string,std::string> parse_demosaic(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();

		if (p[0] == "half" |p[0] == "half_resize" |
#ifdef USE_LIBRTPROCESS
			p[0] == "vng" |p[0] == "rcd" |p[0] == "igv" |p[0] == "ahd" |p[0] == "xtran_fast" |p[0] == "dcb" |p[0] == "amaze" |p[0] == "lmmse" |p[0] == "xtran_markesteijn" |
#endif
			p[0] == "color") {
				pmap["mode"] = p[0];
		}
		else {
			pmap["error"] = string_format("demosaic:ParseError - Unrecognized demosaic option: %s.",p[0].c_str());
			return pmap;
		}

#ifdef USE_LIBRTPROCESS
		if (pmap["mode"] == "dcb") {
			if (psize >= 2) 
				pmap["iterations"] = p[1];
			else
				pmap["iterations"] = "1";
			if (psize >= 3) {
				if (p[2] == "dcb_enhance" | p[2] == "1") 
					pmap["dcb_enhance"] = "true";
				else {
					pmap["dcb_enhance"] = "false";
				}
			}
		}
		if (pmap["mode"] == "amaze") {
			if (psize >= 2) pmap["initgain"] = p[1];
			if (psize >= 3) pmap["border"] = p[2]; 
		}
		if (pmap["mode"] == "lmmse") {
			if (psize >= 2) 
				pmap["iterations"] = p[1];
			else
				pmap["iterations"] = "1";
		}
		if (pmap["mode"] == "xtran_markesteijn") {
			if (psize >= 2) 
				pmap["passes"] = p[1];
			else
				pmap["passes"] = "1";
			if (psize >= 3) {
				if (p[2] == "usecielab") 
					pmap["usecielab"] = "true";
				else {
					pmap["error"] = string_format("demosaic:ParseError - parameter should be 'usecielab', not %s.",p[2].c_str());
					return pmap;
				}
			}
		}
#endif
	}
	return pmap;
}

