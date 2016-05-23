#ifndef CONFIG_H
#define CONFIG_H
#include "global.h"

class Config {
  public:
    Json::Value json;
    Config();
    Config(string);
    void load(string);
    int port;
    string validation_url;
};

#endif
