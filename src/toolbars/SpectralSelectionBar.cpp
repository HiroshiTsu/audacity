/**********************************************************************

Audacity: A Digital Audio Editor

SpectralSelectionBar.cpp

Copyright 2014 Dominic Mazzoni

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

*******************************************************************//**

\class SpectralSelectionBar
\brief (not quite a Toolbar) at foot of screen for setting and viewing the
frequency selection range.

*//****************************************************************//**

\class SpectralSelectionBarListener
\brief A class used to forward events to do
with changes in the SpectralSelectionBar.

*//*******************************************************************/


#include "../Audacity.h"

#include <algorithm>

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/intl.h>
#include <wx/radiobut.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/valtext.h>
#endif
#include <wx/statline.h>

#include "SpectralSelectionBarListener.h"
#include "SpectralSelectionBar.h"

#include "../AudacityApp.h"
#include "../SelectedRegion.h"
#include "../widgets/NumericTextCtrl.h"

#ifdef EXPERIMENTAL_SPECTRAL_EDITING

IMPLEMENT_CLASS(SpectralSelectionBar, ToolBar);

enum {
   SpectralSelectionBarFirstID = 2750,
   OnCenterID,
   OnWidthID,
   OnLowID,
   OnHighID,
   OnChoiceID,
};

BEGIN_EVENT_TABLE(SpectralSelectionBar, ToolBar)
   EVT_SIZE(OnSize)
   EVT_TEXT(OnCenterID, OnCtrl)
   EVT_TEXT(OnWidthID, OnCtrl)
   EVT_TEXT(OnLowID, OnCtrl)
   EVT_TEXT(OnHighID, OnCtrl)
   EVT_CHOICE(OnChoiceID, OnChoice)
   EVT_COMMAND(wxID_ANY, EVT_FREQUENCYTEXTCTRL_UPDATED, OnUpdate)
   EVT_COMMAND(wxID_ANY, EVT_LOGFREQUENCYTEXTCTRL_UPDATED, OnUpdate)
END_EVENT_TABLE()

SpectralSelectionBar::SpectralSelectionBar()
: ToolBar(SpectralSelectionBarID, _("SpectralSelection"), wxT("SpectralSelection"))
, mListener(NULL), mbCenterAndWidth(false)
, mCenter(0.0), mWidth(0.0), mLow(0.0), mHigh(0.0)
, mCenterCtrl(NULL), mWidthCtrl(NULL), mLowCtrl(NULL), mHighCtrl(NULL)
, mChoice(NULL)
{
}

SpectralSelectionBar::~SpectralSelectionBar()
{
   // Do nothing, sizer deletes the controls
}

void SpectralSelectionBar::Create(wxWindow * parent)
{
   ToolBar::Create(parent);
}

void SpectralSelectionBar::Populate()
{
   // This will be inherited by all children:
   SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

   /* we don't actually need a control yet, but we want to use its methods
   * to do some look-ups, so we'll have to create one. We can't make the
   * look-ups static because they depend on translations which are done at
   * runtime */

   wxString frequencyFormatName = mListener
      ? mListener->SSBL_GetFrequencySelectionFormatName()
      : wxString(wxEmptyString);
   wxString logFrequencyFormatName = mListener
      ? mListener->SSBL_GetLogFrequencySelectionFormatName()
      : wxString(wxEmptyString);

   wxFlexGridSizer *mainSizer = new wxFlexGridSizer(1, 1, 1);
   Add(mainSizer, 0, wxALIGN_CENTER_VERTICAL);

   //
   // Top row, choice box
   //

   const wxString choices[2] = {
      _("Center frequency and Width"),
      _("Low and High Frequencies"),
   };
   mChoice = new wxChoice
      (this, OnChoiceID, wxDefaultPosition, wxDefaultSize, 2, choices,
       0, wxDefaultValidator, _("Spectral Selection Specifications"));
   mChoice->SetSelection(mbCenterAndWidth ? 0 : 1);
   mainSizer->Add(mChoice, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND, 5);

   //
   // Bottom row, split into two columns, each with one control
   //

   wxBoxSizer *subSizer = new wxBoxSizer(wxHORIZONTAL);
   if (mbCenterAndWidth) {
      mCenterCtrl = new NumericTextCtrl(
         NumericConverter::FREQUENCY, this, OnCenterID, frequencyFormatName, 0.0);
      mCenterCtrl->SetName(_("Center Frequency:"));
      mCenterCtrl->EnableMenu();
      subSizer->Add(mCenterCtrl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

      mWidthCtrl = new NumericTextCtrl(
         NumericConverter::LOG_FREQUENCY, this, OnWidthID, logFrequencyFormatName, 0.0);
      mWidthCtrl->SetName(wxString(_("Bandwidth:")));
      mWidthCtrl->EnableMenu();
      subSizer->Add(mWidthCtrl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 0);
   }
   else {
      mLowCtrl = new NumericTextCtrl(
         NumericConverter::FREQUENCY, this, OnLowID, frequencyFormatName, 0.0);
      mLowCtrl->SetName(_("Low Frequency:"));
      mLowCtrl->EnableMenu();
      subSizer->Add(mLowCtrl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

      mHighCtrl = new NumericTextCtrl(
         NumericConverter::FREQUENCY, this, OnHighID, frequencyFormatName, 0.0);
      mHighCtrl->SetName(wxString(_("High Frequency:")));
      mHighCtrl->EnableMenu();
      subSizer->Add(mHighCtrl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 0);
   }
   mainSizer->Add(subSizer, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 0);

   mainSizer->Layout();

   Layout();

   SetMinSize(GetSizer()->GetMinSize());
}

void SpectralSelectionBar::UpdatePrefs()
{
   {
      wxCommandEvent e(EVT_FREQUENCYTEXTCTRL_UPDATED);
      e.SetInt((mbCenterAndWidth? mCenterCtrl : mLowCtrl)->GetFormatIndex());
      OnUpdate(e);
   }

   if (mbCenterAndWidth)
   {
      wxCommandEvent e(EVT_LOGFREQUENCYTEXTCTRL_UPDATED);
      e.SetInt(mWidthCtrl->GetFormatIndex());
      OnUpdate(e);
   }

   // Set label to pull in language change
   SetLabel(_("SpectralSelection"));

   // Give base class a chance
   ToolBar::UpdatePrefs();
}

void SpectralSelectionBar::SetListener(SpectralSelectionBarListener *l)
{
   mListener = l;
   SetFrequencySelectionFormatName(mListener->SSBL_GetFrequencySelectionFormatName());
   SetLogFrequencySelectionFormatName(mListener->SSBL_GetLogFrequencySelectionFormatName());
};

void SpectralSelectionBar::OnSize(wxSizeEvent &evt)
{
   Refresh(true);

   evt.Skip();
}

void SpectralSelectionBar::ModifySpectralSelection(bool done)
{
   const double nyq = mListener->SSBL_GetRate() / 2.0;

   double bottom, top;
   if (mbCenterAndWidth) {
      mCenter = mCenterCtrl->GetValue();
      mWidth = mWidthCtrl->GetValue();
      if (mCenter < 0 && mWidth < 0)
         bottom = top = SelectedRegion::UndefinedFrequency;
      else {
         if (mCenter < 0) {
            mWidth = log(std::min(nyq, exp(mWidth)));
            // Choose arbitrary center for the width
            mCenter = sqrt(nyq);
         }
         else if (mWidth < 0) {
            mCenter = std::max(1.0, std::min(nyq, mCenter));
            // Choose arbitrary width for the center
            const double ratio = std::min(mCenter, nyq / mCenter);
            mWidth = log(ratio * ratio);
         }
         else {
            mCenter = std::max(1.0, std::min(nyq, mCenter));
            double ratio = std::min(mCenter, nyq / mCenter);
            mWidth = std::min(2 * log(ratio), mWidth);
         }

         const double ratio = exp(mWidth / 2);
         bottom = mCenter / ratio, top = mCenter * ratio;
      }
   }
   else {
      bottom = mLowCtrl->GetValue();
      top = mHighCtrl->GetValue();

      if (bottom >= 0)
         bottom = std::min(nyq, bottom);
      else
         bottom = SelectedRegion::UndefinedFrequency;

      if (top >= 0)
         top = std::min(nyq, top);
      else
         top = SelectedRegion::UndefinedFrequency;
   }

   // Notify project and track panel, which may change
   // the values again, and call back to us in SetFrequencies()
   mListener->SSBL_ModifySpectralSelection(bottom, top, done);
}

void SpectralSelectionBar::OnCtrl(wxCommandEvent & event)
{
   ModifySpectralSelection(event.GetInt() != 0);
}

void SpectralSelectionBar::OnChoice(wxCommandEvent &)
{
   mbCenterAndWidth = (0 == mChoice->GetSelection());
   ToolBar::ReCreateButtons();
   ValuesToControls();
   Updated();
}

void SpectralSelectionBar::OnUpdate(wxCommandEvent &evt)
{
   int index = evt.GetInt();
   wxWindow *w = FindFocus();
   bool centerFocus = (w && w == mCenterCtrl);
   bool widthFocus = (w && w == mWidthCtrl);
   bool lowFocus = (w && w == mLowCtrl);
   bool highFocus = (w && w == mHighCtrl);

   evt.Skip(false);

   // Save formats before recreating the controls so they resize properly
   wxEventType type = evt.GetEventType();
   if (type == EVT_FREQUENCYTEXTCTRL_UPDATED) {
      NumericTextCtrl *frequencyCtrl = (mbCenterAndWidth ? mCenterCtrl : mLowCtrl);
      wxString frequencyFormatName = frequencyCtrl->GetBuiltinName(index);
      mListener->SSBL_SetFrequencySelectionFormatName(frequencyFormatName);
   }
   else if (mbCenterAndWidth &&
            type == EVT_LOGFREQUENCYTEXTCTRL_UPDATED) {
      wxString logFrequencyFormatName = mWidthCtrl->GetBuiltinName(index);
      mListener->SSBL_SetLogFrequencySelectionFormatName(logFrequencyFormatName);
   }

   // ToolBar::ReCreateButtons() will get rid of our sizers and controls
   // so reset pointers first.
   mCenterCtrl = mWidthCtrl = NULL;
   mLowCtrl = mHighCtrl = NULL;

   ToolBar::ReCreateButtons();
   ValuesToControls();


   if (centerFocus) {
      mCenterCtrl->SetFocus();
   }
   else if (widthFocus) {
      mWidthCtrl->SetFocus();
   }
   else if (lowFocus) {
      mLowCtrl->SetFocus();
   }
   else if (highFocus) {
      mHighCtrl->SetFocus();
   }

   Updated();
}

void SpectralSelectionBar::ValuesToControls()
{
   if (mbCenterAndWidth) {
      mCenterCtrl->SetValue(mCenter);
      mWidthCtrl->SetValue(mWidth);
   }
   else {
      mLowCtrl->SetValue(mLow);
      mHighCtrl->SetValue(mHigh);
   }
}

void SpectralSelectionBar::SetFrequencies(double bottom, double top)
{
   mLow = bottom;
   mHigh = top;

   if (bottom > 0 && top >= bottom)
      mWidth = log(top / bottom), mCenter = sqrt(top * bottom);
   else
      mWidth = mCenter = -1.0;

   ValuesToControls();
}

void SpectralSelectionBar::SetFrequencySelectionFormatName(const wxString & formatName)
{
   NumericTextCtrl *frequencyCtrl = (mbCenterAndWidth ? mCenterCtrl : mLowCtrl);
   frequencyCtrl->SetFormatName(formatName);

   wxCommandEvent e(EVT_FREQUENCYTEXTCTRL_UPDATED);
   e.SetInt(frequencyCtrl->GetFormatIndex());
   OnUpdate(e);
}

void SpectralSelectionBar::SetLogFrequencySelectionFormatName(const wxString & formatName)
{
   if (mbCenterAndWidth) {
      mWidthCtrl->SetFormatName(formatName);

      wxCommandEvent e(EVT_LOGFREQUENCYTEXTCTRL_UPDATED);
      e.SetInt(mWidthCtrl->GetFormatIndex());
      OnUpdate(e);
   }
}

#endif // #ifdef EXPERIMENTAL_SPECTRAL_EDITING