/*
 * move_only.h
 *
 *  Created on: 3 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SWAL_ZERO_OR_RESOURCE_H
#define SWAL_ZERO_OR_RESOURCE_H

#include <utility>

namespace swal {

template <typename Rs>
class zero_or_resource {
public:
	constexpr operator Rs() const {
		return resource;
	}
	constexpr Rs get() const {
		return resource;
	}
	const Rs* get_ptr() const { return &resource; }
protected:
	constexpr zero_or_resource(Rs res) : resource(res){}
	constexpr zero_or_resource(const zero_or_resource&) = default;
	constexpr zero_or_resource(zero_or_resource&& other) noexcept : resource(other.resource) {
		other.resource = 0;
	}
	zero_or_resource& operator=(const zero_or_resource&) = default;
	zero_or_resource& operator=(zero_or_resource&& other) noexcept {
		std::swap(resource, other.resource);
		return *this;
	}
	Rs resource;
};

}

#endif /* SWAL_ZERO_OR_RESOURCE_H */
