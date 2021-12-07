#include <iostream>
#include <restinio/all.hpp>
#include <json_dto/pub.hpp>
#include <string>

/******************************************************************************
                                   PLAYERS_T STRUCT
*******************************************************************************/
struct players_t
{
	players_t() = default;
	
	players_t(std::string players) : m_players{ players}
	{}

	template < typename JSON_IO >
	void
	json_io( JSON_IO & io )
	{
		io
			& json_dto::mandatory( "players", m_players );
	}

	std::string m_players;

};
