#include "interface/CfgManager.h"
#include "interface/CfgManagerT.h"

//**********utils*************************************************************************

//----------Check if the key is in cfg----------------------------------------------------
bool CfgManager::OptExist(string key, int opt)
{
    for(auto& iopt : opts_)
        if(iopt.first.find(key) != string::npos && iopt.second.size()>opt)
            return true;

    return false;
}

//----------Help method, parse single line------------------------------------------------
void CfgManager::ParseSingleLine(const string& line, vector<string>& tokens)
{
    //---parsing utils
    size_t prev=0, pos;
    string delimiter=" ";

    //---line loop
    while((pos = line.find_first_of(delimiter, prev)) != string::npos)
    {
        if(pos > prev)
            tokens.push_back(line.substr(prev, pos-prev));
        prev = pos+1;
        if(line[prev] == '\"')
        {
            delimiter = "\"";
            ++prev;
        }
        else
            delimiter = " ";
    }
    if(prev < line.length())
        tokens.push_back(line.substr(prev, string::npos));

    return;
}

//----------Parse configuration file and setup the configuration--------------------------
void CfgManager::ParseConfigFile(const char* file)
{
    cout << "> CfgManager --- INFO: parsing " << file << endl;
    //---read config file
    ifstream cfgFile(file, ios::in);
    string buffer;
    string current_block="opts";
    map<string, vector<string> > block_opts;
    while(getline(cfgFile, buffer))
    {
        if(buffer.size() == 0 || buffer.at(0) == '#')
            continue;        

        //---parse the current line
        vector<string> tokens;
        ParseSingleLine(buffer, tokens);
        while(tokens.back() == "\\")
        {
            tokens.pop_back();
            getline(cfgFile, buffer);
            ParseSingleLine(buffer, tokens);
        }

        //---new block
        if(tokens.at(0).at(0) == '<')
        {
            if(tokens.at(0).at(1) == '/')
            {
                tokens.at(0).erase(tokens.at(0).begin(), tokens.at(0).begin()+2);
                tokens.at(0).erase(--tokens.at(0).end());
                int last_dot = current_block.find_last_of(".");
                if(tokens.at(0) == current_block.substr(last_dot+1))
                    current_block.erase(last_dot);
                else
                    cout << "> CfgManager --- ERROR: wrong closing block // " << tokens.at(0) << endl;
            }
            else
            {
                tokens.at(0).erase(tokens.at(0).begin());
                tokens.at(0).erase(--tokens.at(0).end());
                current_block += "."+tokens.at(0);
            }
        }
        //---import cfg
        else if(tokens.at(0) == "importCfg")
        {
            tokens.erase(tokens.begin());
            for(auto& cfgFile: tokens)
                ParseConfigFile(cfgFile.c_str());
        }
        //---add key
        else 
        {
            string key=tokens.at(0);
            tokens.erase(tokens.begin());
            opts_[current_block+"."+key] = tokens;
        }
    }
    cfgFile.close();

    //---set automatic info
    char hostname[100];
    gethostname(hostname, 100);
    username_ = string(getpwuid(getuid())->pw_name)+"@"+hostname;
    time_t rawtime;
    time(&rawtime);
    struct tm* t = localtime(&rawtime);
    timestamp_ = to_string(t->tm_mday)+"/"+to_string(t->tm_mon)+"/"+to_string(t->tm_year+1900)+"  "+
        to_string(t->tm_hour)+":"+to_string(t->tm_min)+":"+to_string(t->tm_sec);

    return;
}

//----------Print formatted version of the cfg--------------------------------------------
//---option is the key to be print: default value meas "all keys"
void CfgManager::Print(Option_t* option) const
{
    string argkey = option;
    //---banner
    string banner = "configuration was created by "+username_+" on "+timestamp_;
    for(int i=0; i<banner.size(); ++i)
        cout << "=";
    cout << endl;
    cout << banner << endl;
    for(int i=0; i<banner.size(); ++i)
        cout << "=";
    cout << endl;
    
    //---options
    string prev_block="";
    for(auto& key : opts_)
    {
        string current_block = key.first.substr(5, key.first.find_last_of(".")-5);
        if(argkey == "" || key.first.find(argkey) != string::npos)
        {
            if(current_block != prev_block)
            {
                if(prev_block != "")
                    cout << "+----------" << endl;
                cout << current_block << ":" << endl;
                prev_block = current_block;
            }
            cout << "|----->" << key.first.substr(key.first.find_last_of(".")+1) << ": ";
            for(auto& opt : key.second)
                cout << opt << ", " ;
            cout << endl;
        }
    }
    cout << "+----------" << endl;

    return;
}

//----------Internal error check----------------------------------------------------------
void CfgManager::Errors(string key, int opt)
{
    if(opts_.count(key) == 0)
    {
        cout << "> CfgManager --- ERROR: key '"<< key.substr(5, key.size()) << "' not found" << endl;
        exit(-1);
    }
    if(opt >= opts_[key].size())
    {
        cout << "> CfgManager --- ERROR: option '"<< key.substr(5, key.size()) << "' as less then "
             << opt << "values (" << opts_[key].size() << ")" << endl;
        exit(-1);
    }
    return;
}

//**********operators*********************************************************************

ostream& operator<<(ostream& out, const CfgManager& obj)
{
    //---banner
    out << "configuration: " << endl;
    //---options
    for(auto& key : obj.opts_)
    {
        out << key.first.substr(5) << ":" << endl;
        for(auto& opt : key.second)
            out << "\t" << opt << endl;
    }
    return out;
}
