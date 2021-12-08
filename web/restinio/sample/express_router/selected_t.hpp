#include "players_t.hpp"
#include "game_t.hpp"

/******************************************************************************
                                 selected_t STRUCT
*******************************************************************************/

struct selected_t
{
	selected_t() = default;

	selected_t(game_t game, players_t players)
		:	m_game{ std::move( game ) }
		,	m_players{ std::move( players) }
	{}

	template < typename JSON_IO >
	void
	json_io( JSON_IO & io )
	{
		io
			& json_dto::mandatory( "game", m_game )
			& json_dto::mandatory( "players", m_players);
	}

    game_t m_game;
	players_t m_players;
};
