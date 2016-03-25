#include "interface/CfgManager.h"
#include "interface/CfgManagerT.h"

//**********utils*************************************************************************

//----------Check if the key is in cfg----------------------------------------------------
bool CfgManager::OptExist(std::string key, int opt)
{
    for(auto& iopt : opts_)
        if(iopt.first == "opts."+key && iopt.second.size()>opt)
            return true;

    return false;
}

//----------Help method, parse single line------------------------------------------------
void CfgManager::ParseSingleLine(const std::string& line, std::vector<std::string>& tokens)
{
    //---parsing utils
    size_t prev=0, pos;
    std::string delimiter=" ";

    //---line loop
    while((pos = line.find_first_of(delimiter, prev)) != std::string::npos)
    {
        if(pos > prev)
            tokens.push_back(line.substr(prev, pos-prev));
        prev = pos+1;
        if(line[prev] == '\'')
        {
            delimiter = "\'";
            ++prev;
        }
        else
            delimiter = " ";
    }
    if(prev < line.length())
        tokens.push_back(line.substr(prev, std::string::npos));

    return;
}

//----------Parse configuration file and setup the configuration--------------------------
void CfgManager::ParseConfigFile(const char* file)
{
    std::cout << "> CfgManager --- INFO: parsing " << file << std::endl;
    //---read config file
    std::ifstream cfgFile(file, std::ios::in);
    std::string buffer;
    std::string current_block="opts";
    while(getline(cfgFile, buffer))
    {
        //---strip comments and unneeded whitespace
        while(buffer.size() > 0 && buffer.at(0) == ' ')
            buffer.erase(buffer.begin());
        int comment_pos = buffer.find("#");
        if(comment_pos != std::string::npos)
            buffer.erase(buffer.begin()+comment_pos, buffer.end());
        if(buffer.size() == 0)
            continue;        

        //---parse the current line
        std::vector<std::string> tokens;
        ParseSingleLine(buffer, tokens);
        while(tokens.back() == "\\")
        {
            tokens.pop_back();
            getline(cfgFile, buffer);
            ParseSingleLine(buffer, tokens);
        }
        //---store the option inside the current configuration
        HandleOption(current_block, tokens);
    }
    cfgFile.close();

    //---set automatic info
    char hostname[100];
    gethostname(hostname, 100);
    username_ = std::string(getpwuid(getuid())->pw_name)+"@"+hostname;
    time_t rawtime;
    time(&rawtime);
    struct tm* t = localtime(&rawtime);
    timestamp_ = std::to_string(t->tm_mday)+"/"+std::to_string(t->tm_mon)+"/"+std::to_string(t->tm_year+1900)+"  "+
        std::to_string(t->tm_hour)+":"+std::to_string(t->tm_min)+":"+std::to_string(t->tm_sec);

    return;
}

//----------Parse configuration string and setup the configuration------------------------
void CfgManager::ParseConfigString(const std::string config)
{
    std::string current_block="opts";
    //---parse the current string
    std::vector<std::string> tokens;
    ParseSingleLine(config, tokens);
    HandleOption(current_block, tokens);

    return;
}

//----------Handle single option: key and parameters--------------------------------------
//---private methode called by ParseConfig public methods
void CfgManager::HandleOption(std::string& current_block, std::vector<std::string>& tokens)
{
    //---Handle blocks
    if(tokens.at(0).at(0) == '<')
    {        
        //---close previous block
        if(tokens.at(0).at(1) == '/')
        {
            tokens.at(0).erase(tokens.at(0).begin(), tokens.at(0).begin()+2);
            tokens.at(0).erase(--tokens.at(0).end());
            int last_dot = current_block.find_last_of(".");
            if(tokens.at(0) == current_block.substr(last_dot+1))
                current_block.erase(last_dot);
            else
            {
                std::cout << "> CfgManager --- ERROR: wrong closing block // " << tokens.at(0) << std::endl;
                exit(-1);
            }
        }
        //---open new block
        else 
        {
            bool copy_blocks=false;
            
            tokens.at(0).erase(tokens.at(0).begin());
            //---check if blocks copy is required
            if(tokens.at(0).back() == '=')
            {
                copy_blocks=true;
                tokens.back().erase(--tokens.back().end());
            }
            tokens.at(0).erase(--tokens.at(0).end());            
            current_block += "."+tokens.at(0);
            //---copy from other blocks
            if(copy_blocks)
            {
                tokens.erase(tokens.begin());
                for(auto& block_to_copy: tokens)
                    CopyBlock(current_block, block_to_copy);
            }
        }
    }    
    //---import cfg
    else if(tokens.at(0) == "importCfg")
    {
        tokens.erase(tokens.begin());
        for(auto& cfgFile: tokens)
            ParseConfigFile(cfgFile.c_str());
    }
    //---option line
    else
    {
        std::string key=tokens.at(0);
        tokens.erase(tokens.begin());
            
        //---update key
        if(key.substr(key.size()-2) == "+=")
        {
            key = key.substr(0, key.size()-2);
            for(auto& token : tokens)
            {
                if(OptExist(token))
                {
                    auto extend_opt = GetOpt<std::vector<std::string> >(token);
                    opts_[current_block+"."+key].insert(opts_[current_block+"."+key].end(),
                                                        extend_opt.begin(),
                                                        extend_opt.end());
                }
                else
                    opts_[current_block+"."+key].push_back(token);
            }
        }
        //---copy key
        else if(key.substr(key.size()-1) == "=" && tokens.size() > 0)
        {
            key = key.substr(0, key.size()-1);
            if(OptExist(tokens.at(0)))
                opts_[current_block+"."+key] = GetOpt<std::vector<std::string> >(tokens.at(0));
            else
                std::cout << "> CfgManager --- WARNING: undefined option // " << tokens.at(0) << std::endl;
        }
        //---new key
        else 
            opts_[current_block+"."+key] = tokens;
    }

    return;
}

//----------Copy entire block-------------------------------------------------------------
//---already defined option are overridden
void CfgManager::CopyBlock(std::string& current_block, std::string& block_to_copy)
{
    bool found_any=false;
    //---copy block entries
    for(auto& opt : opts_)
    {
        std::string key = opt.first;
        size_t pos = key.find("."+block_to_copy+".");
        if(pos != std::string::npos)
        {
            std::string new_key = key;
            std::vector<std::string>& opts = opt.second;
            new_key.replace(pos+1, block_to_copy.size(), current_block.substr(5));
            opts_[new_key] = opts;

            found_any=true;
        }
    }
    //---block not found
    if(!found_any)
        std::cout << "> CfgManager --- WARNING: requested block (" << block_to_copy << ") is not defined" << std::endl;

    return;
}

//----------Print formatted version of the cfg--------------------------------------------
//---option is the key to be print: default value meas "all keys"
void CfgManager::Print(Option_t* option) const
{
    std::string argkey = option;
    //---banner
    std::string banner = "configuration was created by "+username_+" on "+timestamp_;
    for(int i=0; i<banner.size(); ++i)
        std::cout << "=";
    std::cout << std::endl;
    std::cout << banner << std::endl;
    for(int i=0; i<banner.size(); ++i)
        std::cout << "=";
    std::cout << std::endl;
    
    //---options
    std::string prev_block="";
    for(auto& key : opts_)
    {
        std::string current_block = key.first.substr(5, key.first.find_last_of(".")-5);
        if(argkey == "" || key.first.find(argkey) != std::string::npos)
        {
            if(current_block != prev_block)
            {
                if(prev_block != "")
                    std::cout << "+----------" << std::endl;
                std::cout << current_block << ":" << std::endl;
                prev_block = current_block;
            }
            std::cout << "|----->" << key.first.substr(key.first.find_last_of(".")+1) << ": ";
            for(auto& opt : key.second)
                std::cout << opt << ", " ;
            std::cout << std::endl;
        }
    }
    std::cout << "+----------" << std::endl;

    return;
}

//----------Internal error check----------------------------------------------------------
void CfgManager::Errors(std::string key, int opt)
{
    if(opts_.count(key) == 0)
    {
        std::cout << "> CfgManager --- ERROR: key '"<< key.substr(5, key.size()) << "' not found" << std::endl;
        exit(-1);
    }
    if(opt >= opts_[key].size())
    {
        std::cout << "> CfgManager --- ERROR: option '"<< key.substr(5, key.size()) << "' as less then "
             << opt << "values (" << opts_[key].size() << ")" << std::endl;
        exit(-1);
    }
    return;
}

//**********operators*********************************************************************

std::ostream& operator<<(std::ostream& out, const CfgManager& obj)
{
    //---banner
    out << "configuration: " << std::endl;
    //---options
    for(auto& key : obj.opts_)
    {
        out << key.first.substr(5) << ":" << std::endl;
        for(auto& opt : key.second)
            out << "\t" << opt << std::endl;
    }
    return out;
}
