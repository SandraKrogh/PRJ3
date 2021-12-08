#include <iostream>

#include "web/restinio/sample/express_router/selected_t.hpp"

//biblioteker til fopen fil
#include <fstream>
#include <streambuf>
#include <stdio.h>


class RPIdata
{
    public:
    RPIdata(selected_t &selected) : m_selected(selected)
    {

    } 

   void retrieveSelected() 
    {
	    std::cout << "Shit det går godt" << std::endl;
        
        std::ifstream t("settings.txt");
		std::stringstream fileBuffer;
		fileBuffer << t.rdbuf();
		m_selected = json_dto::from_json< selected_t >( fileBuffer.str() );
		t.close();
    }

    void saveSelected() const
    {
        std::cout <<"Hello saveSelected" << std::endl;

        std::ofstream myFile;
		myFile.open("settings.txt");
		myFile << json_dto::to_json(m_selected);
		myFile.close();
    }

    private:
    selected_t &m_selected;
    char fileBuffer[10000];
};