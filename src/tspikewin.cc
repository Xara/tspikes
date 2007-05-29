#include "tspikewin.h"
#include "glspikes.h"

TSpikeWin::TSpikeWin(NetworkInterface * pNetwork) : 

  pNetwork_(pNetwork), 
  
  spVectpList_(), 


  clusterTable_(2, 3), 
  spikeWaveTable_(2, 2), 
  spikeWaveVBox_(false, 0), 
  clusterViewVBox_(false, 0), 
  mainHBox_(false, 0), 

  clusterViewXY_(&spVectpList_, VIEW12), 
  clusterViewXA_(&spVectpList_, VIEW13), 
  clusterViewXB_(&spVectpList_, VIEW14), 
  clusterViewYA_(&spVectpList_, VIEW23), 
  clusterViewYB_(&spVectpList_, VIEW24), 
  clusterViewAB_(&spVectpList_, VIEW34), 
  
  spikeWaveViewX_(CHANX), 
  spikeWaveViewY_(CHANY), 
  spikeWaveViewA_(CHANA), 
  spikeWaveViewB_(CHANB)    
    
{
  //
  // Top-level window.
  //
  
  set_title("Tetrode Spike Viewer");

  set_reallocate_redraws(true);
  spVectpList_.push_back(new GLSPVect_t); 
  add(mainHBox_); 

  int clusterWidth = 180; 
  clusterViewXY_.set_size_request(clusterWidth, clusterWidth);
  clusterViewXA_.set_size_request(clusterWidth, clusterWidth);
  clusterViewXB_.set_size_request(clusterWidth, clusterWidth);
  clusterViewYA_.set_size_request(clusterWidth, clusterWidth);
  clusterViewYB_.set_size_request(clusterWidth, clusterWidth);
  clusterViewAB_.set_size_request(clusterWidth, clusterWidth);

  
  clusterTable_.attach(clusterViewXY_, 0, 1, 0, 1);
  clusterTable_.attach(clusterViewXA_, 1, 2, 0, 1);
  clusterTable_.attach(clusterViewXB_, 2, 3, 0, 1);
  clusterTable_.attach(clusterViewYA_, 0, 1, 1, 2);
  clusterTable_.attach(clusterViewYB_, 1, 2, 1, 2);
  clusterTable_.attach(clusterViewAB_, 2, 3, 1, 2);


  rateTimeline_.set_size_request(clusterWidth*3, 60); 

  clusterViewVBox_.pack_start(clusterTable_) ;
  clusterViewVBox_.pack_start(rateTimeline_); 
  
  spikeWaveViewX_.set_size_request(175, clusterWidth); 
  spikeWaveViewY_.set_size_request(175, clusterWidth); 
  spikeWaveViewA_.set_size_request(175, clusterWidth); 
  spikeWaveViewB_.set_size_request(175, clusterWidth); 

  spikeWaveTable_.attach(spikeWaveViewX_, 0, 1, 0, 1); 
  spikeWaveTable_.attach(spikeWaveViewY_, 1, 2, 0, 1); 
  spikeWaveTable_.attach(spikeWaveViewA_, 0, 1, 1, 2); 
  spikeWaveTable_.attach(spikeWaveViewB_, 1, 2, 1, 2); 

 
  spikeWaveVBox_.pack_start(spikeWaveTable_); 
  
  mainHBox_.pack_start(spikeWaveVBox_); 
  mainHBox_.pack_start(clusterViewVBox_);   
  //
  // Show window.
  //

  float decay = 0.05; 
  clusterViewXY_.setView(spVectpList_.begin(), 
			 spVectpList_.end(), 
			 decay, LOG); 
  clusterViewXY_.setViewingWindow(0, 0, float(150e-6), float(150e-6));

  clusterViewXA_.setView(spVectpList_.begin(), 
			 spVectpList_.end(), 
			 decay, LOG); 
  clusterViewXA_.setViewingWindow(0, 0, float(150e-6), float(150e-6));

  clusterViewXB_.setView(spVectpList_.begin(), 
			 spVectpList_.end(), 
			 decay, LOG); 
  clusterViewXB_.setViewingWindow(0, 0, float(150e-6), float(150e-6));

  clusterViewYA_.setView(spVectpList_.begin(), 
			 spVectpList_.end(), 
			 decay, LOG); 
  clusterViewYA_.setViewingWindow(0, 0, float(150e-6), float(150e-6));

  clusterViewYB_.setView(spVectpList_.begin(), 
			 spVectpList_.end(), 
			 decay, LOG); 
  clusterViewYB_.setViewingWindow(0, 0, float(150e-6), float(150e-6));

  clusterViewAB_.setView(spVectpList_.begin(), 
			 spVectpList_.end(), 
			 decay, LOG); 
  clusterViewAB_.setViewingWindow(0, 0, float(150e-6), float(150e-6));


   spikeWaveViewX_.setViewingWindow(0, -100e-6, 32, 280e-6); 
   spikeWaveViewY_.setViewingWindow(0, -100e-6, 32, 280e-6); 
   spikeWaveViewA_.setViewingWindow(0, -100e-6, 32, 280e-6); 
   spikeWaveViewB_.setViewingWindow(0, -100e-6, 32, 280e-6); 

  show_all();

  Glib::signal_idle().connect( sigc::mem_fun(*this, &TSpikeWin::on_idle) );
  Glib::signal_io().connect(sigc::mem_fun(*this, &TSpikeWin::dataRXCallback), 
			    pNetwork_->getDataFifoPipe(), Glib::IO_IN); 
  
  
}

TSpikeWin::~TSpikeWin()
{

}

bool TSpikeWin::on_idle()

{
  clusterViewXY_.get_window()->invalidate_rect(get_allocation(), true);
  clusterViewXA_.get_window()->invalidate_rect(get_allocation(), true);
  clusterViewXB_.get_window()->invalidate_rect(get_allocation(), true);
  clusterViewYA_.get_window()->invalidate_rect(get_allocation(), true);
  clusterViewYB_.get_window()->invalidate_rect(get_allocation(), true);
  clusterViewAB_.get_window()->invalidate_rect(get_allocation(), true);


  spikeWaveViewX_.get_window()->invalidate_rect(get_allocation(), true);
  spikeWaveViewY_.get_window()->invalidate_rect(get_allocation(), true);
  spikeWaveViewA_.get_window()->invalidate_rect(get_allocation(), true);
  spikeWaveViewB_.get_window()->invalidate_rect(get_allocation(), true);

//   double seconds = timer_.elapsed();
  
//   if (seconds >= 1.0)
//     {
//       std::cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
//       std::cout.setf(std::ios::fixed, std::ios::floatfield);
//       std::cout.precision(3);
//       int m_Frames = clusterViewXY_.getFrames(); 
//       std::cout << m_Frames << " frames in "
//                 << seconds << " seconds = "
//                 << (m_Frames / seconds) << " FPS\n";
//       timer_.reset();

//     }

//   double dseconds = dtimer_.elapsed();
//   if (dseconds >= 0.1)
//     {

//       if (spvlp != spvlsrc.end())
// 	{
// 	  spvl.push_back(*spvlp); 
// 	  spvlp++; 
// 	} else {
// 	  spvl.push_back(new GLSPVect); 
// 	}
      
//       dtimer_.reset(); 
//     }

  
  return true;
}


bool TSpikeWin::dataRXCallback(Glib::IOCondition io_condition)
{
  
  if ((io_condition & Glib::IO_IN) == 0) {
    std::cerr << "Invalid fifo response" << std::endl;
    return false; 
  }
  else 
    {
      char x; 
      read(pNetwork_->getDataFifoPipe(), &x, 1); 
      DataPacket_t * rdp = pNetwork_->getNewData(); 
      
      // is this a spike? 
      if (rdp->typ == TSPIKE)
	{

	  // convert to a real spike packet
	  TSpike_t ts = rawToTSpike(rdp); 

	  // now what do we do with it? 
	  appendTSpikeToSpikewaves(ts); 
	  appendTSpikeToSPL(ts); 
	}
      else 
	{
	  std::cout << "Not a spike?"  << std::endl; 
	}

    
    }
  return true; 
}


void TSpikeWin::appendTSpikeToSpikewaves(const TSpike_t & tspike)
{
  GLSpikeWave_t x, y, a, b; 
  x.ts = tspike.time; 
  y.ts = tspike.time; 
  a.ts = tspike.time; 
  b.ts = tspike.time; 
  float SCALE = 1e9; 
  
  for (int i = 0; i < TSPIKEWAVE_LEN; i++)
    {
      x.wave.push_back(float(tspike.x.wave[i]) / SCALE); 
      y.wave.push_back(float(tspike.y.wave[i]) / SCALE); 
      a.wave.push_back(float(tspike.a.wave[i]) / SCALE); 
      b.wave.push_back(float(tspike.b.wave[i]) / SCALE); 

    }

  spikeWaveViewX_.newSpikeWave(x); 
  spikeWaveViewY_.newSpikeWave(y); 
  spikeWaveViewA_.newSpikeWave(a); 
  spikeWaveViewB_.newSpikeWave(b); 

}

void TSpikeWin::appendTSpikeToSPL(const TSpike_t & tspike)
{
  GLSpikePoint_t sp; 
  sp.ts = tspike.time; 
  sp.p1 = -100e6; 
  sp.p2 = -100e6; 
  sp.p3 = -100e6; 
  sp.p4 = -100e6; 
  sp.tchan = - 1 ;
  
  // remember tspike is in nv
  float SCALE = 1e9; 

  for (int i = 0; i < TSPIKEWAVE_LEN; i++)
    {
      GLfloat xval = float(tspike.x.wave[i]) / SCALE; 
      GLfloat yval = float(tspike.y.wave[i]) / SCALE; 
      GLfloat aval = float(tspike.a.wave[i]) / SCALE; 
      GLfloat bval = float(tspike.b.wave[i]) / SCALE; 

      GLfloat xthresh = float(tspike.x.threshold) / SCALE; 
      GLfloat ythresh = float(tspike.y.threshold) / SCALE; 
      GLfloat athresh = float(tspike.a.threshold) / SCALE; 
      GLfloat bthresh = float(tspike.b.threshold) / SCALE; 

      if (sp.p1 < xval)
	sp.p1 = xval; 
      
      if (sp.tchan < 0 & xval > xthresh)
	{
	  sp.tchan = 0; 
	}

      if (sp.p2 < yval)
	sp.p2 = yval; 
      
      if (sp.tchan < 0 & yval > ythresh)
	{
	  sp.tchan = 1; 
	}

      if (sp.p3 < aval)
	sp.p3 = aval; 
      
      if (sp.tchan < 0 & aval > athresh)
	{
	  sp.tchan = 2; 
	}

      if (sp.p4 < bval)
	sp.p4 = bval; 
      
      if (sp.tchan < 0 & bval > bthresh)
	{
	  sp.tchan = 3; 
	}
      
    }
  

  spVectpList_.back()->push_back(sp); 
  if (spVectpList_.back()->size() > 1000)
    {
      spVectpList_.push_back(new GLSPVect_t); 
    }
  
 
 
}
