/*
  TODO
 */


#include <pthread.h>

#define TRUE (1==1)
#define FALSE (!TRUE)


enum globMode {
  automaticMode = 0,
  onMode,
  offMode
};

enum globCommand {
  forcedOff = 0,
  forcedOn,
  automaticOn,
  reachedSwitchOff,
  reachedSwitchOn,
  setConfig
};


  
