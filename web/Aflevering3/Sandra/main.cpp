#include <iostream>

#include <restinio/all.hpp>

#include <json_dto/pub.hpp>

#include <string>


struct Place
{
	Place() = default;

	Place(std::string placeName,
			std::string lat,
			std::string lon)
			: m_placeName(placeName),
			  m_lat(lat),
			  m_lon(lon)
			{}

	template < typename JSON_IO >
	void
	json_io( JSON_IO & io )
	{
		io
			& json_dto::mandatory( "name", m_placeName )
			& json_dto::mandatory( "lat", m_placeName )
			& json_dto::mandatory( "lon", m_placeName );
	}
	
	std::string m_placeName;
	std::string m_lat;
	std::string m_lon;

};


struct weather_t
{
	weather_t() = default;

	weather_t(
		std::string id,
		std::string date,
		std::string time,
		std::string temperature,
		std::string humidity,
		Place place)
		:	m_id{ std::move( id )}
		,	m_date{ std::move( date )}
		,	m_time{ std::move( time )}
		,	m_temperature{ std::move( temperature )}
		,	m_humidity{ std::move( humidity )}
		,   m_place{ std::move( place )}
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
			& json_dto::mandatory( "humidity", m_humidity ) 
			& json_dto::mandatory( "place", m_place ); 
	}

	std::string m_id;
	std::string m_date;
	std::string m_time;
	std::string m_temperature;
	std::string m_humidity;
	Place m_place;
	
};

using weather_collection_t = std::vector< weather_t >;

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;



class weather_handler_t
{
public :
	explicit weather_handler_t( weather_collection_t & weathers )
		:	m_weathers( weathers )
	{}

	weather_handler_t( const weather_handler_t & ) = delete;
	weather_handler_t( weather_handler_t && ) = delete;

	auto on_weathers_list(
		const restinio::request_handle_t& req, rr::route_params_t ) const
	{
		auto resp = init_resp( req->create_response() );

		/*
		resp.set_body( "Weather Report\n" );

			const auto & b = m_weathers[0];
			
			resp.append_body( 
				"ID: " + b.m_id + "\n"
				+ "Date: " + b.m_date + "\n"
				+ "Time: " + b.m_time + "\n"
				+ "Temperature: " + b.m_temperature + "\n"
				+ "Humidity: " + b.m_humidity + "\n"
				+ "Place: \n"
				+ "Placename: " + b.m_place.m_placeName + "\n"
				+ "Lat: " + std::to_string(b.m_place.m_lat) + "\n"
				+ "Lon: " + std::to_string(b.m_place.m_lon) + "\n");
				*/
		
		/*
		for( std::size_t i = 0; i < m_weathers.size(); ++i )
		{
			resp.append_body( std::to_string( i + 1 ) + ". " );
			const auto & b = m_weathers[ i ];
			resp.append_body( b.m_title + "[" + b.m_author + "]\n" );
		}
		*/

		resp.set_body(json_dto::to_json(m_weathers));

		return resp.done();
	}

	auto on_weather_get(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		const auto weathernum = restinio::cast_to< std::uint32_t >( params[ "weathernum" ] );

		auto resp = init_resp( req->create_response() );


/*
		if( 0 != weathernum && weathernum <= m_weathers.size() )
		{
			const auto & b = m_weathers[ weathernum - 1 ];
			resp.set_body(
				"weather #" + std::to_string( weathernum ) + " is: " +
					b.m_title + " [" + b.m_author + "]\n" );
		}
		else
		{
			resp.set_body(
				"No weather with #" + std::to_string( weathernum ) + "\n" );
		}
*/
		return resp.done();
	}

	// LAV DENNE FUNKTION %%¤¤¤¤¤¤¤¤%¤%¤%¤
	auto on_date_get(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		auto resp = init_resp( req->create_response() );

		try
		{
			auto date = restinio::utils::unescape_percent_encoding( params[ "date" ] );

			for( std::size_t i = 0; i < m_weathers.size(); ++i )
			{
				const auto & b = m_weathers[ i ];
				if( date == b.m_date )
				{
					resp.set_body(json_dto::to_json(m_weathers[i]));
				}
			}
		}
		catch( const std::exception & )
		{
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_three_get(
		const restinio::request_handle_t& req, rr::route_params_t params)
	{
		auto resp = init_resp( req->create_response() );

		try
		{
			std::vector<weather_t> weather_three;

			//weather_three.resize(3);

			int i = 0;
			for(auto iter = m_weathers.rbegin(); iter != m_weathers.rend() && i != 4; ++iter, ++i)
			{
				weather_three.push_back(*iter);
				std::cout << i << " - " << iter->m_id << std::endl;
			}

			resp.set_body(json_dto::to_json(weather_three));

		}
		catch( const std::exception & )
		{
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_new_weather(
		const restinio::request_handle_t& req, rr::route_params_t )
	{
		auto resp = init_resp( req->create_response() );

		try
		{
			m_weathers.emplace_back(
				json_dto::from_json< weather_t >( req->body() ) );
		}
		catch( const std::exception &  )
		{
			std::cout << req->body() << std::endl;
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_weather_update(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		const auto weathernum = restinio::cast_to< std::uint32_t >( params[ "weathernum" ] );

		auto resp = init_resp( req->create_response() );

		try
		{
			auto b = json_dto::from_json< weather_t >( req->body() );

			if( 0 != weathernum && weathernum <= m_weathers.size() )
			{
				m_weathers[ weathernum - 1 ] = b;
			}
			else
			{
				mark_as_bad_request( resp );
				resp.set_body( "No weather with #" + std::to_string( weathernum ) + "\n" );
			}
		}
		catch( const std::exception & )
		{
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_weather_delete(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		const auto weathernum = restinio::cast_to< std::uint32_t >( params[ "weathernum" ] );

		auto resp = init_resp( req->create_response() );

		if( 0 != weathernum && weathernum <= m_weathers.size() )
		{
			/*
			const auto & b = m_weathers[ weathernum - 1 ];
			resp.set_body(
				"Delete weather #" + std::to_string( weathernum ) + ": " +
					b.m_title + "[" + b.m_author + "]\n" );
*/
			m_weathers.erase( m_weathers.begin() + ( weathernum - 1 ) );
		}
		else
		{
			resp.set_body(
				"No weather with #" + std::to_string( weathernum ) + "\n" );
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
	weather_collection_t & m_weathers;

	template < typename RESP >
	static RESP

	// Tilføjet en ændret init_resp fra Søren
	init_resp( RESP resp )
  	{
		resp
      	.append_header( "Server", "RESTinio sample server /v.0.6" )
      	.append_header_date_field()
      	.append_header( "Content-Type", "application/json" )
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





auto server_handler( weather_collection_t & weather_collection )
{
	auto router = std::make_unique< router_t >();
	auto handler = std::make_shared< weather_handler_t >( std::ref(weather_collection) );

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
	router->http_get( "/", by( &weather_handler_t::on_weathers_list ) );
	router->http_post( "/", by( &weather_handler_t::on_new_weather ) );

	//Tilføjet fra Søren
	router->add_handler(restinio::http_method_options(), "/", by( &weather_handler_t::options ) );

	// Disable all other methods for '/'.
	router->add_handler(
			restinio::router::none_of_methods(
					restinio::http_method_get(), restinio::http_method_post() ),
			"/", method_not_allowed );

	// Handler for '/wsbydate/:date' path.
	// Klient kalder "/wsbydate/:date" og serveren kalder så "on_date_get"
	router->http_get( "/wsbydate/:date", by( &weather_handler_t::on_date_get ) );

	// Handler for '/three' path.
	// Klient kalder "/three" og serveren kalder så "on_three_get"
	router->http_get( "/three", by( &weather_handler_t::on_three_get ) );

	// Disable all other methods for '/author/:author'.
	router->add_handler(
			restinio::router::none_of_methods( restinio::http_method_get() ),
			"/author/:author", method_not_allowed );

	// Handlers for '/:weathernum' path.
	router->http_get(
			R"(/:weathernum(\d+))",
			by( &weather_handler_t::on_weather_get ) );
	router->http_put(
			R"(/:weathernum(\d+))",
			by( &weather_handler_t::on_weather_update ) );
	router->http_delete(
			R"(/:weathernum(\d+))",
			by( &weather_handler_t::on_weather_delete ) );

	// Disable all other methods for '/:weathernum'.
	router->add_handler(
			restinio::router::none_of_methods(
					restinio::http_method_get(),
					restinio::http_method_post(),
					restinio::http_method_delete() ),
			R"(/:weathernum(\d+))", method_not_allowed );

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
	
	
		weather_collection_t weather_collection{
			{ "1", "20211105", "12:15", "13,1", "70%", {"Aarhus N", "13.692", "19.438"}}};


		restinio::run(
			restinio::on_this_thread< traits_t >()
				.address( "localhost" )
				.request_handler( server_handler( weather_collection ) )
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
