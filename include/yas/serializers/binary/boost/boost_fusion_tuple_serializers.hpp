
// Copyright (c) 2010-2012 niXman (i dot nixman dog gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
//
// Boost Software License - Version 1.0 - August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef _yas__binary__fusion_tuple_serializer_hpp__included_
#define _yas__binary__fusion_tuple_serializer_hpp__included_

#include <yas/detail/config/config.hpp>

#if defined(YAS_HAS_BOOST_FUSION)

#include <stdexcept>

#include <yas/detail/type_traits/type_traits.hpp>
#include <yas/detail/type_traits/properties.hpp>
#include <yas/detail/type_traits/selector.hpp>
#include <yas/detail/preprocessor/preprocessor.hpp>

#include <boost/assert.hpp>
#include <boost/fusion/tuple.hpp>

namespace yas {
namespace detail {

/***************************************************************************/

#define YAS__BINARY__WRITE_BOOST_FUSION_TUPLE_ITEM(unused, idx, type) \
	if ( is_pod<YAS_PP_CAT(type, idx)>::value ) \
		ar.write(reinterpret_cast<const char*>(&boost::fusion::at_c<idx>(tuple)), sizeof(YAS_PP_CAT(type, idx))); \
	else \
		ar & boost::fusion::at_c<idx>(tuple);

#define YAS__BINARY__READ_BOOST_FUSION_TUPLE_ITEM(unused, idx, type) \
	if ( is_pod<YAS_PP_CAT(type, idx)>::value ) \
		ar.read(reinterpret_cast<char*>(&boost::fusion::at_c<idx>(tuple)), sizeof(YAS_PP_CAT(type, idx))); \
	else \
		ar & boost::fusion::at_c<idx>(tuple);

#define YAS__BINARY__GENERATE_EMPTY_SAVE_SERIALIZE_BOOST_FUSION_TUPLE_SPEC() \
	template<> \
	struct serializer<e_type_type::not_a_pod,e_ser_method::use_internal_serializer, \
		e_archive_type::binary, e_direction::out, boost::fusion::tuple<> > \
	{ \
		template<typename Archive> \
		static void apply(Archive&, const boost::fusion::tuple<>&) {} \
	};

#define YAS__BINARY__GENERATE_EMPTY_LOAD_SERIALIZE_BOOST_FUSION_TUPLE_SPEC() \
	template<> \
	struct serializer<e_type_type::not_a_pod,e_ser_method::use_internal_serializer, \
		e_archive_type::binary, e_direction::in, boost::fusion::tuple<> > \
	{ \
		template<typename Archive> \
		static void apply(Archive&, boost::fusion::tuple<>&) {} \
	};

#define YAS__BINARY__GENERATE_SAVE_SERIALIZE_BOOST_FUSION_TUPLE_FUNCTION_SPEC(unused, count, text) \
	template<YAS_PP_ENUM_PARAMS(YAS_PP_INC(count), typename T)> \
	struct serializer<e_type_type::not_a_pod,e_ser_method::use_internal_serializer, \
		e_archive_type::binary, e_direction::out, \
		boost::fusion::tuple<YAS_PP_ENUM_PARAMS(YAS_PP_INC(count), T)> > \
	{ \
		template<typename Archive> \
		static void apply(Archive& ar, \
			const boost::fusion::tuple<YAS_PP_ENUM_PARAMS(YAS_PP_INC(count), T)>& tuple) \
		{ \
			const yas::uint8_t size = YAS_PP_INC(count); \
			ar.write(reinterpret_cast<const char*>(&size), sizeof(size)); \
			YAS_PP_REPEAT( \
				YAS_PP_INC(count), \
				YAS__BINARY__WRITE_BOOST_FUSION_TUPLE_ITEM, \
				T \
			) \
		} \
	};

#define YAS__BINARY__GENERATE_SAVE_SERIALIZE_BOOST_FUSION_TUPLE_FUNCTIONS_SPEC(count) \
	YAS__BINARY__GENERATE_EMPTY_SAVE_SERIALIZE_BOOST_FUSION_TUPLE_SPEC() \
	YAS_PP_REPEAT( \
		count, \
		YAS__BINARY__GENERATE_SAVE_SERIALIZE_BOOST_FUSION_TUPLE_FUNCTION_SPEC, \
		~ \
	)

#define YAS__BINARY__GENERATE_LOAD_SERIALIZE_BOOST_FUSION_TUPLE_FUNCTION_SPEC(unused, count, text) \
	template<YAS_PP_ENUM_PARAMS(YAS_PP_INC(count), typename T)> \
	struct serializer<e_type_type::not_a_pod,e_ser_method::use_internal_serializer, \
		e_archive_type::binary, e_direction::in, \
		boost::fusion::tuple<YAS_PP_ENUM_PARAMS(YAS_PP_INC(count), T)> > \
	{ \
		template<typename Archive> \
		static void apply(\
			Archive& ar, \
			boost::fusion::tuple<YAS_PP_ENUM_PARAMS(YAS_PP_INC(count), T)>& tuple) \
		{ \
			yas::uint8_t size = 0; \
			ar.read(reinterpret_cast<char*>(&size), sizeof(size)); \
			if ( size != YAS_PP_INC(count) ) throw std::runtime_error("size error on deserialize fusion::tuple"); \
			YAS_PP_REPEAT( \
				YAS_PP_INC(count), \
				YAS__BINARY__READ_BOOST_FUSION_TUPLE_ITEM, \
				T \
			) \
		} \
	};

#define YAS__BINARY__GENERATE_LOAD_SERIALIZE_BOOST_FUSION_TUPLE_FUNCTIONS_SPEC(count) \
	YAS__BINARY__GENERATE_EMPTY_LOAD_SERIALIZE_BOOST_FUSION_TUPLE_SPEC() \
	YAS_PP_REPEAT( \
		count, \
		YAS__BINARY__GENERATE_LOAD_SERIALIZE_BOOST_FUSION_TUPLE_FUNCTION_SPEC, \
		~ \
	)

/***************************************************************************/

YAS__BINARY__GENERATE_SAVE_SERIALIZE_BOOST_FUSION_TUPLE_FUNCTIONS_SPEC(FUSION_MAX_VECTOR_SIZE)
YAS__BINARY__GENERATE_LOAD_SERIALIZE_BOOST_FUSION_TUPLE_FUNCTIONS_SPEC(FUSION_MAX_VECTOR_SIZE)

/***************************************************************************/

} // namespace detail
} // namespace yas

#endif // defined(YAS_HAS_BOOST_FUSION)

#endif // _yas__binary__fusion_tuple_serializer_hpp__included_
