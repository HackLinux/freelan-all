/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libasiotap.
 *
 * libasiotap is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libasiotap is distributed in the hope that it will be useful, but
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
 * If you intend to use libasiotap in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file posix_tap_adapter.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The posix tap adapter class.
 */

#include "posix/posix_tap_adapter.hpp"

namespace asiotap
{
	std::map<std::string, std::string> posix_tap_adapter::enumerate(tap_adapter_layer _layer)
	{
		std::map<std::string, std::string> result;

		struct ifaddrs* addrs = nullptr;

		if (getifaddrs(&addrs) != -1)
		{
			boost::shared_ptr<struct ifaddrs> paddrs(addrs, freeifaddrs);

			for (struct ifaddrs* ifa = paddrs.get(); ifa != NULL ; ifa = ifa->ifa_next)
			{
				const std::string name(ifa->ifa_name);

				switch (_layer)
				{
					case tap_adapter_layer::ethernet:
					{
						if (name.substr(0, 3) == "tap")
						{
							result[name] = name;
						}
					}
					case tap_adapter_layer::ip:
					{
						if (name.substr(0, 3) == "tun")
						{
							result[name] = name;
						}
					}
				}
			}
		}

		return result;
	}
}
