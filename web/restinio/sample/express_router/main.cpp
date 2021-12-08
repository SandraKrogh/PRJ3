#include <restinio/all.hpp>

#include <json_dto/pub.hpp>

#include <string>

#include "../../../../RPIdata.hpp"
#include "../../../../PSoCcom.hpp"

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

/******************************************************************************
                                   selected HANDLER_T
*******************************************************************************/
class selected_handler_t
{
public :
	explicit selected_handler_t( selected_t &selected)
		:	m_selected( selected ), m_RPIdata(new RPIdata(m_selected)), m_PSoCcom(new PSoCcom)
	{
	}

	selected_handler_t( const selected_handler_t & ) = delete;
	selected_handler_t( selected_handler_t && ) = delete;

	~selected_handler_t()
	{
		delete m_RPIdata;

		delete m_PSoCcom;
	}
/******************************************************************************
                        	ON PLAYERS POST, GEM PLAYERS
*******************************************************************************/

	auto on_players_post(
		const restinio::request_handle_t& req, rr::route_params_t ) const
	{
		auto resp = init_resp( req->create_response() );

		players_t p = json_dto::from_json< players_t >( req->body() );
		m_selected.m_players = p.m_players;

		std::cout << "On players post" << std::endl;

		m_RPIdata->saveSelected();

		return resp.done();
	}



/******************************************************************************
                                   ON HOME, START SIDEN
*******************************************************************************/

	auto on_home(
		const restinio::request_handle_t& req, rr::route_params_t )
	{
		auto resp = init_resp( req->create_response() );

		try
		{
			std::ifstream t("home.html");
			std::stringstream fileBuffer;
			fileBuffer << t.rdbuf();

			std::cout << "HTML:" << fileBuffer.str() << std::endl;
			resp.set_body(fileBuffer.str());

			m_RPIdata->retrieveSelected();

			resp.set_body(json_dto::to_json(m_selected));

		}
		catch( const std::exception &  )
		{
			std::cout << req->body() << std::endl;
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

/******************************************************************************
                                   ON START GAME, START SPIL
*******************************************************************************/

	auto on_start_game(
		const restinio::request_handle_t& req, rr::route_params_t )
	{
		
		std::cout << "on_start_game kaldt" << std::endl;

		auto resp = init_resp( req->create_response() );

		try
		{
			
			m_RPIdata->retrieveSelected();

			m_PSoCcom->startSpil(m_selected.m_game.m_game, m_selected.m_players.m_players);

			std::ifstream t("startGame.html");
			std::stringstream fileBuffer;
			fileBuffer << t.rdbuf();

			std::cout << "HTML:" << fileBuffer.str() << std::endl;
			resp.set_body(fileBuffer.str());

		}
		catch( const std::exception &  )
		{
			std::cout << req->body() << std::endl;
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

/******************************************************************************
                      ON game post, SÆTTER TYPE SPIL
*******************************************************************************/

	auto on_game_post(
		const restinio::request_handle_t& req, rr::route_params_t )
	{
		auto resp = init_resp( req->create_response() );

		game_t p = json_dto::from_json< game_t >( req->body() );
		m_selected.m_game = p.m_game;

		std::cout << "On game post" << std::endl;

		std::cout << "valgt spil " << p.m_game << std::endl;

		m_RPIdata->saveSelected();

		return resp.done();
	}

/******************************************************************************
    				ON SET PLAYERS, VISER PLAYER MENU
*******************************************************************************/

	auto on_set_players(
		const restinio::request_handle_t& req, rr::route_params_t )
	{
		auto resp = init_resp( req->create_response() );

		try
		{
			std::ifstream t("setPlayers.html");
			std::stringstream fileBuffer;
			fileBuffer << t.rdbuf();


			std::cout << "HTML:" << fileBuffer.str() << std::endl;
			resp.set_body(fileBuffer.str());
		}
		catch( const std::exception &  )
		{
			std::cout << req->body() << std::endl;
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

/******************************************************************************
    				ON SET GAME, VISER GAME MENU
*******************************************************************************/

	auto on_set_game(
		const restinio::request_handle_t& req, rr::route_params_t )
	{
		auto resp = init_resp( req->create_response() );

		try
		{
			std::ifstream t("setGame.html");
			std::stringstream fileBuffer;
			fileBuffer << t.rdbuf();


			std::cout << "HTML:" << fileBuffer.str() << std::endl;
			resp.set_body(fileBuffer.str());
		}
		catch( const std::exception &  )
		{
			std::cout << req->body() << std::endl;
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

/******************************************************************************
                   ON_SETTINGS, SE NUVÆRENDE INDSTILLINGER
*******************************************************************************/

	auto on_settings(
		const restinio::request_handle_t& req, rr::route_params_t )
	{
		auto resp = init_resp( req->create_response() );

		try
		{
			
			std::ifstream t("settings.txt");
			std::stringstream fileBuffer;
			fileBuffer << t.rdbuf();

			std::cout << "HTML:" << fileBuffer.str() << std::endl;
			resp.set_body(fileBuffer.str());

		}
		catch( const std::exception &  )
		{
			std::cout << req->body() << std::endl;
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

/******************************************************************************
                         OPTIONS, TILFØJET FRA SØREN
*******************************************************************************/
  auto options(restinio::request_handle_t req,
           restinio::router::route_params_t)
  {
    const auto methods = "OPTIONS, GET, POST, PATCH, DELETE";
    auto resp = init_resp( req->create_response() );
    resp.append_header(restinio::http_field::access_control_allow_methods, methods);
    resp.append_header(restinio::http_field::access_control_allow_headers, "content-type");
    resp.append_header(restinio::http_field::access_control_max_age, "86400");
    return resp.done();
  }

/******************************************************************************
                       PRIVATE ATR. FOR GAME HANDLER
*******************************************************************************/

private :
	selected_t &m_selected;
	RPIdata *m_RPIdata;
	PSoCcom *m_PSoCcom;
	
	template < typename RESP >
	static RESP

	init_resp( RESP resp )
  	{
		resp
      	.append_header( "Server", "RESTinio sample server /v.0.6" )
      	.append_header_date_field()
      	.append_header( "Content-Type", "text/html; charset=utf-8" )
      	.append_header(restinio::http_field::access_control_allow_origin, "*");
    	return resp;
  	}

	template < typename RESP >
	static void
	mark_as_bad_request( RESP & resp )
	{
		resp.header().status_line( restinio::status_bad_request() );
	}
};

/******************************************************************************
                                   SERVER HANDLERS
*******************************************************************************/

auto server_handler( selected_t & selected )
{
	auto router = std::make_unique< router_t >();
	auto handler = std::make_shared< selected_handler_t >( std::ref(selected) );

	auto by = [&]( auto method ) {
		using namespace std::placeholders;
		return std::bind( method, handler, _1, _2 );
	};

	auto method_not_allowed = []( const auto & req, auto ) {
			return req->create_response( restinio::status_method_not_allowed() )
					.connection_close()
					.done();
		};

	// Handlers for '/' path.
	router->http_get( "/", by( &selected_handler_t::on_home ) );

	// Redirect to selected or players
	router->http_get( "/game", by( &selected_handler_t::on_set_game) );
	router->http_get( "/players", by( &selected_handler_t::on_set_players ) );
	router->http_get( "/startgame", by( &selected_handler_t::on_start_game ) );
	router->http_get( "/settings", by( &selected_handler_t::on_settings ) ); //se nuværende indstillinger 

	// Post number of players
	router->http_post( "/players", by( &selected_handler_t::on_players_post ) );

	// Post det valgte spil
	router->http_post( "/game", by( &selected_handler_t::on_game_post ) );

	//Tilføjet fra Søren
	router->add_handler(restinio::http_method_options(), "/", by( &selected_handler_t::options ) );

	// Disable all other methods for '/'.
	router->add_handler(
			restinio::router::none_of_methods(
					restinio::http_method_get(), restinio::http_method_post() ),
			"/api", method_not_allowed );

	// Disable all other methods for '/author/:author'.
	router->add_handler(
			restinio::router::none_of_methods( restinio::http_method_get() ),
			"/author/:author", method_not_allowed );

	// HVAD ER DET HER ??%?%¤)?)%")&)¤&)?#%=&?#?=&?¤#=%"

	// Disable all other methods for '/:selectednum'.
	/*
	router->add_handler(
			restinio::router::none_of_methods(
					restinio::http_method_get(),
					restinio::http_method_post(),
					restinio::http_method_delete() ),
			R"(/:selectednum(\d+))", method_not_allowed );*/

	return router;
}
/******************************************************************************
                                   MAIN
*******************************************************************************/

int main()
{
	using namespace std::chrono;

	try
	{
		using traits_t =
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				restinio::single_threaded_ostream_logger_t,
				router_t >;
	

		selected_t selected;

		restinio::run(
			restinio::on_this_thread< traits_t >()
				.address( "0.0.0.0" )
				//.request_handler( server_handler( selected_collection ) )
				.request_handler( server_handler( selected ) )
				.read_next_http_message_timelimit( 10s )
				.write_http_response_timelimit( 1s )
				.handle_request_timeout( 1s ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
