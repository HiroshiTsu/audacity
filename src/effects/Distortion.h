/**********************************************************************

  Audacity: A Digital Audio Editor

  Distortion.h

  Steve Daulton

**********************************************************************/

#ifndef __AUDACITY_EFFECT_DISTORTION__
#define __AUDACITY_EFFECT_DISTORTION__

#include <wx/arrstr.h>
#include <wx/string.h>
#include <wx/slider.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>

#include <queue>

#include "Effect.h"

class ShuttleGui;

#define DISTORTION_PLUGIN_SYMBOL XO("Distortion")
#define STEPS 1024      // number of +ve or -ve steps in lookup tabe
#define TABLESIZE 2049  // size of lookup table (steps * 2 + 1)

class EffectDistortionState
{
public:
   float       samplerate;
   sampleCount skipcount;
   int         tablechoiceindx;
   bool        dcblock;
   double      threshold;
   double      noisefloor;
   double      param1;
   double      param2;
   int         repeats;

   // DC block filter variables
   std::queue<float> queuesamples;
   double queuetotal;
};

WX_DECLARE_OBJARRAY(EffectDistortionState, EffectDistortionStateArray);

class EffectDistortion final : public Effect
{
public:
   EffectDistortion();
   virtual ~EffectDistortion();

   struct Params
   {
      int    mTableChoiceIndx;
      bool   mDCBlock;
      double mThreshold_dB;
      double mNoiseFloor;
      double mParam1;
      double mParam2;
      int    mRepeats;
   };

   // IdentInterface implementation

   wxString GetSymbol() override;
   wxString GetDescription() override;

   // EffectIdentInterface implementation

   EffectType GetType() override;
   bool SupportsRealtime() override;

   // EffectClientInterface implementation

   int GetAudioInCount() override;
   int GetAudioOutCount() override;
   bool ProcessInitialize(sampleCount totalLen, ChannelNames chanMap = NULL) override;
   sampleCount ProcessBlock(float **inBlock, float **outBlock, sampleCount blockLen) override;
   bool RealtimeInitialize() override;
   bool RealtimeAddProcessor(int numChannels, float sampleRate) override;
   bool RealtimeFinalize() override;
   sampleCount RealtimeProcess(int group,
                               float **inbuf,
                               float **outbuf,
                               sampleCount numSamples) override;
   bool GetAutomationParameters(EffectAutomationParameters & parms) override;
   bool SetAutomationParameters(EffectAutomationParameters & parms) override;
   wxArrayString GetFactoryPresets() override;
   bool LoadFactoryPreset(int id) override;

   // Effect implementation

   void PopulateOrExchange(ShuttleGui & S) override;
   bool TransferDataToWindow() override;
   bool TransferDataFromWindow() override;

private:

   enum control
   {
      ID_Type = 10000,
      ID_DCBlock,
      ID_Threshold,
      ID_NoiseFloor,
      ID_Param1,
      ID_Param2,
      ID_Repeats,
   };
   // EffectDistortion implementation

   void InstanceInit(EffectDistortionState & data, float sampleRate);
   sampleCount InstanceProcess(EffectDistortionState & data,
                               float **inBlock,
                               float **outBlock,
                               sampleCount blockLen);

   // Control Handlers

   void OnTypeChoice(wxCommandEvent & evt);
   void OnDCBlockCheckbox(wxCommandEvent & evt);
   void OnThresholdText(wxCommandEvent & evt);
   void OnThresholdSlider(wxCommandEvent & evt);
   void OnNoiseFloorText(wxCommandEvent & evt);
   void OnNoiseFloorSlider(wxCommandEvent & evt);
   void OnParam1Text(wxCommandEvent & evt);
   void OnParam1Slider(wxCommandEvent & evt);
   void OnParam2Text(wxCommandEvent & evt);
   void OnParam2Slider(wxCommandEvent & evt);
   void OnRepeatsText(wxCommandEvent & evt);
   void OnRepeatsSlider(wxCommandEvent & evt);
   void UpdateUI();
   void UpdateControl(control id, bool enable, wxString name);
   void UpdateControlText(wxTextCtrl *textCtrl, wxString &string, bool enabled);

   void MakeTable();
   float WaveShaper(float sample);
   float DCFilter(EffectDistortionState & data, float sample);

   // Preset tables for gain lookup

   void HardClip();           // hard clipping
   void SoftClip();           // soft clipping
   void ExponentialTable();   // exponential mapping
   void LogarithmicTable();   // logarithmic mapping
   void HalfSinTable();
   void CubicTable();
   void EvenHarmonicTable();
   void SineTable();
   void Leveller();           // 'Leveller' wavetable is modeled on the legacy effect of the same name.
   void Rectifier();          // 0% = Dry, 50% = half-wave rectified, 100% = full-wave rectified (abs value).
   void HardLimiter();        // Same effect as the LADSPA "hardLimiter 1413"

   // Wavetable helper functions

   void CopyHalfTable();   // for symmetric tables

   // Used by Soft Clipping but could be used for other tables.
   // Log curve formula: y = T + (((e^(RT - Rx)) - 1) / -R)
   // where R is the ratio, T is the threshold, and x is from T to 1. 
   inline float LogCurve(double threshold, float value, double ratio);

   // Used by Cubic curve but could be used for other tables
   // Cubic formula: y = x - (x^3 / 3.0)
   inline double Cubic(double x);


private:
   EffectDistortionState mMaster;
   EffectDistortionStateArray mSlaves;

   double mTable[TABLESIZE];
   double mThreshold;
   bool mbSavedFilterState;

   // mMakeupGain is used by some distortion types to pass the
   // amount of gain required to bring overall effect gain to unity
   double mMakeupGain;

   int mTypChoiceIndex;
   wxArrayString mTableTypes;

   wxChoice *mTypeChoiceCtrl;
   wxTextCtrl *mThresholdT;
   wxTextCtrl *mNoiseFloorT;
   wxTextCtrl *mParam1T;
   wxTextCtrl *mParam2T;
   wxTextCtrl *mRepeatsT;

   wxSlider *mThresholdS;
   wxSlider *mNoiseFloorS;
   wxSlider *mParam1S;
   wxSlider *mParam2S;
   wxSlider *mRepeatsS;

   wxCheckBox *mDCBlockCheckBox;

   wxStaticText *mThresholdTxt;
   wxStaticText *mNoiseFloorTxt;
   wxStaticText *mParam1Txt;
   wxStaticText *mParam2Txt;
   wxStaticText *mRepeatsTxt;
   
   wxString mOldThresholdTxt;
   wxString mOldmNoiseFloorTxt;
   wxString mOldParam1Txt;
   wxString mOldParam2Txt;
   wxString mOldRepeatsTxt;

   Params mParams;

   DECLARE_EVENT_TABLE();
};

#endif
