#include "somanetcodec.h"
#include <tslogging.h>

TSpikeChannelState::TSpikeChannelState()
{
  gain = 0; 
  threshold = 0; 
  hpf = false; 
  filtid = 0; 
  rangeMin = 0; 
  rangeMax = 0; 
}

// SomaNetworkCodec::SomaNetworkCodec(pNetworkInterface_t pNetwork, int src, 
// 				   chanproplist_t channels) :
//   pNetwork_(pNetwork), 
//   dsrc_(src), 
//   chanprops_(channels), 
//   dspStateProxy_(src, sigc::mem_fun(*this, &SomaNetworkCodec::sendEvent))
// {

//   Glib::signal_io().connect(sigc::mem_fun(*this, &SomaNetworkCodec::dataRXCallback), 
// 			    pNetwork_->getDataFifoPipe(), Glib::IO_IN); 
//   Glib::signal_io().connect(sigc::mem_fun(*this, &SomaNetworkCodec::eventRXCallback), 
// 			    pNetwork_->getEventFifoPipe(), Glib::IO_IN); 
  
//   channelStateCache_.resize(channels.size()); 

//   dspStateProxy_.acqdatasrc.linkStatus().connect(
// 						 sigc::mem_fun(*this, &SomaNetworkCodec::dspLinkStatus)); 
//   dspStateProxy_.acqdatasrc.gain().connect(
//  					   sigc::mem_fun(*this, &SomaNetworkCodec::dspGain)); 
//   dspStateProxy_.acqdatasrc.hpfen().connect(
//  					   sigc::mem_fun(*this, &SomaNetworkCodec::dspHPFen)); 
  
//   dspStateProxy_.acqdatasrc.range().connect(
//  					   sigc::mem_fun(*this, &SomaNetworkCodec::dspRange)); 
  
//   dspStateProxy_.tspikesink.thold().connect(
// 					    sigc::mem_fun(*this, &SomaNetworkCodec::dspThold)); 
  
//   dspStateProxy_.tspikesink.filterID().connect(
// 					    sigc::mem_fun(*this, &SomaNetworkCodec::dspFilterID)); 

// }

SomaNetworkCodec::SomaNetworkCodec(pNetworkInterface_t pNetwork, int src) :
  pNetwork_(pNetwork), 
  dsrc_(src),
  dspStateProxy_(NULL)
{

  chanprop_t x, y, a,  b; 
  x.chan = 0; 
  x.name = "X"; 
  y.chan = 1; 
  y.name = "Y"; 
  a.chan = 2; 
  a.name = "A"; 
  b.chan = 3; 
  b.name = "B"; 
  chanprops_.push_back(x); 
  chanprops_.push_back(y); 
  chanprops_.push_back(a); 
  chanprops_.push_back(b); 
  
  Glib::signal_io().connect(sigc::mem_fun(*this, &SomaNetworkCodec::dataRXCallback), 
			    pNetwork_->getDataFifoPipe(), Glib::IO_IN); 
  Glib::signal_io().connect(sigc::mem_fun(*this, &SomaNetworkCodec::eventRXCallback), 
			    pNetwork_->getEventFifoPipe(), Glib::IO_IN); 
  
  std::cout << "Running network" << std::endl; 
  pNetwork_->run(); 

  channelStateCache_.resize(chanprops_.size()); 
  const uint64_t DSP_TIMEOUT = 50000; // 1 sec
  dspStateProxy_ = new somadspio::StateProxy(src, sigc::mem_fun(*this, &SomaNetworkCodec::sendEvent), 
					     DSP_TIMEOUT);
  dspStateProxy_->acqdatasrc.linkStatus().connect(
						 sigc::mem_fun(*this, &SomaNetworkCodec::dspLinkStatus)); 
  dspStateProxy_->acqdatasrc.gain().connect(
 					   sigc::mem_fun(*this, &SomaNetworkCodec::dspGain)); 
  dspStateProxy_->acqdatasrc.hpfen().connect(
 					   sigc::mem_fun(*this, &SomaNetworkCodec::dspHPFen)); 
  
  dspStateProxy_->acqdatasrc.range().connect(
 					   sigc::mem_fun(*this, &SomaNetworkCodec::dspRange)); 
  
  dspStateProxy_->tspikesink.thold().connect(
					    sigc::mem_fun(*this, &SomaNetworkCodec::dspThold)); 
  
  dspStateProxy_->tspikesink.filterID().connect(
					    sigc::mem_fun(*this, &SomaNetworkCodec::dspFilterID)); 
  
  
  
}



sigc::signal<void, int, TSpikeChannelState> &  SomaNetworkCodec::signalSourceStateChange() {
  return signalSourceStateChange_; 

}

sigc::signal<void, somatime_t> & SomaNetworkCodec::signalTimeUpdate()
{
  return signalTimeUpdate_; 
}

sigc::signal<void, const TSpike_t &> & SomaNetworkCodec::signalNewTSpike()
{
  
  return signalNewTSpike_; 
  
}


void SomaNetworkCodec::processNewData(pDataPacket_t  dp)
{
  if (dp->typ == TSPIKE ) {
    TSpike_t newTSpike = rawToTSpike(dp); 
    signalNewTSpike_.emit(newTSpike); 
  } else {
    // somehow we received a datapacket that wasn't for us
  }


}

void SomaNetworkCodec::processNewEvents(pEventList_t pEventList)
{
  EventList_t::iterator pe; 
  for (pe = pEventList->begin(); pe != pEventList->end(); pe++) {
   
      parseEvent(*pe); 
  }
        
}

void SomaNetworkCodec::parseEvent(const Event_t & evt)
{

  if (evt.src == 0x00 && evt.cmd == 0x10 )
    {
      // this is the time
      somatime_t stime = 0; 
      stime = evt.data[0]; 
      stime = stime << 16; 
      stime |= evt.data[1]; 
      stime = stime << 16; 
      stime |= evt.data[2]; 
      if (stime % 500 == 0)  {
	// normal time events are just too frequent. 
	signalTimeUpdate_.emit(stime); 
	dspStateProxy_->setTime(stime); 
      }
    } else {
    if (dspStateProxy_) {
      if (evt.src > 0 && evt.src < 70) {
	dspStateProxy_->newEvent(evt); 
      }
    }
  }
  
  //   bool stateCacheUpdate = false; 
//   if (evt.src == dsrc_to_esrc(dsrc_) && evt.cmd  == 0x92) {

//     // state update
//     int tgtchan = evt.data[0] & 0xFF; 
//     STATEPARM param = toStateParm((evt.data[0] >> 8) & 0xFF); 

//     int pos = -1;
//     int i = 0; 
    
//     // Find the channel

//     for (chanproplist_t::iterator c = chanprops_.begin(); 
// 	 c != chanprops_.end(); c++) 
//       {
// 	if (c->chan == tgtchan) {
// 	  pos = i; 
// 	} 
// 	i++; 
//       }

//     // if the channel was found, set the value. 
//     if (pos > -1 ) {

//       // find the appropriate state cache value: 
      
//       switch(param) {
//       case GAIN: 
// 	{
// 	  channelStateCache_[pos].gain = evt.data[1]; 
// 	  break; 
// 	}
//       case RANGE: 
// 	{
// 	  int32_t min, max; 
// 	  min = evt.data[1];
// 	  min = (min << 16) | (evt.data[2]); 
// 	  max = evt.data[3]; 
// 	  max = (max << 16) | (evt.data[4]); 
// 	  channelStateCache_[pos].rangeMin =  min; 
// 	  channelStateCache_[pos].rangeMax = max; 
// 	  break; 
// 	}
//       case THOLD:
// 	{
// 	  int32_t thold (0); 
// 	  thold = evt.data[1]; 
// 	  thold = (thold << 16 ) | evt.data[2]; 
// 	  channelStateCache_[pos].threshold = thold; 
// 	  break; 
// 	}
//       case HPF:
// 	{
// 	  if (evt.data[1] == 0) {
// 	    channelStateCache_[pos].hpf = false; 
// 	  } else {
// 	    channelStateCache_[pos].hpf = true; 
// 	  } 
// 	  break; 
// 	}
//       case FILT: 
// 	{
// 	  channelStateCache_[pos].filtid = evt.data[1]; 
// 	  break; 
// 	}
//       default: 
// 	std::cerr << "Should not get here; unknown property update"
// 		  << param << std::endl;
//       }
      
//     } else {
//       std::cerr << "This channel update was not for us" 
// 		<< pos << ' ' << i << std::endl; 
//     }

//     signalSourceStateChange_.emit(tgtchan, channelStateCache_[pos]); 
    
//   }

  // if event is a state update; we do the state-update-dance
  
  // emit relevant signals


}

bool SomaNetworkCodec::dataRXCallback(Glib::IOCondition io_condition)
{
  if ((io_condition & Glib::IO_IN) == 0) {
    std::cerr << "Invalid fifo response" << std::endl;
    return false; 
  }
  else 
    {
      char x; 
      read(pNetwork_->getDataFifoPipe(), &x, 1); 
      pDataPacket_t rdp = pNetwork_->getNewData(); 

      processNewData(rdp); 
      
    }
  return true; 
}


bool SomaNetworkCodec::eventRXCallback(Glib::IOCondition io_condition)
{
  
  if ((io_condition & Glib::IO_IN) == 0) {
    std::cerr << "Invalid fifo response" << std::endl;
    return false; 
  }
  else 
    {
      char x; 
      read(pNetwork_->getEventFifoPipe(), &x, 1); 
      pEventList_t pel = pNetwork_->getNewEvents()->events; 
      processNewEvents(pel); 
    }
  return true; 
}


void SomaNetworkCodec::querySourceState(int chan) {
  // send an event to query


//   EventTXList_t etl(6); 
//   for(int i = 0; i < 6; i++) {
//     etl[i].destaddr[dsrc_to_esrc(dsrc_)] = 1; // THIS SOURCE DEVICE
//     etl[i].event.src = 4; 
//     etl[i].event.cmd = 0x91; 
//     etl[i].event.data[0] = chan ; 
//   }

//   etl[0].event.data[1] = GAIN; 
//   etl[1].event.data[1] = THOLD; 
//   etl[2].event.data[1] = HPF; 
//   etl[3].event.data[1] = FILT; 
//   etl[4].event.data[1] = RANGE;
//   etl[5].event.data[1] = RANGE; 

//   pNetwork_->sendEvents(etl); 

}

void SomaNetworkCodec::refreshStateCache() {
  // send refresh events for all
  chanproplist_t::iterator c; 
  for (c = chanprops_.begin() ; c != chanprops_.end(); c++) {
    querySourceState(c->chan); 
  }

}

TSpikeChannelState SomaNetworkCodec::getChannelState(int channel)
{
  // 
  return channelStateCache_[channel]; 
}

void SomaNetworkCodec::setChannelState(int chan, const TSpikeChannelState & newstate )
{

  if (newstate.gain != channelStateCache_[chan].gain) {
    // GAIN SETTING
    dspStateProxy_->acqdatasrc.setGain(chan, newstate.gain); 

  } else if (newstate.hpf != channelStateCache_[chan].hpf) {
    
    dspStateProxy_->acqdatasrc.setHPFen(chan, newstate.hpf); 

  } else  if (newstate.filtid != channelStateCache_[chan].filtid) {

    dspStateProxy_->tspikesink.setFilterID(chan, newstate.filtid); 

  } else if (newstate.threshold != channelStateCache_[chan].threshold) {
    // THRESHOLD
    dspStateProxy_->tspikesink.setThold(chan, newstate.threshold); 
  }
}


void SomaNetworkCodec::sendEvent(const EventTXList_t & etxl)
{
  std::cout << "sending events" << std::endl; 
  pNetwork_->sendEvents(etxl); 
  
}


void SomaNetworkCodec::dspLinkStatus(bool)
{
  // FIXME: not clear what really to do here
}

void SomaNetworkCodec::dspMode(int mode)
{
  // FIXME: not clear what really to do here

}

void SomaNetworkCodec::dspGain(int chan, int gain)
{
  if (chan  < chanprops_.size()) {
    channelStateCache_[chan].gain = gain; 
    signalSourceStateChange_.emit(chan, channelStateCache_[chan]); 
  }
}

void SomaNetworkCodec::dspHPFen(int chan, bool hpfen)
{
  if (chan  < chanprops_.size()) {
    channelStateCache_[chan].hpf = hpfen; 
    signalSourceStateChange_.emit(chan, channelStateCache_[chan]); 
  }

}

void SomaNetworkCodec::dspRange(int chan, somadspio::range_t range)
{

  
  TSL_(info) << "SomaNetworkCodec: dsp range for chan = " 
	     << chan << " range.first = " << range.first 
	     << " range.second =" << range.second; 
  
  if (chan  < 4) {
    if (range.first < range.second) {
      channelStateCache_[chan].rangeMin = range.first; 
      channelStateCache_[chan].rangeMax = range.second; 
      signalSourceStateChange_.emit(chan, channelStateCache_[chan]); 
    }
  }
}

void SomaNetworkCodec::dspThold(int chan, int thold)
{
  if (chan  < 4) {
    channelStateCache_[chan].threshold  = thold; 
    signalSourceStateChange_.emit(chan, channelStateCache_[chan]); 
  }
}

void SomaNetworkCodec::dspFilterID(int chan, somadspio::filterid_t fid)
{
  if (chan  < 4) {
    channelStateCache_[chan].filtid  = fid; 
    signalSourceStateChange_.emit(chan, channelStateCache_[chan]); 
  }
}

