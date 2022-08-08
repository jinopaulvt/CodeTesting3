#include "BULLH.hpp"


bulletin_resources::bulletin_resources(){
    ReadIn = 0;
    messages_num = 0;
    messagesUp.clear();
}

int bulletin_resources::load_board(){

    fstream fs;
    fs.open(SUbname, fstream::in | fstream::out);
    int TS;
    while(fs >> TS){
        if (TS > messages_num){
            messages_num++;
        }

        string message;
        getline(fs, message);
        message.erase(message.begin());
        messagesUp[TS] = message;
    }
    
    fs.close();

    return 0;
}

int bulletin_resources::set_filename(char* name){
    sprintf(SUbname, "%s", name);
    return 0;
}

int bulletin_resources::load_message(int q, char* msg){
    if (q > messages_num || q <= 0)
        return 1;
    strcpy(msg, messagesUp[q].c_str());
    return 0;
}

int bulletin_resources::get_messages_num(){
    return messages_num;
}

int bulletin_resources::write_message(int q, char* user, char* msg, int size){
    if ( q > messages_num )
        messages_num++;
    if (size == 0)
        return 1;
    fstream fs;
    fs.open(SUbname, fstream::out | fstream:: app);
    fs << q << "/" << user << "/";
    fs.write(msg, size);
    fs << endl;
    fs.close();
    return 0;
}

char* bulletin_resources::get_filename(){
    return SUbname;
}