#ifndef __PICPROCESSORCOLORSPACE_H__
#define __PICPROCESSORCOLORSPACE_H__

#include "PicProcessor.h"


class PicProcessorColorSpace: public PicProcessor
{
	public:
		PicProcessorColorSpace(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters);
		void showParams();
		bool processPic();
};

#endif
