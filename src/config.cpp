#include "config.h"

Config::Config(){
}

Config::Config(string fname){
  load(fname);
}

void Config::load(string fname){
  bool parsingSuccessful;
  std::ifstream ifs(fname.c_str());
  std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
  Json::Reader reader;
  parsingSuccessful = reader.parse(content, json);
  if (parsingSuccessful){
    port = json["port"].asInt();
    validation_url = json["validation_url"].asString();
    cout << "Loaded configuration:" << endl;
    cout << "\t" << "Port: " << port << endl;
    cout << "\t" << "Validation_url: " << validation_url << endl;
  }
}
