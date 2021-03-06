/*
 *  See header file for a description of this class.
 *
 *  \author Paolo Ronchese INFN Padova
 *
 */

//----------------------
// This Class' Header --
//----------------------
#include "CondFormats/DTObjects/interface/DTTPGParameters.h"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "CondFormats/DTObjects/interface/DTBufferTree.h"

//---------------
// C++ Headers --
//---------------
#include <iostream>
#include <sstream>

//-------------------
// Initializations --
//-------------------


//----------------
// Constructors --
//----------------
DTTPGParameters::DTTPGParameters():
  dataVersion( " " ),
  nsPerCount( 25.0 / 32.0 ),
  clockLength( 32 ),
  dBuf(new DTBufferTree<int,int>) {
  dataList.reserve( 250 );
}


DTTPGParameters::DTTPGParameters( const std::string& version ):
  dataVersion( version ),
  nsPerCount( 25.0 / 32.0 ),
  clockLength( 32 ),
  dBuf(new DTBufferTree<int,int>) {
  dataList.reserve( 250 );
}


DTTPGParameters& DTTPGParameters::operator=(DTTPGParameters const& rhs) {
  if (this != &rhs) {
    dataVersion = rhs.dataVersion;
    nsPerCount = rhs.nsPerCount;
    clockLength = rhs.clockLength;
    dataList = rhs.dataList;
    initialize();
  }
  return *this;
}


DTTPGParametersId::DTTPGParametersId() :
    wheelId( 0 ),
  stationId( 0 ),
   sectorId( 0 ) {
}


DTTPGParametersData::DTTPGParametersData() :
  nClock( 0 ),
  tPhase( 0.0 ) {
}


//--------------
// Destructor --
//--------------
DTTPGParameters::~DTTPGParameters() {
}


DTTPGParametersId::~DTTPGParametersId() {
}


DTTPGParametersData::~DTTPGParametersData() {
}


//--------------
// Operations --
//--------------
int DTTPGParameters::get( int   wheelId,
                          int stationId,
                          int  sectorId,
                          int&    nc,
                          float&  ph,
                          DTTimeUnits::type unit ) const {

  nc = 0;
  ph = 0.0;

  std::vector<int> chanKey;
  chanKey.reserve(3);
  chanKey.push_back(   wheelId );
  chanKey.push_back( stationId );
  chanKey.push_back(  sectorId );
  int ientry;
  int searchStatus = dBuf->find( chanKey.begin(), chanKey.end(), ientry );
  if ( !searchStatus ) {
    const DTTPGParametersData& data( dataList[ientry].second );
    nc = data.nClock;
    ph = data.tPhase;
    if ( unit == DTTimeUnits::ns ) {
      ph *= nsPerCount;
    }
  }

  return searchStatus;

}


int DTTPGParameters::get( const DTChamberId& id,
                          int&    nc,
                          float&  ph,
                          DTTimeUnits::type unit ) const {
  return get( id.wheel(),
              id.station(),
              id.sector(),
              nc, ph, unit );
}


float DTTPGParameters::totalTime( int   wheelId,
                                  int stationId,
                                  int  sectorId,
                                  DTTimeUnits::type unit ) const {
  int cl = 0;
  float ph = 0.0;
  get( wheelId, stationId, sectorId, cl, ph, unit );
  if ( unit == DTTimeUnits::ns ) return ( cl * clock() * nsPerCount ) + ph;
  else                           return ( cl * clock()              ) + ph;
}


float DTTPGParameters::totalTime( const DTChamberId& id,
                                  DTTimeUnits::type unit ) const {
  return totalTime( id.wheel(),
                    id.station(),
                    id.sector(),
                    unit );
}


int DTTPGParameters::clock() const {
  return clockLength;
}


float DTTPGParameters::unit() const {
  return nsPerCount;
}


const
std::string& DTTPGParameters::version() const {
  return dataVersion;
}


std::string& DTTPGParameters::version() {
  return dataVersion;
}


void DTTPGParameters::clear() {
  dataList.clear();
  initialize();
  return;
}


int DTTPGParameters::set( int   wheelId,
                          int stationId,
                          int  sectorId,
                          int    nc,
                          float  ph,
                          DTTimeUnits::type unit ) {

  if ( unit == DTTimeUnits::ns ) {
    ph /= nsPerCount;
  }

  std::vector<int> chanKey;
  chanKey.reserve(3);
  chanKey.push_back(   wheelId );
  chanKey.push_back( stationId );
  chanKey.push_back(  sectorId );
  int ientry;
  int searchStatus = dBuf->find( chanKey.begin(), chanKey.end(), ientry );

  if ( !searchStatus ) {
    DTTPGParametersData& data( dataList[ientry].second );
    data.nClock = nc;
    data.tPhase = ph;
    return -1;
  }
  else {
    DTTPGParametersId key;
    key.  wheelId =   wheelId;
    key.stationId = stationId;
    key. sectorId =  sectorId;
    DTTPGParametersData data;
    data.nClock = nc;
    data.tPhase = ph;
    ientry = dataList.size();
    dataList.push_back( std::pair<DTTPGParametersId,
                                  DTTPGParametersData>( key, data ) );
    dBuf->insert( chanKey.begin(), chanKey.end(), ientry );
    return 0;
  }

  return 99;

}


int DTTPGParameters::set( const DTChamberId& id,
                          int    nc,
                          float  ph,
                          DTTimeUnits::type unit ) {
  return set( id.wheel(),
              id.station(),
              id.sector(),
              nc, ph, unit );
}


void DTTPGParameters::setClock( int clock ) {
  clockLength = clock;
}


void DTTPGParameters::setUnit( float unit ) {
  nsPerCount = unit;
}


DTTPGParameters::const_iterator DTTPGParameters::begin() const {
  return dataList.begin();
}


DTTPGParameters::const_iterator DTTPGParameters::end() const {
  return dataList.end();
}


std::string DTTPGParameters::mapName() const {
  std::stringstream name;
  name << dataVersion << "_map_TTPG" << this;
  return name.str();
}


void DTTPGParameters::initialize() {

  dBuf->clear();

  int entryNum = 0;
  int entryMax = dataList.size();
  std::vector<int> chanKey;
  chanKey.reserve(3);
  while ( entryNum < entryMax ) {

    const DTTPGParametersId& chan = dataList[entryNum].first;

    chanKey.clear();
    chanKey.push_back( chan.  wheelId );
    chanKey.push_back( chan.stationId );
    chanKey.push_back( chan. sectorId );
    dBuf->insert( chanKey.begin(), chanKey.end(), entryNum++ );

  }

  return;

}

