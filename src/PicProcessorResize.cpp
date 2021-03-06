#include "PicProcessor.h"
#include "PicProcessorResize.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "myFloatCtrl.h"
#include "myIntegerCtrl.h"
#include "gimage_parse.h"
#include "gimage_process.h"
#include "util.h"

#include <wx/spinctrl.h>

#define RESIZEENABLE 7400
#define BLURENABLE  7401
#define BLURSIGMA 7402

class ResizePanel: public PicProcPanel
{
	public:
		ResizePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM).Expand();
			wxGridBagSizer *g = new wxGridBagSizer();

			wxArrayString algos;
			algos.Add("box");
			algos.Add("bilinear");
			algos.Add("bspline");
			algos.Add("bicubic");
			algos.Add("catmullrom");
			algos.Add("lanczos3");
			wxArrayString p = split(params,",");

			enablebox = new wxCheckBox(this, RESIZEENABLE, _("resize:"));
			enablebox->SetValue(true);
			g->Add(enablebox, wxGBPosition(0,0), wxGBSpan(1,2), wxALIGN_LEFT | wxALL, 3);
			g->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)),  wxGBPosition(1,0), wxGBSpan(1,4), wxALIGN_LEFT | wxBOTTOM | wxEXPAND, 10);

			widthedit = new myIntegerCtrl(this, wxID_ANY, _("width:"), atoi(p[0].c_str()), 0, 100000, wxDefaultPosition, wxSize(50,TEXTCTRLHEIGHT));
			widthedit->SetToolTip(_("width in pixels, 0 preserves aspect.\nIf you type a number, type Enter to update the image."));
			g->Add(widthedit, wxGBPosition(2,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			heightedit = new myIntegerCtrl(this, wxID_ANY, _("height:"), atoi(p[1].c_str()), 0, 100000, wxDefaultPosition, wxSize(50,TEXTCTRLHEIGHT));
			heightedit->SetToolTip(_("height in pixels, 0 preserves aspect. \nIf you type a number, type Enter to update the image."));
			g->Add(heightedit, wxGBPosition(2,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);		

			algoselect = new wxRadioBox (this, wxID_ANY, _("Resize Algorithm"), wxDefaultPosition, wxDefaultSize,  algos, 1, wxRA_SPECIFY_COLS);
			algoselect->SetSelection(algoselect->FindString(wxString(myConfig::getConfig().getValueOrDefault("tool.resize.algorithm","lanczos3"))));
			if (p.size() >=3) {
				for (int i=0; i<algos.size(); i++) {
					if (p[2] == algos[i]) algoselect->SetSelection(i);
				}
			}
			g->Add(algoselect, wxGBPosition(4,0), wxGBSpan(1,4), wxALIGN_LEFT | wxALL, 5);	

#ifdef PREBLUR
			g->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)),  wxGBPosition(5,0), wxGBSpan(1,4), wxALIGN_LEFT | wxBOTTOM | wxEXPAND, 10);
			blurbox = new wxCheckBox(this, BLURENABLE, _("enable pre-blur:"));
			blurbox->SetValue(false);
			g->Add(blurbox, wxGBPosition(6,0), wxGBSpan(1,4), wxALIGN_LEFT | wxALL, 3);
			blursigma = new myFloatCtrl(this, BLURSIGMA, 1.0, 1, wxDefaultPosition, wxSize(50,TEXTCTRLHEIGHT));
			g->Add(new wxStaticText(this,wxID_ANY, _("sigma:")), wxGBPosition(7,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			g->Add(blursigma, wxGBPosition(7,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			//blurkernel= ...
#endif

			SetSizerAndFit(g);
			SetFocus();
			t.SetOwner(this);
			Bind(myINTEGERCTRL_UPDATE, &ResizePanel::paramChanged, this);
			Bind(myINTEGERCTRL_CHANGE, &ResizePanel::onWheel, this);
			Bind(wxEVT_TIMER, &ResizePanel::OnTimer, this);
			Bind(wxEVT_RADIOBOX,&ResizePanel::paramChanged, this);	
			Bind(wxEVT_CHECKBOX, &ResizePanel::onEnable, this, RESIZEENABLE);
#ifdef PREBLUR
			Bind(wxEVT_CHECKBOX, &ResizePanel::paramChanged, this, BLURENABLE);
#endif
			Bind(wxEVT_CHAR_HOOK, &ResizePanel::OnKey,  this);
			Thaw();
		}

		void onEnable(wxCommandEvent& event)
		{
			if (enablebox->GetValue()) 
				q->enableProcessing(true);
				
			else 
				q->enableProcessing(false);
			process();
		}

		void onWheel(wxCommandEvent& event)
		{
#ifdef PREBLUR
			if (event.GetId() == BLURSIGMA) {
				if (blurbox->GetValue()) t->Start(500,wxTIMER_ONE_SHOT);
			}
			else {
				t.Start(500,wxTIMER_ONE_SHOT);
			}
#else
			t.Start(500,wxTIMER_ONE_SHOT);
#endif
		}


		void OnTimer(wxTimerEvent& event)
		{
			process();
			event.Skip();
		}

		void paramChanged(wxCommandEvent& event)
		{
			process();
			event.Skip();
		}

		void process()
		{
#ifdef PREBLUR
			if (blurbox->GetValue()) 
				q->setParams(wxString::Format("%d,%d,%s,%s,%f,%d",widthedit->GetIntegerValue(),heightedit->GetIntegerValue(),algoselect->GetString(algoselect->GetSelection()),"blur",blursigma->GetFloatValue(),6));
			else
#endif
				q->setParams(wxString::Format("%d,%d,%s",widthedit->GetIntegerValue(),heightedit->GetIntegerValue(),algoselect->GetString(algoselect->GetSelection())));
			q->processPic();
		}


	private: 
		myIntegerCtrl *widthedit, *heightedit;
#ifdef PREBLUR
		myFloatCtrl *blursigma, *blurkernel;
		wxCheckBox *blurbox;
#endif
		wxRadioBox *algoselect;
		wxCheckBox *enablebox;
		wxTimer t;

};


PicProcessorResize::PicProcessorResize(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//parm tool.resize.x: Default resize of the width dimension.  Default=640
	wxString x =  wxString(myConfig::getConfig().getValueOrDefault("tool.resize.x","640"));
	//parm tool.resize.y: Default resize of the height dimension.  Default=0 (calculate value to preserve aspect)
	wxString y =  wxString(myConfig::getConfig().getValueOrDefault("tool.resize.y","0"));
	//parm tool.resize.algorithm: Interpolation algorithm to use when none is specified.  Default=lanczos3  
	//template tool.resize.algorithm=box|bilinear|bspline|bicubic|catmullrom|lanczos3
	wxString algo = wxString(myConfig::getConfig().getValueOrDefault("tool.resize.algorithm","lanczos3"));

	// resize:[x],[y],[algorithm]
	wxArrayString cp = split(getParams(),",");
	if (cp.size() == 3) { 
		x = cp[0];
		y = cp[1];
		algo  = cp[2];
	}
	if (cp.size() == 2) { 
		x = cp[0];
		y = cp[1];
	}
	if (cp.size() == 1) {
		setParams(wxString::Format("%s",cp[0]));
		return;
	} 
	setParams(wxString::Format("%s,%s,%s",x,y,algo));
}

void PicProcessorResize::createPanel(wxSimplebook* parent)
{
	toolpanel = new ResizePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorResize::processPicture(gImage *processdib) 
{
	if (!processingenabled) return true;
	
	((wxFrame*) m_display->GetParent())->SetStatusText(_("resize..."));
	bool ret = true;
	std::map<std::string,std::string> result;

	std::map<std::string,std::string> params;
	std::string pstr = getParams().ToStdString();

	if (!pstr.empty())
		params = parse_resize(std::string(pstr));
	
	if (params.find("error") != params.end()) {
		wxMessageBox(params["error"]);
		ret = false; 
	}
	else if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		wxMessageBox("Error - no mode");
		ret = false;
	}
	else { 
		result = process_resize(*dib, params);
		
		if (result.find("error") != result.end()) {
			wxMessageBox(wxString(result["error"]));
			ret = false;
		}
		else {
			if (paramexists(result,"treelabel")) m_tree->SetItemText(id, wxString(result["treelabel"]));
			m_display->SetModified(true);
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || 
				(myConfig::getConfig().getValueOrDefault("tool.resize.log","0") == "1"))
					log(wxString::Format(_("tool=resize,%s,imagesize=%dx%d,threads=%s,time=%s"),
						params["mode"].c_str(),
						dib->getWidth(), 
						dib->getHeight(),
						result["threadcount"].c_str(),
						result["duration"].c_str())
					);
		}
	}

	dirty=false;
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	return ret;
}






