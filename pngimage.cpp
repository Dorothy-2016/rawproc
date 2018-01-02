#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <zlib.h>

#include <string>
#include <map>

#include "gimage/strutil.h"


const char * pngVersion()
{
	return PNG_LIBPNG_VER_STRING;
}

#define PNG_BYTES_TO_CHECK 8
bool _checkPNG(const char *filename)
{
	char buf[PNG_BYTES_TO_CHECK];
	FILE *fp;

	/* Open the prospective PNG file. */
	if ((fp = fopen(filename, "rb")) == NULL)
		return 0;

	/* Read in some of the signature bytes */
	if (fread(buf, 1, PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK)
		return 0;

	/* Compare the first PNG_BYTES_TO_CHECK bytes of the signature.
	Return nonzero (true) if they match */

	return(!png_sig_cmp((png_bytep)buf, 0, PNG_BYTES_TO_CHECK));
}

bool _loadPNGInfo(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits, std::map<std::string,std::string> &info)
{
	char *img;
	unsigned w, h, c, b;

	png_byte color_type;
	png_byte bit_depth;
	png_bytep *row_pointers;

	FILE *fp = fopen(filename, "rb");

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png) return NULL;

	png_infop pinfo = png_create_info_struct(png);
	if(!pinfo) return NULL;

	if(setjmp(png_jmpbuf(png))) return NULL;

	png_init_io(png, fp);

	png_read_info(png, pinfo);

	*width      = png_get_image_width(png, pinfo);
  	*height     = png_get_image_height(png, pinfo);
	*numcolors  = png_get_channels(png, pinfo);
	*numbits    = png_get_bit_depth(png, pinfo);

	fclose(fp);
	if (png && pinfo)
		png_destroy_write_struct(&png, &pinfo);
}

/*
char * _loadPNGSimple(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits, std::map<std::string,std::string> &info, std::string params="", char ** icc_m=NULL, unsigned  *icclength=0)
{
	char *img;
	unsigned w, h, c, b;

	png_image pimage;
	memset(&pimage, 0, (sizeof pimage));
	pimage.version = PNG_IMAGE_VERSION;
	
	png_image_begin_read_from_file(&pimage, filename);
	pimage.format = PNG_FORMAT_RGB;
	
	img = (char *) malloc(PNG_IMAGE_SIZE(pimage));
	w = pimage.width;
	h = pimage.height;
	c = PNG_IMAGE_PIXEL_CHANNELS(pimage.format);
	b = PNG_IMAGE_PIXEL_COMPONENT_SIZE(pimage.format);
	
	int stride = PNG_IMAGE_ROW_STRIDE(pimage);
	png_image_finish_read(&pimage, NULL, img, 0, NULL);
	png_image_free(&pimage);
	*width = w;
	*height = h;
	*numcolors = c;
	*numbits = b;
	
	std::map<std::string,std::string> inf;
	info = inf;

	*icc_m = NULL;
	*icclength = 0;

	return img;

}
*/

char * _loadPNG(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits, std::map<std::string,std::string> &info, std::string params="", char ** icc_m=NULL, unsigned  *icclength=0)
{
	char *img;
	unsigned w, h, c, b;

	png_byte color_type;
	png_byte bit_depth;
	png_bytep *row_pointers;

	FILE *fp = fopen(filename, "rb");

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png) return NULL;

	png_infop pinfo = png_create_info_struct(png);
	if(!pinfo) return NULL;

	if(setjmp(png_jmpbuf(png))) return NULL;

	png_init_io(png, fp);

	png_read_info(png, pinfo);

	*width      = png_get_image_width(png, pinfo);
  	*height     = png_get_image_height(png, pinfo);
	*numcolors  = png_get_channels(png, pinfo);
	*numbits    = png_get_bit_depth(png, pinfo);
	
	img = new char[(*width)*(*height)*(*numcolors)*((*numbits)/8)];

	png_read_update_info(png, pinfo);
	
	unsigned stride = png_get_rowbytes(png,pinfo);

	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * (*height));
	for(int y = 0; y < (*height); y++) {
		row_pointers[y] = (png_byte*)malloc(stride);
	}

	png_read_image(png, row_pointers);

	char * dst = img;
	for(int y = 0; y < (*height); y++) {
		memcpy(dst, row_pointers[y], stride);
		dst += stride;
	}

	for(int y = 0; y < (*height); y++) {
		free(row_pointers[y]);
	}
	free(row_pointers);

	fclose(fp);
	
	std::map<std::string,std::string> inf;
	info = inf;

	*icc_m = NULL;
	*icclength = 0;
	
	png_destroy_read_struct(&png, &pinfo, NULL);
	png=NULL;
	pinfo=NULL;
	
	if (png && pinfo)
	png_destroy_write_struct(&png, &pinfo);

	return img;

}

/*
bool _writePNG(const char *filename, char *imagedata, unsigned width, unsigned height, unsigned numcolors, unsigned numbits, std::map<std::string,std::string> info, std::string params, char *iccprofile, unsigned iccprofilelength)
{
	png_image pimage;
	memset(&pimage, 0, (sizeof pimage));
	pimage.version = PNG_IMAGE_VERSION;
	pimage.width = width;
	pimage.height = height;
	pimage.format = PNG_FORMAT_RGB;

	png_image_write_to_file (&pimage, filename, 0, imagedata, 0, NULL);
	png_image_free(&pimage);

	return true;
}
*/

bool _writePNG(const char *filename, char *imagedata, unsigned width, unsigned height, unsigned numcolors, unsigned numbits, std::map<std::string,std::string> info, std::string params, char *iccprofile, unsigned iccprofilelength)
{
	std::map<std::string,std::string> p = parseparams(params);

	png_bytep *row_pointers;
	
	FILE *fp = fopen(filename, "wb");
	if(!fp) return NULL;

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) return NULL;

	png_infop pinfo = png_create_info_struct(png);
	if (!pinfo) return NULL;

	if (setjmp(png_jmpbuf(png))) return NULL;

	png_init_io(png, fp);
	
	int compression = 3;
	if (p.find("compression") != p.end()) 
		compression = atoi(p["compression"].c_str());

	png_set_compression_level(png, compression);
	png_set_compression_strategy(png, 2);
	png_set_filter(png,0,PNG_FILTER_SUB);
	
	png_set_IHDR(
		png,
		pinfo,
		width, height,
		numbits,
		PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);

	if (iccprofile) png_set_iCCP(png, pinfo, (png_charp) "", PNG_COMPRESSION_TYPE_BASE, iccprofile, iccprofilelength);


	png_write_info(png, pinfo);
	
	png_bytep img = (png_bytep) imagedata;
	unsigned stride = png_get_rowbytes(png,pinfo);

	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for(int y = 0; y < height; y++) {
		row_pointers[y] = img;
		img += stride;
	}

	png_write_image(png, row_pointers);
	png_write_end(png, NULL);
	free(row_pointers);

	fclose(fp);
	
	return true;
	
}
