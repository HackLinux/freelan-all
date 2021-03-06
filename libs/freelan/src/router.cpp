/*
 * libfreelan - A C++ library to establish peer-to-peer virtual private
 * networks.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libfreelan.
 *
 * libfreelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfreelan is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 *
 * If you intend to use libfreelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file router.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A router class.
 */

#include "router.hpp"

#include <cassert>

#include <boost/foreach.hpp>

#include <asiotap/osi/ipv4_helper.hpp>
#include <asiotap/osi/ipv6_helper.hpp>

namespace freelan
{
	void router::async_write(port_index_type index, boost::asio::const_buffer data, port_type::write_handler_type handler)
	{
		const port_list_type::const_iterator port_entry = get_target_for(index, data);

#if FREELAN_DEBUG
		if (port_entry != m_ports.end())
		{
			std::cerr << "Routing " << buffer_size(data) << " byte(s) of data from " << index << " to " << port_entry->first << std::endl;
		}
		else
		{
			std::cerr << "Routing " << buffer_size(data) << " byte(s) of data from " << index << ": no route." << std::endl;
		}
#endif

		if (port_entry != m_ports.end())
		{
			port_entry->second.async_write(data, handler);
		}
	}

	router::port_list_type::const_iterator router::get_target_for(port_index_type index, boost::asio::const_buffer data)
	{
		// Try IPv4 first because it is more likely.

		m_ipv4_filter.parse(data);

		if (m_ipv4_filter.get_last_helper())
		{
			const boost::asio::ip::address_v4 destination = m_ipv4_filter.get_last_helper()->destination();

			m_ipv4_filter.clear_last_helper();

			return get_target_for(index, destination);
		}
		else
		{
			m_ipv6_filter.parse(data);

			if (m_ipv6_filter.get_last_helper())
			{
				const boost::asio::ip::address_v6 destination = m_ipv6_filter.get_last_helper()->destination();

				m_ipv6_filter.clear_last_helper();

				return get_target_for(index, destination);
			}
		}

		// Frame of other types than IPv4 or IPv6 are silently dropped.
		return m_ports.end();
	}

	template <typename AddressType>
	router::port_list_type::const_iterator router::get_target_for(port_index_type index, const AddressType& dest_addr)
	{
		const router::port_list_type::const_iterator source_port_entry = m_ports.find(index);

		if (source_port_entry != m_ports.end())
		{
			const auto& routes_ports = routes();

			for (auto&& route_port : routes_ports)
			{
				if (has_address(route_port.first, dest_addr))
				{
					const port_list_type::const_iterator port_entry = m_ports.find(route_port.second);

					if (m_configuration.client_routing_enabled || (source_port_entry->second.group() != port_entry->second.group()))
					{
						return port_entry;
					}
				}
			}
		}

		// No route for the current frame so we return an invalid iterator.
		return m_ports.end();
	}

	const router::routes_port_type& router::routes() const
	{
		if (!m_routes)
		{
			// The routes were invalidated, we recompile them.

			m_routes = routes_port_type();

			// We add all the port routes to the routes list.
			// These are sorted automatically by the container.
			for (port_list_type::const_iterator port = m_ports.begin(); port != m_ports.end(); ++port)
			{
				const auto& local_routes = port->second.local_routes();

				for (auto&& route : local_routes)
				{
					m_routes->insert(std::make_pair(route, port->first));
				}
			}
		}

		return *m_routes;
	}
}
