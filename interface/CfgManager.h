#ifndef __CFG_MANAGER__
#define __CFG_MANAGER__

#include <unistd.h>
#include <pwd.h>
#include <time.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <iterator>
#include <map>
#include <vector>
#include <string>
#include <algorithm> 
#include <iomanip>

#include "TObject.h"

using namespace std;

class CfgManager: public TObject
{
public:
    //---ctors---
    CfgManager() {};
    CfgManager(map<string, vector<string> >* defaultCfg) {opts_=*defaultCfg;};
    CfgManager(const char* file) {ParseConfigFile(file);};
    //---dtor---
    ~CfgManager() {};

    //---getters---
    template<typename T=bool> T GetOpt(string key, int opt=0);
                    
    //---setters---
    inline void            SetOpt(const char* key, vector<string>& v_opt)
        {opts_["opts."+string(key)]=v_opt;};
    inline void            SetOpt(string& key, vector<string>& v_opt)
        {opts_["opts."+key]=v_opt;};

    //---utils
    bool                   OptExist(string key, int opt=0);
    inline void            ParseConfigFile(string* file) {ParseConfigFile(file->c_str());};
    void                   ParseConfigFile(const char* file);
    virtual void           Print(Option_t* option="") const;   
    
    //---operators---
    friend ostream& operator<<(ostream& out, const CfgManager& obj);

private:
    //---utils---
    void                    ParseSingleLine(const string& line, vector<string>& tokens);
    void                    Errors(string key, int opt=0);

private:
    map<string, vector<string> >  opts_;
    string username_;
    string timestamp_;

    //---ROOT dictionary
    ClassDef(CfgManager, 1)
};

#endif
