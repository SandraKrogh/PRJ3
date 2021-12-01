#include <iostream>

#include <restinio/all.hpp>

#include <json_dto/pub.hpp>

#include <string>


int BUFSIZE = 1000;
char fileBuffer[10000];

//biblioteker til fopen fil
#include <fstream>
#include <streambuf>
#include <stdio.h>

/*struct weather_t
{
	weather_t() = default;

	weather_t(
		std::string id,
		std::string date,
		std::string time,
		std::string temperature,
		std::string humidity)
		:	m_id{ std::move( id )}
		,	m_date{ std::move( date )}
		,	m_time{ std::move( time )}
		,	m_temperature{ std::move( temperature )}
		,	m_humidity{ std::move( humidity )}
	{}

	template < typename JSON_IO >
	void
	json_io( JSON_IO & io )
	{
		io
			& json_dto::mandatory( "id", m_id )
			& json_dto::mandatory( "date", m_date )
			& json_dto::mandatory( "time", m_time )
			& json_dto::mandatory( "temperature", m_temperature )
			& json_dto::mandatory( "humidity", m_humidity ); 
	}

	std::string m_id;
	std::string m_date;
	std::string m_time;
	std::string m_temperature;
	std::string m_humidity;
	
};*/

struct game_t
{
	game_t() = default;
	game_t(
		std::string id, std::string name, std::string players)
		:	m_id{ std::move( id ) }
		,	m_name{ std::move( name ) }
		,	m_players{ std::move( players)}
	{}

	template < typename JSON_IO >
	void
	json_io( JSON_IO & io )
	{
		io
			& json_dto::mandatory( "id", m_id )
			& json_dto::mandatory( "name", m_name )
			& json_dto::mandatory( "players", m_players );
	}

    std::string m_id;
    std::string m_name;
    std::string m_players;
};

using game_collection_t = std::vector< game_t >;

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;


class game_handler_t
{
public :
	explicit game_handler_t( game_collection_t & games )
		:	m_games( games )
	{}

	game_handler_t( const game_handler_t & ) = delete;
	game_handler_t( game_handler_t && ) = delete;

	auto on_games_list(
		const restinio::request_handle_t& req, rr::route_params_t ) const
	{
		auto resp = init_resp( req->create_response() );

		resp.set_body(json_dto::to_json(m_games));

		return resp.done();
	}

	auto on_game_get(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		const auto gamenum = restinio::cast_to< std::uint32_t >( params[ "gamenum" ] );

		auto resp = init_resp( req->create_response() );

		return resp.done();
	}


	auto on_new_game(
		const restinio::request_handle_t& req, rr::route_params_t )
	{
		auto resp = init_resp( req->create_response() );

		try
		{
			m_games.emplace_back(
				json_dto::from_json< game_t >( req->body() ) );
		}
		catch( const std::exception &  )
		{
			std::cout << req->body() << std::endl;
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_start(
		const restinio::request_handle_t& req, rr::route_params_t )
	{
		auto resp = init_resp( req->create_response() );
		//auto resp = "text/html; charset=utf-8";

		try
		{
			std::ifstream t("client.txt");
			std::stringstream fileBuffer;
			fileBuffer << t.rdbuf();


			std::cout << "HTML:" << fileBuffer.str() << std::endl;
			resp.set_body(fileBuffer.str());

			//json_dto::from_json< game_t >( req->body() ) );
		}
		catch( const std::exception &  )
		{
			std::cout << req->body() << std::endl;
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_game_update(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		const auto gamenum = restinio::cast_to< std::uint32_t >( params[ "gamenum" ] );

		auto resp = init_resp( req->create_response() );

		try
		{
			auto b = json_dto::from_json< game_t >( req->body() );

			if( 0 != gamenum && gamenum <= m_games.size() )
			{
				m_games[ gamenum - 1 ] = b;
			}
			else
			{
				mark_as_bad_request( resp );
				resp.set_body( "No game with #" + std::to_string( gamenum ) + "\n" );
			}
		}
		catch( const std::exception & )
		{
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_game_delete(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		const auto gamenum = restinio::cast_to< std::uint32_t >( params[ "gamenum" ] );

		auto resp = init_resp( req->create_response() );

		if( 0 != gamenum && gamenum <= m_games.size() )
		{

			m_games.erase( m_games.begin() + ( gamenum - 1 ) );
		}
		else
		{
			resp.set_body(
				"No game with #" + std::to_string( gamenum ) + "\n" );
		}

		return resp.done();
	}

	// Tilføjet funktion fra Søren
  auto options(restinio::request_handle_t req,
           restinio::router::route_params_t /*params*/)
  {
    const auto methods = "OPTIONS, GET, POST, PATCH, DELETE";
    auto resp = init_resp( req->create_response() );
    resp.append_header(restinio::http_field::access_control_allow_methods, methods);
    resp.append_header(restinio::http_field::access_control_allow_headers, "content-type");
    resp.append_header(restinio::http_field::access_control_max_age, "86400");
    return resp.done();
  }

private :
	game_collection_t & m_games;
	


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

auto server_handler( game_collection_t & game_collection )
{
	auto router = std::make_unique< router_t >();
	auto handler = std::make_shared< game_handler_t >( std::ref(game_collection) );

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
	router->http_get( "/api", by( &game_handler_t::on_games_list ) );
	router->http_post( "/api", by( &game_handler_t::on_new_game ) );

	router->http_get( "/", by( &game_handler_t::on_start ) );

	//Tilføjet fra Søren
	router->add_handler(restinio::http_method_options(), "/", by( &game_handler_t::options ) );

	// Disable all other methods for '/'.
	router->add_handler(
			restinio::router::none_of_methods(
					restinio::http_method_get(), restinio::http_method_post() ),
			"/api", method_not_allowed );

	// Disable all other methods for '/author/:author'.
	router->add_handler(
			restinio::router::none_of_methods( restinio::http_method_get() ),
			"/author/:author", method_not_allowed );

	// Handlers for '/:gamenum' path.
	router->http_get(
			R"(/:gamenum(\d+))",
			by( &game_handler_t::on_game_get ) );
	router->http_put(
			R"(/:gamenum(\d+))",
			by( &game_handler_t::on_game_update ) );
	router->http_delete(
			R"(/:gamenum(\d+))",
			by( &game_handler_t::on_game_delete ) );

	// Disable all other methods for '/:gamenum'.
	router->add_handler(
			restinio::router::none_of_methods(
					restinio::http_method_get(),
					restinio::http_method_post(),
					restinio::http_method_delete() ),
			R"(/:gamenum(\d+))", method_not_allowed );

	return router;
}


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
	
	
		game_collection_t game_collection{
			{"1", "Fisk", "5"}};

		restinio::run(
			restinio::on_this_thread< traits_t >()
				.address( "localhost" )
				.request_handler( server_handler( game_collection ) )
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
