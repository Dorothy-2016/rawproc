#include "myBatchDialog.h"
#include "rawprocFrm.h"
#include "myRowSizer.h"
#include "myConfig.h"
#include "util.h"

//#include <wx/stattext.h>
#include "wx/process.h"
#include <wx/utils.h> 
#include <wx/clipbrd.h>

#define BATCHPROCESS 2010
#define BATCHSHOW 2011

myBatchDialog::myBatchDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size):
wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE) // | wxRESIZE_BORDER)
{
	wxFileName dirspec = wxFileName(wxFileName::GetCwd(), "");
	wxFileName inputspec = ((rawprocFrm *) parent)->getFileName().GetFullPath();
	wxFileName outputspec = ((rawprocFrm *) parent)->getSourceFileName().GetFullPath();
	inputspec.MakeRelativeTo();
	outputspec.MakeRelativeTo();
	inputspec.SetName("*");	
	outputspec.SetName("*");
	wxString roottool = inputfilecommand(((rawprocFrm *) parent)->getRootTool())[1];
	if (roottool != "") roottool = ":"+roottool;

	directory = new wxTextCtrl(this, wxID_ANY, wxFileName::GetCwd(), wxDefaultPosition, wxSize(500,25));
	inputfilespec = new wxTextCtrl(this, wxID_ANY, inputspec.GetFullPath()+roottool, wxDefaultPosition, wxSize(500,TEXTHEIGHT+5));
	outputfilespec = new wxTextCtrl(this, wxID_ANY, outputspec.GetFullPath(), wxDefaultPosition, wxSize(500,TEXTHEIGHT+5));
	toolchain = new wxTextCtrl(this, wxID_ANY, ((rawprocFrm *) parent)->getToolChain(), wxDefaultPosition, wxSize(500,TEXTHEIGHT*4), wxTE_MULTILINE);

	wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
	myRowSizer *s = new myRowSizer(wxSizerFlags().Expand());
	s->AddRowItem(directory,flags);
	s->NextRow();
	s->AddRowItem(inputfilespec,flags);
	s->NextRow();
	s->AddRowItem(outputfilespec,flags);
	s->NextRow();
	s->AddRowItem(toolchain,flags);
	s->NextRow();
	s->AddRowItem(new wxButton(this, wxID_OK, "Dismiss", wxDefaultPosition, wxDefaultSize),flags);
	s->AddRowItem(new wxButton(this, BATCHPROCESS, "Process", wxDefaultPosition, wxDefaultSize),flags);
	s->AddRowItem(new wxButton(this, BATCHSHOW, "Show", wxDefaultPosition, wxDefaultSize),flags);
	s->End();
	SetSizerAndFit(s);

	Bind(wxEVT_BUTTON, &myBatchDialog::OnProcess, this, BATCHPROCESS);
	Bind(wxEVT_BUTTON, &myBatchDialog::OnShow, this, BATCHSHOW);
}

wxString myBatchDialog::ConstructCommand()
{
	//parm batch.termcommand: path/executable to use as the batch command shell. Default: wxcmd (somewhere in $PATH)
	wxString term = wxString(myConfig::getConfig().getValueOrDefault("batch.termcommand","wxcmd"));
	//batch.imgcommand: path/executable for the img command line raw processor. Default: img (somewhere in $PATH)
	wxString img = wxString(myConfig::getConfig().getValueOrDefault("batch.imgcommand","img"));
	//parm batch.termcommand.options: [options], inserts the specified command line options after the term command, before the batch command. Use -x for wxcmd to exit after the batch command is complete.
	//parm batch.termcommand.options = (command line options): If present, the specified command line switches will be appended to the termcommand. For wxcmd, use '-x' to autodismiss the batch command when it is complete.
	if (myConfig::getConfig().exists("batch.termcommand.options")) term += " " + wxString(myConfig::getConfig().getValueOrDefault("batch.termcommand.options",""));
	//parm batch.imgcommand.options = (command line options): If present, the specified command line switches will be appended to the imgcommand. For rawproc's img, see the help file for valid options.
	if (myConfig::getConfig().exists("batch.imgcommand.options")) img += " " + wxString(myConfig::getConfig().getValueOrDefault("batch.imgcommand.options",""));
	wxString toolchainstr = toolchain->GetValue().Trim();
	toolchainstr = "\"" + toolchainstr + "\"";
	toolchainstr.Replace(" ","\" \"");
	return wxString::Format("%s %s \"%s\" %s \"%s\"",term,img,inputfilespec->GetValue(),toolchainstr,outputfilespec->GetValue());
}

void myBatchDialog::OnProcess(wxCommandEvent& event)
{
	wxExecute(ConstructCommand());
}

void myBatchDialog::OnShow(wxCommandEvent& event)
{
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData( new wxTextDataObject(ConstructCommand()) );
		wxTheClipboard->Close();
	}
	wxMessageBox(ConstructCommand());
}



