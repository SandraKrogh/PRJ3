#include <iostream>
#include <restinio/all.hpp>
#include <json_dto/pub.hpp>
#include <string>

/******************************************************************************
                                   game_t STRUCT
*******************************************************************************/
struct game_t
{
	game_t() = default;
	
	game_t(std::string game) : m_game{ game}
	{}

	template < typename JSON_IO >
	void
	json_io( JSON_IO & io )
	{
		io
			& json_dto::mandatory( "game", m_game );
	}

	std::string m_game;

};
